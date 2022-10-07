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

세그먼트 디스크립터는 **세그먼트의 정보를 저장하는 자료구조**로, 크게 **코드 세그먼트 디스크립터**와 **데이터 세그먼트 디스크립터**가 있다.

우리가 만들 mint64 OS는 보호모드의 기본적인 기능만 사용할 예정으로 코드, 데이터 세그먼트만 사용한다고 한다.

<br>

**코드 세그먼트 디스크립터**는 **실행 가능한 코드가 포함된 세그먼트의 정보를 저장**하고 **CS 세그먼트 레지스터에 사용**된다.

**데이터 세그먼트 디스크립터**는 데이터가 포함된 세그먼트 (스택 등)의 정보를 저장하고, CS 세그먼트 레지스터를 제외한 세그먼트 레지스터가 사용할 수 있다.

<br>

보호 모드의 세그먼트 디스크립터는 8바이트로, 다양한 필드가 있다.

<br>

![image](https://user-images.githubusercontent.com/52172169/194510281-e59ba1b2-0ce6-4d4e-a4ab-faa0d0ec1684.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/194510341-a6b9bf88-ebad-42aa-9cec-411401528632.png)

<br>

우리한테 필요한 정보 즉, 우리가 만들 mint64 OS에서 사용할 세그먼트의 조건은 다음과 같다.

<br>

![image](https://user-images.githubusercontent.com/52172169/194510450-bd2f8276-932c-403a-a4ab-0e5fcdb8ca62.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>

### 코드 세그먼트 디스크립터와 데이터 세그먼트 디스크립터 설정

<br>




<br><br>
<hr style="border: 2px solid;">
<br><br>
