# Implementing a Parser and AST
---

lexer를 통해 kaleidoscope parser를 만들어 보고 AST를 정의해본다.

<br><br>

## Abstract Syntax Table (AST)
---


```c
/// ExprAST - Base class for all expression nodes.
class ExprAST {
public:
  virtual ~ExprAST() {}
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
  double Val;

public:
  NumberExprAST(double Val) : Val(Val) {}
};
```

<br>

위의 코드를 보면 NumberExprAST 서브 클래스는 1.0 같은 숫자형을 캡쳐한다.

<br>

```c
/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
  std::string Name;

public:
  VariableExprAST(const std::string &Name) : Name(Name) {}
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
  char Op;
  std::unique_ptr<ExprAST> LHS, RHS;

public:
  BinaryExprAST(char op, std::unique_ptr<ExprAST> LHS,
                std::unique_ptr<ExprAST> RHS)
    : Op(op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
  std::string Callee;
  std::vector<std::unique_ptr<ExprAST>> Args;

public:
  CallExprAST(const std::string &Callee,
              std::vector<std::unique_ptr<ExprAST>> Args)
    : Callee(Callee), Args(std::move(Args)) {}
};
```

<br>

kaleidoscope 언어의 기본적은 AST 형태는 위의 코드와 같다.

VariableExprAST는 변수를 캡쳐하고, BinaryExprAST는 opcode를, CallExprAST는 함수와 인자를 캡쳐한다.

<br><br>

## Parser Basic
---

AST 구축을 위해 parser 코드를 정의해야 한다.

```x+y```는 렉서에 의해 3개의 토큰으로 분리되는데 이를 아래와 같이 분리할 수 있다.

<br>

```c
auto LHS = std::make_unique<VariableExprAST>("x");
auto RHS = std::make_unique<VariableExprAST>("y");
auto Result = std::make_unique<BinaryExprAST>('+', std::move(LHS), std::move(RHS));
```

<br>

기본적인 helper를 만들어보면 아래와 같다.

<br>

```c
/// CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
/// token the parser is looking at.  getNextToken reads another token from the
/// lexer and updates CurTok with its results.
static int CurTok;
static int getNextToken() {
  return CurTok = gettok();
}
```

<br>

이 코드를 통해 lexer가 반환할 토큰을 미리 볼 수 있다.

또한 에러를 출력하게 해주는 helper를 통해 에러를 다룰 수 있다.

<br>

```c
/// LogError* - These are little helper functions for error handling.
std::unique_ptr<ExprAST> LogError(const char *Str) {
  fprintf(stderr, "LogError: %s\n", Str);
  return nullptr;
}
std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
  LogError(Str);
  return nullptr;
}
```

<br><br>

## Basic Expression Parsing
---



