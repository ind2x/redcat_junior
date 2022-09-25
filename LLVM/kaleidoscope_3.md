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












