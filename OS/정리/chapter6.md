# 32비트 보호모드로 전환하자

<br>

![image](https://user-images.githubusercontent.com/52172169/194505865-dcaa6ec8-e61a-4536-a59c-e5dba44a41b1.png)

<br>

리얼모드에서 보호모드로 전환하는 과정은 6단계를 거친다.

**상위 2단계**는 **보호모드에서 반드시 필요한 자료구조**인 **세그먼트 디스크립터**와 **GDT**를 **생성하는 과정**이다.

두 자료구조는 보호모드로 진입하자마자 참조되므로 미리 설정해줘야 한다.

나머지 하위 4단계는 생성한 자료구조를 프로세스에 설정 및 초기화 하는 단계이다. 

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 세그먼트 디스크립터 생성

<br>




<br><br>
<hr style="border: 2px solid;">
<br><br>

