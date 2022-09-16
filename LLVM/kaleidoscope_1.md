## Kaleidoscope Introduction and the Lexer
---

kaleidoscope는 함수를 정의하고 조건, 수학 등을 사용하는 절차적인 언어라고 한다.

kaleidoscope에서는 단순함을 위해 하나의 데이터 타입만 지원하는데, 64-bit floating point type(c언어의 double)만 지원한다.

그래서 타입을 선언할 필요가 없으며 암묵적으로 이중 정밀도라고 한다.

또한 표준 라이브러리 기능을 호출하도록 허용하며, extern 키워드로 사용 전에 함수를 정의할 수 있다.

<br>

```c
extern sin(arg);
extern cos(arg);
extern atan2(arg1 arg2);

atan2(sin(.4), cos(42))
```

<br><br>

## Lexer
---

Lexer : <a href="https://coding-groot.tistory.com/17">coding-groot.tistory.com/17</a>

<br>

언어 구현 시 가장 필요한 부분은 텍스트 파일을 처리하고 그것을 해석하는 것이다. 

대표적으로 lexer를 통해 입력을 토큰으로 나누는 것이라고 한다.

lexer는 그냥 입력값을 데이터형처럼 형태별로 구분지어놓은 것이라고 생각하면 될 것 같다.

<br>

```c
// The lexer returns tokens [0-255] if it is an unknown character, otherwise one
// of these for known things.
enum Token {
  tok_eof = -1,

  // commands
  tok_def = -2,
  tok_extern = -3,

  // primary
  tok_identifier = -4,
  tok_number = -5,
};

static std::string IdentifierStr; // Filled in if tok_identifier
static double NumVal;             // Filled in if tok_number
```

<br>

토큰이 만약 식별자라면 IdentifierStr 변수에 저장되고, 1.0 같은 숫자형이라면 NumVal 변수에 저장된다.

토큰이 '+' 같은 값이라면 unknown이 된다.

즉, 렉서에서 반환하는 각각의 토큰들은 토큰의 enum 값 중 하나가 되거나 unknown이 되어 ascii 값을 반환하게 된다.

실제로는 gettok() 함수를 통해 구현된다고 한다.

<br>

```c
/// gettok - Return the next token from standard input.
static int gettok() {
  static int LastChar = ' ';

  // Skip any whitespace.
  while (isspace(LastChar))
    LastChar = getchar();
```

<br>

공백을 읽지 않고, def, extern 같은 특정 키워드를 읽어낼 줄 알아야 하며, 파일의 끝인지 (EOF), newline인지 carrage return 등을 확인한다.
