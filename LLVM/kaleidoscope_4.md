# Kaleidoscope: Adding JIT and Optimizer Support

<br>

이번 챕터에서는 ```adding optimizer support to your language```과 ```adding JIT compiler support```를 해준다고 한다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## Trivial Constant Folding

<br>

LLVM에서는 AST에서 constant folding을 지원하지 않고, LLVM IR에서 자체적으로 constant folding 할 부분이 있는지 찾아서 수행한다.

그래서 아래와 같이 constant folding을 해주는 것 같다.

<br>

```shell
ready> def test(x) 1+2+x;
Read function definition:
define double @test(double %x) {
entry:
        %addtmp = fadd double 3.000000e+00, %x
        ret double %addtmp
}
```

<br>

또한 아래와 같은 LHS = RHS 인 상황도 있다.

<br>

```shell
ready> def test(x) (1+2+x)*(x+(1+2));
ready> Read function definition:
define double @test(double %x) {
entry:
        %addtmp = fadd double 3.000000e+00, %x
        %addtmp1 = fadd double %x, 3.000000e+00
        %multmp = fmul double %addtmp, %addtmp1
        ret double %multmp
}
```

<br>

이 경우 LHS = RHS이기 때문에 굳이 addtmp, addtmp1을 만들어서 multmp를 하는 것이 아닌, addtmp 만으로만 multmp을 표시해주고 싶을 것이다.

그러나 이것은 가능하려면 두 가지 변환이 필요한데, add 명령어에 관한 중복 표현 삭제(Common Subexpression Elimination (CSE))와 식의 재연결(reassociation of expressions)이 필요하다.

LLVM에서는 passes 형태로 광범위한 형태의 최적화를 지원한다고 한다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## LLVM Optimization Passes

<br>

최적화 하는 방식에 두 가지 기능이 있는데, 전체 코드를 읽어서 최적화 하는 방식(whole module passes)과 함수 하나씩 최적화 하는 방식(per-function passes)가 있다.

```per-function``` 방식을 이용할 거고 이를 위해서는 ```FunctionPassManager```를 설해야 한다.

```FunctionPassManager```는 ```LegacyPassManager.h```의 ```llvm::legacy::FunctionPassManager``` 의 형태로 있다.

최적화하려는 모듈마다 ```FunctionPassManager```가 필요하므로 초기화하고 생성하는 함수를 작성해준다.

<br>

```shell
void InitializeModuleAndPassManager(void) {
  // Open a new module.
  TheModule = std::make_unique<Module>("my cool jit", TheContext);

  // Create a new pass manager attached to it.
  TheFPM = std::make_unique<legacy::FunctionPassManager>(TheModule.get());

  // Do simple "peephole" optimizations and bit-twiddling optzns.
  TheFPM->add(createInstructionCombiningPass());
  
  // Reassociate expressions.
  TheFPM->add(createReassociatePass());         // 위에서 설명한 변환 1
  
  // Eliminate Common SubExpressions.
  TheFPM->add(createGVNPass());                 // 위에서 설명한 변환 2
  
  // Simplify the control flow graph (deleting unreachable blocks, etc).
  TheFPM->add(createCFGSimplificationPass());

  TheFPM->doInitialization();
}
```

<br>

그래서 ```FunctionAST::codegen()```에서 함수 IR을 생성한 후 반환되기 전에 위의 최적화 함수를 호출하여 최적화를 해준 뒤 반환해준다.

<br>

```cpp
if (Value *RetVal = Body->codegen()) {
  // Finish off the function.
  Builder.CreateRet(RetVal);

  // Validate the generated code, checking for consistency.
  verifyFunction(*TheFunction);

  // Optimize the function.
  TheFPM->run(*TheFunction);    // FunctionAST::codegen()에서 이 부분이 추가된 것

  return TheFunction;
}
```

<br>

그래서 LHS = RHS인 상황에서 하고자 했던 최적화를 할 수 있게 된다.

<br>

```shell
ready> def test(x) (1+2+x)*(x+(1+2));
ready> Read function definition:
define double @test(double %x) {
entry:
        %addtmp = fadd double %x, 3.000000e+00
        %multmp = fmul double %addtmp, %addtmp
        ret double %multmp
}
```

<br><br>
<hr style="border: 2px solid;">
<br><br>

## Adding a JIT Compiler

<br>

kaleidoscope의 기본 아이디어는 사용자가 입력한 표현식을 즉시 평가하는 즉, 1+2를 입력하면 바로 3을 평가하여 출력해야 한다.

