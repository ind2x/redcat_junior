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

이를 mutable 변수라고 하는데, 문제는 'phi node를 누가 배치할 것인가' 라고 하는데 이를 llvm에서 스택 변수(메모리 객체)를 이용해서 해결한다고 한다.

llvm에서 스택 변수를 만드는 방법은 ```alloca``` 명령어를 이용하는 것이라고 한다.

<br>

![image](https://user-images.githubusercontent.com/52172169/200567882-bd9d36b9-f367-49c5-9959-30508bf43687.png)

<br>

load와 store 명령어를 이용해서 불러오고 저장한다.




<br><br>
<hr style="border: 2px solid;">
<br><br>
