#include "KaleidoscopeJIT.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace llvm;
using namespace llvm::orc;

// lexer

enum Token {
    tok_eof = -1,
    tok_def = -2,
    tok_extern = -3,
    tok_identifier = -4,
    tok_number = -5
};

static std::string IdentifierStr;
static double NumVal;

static int gettok()
{
    static int LastChar = ' ';
    while(isspace(LastChar)) { 
        LastChar = getchar(); 
    }

    if(isalpha(LastChar))
    {
        IdentifierStr = LastChar;
        while(isalnum((LastChar = getchar()))) { 
            IdentifierStr += LastChar; 
        }

        if(IdentifierStr == "def") { 
            return tok_def; 
        }
        
        if(IdentifierStr == "extern") { 
            return tok_extern; 
        }
        
        return tok_identifier;
    }

    if(isdigit(LastChar) || LastChar == '.') 
    {
        std::string NumStr;
        do {
            NumStr += LastChar;
            LastChar = getchar();
        } while(isdigit(LastChar) || LastChar == '.');

        NumVal = strtod(NumStr.c_str(), nullptr);
        return tok_number;
    }

    if(LastChar == '#')
    {
        do {
            LastChar = getchar();
        } while(LastChar != EOF && LastChar != '\n' && LastChar != '\r');

        if(LastChar != EOF) { return gettok(); }
    }

    if(LastChar == EOF) { return tok_eof; }

    int ThisChar = LastChar;
    LastChar = getchar();
    return ThisChar;
};

// AST

namespace
{
    class ExprAST {
        public: 
            virtual ~ExprAST() = default;
            
            virtual Value *codegen() = 0;
    };

    class NumberExprAST : public ExprAST {
        double Val;

        public:
            NumberExprAST(double Val) : Val(Val) {} // Val = Val(argument)
            
            Value *codegen() override;
    };

    class VariableExprAST : public ExprAST {
        std::string Name;

        public:
            VariableExprAST(std::string &Name) : Name(Name) {}
            
            Value *codegen() override;
    };

    class BinaryExprAST : public ExprAST {
        char Op;
        std::unique_ptr<ExprAST> LHS, RHS;

        public:
            BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS) : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
            
            Value *codegen() override;
    };

    class CallExprAST : public ExprAST {
        std::string Callee;
        std::vector<std::unique_ptr<ExprAST>> Args;

        public:
            CallExprAST(std::string &Callee, std::vector<std::unique_ptr<ExprAST>> Args) : Callee(Callee), Args(std::move(Args)) {}

            Value *codegen() override;
    };

    class PrototypeAST {
        std::string Name;
        // std::vector<std::string> Args;

        public:
            std::vector<std::string> Args; // make args to public member for access
            PrototypeAST(const std::string &Name, std::vector<std::string> Args) : Name(Name), Args(std::move(Args)) {}

            Function *codegen();

            const std::string &getName() const { return Name; }
            
    };

    class FunctionAST {
        std::unique_ptr<PrototypeAST> Proto;
        std::unique_ptr<ExprAST> Body;

        public:
            FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<ExprAST> Body) : Proto(std::move(Proto)), Body(std::move(Body)) {}

            Function *codegen();
    };
}

// AST End


// Parser

static int CurTok;
static int getNextToken() { return CurTok = gettok(); }

static std::map<char,int> BinopPrecedence; // global var

static int GetTokPrecedence() 
{
    if(!isascii(CurTok)) { return -1; }

    int TokPrec = BinopPrecedence[CurTok];
    if(TokPrec <= 0) { return -1; }
    return TokPrec;
}

std::unique_ptr<ExprAST> LogError(const char *Str)
{
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char *Str)
{
    LogError(Str);
    return nullptr;
}

static std::unique_ptr<ExprAST> ParseExpression();

static std::unique_ptr<ExprAST> ParseNumberExpr() {
  auto Result = std::make_unique<NumberExprAST>(NumVal);
  getNextToken(); // consume the number
  return std::move(Result);
}

