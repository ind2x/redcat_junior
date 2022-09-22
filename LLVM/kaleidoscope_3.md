# Code generation to LLVM IR

이 장에서는 AST를 LLVM IR로 변환하는 방법에 대해 설명한다.

또한 llvm이 어떤 작업을 수행하는지 일부 보여준다.

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












