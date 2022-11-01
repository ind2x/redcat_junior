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

then과 else의 코드는 대체로 비슷하지만 else의 첫 번째 줄만 다르다는 점을 주목해야 한다.

이 코드는 함수에 else 블록을 추가하는 코드이다.

왜 다르냐하면, 2번째 사진에서 then과 else 블록을 만들었지만 then 블록은 TheFunction에 추가(insert)되었지만, else 블록은 추가되지 않았음을 알 수 있다.

마지막으로 두 코드들을 병합하는 과정이 남았다.

<br>

![image](https://user-images.githubusercontent.com/52172169/199176228-69a9c04f-e0a1-4be0-83ac-6803dd7c568b.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>

## for

<br>

+ Expression

![image](https://user-images.githubusercontent.com/52172169/199195637-a8f87709-7de1-496f-a371-d379d6f08fc7.png)

<br>

+ Lexer

![image](https://user-images.githubusercontent.com/52172169/199178509-6372506b-2ee8-4ea5-baca-cafadb1bdd4a.png)

<br>

+ AST

![image](https://user-images.githubusercontent.com/52172169/199178570-e1173a15-82cc-4e03-bb05-eedea980144a.png)

<br>

+ Parser

![image](https://user-images.githubusercontent.com/52172169/199179138-efdb7a03-23b4-497d-af44-18330265d547.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/199179387-9a5d8969-15df-4d5f-8f73-f22bbea63dfb.png)

<br>

+ IR

![image](https://user-images.githubusercontent.com/52172169/199179635-5044d463-a1ac-455a-a7aa-93da9f20ad35.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>