/// parenexpr ::= '(' expression ')'
static std::unique_ptr<ExprAST> ParseParenExpr() {
  getNextToken(); // eat (.
  auto V = ParseExpression();
  if (!V)
    return nullptr;

  if (CurTok != ')')
    return LogError("expected ')'");
  getNextToken(); // eat ).
  return V;
}

static std::unique_ptr<ExprAST> ParseIdentifierExpr()
{
    std::string IdName = IdentifierStr;

    getNextToken();

    if(CurTok != '(') { return std::make_unique<VariableExprAST>(IdName); }

    getNextToken();
    std::vector<std::unique_ptr<ExprAST>> Args;

    if(CurTok != ')')
    {
        while(true)
        {
            if(auto Arg = ParseExpression()) { Args.push_back(std::move(Arg)); }
            else { return nullptr; }

            if(CurTok == ')') break;

            if(CurTok != ',') { return LogError("Expected ')' or ',' in argument list"); }
            
            getNextToken();
        }
    }

    getNextToken();

    return std::make_unique<CallExprAST>(IdName, std::move(Args)); 
}

static std::unique_ptr<ExprAST> ParsePrimary()
{
    switch(CurTok)
    {
        default: 
            return LogError("unknown token when expecting an expression");
        case tok_identifier:
            return ParseIdentifierExpr();
        case tok_number:
            return ParseNumberExpr();
        case '(':
            return ParseParenExpr();
    }
}

static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS)
{
    while(true)
    {
        int TokPrec = GetTokPrecedence();

        if(TokPrec < ExprPrec) { return LHS; }
        
        int BinOp = CurTok;
        getNextToken();

        auto RHS = ParsePrimary();
        if(!RHS) { return nullptr; }

        int NextPrec = GetTokPrecedence();
        if(TokPrec < NextPrec)
        {
            RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
            if(!RHS) { return nullptr; }
        }

        LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
    }
}

static std::unique_ptr<ExprAST> ParseExpression()
{
    auto LHS = ParsePrimary();
    if(!LHS) { return nullptr; }

    return ParseBinOpRHS(0, std::move(LHS));
}

static std::unique_ptr<PrototypeAST> ParsePrototype()
{
    if(CurTok != tok_identifier) { 
        return LogErrorP("Expected function name in prototype"); 
    }

    std::string FnName = IdentifierStr;
    getNextToken();

    if(CurTok != '(') { 
        return LogErrorP("Expected '(' in prototype"); 
    }

    std::vector<std::string> ArgNames;
    while(getNextToken() == tok_identifier)
    {
        ArgNames.push_back(IdentifierStr);
    }
    
    if(CurTok != ')') { 
        LogErrorP("Expected ')' in prototype"); 
    }

    getNextToken();

    return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}

static std::unique_ptr<FunctionAST> ParseDefinition()
{
    getNextToken();
    auto Proto = ParsePrototype();
    if(!Proto) { return nullptr; }

    if(auto E = ParseExpression())
    {
        return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    }
    return nullptr;
}

static std::unique_ptr<FunctionAST> ParseTopLevelExpr()
{
    if(auto E = ParseExpression())
    {
        auto Proto = std::make_unique<PrototypeAST>("__anon_expr",std::vector<std::string>()); // name, arg
        return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    }
    return nullptr;
}

static std::unique_ptr<PrototypeAST> ParseExtern()
{
    getNextToken();
    return ParsePrototype();
}

// Parser END

// Code Generation == IR

static std::unique_ptr<LLVMContext> TheContext;
static std::unique_ptr<Module> TheModule;
static std::unique_ptr<IRBuilder<>> Builder;
static std::map<std::string, Value *> NamedValues;

// added in chapter 4
static std::unique_ptr<legacy::FunctionPassManager> TheFPM;
static std::unique_ptr<KaleidoscopeJIT> TheJIT;
static std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;
static ExitOnError ExitOnErr;

Value *LogErrorV(const char *Str)
{
    LogError(Str);
    return nullptr;
}

