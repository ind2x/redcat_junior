# Kaleidoscope: Extending the Language: User-defined Operators
## User-defined Operators: the Idea

<br>

챕터 6은 user-defined 연산자를 추가해는 챕터이다.

추가할 연산자는 unary operator와 binary operator이다.

아래가 그 예시이다.

<br>

![image](https://user-images.githubusercontent.com/52172169/199666238-1d07e620-bb63-4104-b600-ded08af57ecc.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>

## User-defined Binary Operators

<br>

+ Lexer

![image](https://user-images.githubusercontent.com/52172169/199666574-286549fa-83c4-47d2-93fc-3369a27ae5f9.png)

<br>

이번 절에서는 우리는 이미 연산자에 관한 AST와 Parser를 구현해놓았기 때문에 새롭게 구현할 필요가 없고 수정을 해주기만 하면 된다.

맨 처음 사진을 보면은 def를 통해 정의를 해주기 때문에 PrototypeAST로 들어가게 된다.

따라서 우리는 PrototypeAST를 수정해줘야 한다.

<br>

![image](https://user-images.githubusercontent.com/52172169/199668108-53f97e31-0978-4b8a-8118-5457e68c7500.png)

<br>

마찬가지로 ParsePrototype 파서 코드 또한 수정해줘야 한다.

<br>

![image](https://user-images.githubusercontent.com/52172169/199672795-4e9cc641-6c6d-4519-83a5-9d0d3df066bc.png)

![image](https://user-images.githubusercontent.com/52172169/199672825-c9a26325-ca4c-45f7-934f-cb9940f81742.png)

<br>









<br><br>
<hr style="border: 2px solid;">
<br><br>

## User-defined Unary Operators

<br>




<br><br>
<hr style="border: 2px solid;">
<br><br>
