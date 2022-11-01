# Kaleidoscope: Extending the Language: Control Flow

<br>

이번 장에서는 조건문과 간단한 for문 처리 코드를 구현해 줄 것이다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## If/Then/Else

<br>

+ Lexer

![image](https://user-images.githubusercontent.com/52172169/199024995-0f53444f-3165-4453-9e1a-94d1c74ad024.png)

<br>

+ AST

![image](https://user-images.githubusercontent.com/52172169/199025273-b255c3db-abe6-4f47-82dd-0c6315895ba0.png)

<br>

+ Parser

![image](https://user-images.githubusercontent.com/52172169/199033052-316cd160-7c40-46a5-a1fd-c66c4b33cf0a.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/199033098-9a26650a-03ad-4389-9afd-26431388121c.png)

<br>

IR 부분은 좀 흥미로운 부분으로 아래와 같이 나뉘어진다.

<br>

![image](https://user-images.githubusercontent.com/52172169/199033288-abcda7d5-a7ef-4b6d-9e12-464cd27e3376.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/199033350-dab7bf1d-3030-490e-944a-824365c263a6.png)

<br>

Code Generation은 다음과 같다.

<br>

![image](https://user-images.githubusercontent.com/52172169/199034012-d80bc76d-27bd-4534-9d57-b5e946ff4a63.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/199175463-2befae3e-193d-4fe8-8eb3-d81fd7fcc93a.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/199175530-173f4304-0647-43b7-abb5-85fcd93b94a1.png)


<br>

![image](https://user-images.githubusercontent.com/52172169/199175375-733270c6-a196-4f1a-ac5d-be21ba2c59fe.png)

<br>


<br>



<br><br>
<hr style="border: 2px solid;">
<br><br>

## for

<br>



<br><br>
<hr style="border: 2px solid;">
<br><br>