Function *getFunction(std::string Name) // added
{
    if(auto *F = TheModule->getFunction(Name)) {
        return F;
    }
    
    auto FI = FunctionProtos.find(Name);
    if(FI != FunctionProtos.end()) {
        return FI->second->codegen();
    }

    return nullptr;
}

Value *NumberExprAST::codegen()
{
    return ConstantFP::get(*TheContext, APFloat(Val));
}

Value *VariableExprAST::codegen()
{
    Value *V = NamedValues[Name];
    if(!V) { return LogErrorV("Unknown variable name"); }
    
    return V;
}

Value *BinaryExprAST::codegen()
{
    Value *L = LHS->codegen();
    Value *R = RHS->codegen();

    if(!L || !R) { return nullptr; }

    switch(Op)
    {
        case '+':
            return Builder->CreateFAdd(L, R, "AddTmp");
        case '-':
            return Builder->CreateFSub(L, R, "SubTmp");
        case '*':
            return Builder->CreateFMul(L, R, "MulTmp");
        case '<':
            L = Builder->CreateFCmpULT(L, R, "CmpTmp");
            return Builder->CreateUIToFP(L, Type::getDoubleTy(*TheContext), "BoolTmp");
        default:
            return LogErrorV("invalid binary operator");
    }
}

Value *CallExprAST::codegen()
{
    // Function *CalleeF = TheModule->getFunction(Callee);
    Function *CalleeF = getFunction(Callee);
    if(!CalleeF) { 
        return LogErrorV("Unknown function referenced"); 
    }

    if(CalleeF->arg_size() != Args.size()) {
        return LogErrorV("Incorrect # arguments passed");
    }

    std::vector<Value *> ArgsV;
    for(unsigned i = 0, e = Args.size(); i != e; i++)
    {
        ArgsV.push_back(Args[i]->codegen());
        if(!ArgsV.back()) { 
            return nullptr; 
        }
    }
    
    return Builder->CreateCall(CalleeF, ArgsV, "CallTmp");
}

Function *PrototypeAST::codegen()
{
    std::vector<Type *> Doubles(Args.size(), Type::getDoubleTy(*TheContext));
    FunctionType *FT = FunctionType::get(Type::getDoubleTy(*TheContext), Doubles, false);

    Function *F = Function::Create(FT, Function::ExternalLinkage, Name, TheModule.get());

    unsigned Idx = 0;
    for(auto &Arg : F->args())
    {
        Arg.setName(Args[Idx++]);
    }

    return F;
}

Function *FunctionAST::codegen()
{
    // Function *TheFunction = TheModule->getFunction(Proto->getName()); 
    // parse func name

    auto &P = *Proto;
    FunctionProtos[Proto->getName()] = std::move(Proto);
    Function *TheFunction = getFunction(P.getName());
    
    if(!TheFunction) {
        return nullptr;
    }

    BasicBlock *BB = BasicBlock::Create(*TheContext, "Entry", TheFunction);
    Builder->SetInsertPoint(BB);

    NamedValues.clear();

    /*  코드 수정해야 함
    unsigned Idx = 0;
    for (auto ProtoBegin = Proto->Args.begin(), ProtoEnd = Proto->Args.end();
        ProtoBegin != ProtoEnd; ++ProtoBegin) {
        TheFunction->getArg(Idx)->setName(*ProtoBegin);
    }
    */

    for(auto &Arg : TheFunction->args())
    {
        NamedValues[std::string(Arg.getName())] = &Arg;
    }

    if(Value *RetVal = Body->codegen())
    {
        Builder->CreateRet(RetVal);

        verifyFunction(*TheFunction);

        return TheFunction;
    }

    TheFunction->eraseFromParent();
    return nullptr;
}

// Code Generation End


// Top-Level Parsing and JIT Driver

/*
static void InitiallizeModule()
{
    TheContext = std::make_unique<LLVMContext>();
    TheModule = std::make_unique<Module>("My Fking JIT", *TheContext);

    Builder = std::make_unique<IRBuilder<>>(*TheContext);
}
*/

