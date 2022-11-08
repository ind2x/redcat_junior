# Mutable Variables

<br>

```c
int G, H;
int test(_Bool Condition) {
  int X;
  if (Condition)
    X = G;
  else
    X = H;
  return X;
}
```

<br>

위의 간단한 코드에서 보면 변수 X는 조건에 따라 G와 H의 값을 할당받는다. 

이 코드에 대한 IR를 생성하면 다음과 같다.

<br>

![image](https://user-images.githubusercontent.com/52172169/200568581-59b06b99-acf4-46aa-9d67-f0ede19d882f.png)

<br>

변수 X를 mutable 변수라고 하는데, 이 코드에서 문제는 'phi node를 누가 배치할 것인가' 라고 하는데 이를 llvm에서 스택 변수(메모리 객체)를 이용해서 해결한다고 한다.

llvm에서 스택 변수를 만드는 방법은 ```alloca``` 명령어를 이용하는 것이라고 한다.

<br>

![image](https://user-images.githubusercontent.com/52172169/200567882-bd9d36b9-f367-49c5-9959-30508bf43687.png)

<br>

load와 store 명령어를 이용해서 불러오고 저장한다.

따라서 이 방법을 이용해서 아래와 같이 phi node 대신에 IR를 생성할 수 있다.

<br>

![image](https://user-images.githubusercontent.com/52172169/200569151-d366f925-4d5d-46c7-92b6-43b4676aff3a.png)

<br><br>

이제 연산자 '=' 과 변수를 정의하는 기능을 추가해 줄 것이다.

<br>

![image](https://user-images.githubusercontent.com/52172169/200571966-c4e0c1f4-c422-4062-959e-2adf986b4357.png)

<br>




<br><br>
<hr style="border: 2px solid;">
<br><br>
