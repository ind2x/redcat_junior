# Code generation to LLVM IR

이 장에서는 AST를 LLVM IR로 변환하는 방법에 대해 설명한다.

또한 llvm이 어떤 작업을 수행하는지 일부 보여준다.

<br>

검색해본 결과 code generation이란 Intermediate Code를 만드는 단계로 사용자의 코드를 최종적으로는 컴퓨터가 알아들을 수 있는 assembly code로 변환해줘야 한다.

하지만 assembly code는 사용자의 readability가 너무 떨어지고 target machine마다 assembly code도 다르기 때문에 이를 바로 적용하기 위해서는 복잡한 과정을 거쳐야 한다.

그래서 중간 단계의 machine independent code가 필요하였고 이 과정에서 나온 것이 Intermediate Code이다.

즉, 이 Intermediate Code를 만드는 과정이 Intermediate Code generation이고 줄여서 IR이라고 하는 것이라고 한다.

<br>

출처 : <a href="https://talkingaboutme.tistory.com/entry/Compiler-Intermediate-Code-Generation-실습">talkingaboutme.tistory.com/entry/Compiler-Intermediate-Code-Generation-실습</a>

<br><br>

## Code Generation Setup

LLVM IR을 제작하기 위해서 ExprAST에 virtual codegen 메소드를 넣어줘야 한다.

<br>

```cpp
/// ExprAST - Base class for all expression nodes.
class ExprAST {
public:
  virtual ~ExprAST() = default;
  virtual Value *codegen() = 0;
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
  double Val;

public:
  NumberExprAST(double Val) : Val(Val) {}
  Value *codegen() override;
};
...
```

<br>

Value 클래스는 LLVM에서 정적 단일 할당(SSA) 레지스터 또는 SSA 값을 나타내는 데 사용되는 클래스라고 한다.

SSA란 변수의 값 할당은 한 번만 가능하다는 의미이다.

이 방법이 가장 간단하여 이렇게 한 것이라고 한다.

또한 파서에 있는 LogError와 같은 함수를 사용한다.

<br>

```cpp
static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);
static std::unique_ptr<Module> TheModule;
static std::map<std::string, Value *> NamedValues;

Value *LogErrorV(const char *Str) {
  LogError(Str);
  return nullptr;
}
```

<br>

TheContext 는 type 및 constant 값 테이블과 같은 많은 핵심 LLVM 데이터 구조를 소유하는 불투명 개체라고 한다.

Builder 개체는 LLVM 명령을 쉽게 생성할 수 있는 도우미 개체이다.

TheModule은 함수와 전역 변수를 포함하는 LLVM 구성체이다.

NamedValues는 현재 범위에서 정의된 값과 해당 LLVM 표현이 무엇인지 추적한다.

<br><br>

## Expression Code generation

```cpp
Value *NumberExprAST::codegen() {
  return ConstantFP::get(TheContext, APFloat(Val));
}
```

<br>

LLVM IR에서 숫자 상수는 ConstantFP 클래스로 표현되며, 이 클래스는 APFloat의 숫자 값을 내부적으로 유지한다.

APFloat는 임의 정밀도의 부동소수점 상수를 유지하는 기능을 가지고 있다.

이 코드는 기본적으로 ConstantFP를 생성하고 반환한다.

<br>

```cpp
Value *VariableExprAST::codegen() {
  // Look this variable up in the function.
  Value *V = NamedValues[Name];
  if (!V)
    LogErrorV("Unknown variable name");
  return V;
}
```

<br>

변수에 대해서는 간단하게 NamedValues에 있는 값을 리턴한다.

<br>

```cpp
Value *BinaryExprAST::codegen() {
  Value *L = LHS->codegen();
  Value *R = RHS->codegen();
  if (!L || !R)
    return nullptr;

  switch (Op) {
  case '+':
    return Builder.CreateFAdd(L, R, "addtmp");
  case '-':
    return Builder.CreateFSub(L, R, "subtmp");
  case '*':
    return Builder.CreateFMul(L, R, "multmp");
  case '<':
    L = Builder.CreateFCmpULT(L, R, "cmptmp");
    // Convert bool 0/1 to double 0.0 or 1.0
    return Builder.CreateUIToFP(L, Type::getDoubleTy(TheContext),
                                "booltmp");
  default:
    return LogErrorV("invalid binary operator");
  }
}
```

<br>

바이너리 연산자는 switch를 통해 구분하여 올바르게 명령을 수행하게 하는데, IRBuilder는 새로 생성된 명령어를 삽입할 위치를 알고 있다고 한다.

필요한 것은 어떤 명령어를 만들 것인지(e.g with CreateFAdd), 어떤 피연산자를 사용할 것인지(L, R)를 지정하고, 생성된 명령어의 이름을 제공하는 것이다.

<br>

```cpp
Value *CallExprAST::codegen() {
  // Look up the name in the global module table.
  Function *CalleeF = TheModule->getFunction(Callee);
  if (!CalleeF)
    return LogErrorV("Unknown function referenced");

  // If argument mismatch error.
  if (CalleeF->arg_size() != Args.size())
    return LogErrorV("Incorrect # arguments passed");

  std::vector<Value *> ArgsV;
  for (unsigned i = 0, e = Args.size(); i != e; ++i) {
    ArgsV.push_back(Args[i]->codegen());
    if (!ArgsV.back())
      return nullptr;
  }

  return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
}
```

<br>

함수 호출에 대한 코드생성은 먼저 TheModule(함수와 전역 변수를 포함하는 LLVM 구성체) 에서 해당 함수가 있는지 확인한다.

그 다음 전달할 각 인수를 반복적으로 코딩하고 LLVM 호출 명령을 생성한다.

위 4가지는 Kaleidoscope에서 기본적으로 제공하고, 내가 추가할 수 있다.

<br><br>

## Function Code Generation

마지막 부분에서 extern이 def보다 우선순위가 높다고 하는데, 이 과정에서 발생되는 문제점으로 중복 함수명을 처리할 때 에러가 발생한다.

<br>

```cpp
extern foo(a);     # ok, defines foo.
def foo(b) b;      # Error: Unknown variable name. (decl using 'a' takes precedence).
```

<br>

해결 방법은 여러가지라..


