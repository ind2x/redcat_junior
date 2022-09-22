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

