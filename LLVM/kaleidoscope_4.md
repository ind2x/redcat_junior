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



<br><br>
<hr style="border: 2px solid;">
<br><br>