JIT compile 기능을 추가하기 위해서는 마찬가지로 코드 생성 및 JIT 선언과 초기화를 해줘야 한다.

<br>

```cpp
static std::unique_ptr<KaleidoscopeJIT> TheJIT;
...
int main() {
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();

  // Install standard binary operators.
  // 1 is lowest precedence.
  BinopPrecedence['<'] = 10;
  BinopPrecedence['+'] = 20;
  BinopPrecedence['-'] = 20;
  BinopPrecedence['*'] = 40; // highest.

  // Prime the first token.
  fprintf(stderr, "ready> ");
  getNextToken();

  TheJIT = std::make_unique<KaleidoscopeJIT>();         // JIT 선언

  // Run the main "interpreter loop" now.
  MainLoop();

  return 0;
}
```

<br>

```cpp
void InitializeModuleAndPassManager(void) {
  // Open a new module.
  TheModule = std::make_unique<Module>("my cool jit", TheContext);
  TheModule->setDataLayout(TheJIT->getTargetMachine().createDataLayout());      // JIT 추

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
```

<br>

KaleidoscopeJIT 클래스는 이러한 튜토리얼을 위해 특별히 제작된 간단한 JIT이며, ```llvm-src/examples/Kaleidoscope/include/KaleidoscopeJIT.h```의 LLVM 소스 코드에서 사용할 수 있다.

그래서 이번 챕터에서는 이 간단한 JIT 기능을 추가하여 아래와 같이 코드를 수정하여 사용할 수 있다.

<br>

```cpp
static void HandleTopLevelExpression() {
  // Evaluate a top-level expression into an anonymous function.
  if (auto FnAST = ParseTopLevelExpr()) {
    if (FnAST->codegen()) {

      // JIT the module containing the anonymous expression, keeping a handle so
      // we can free it later.
      auto H = TheJIT->addModule(std::move(TheModule));
      InitializeModuleAndPassManager();

      // Search the JIT for the __anon_expr symbol.
      auto ExprSymbol = TheJIT->findSymbol("__anon_expr");
      assert(ExprSymbol && "Function not found");

      // Get the symbol's address and cast it to the right type (takes no
      // arguments, returns a double) so we can call it as a native function.
      double (*FP)() = (double (*)())(intptr_t)ExprSymbol.getAddress();
      fprintf(stderr, "Evaluated to %f\n", FP());

      // Delete the anonymous expression module from the JIT.
      TheJIT->removeModule(H);
    }
```

<br>

addModule 는 LLVM IR 모듈을 JIT에 할당하여 함수를 실행할 수 있게 한다.

removeModule는 모듈을 제거하여 해당 모듈의 코드와 관련된 메모리를 확보한다.

findSymbol 는 컴파일된 코드에 대한 포인터를 조회할 수 있게 한다.

<br>

parsing과 codegen이 성공하면 top-level expression가 포함된 모듈을 JIT에 추가해야 한다.

위의 코드를 보면 먼저 codegen을 호출을 하고 통과하면 나중에 JIT에서 모듈을 제거하는데 사용할 handle(```auto H = TheJIT->addModule(std::move(TheModule));```)을 반환하는 addModule을 호출한다.

모듈이 JIT에 추가되면 더 이상 수정할 수 없으므로 ```InitializeModuleAndPassManager()```를 호출하여 새 모듈을 열어 후속 코드를 잡는다.

<br>

이제 모듈이 JIT에 추가되었는데 최종 생성되는 코드에 대한 포인터를 얻어야 한다. (```auto ExprSymbol = TheJIT->findSymbol("__anon_expr");```)

(ParseTopLevelExpr 에서 PrototypeAST에 name 값으로 ```__anon_expr```을 준다.)

그 다음 ```double (*FP)() = (double (*)())(intptr_t)ExprSymbol.getAddress();```를 통해 getAddress 메소드를 호출하여 해당 심볼의 메모리 내의 주소를 가져온다.

메모리를 구했다면 삭제해주면 된다. (```TheJIT->removeModule(H);```)

<br>

따라서 테스트 해보면 다음과 같다.

<br>

```cpp
ready> 4+5;
Read top-level expression:
define double @0() {
entry:
  ret double 9.000000e+00
}

Evaluated to 9.000000
```

<br>

바로 4+5 식을 평가하여 9를 계산하여 리턴해주는 것을 볼 수 있다.

<br><br>
<hr style="border: 2px solid;">
<br><br>