static void InitializeModuleAndPassManager()
{
    // Open a new context and module.
    TheContext = std::make_unique<LLVMContext>();
    TheModule = std::make_unique<Module>("my cool jit", *TheContext);
    TheModule->setDataLayout(TheJIT->getDataLayout());

    // Create a new builder for the module.
    Builder = std::make_unique<IRBuilder<>>(*TheContext);

    // Create a new pass manager attached to it.
    TheFPM = std::make_unique<legacy::FunctionPassManager>(TheModule.get());

    // Do simple "peephole" optimizations and bit-twiddling optzns.
    TheFPM->add(createInstructionCombiningPass());
    // Reassociate expressions.
    TheFPM->add(createReassociatePass());
    // Eliminate Common SubExpressions.
    TheFPM->add(createGVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    TheFPM->add(createCFGSimplificationPass());

    TheFPM->doInitialization();
}

static void HandleDefinition()
{
    if (auto FnAST = ParseDefinition())
    {
        if (auto *FnIR = FnAST->codegen())
        {
            fprintf(stderr, "Read function definition:");
            FnIR->print(errs());
            fprintf(stderr, "\n");
            ExitOnErr(TheJIT->addModule(
                ThreadSafeModule(std::move(TheModule), std::move(TheContext))));
            InitializeModuleAndPassManager();
        }
    }
    else
    {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void HandleExtern()
{
    if (auto ProtoAST = ParseExtern())
    {
        if (auto *FnIR = ProtoAST->codegen())
        {
            fprintf(stderr, "Read extern: ");
            FnIR->print(errs());
            fprintf(stderr, "\n");
            FunctionProtos[ProtoAST->getName()] = std::move(ProtoAST);
        }
    }
    else
    {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void HandleTopLevelExpression()
{
    // Evaluate a top-level expression into an anonymous function.
    if (auto FnAST = ParseTopLevelExpr())
    {
        if (FnAST->codegen())
        {
            // Create a ResourceTracker to track JIT'd memory allocated to our
            // anonymous expression -- that way we can free it after executing.
            auto RT = TheJIT->getMainJITDylib().createResourceTracker();

            auto TSM = ThreadSafeModule(std::move(TheModule), std::move(TheContext));
            ExitOnErr(TheJIT->addModule(std::move(TSM), RT));
            InitializeModuleAndPassManager();

            // Search the JIT for the __anon_expr symbol.
            auto ExprSymbol = ExitOnErr(TheJIT->lookup("__anon_expr"));

            // Get the symbol's address and cast it to the right type (takes no
            // arguments, returns a double) so we can call it as a native function.
            double (*FP)() = (double (*)())(intptr_t)ExprSymbol.getAddress();
            fprintf(stderr, "Evaluated to %f\n", FP());

            // Delete the anonymous expression module from the JIT.
            ExitOnErr(RT->remove());
        }
    }
    else
    {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void MainLoop()
{
    while(true)
    {
        fprintf(stderr, "ready> ");
        switch(CurTok)
        {
            case tok_eof:
                return;
            case ';':
                getNextToken();
                break;
            case tok_def:
                HandleDefinition();
                break;
            case tok_extern:
                HandleExtern();
                break;
            default:
                HandleTopLevelExpression();
                break;
        }
    }
}

// Top Level End

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

/// putchard - putchar that takes a double and returns 0.
extern "C" DLLEXPORT double putchard(double X)
{
    fputc((char)X, stderr);
    return 0;
}

/// printd - printf that takes a double prints it as "%f\n", returning 0.
extern "C" DLLEXPORT double printd(double X)
{
    fprintf(stderr, "%f\n", X);
    return 0;
}

// Main Driver

int main()
{
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();

    BinopPrecedence['<'] = 10;
    BinopPrecedence['+'] = 20;
    BinopPrecedence['-'] = 20;
    BinopPrecedence['*'] = 40;

    fprintf(stderr, "ready> ");
    getNextToken();


    TheJIT = ExitOnErr(KaleidoscopeJIT::Create());
    
    InitializeModuleAndPassManager();

    MainLoop();

    return 0;
}
