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

이제 AST의 codegen 수정한다.

<br>

![image](https://user-images.githubusercontent.com/52172169/199673706-beb48d43-0cba-4b0b-bb05-329d957a7ab5.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/199677119-d03e135b-5611-46aa-b614-bca8072637ec.png)

<br>

마지막의 top-level 코드인 FunctionAST codegen에서는 우리가 정의한 연산자의 우선순위를 설정해주는 것을 볼 수 있다.

그 다음 TheContext에 넣어준다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## User-defined Unary Operators

<br>

unary operator는 AST와 Parser가 없으므로 생성해준다.

<br>

+ AST

![image](https://user-images.githubusercontent.com/52172169/199680475-d3cf817d-c0be-49c7-9d43-136baf99e48a.png)

<br>

+ Parser

![image](https://user-images.githubusercontent.com/52172169/199680527-7abd2ab7-15df-490d-81a6-683200f9212d.png)

<br>

정의한 ParseUnary를 호출해주는 부분으로 ParseBinOpRHS에서 불러준다.

<br>

![image](https://user-images.githubusercontent.com/52172169/199682651-21c2eac1-0e78-4db7-aea5-9ec215fdc66f.png)

<br>

그 다음 추가할 부분은 def를 이용해서 불러오므로 결국 ParsePrototype에서 함수인지, 단항 연산자인지, 이항 연산자인지 등을 구분해줘야 한다. 

따라서 여기에 tok_unary를 추가해준다.

<br>

![image](https://user-images.githubusercontent.com/52172169/199684646-cda7f295-26b5-49f7-9ded-f7c45f068eb9.png)

<br>

마지막으로 unary 코드를 생성하는 IR 코드를 작성해준다.

<br>

![image](https://user-images.githubusercontent.com/52172169/199712477-68701123-fba1-4b96-8ddf-ec8d7330d41e.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>

## Kicking The Tires

<br>



<br><br>
<hr style="border: 2px solid;">
<br><br>
