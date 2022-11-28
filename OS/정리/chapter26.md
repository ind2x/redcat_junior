# C 표준 입출력 함수를 추가하자
## C 표준 입출력 함수 설계

<br>

우린 POSIX 함수 중 파일 입출력과 관련된 함수들을 구현할 것이다.

아래는 구현할 함수 목록이다.

<br>

![image](https://user-images.githubusercontent.com/52172169/204198435-2746902f-9c8b-4ef1-b8dd-a7dbfafc92e6.png)

<br>

```size_t``` 타입은 ```unsigned int```형을 재정의한 것.

<br>

![image](https://user-images.githubusercontent.com/52172169/204198499-bb08009b-d260-47bc-b416-2c3ef58a1743.png)

<br>

FILE, DIR 자료구조를 먼저 우리 특성에 맞게 정의를 해준다.

표준 입출력 함수처럼 클러스터 단위가 아니라 바이트 단위로 처리해주기 위해서는 현재 작업이 수행되는 위치에 따라 클러스터를 이동하는 부분이 필요하다.

여러 필요한 부분을 모아 FILE 자료구조를 정의해준다.

<br>

![image](https://user-images.githubusercontent.com/52172169/204199495-f4d4e96d-b3ae-4c9a-8742-7e2012550ebc.png)

<br>

디렉터리 관련 함수는 읽기 기능만 제공하므로 현재 루트 디렉터리에서 어느 오프셋의 엔트리를 읽고 있는지만 알면 된다.

<br>

![image](https://user-images.githubusercontent.com/52172169/204199731-0bfcd8b1-5d7e-45d0-b527-413c8b0ecbc3.png)

<br>

그 다음 두 자료구조를 합쳐 풀 형태로 관리한다. (MINT64 OS에서는 모든 자료구조를 태스크 풀처럼 풀 형태로 관리한다고 한다..)

합칠 때, Union이라는 공용체를 사용하는데, Union은 한 메모리 공간을 여러 타입으로 사용하고자 할 때 사용한다고 한다.

<br>

![image](https://user-images.githubusercontent.com/52172169/204200640-227b0276-c05c-44f6-bd75-92c2d832ddaa.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 고수준 함수 구현

<br>

먼저 핸들 자료구조가 추가되었으므로 초기화 전에 메모리를 할당해줘야 하는데 이 부분은 동적 메모리로 할당할 것이다.

핸들을 할당하는 함수와 해제하는 함수는 간단하고, 타입은 3가지 (파일, 디렉터리, Free(빈))로 구분한다.

할당은 핸들 풀에서 Free 핸들을 찾아서 반환하고 할당된 것으로 표시해주고, 해제는 해당 핸들을 빈 타입으로 설정해주면 된다.

여기서 주의점은 우리는 디렉터리를 만들지 않고 파일만 생성할 수 있기 때문에 파일 타입으로만 설정해주면 된다.

<br>

여기부터는 위의 파일 함수, 디렉터리 함수를 정의해주는 것으로 p.1054부터 확인해주면 된다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 빌드

<br>

![image](https://user-images.githubusercontent.com/52172169/204286436-c82a5bd3-b659-4c93-bbe6-5a23f119088d.png)

<br>

testfileio 명령어를 실행하면 계속 에러가 발생하는데 이유를 모르겠다..

<br><br>
<hr style="border: 2px solid;">
<br><br>
