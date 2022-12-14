# 키보드 디바이스 드라이버 추가
## 키보드 컨트롤러의 구조와 기능

<br>

![image](https://user-images.githubusercontent.com/52172169/196896404-7d4f1ff8-2e0c-48a0-81fe-cfe1a99b5ccd.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/196896598-873a84b5-770f-4dc4-9489-7b9a13e5caab.png)

<br>

레지스터의 크기는 모두 **1바이트**이며, 그 중 **상태 레지스터는 가장 중요한 레지스터**로, 키보드 컨트롤러에 값을 읽고 쓰려면 반드시 체크해야 하는 비트를 포함하고 있다.

아래가 그 비트들이다.

<br>

![image](https://user-images.githubusercontent.com/52172169/196896619-96beecdc-d269-4841-93da-7b563a86d37e.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/196896661-d6a483d2-90f4-4391-acd8-074aff22db3f.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 키보드 컨트롤러 제어
### 키보드와 키보드 컨트롤러 활성화
---

<br>

우선 기본적으로 BIOS에 의해 키보드가 활성화되므로 굳이 해주지 않아도 되는데, 혹시 모르니까 해준다.

키보드를 활성화시켜주려면 커맨드 포트 키보드 디바이스를 활성화한다는 의미의 0xAE 를 보낸다. (위의 키보드 컨트롤러 커맨드 확인)

그러나 이것이 키보드 컨트롤러가 활성화 된 것이지 **키보드가 활성화 된 것은 아니다.**

따라서 **키보드 자체에도 활성화한다고 보내야 하는데**, 입력 버퍼에 직접 키보드 커맨드 0xF4를 보내주면 된다. (아래 사진 확인)

키보드는 컨트롤러와 달리 응답 값을 보내주는데, ACK(0xFA)를 보내주므로 이를 통해 확인하여 오지 않았다면 재시도를 하거나 중단해야 한다.

<br>

![image](https://user-images.githubusercontent.com/52172169/196907526-d8d1ff7d-59a4-4f51-9d7e-7c797878b0c2.png)

<br>

그래서 활성화 해주려면 키보드 컨트롤러에 0xAE를 보내주면 되는데, 문제는 컨트롤러는 CPU와 달리 처리속도가 상대적으로 느리다.

우리는 키보드에도 명령을 보내줘야 하므로 **상태 레지스터**를 통해 입력 버퍼와 출력 버퍼의 상태를 확인하여 값이 없으면 보내고 값이 있다면 읽어오도록 처리해줌으써 효율적으로 처리할 수 있다.

<br>

출력값과 입력값을 읽고 보낼 때, InPortByte와 OutPortByte 함수를 정의하여 이 함수들을 통해 I/O 포트에서 값을 I/O 버퍼로 가져온다.

즉, **InPortByte는 프로세서가 출력 버퍼에서 값을 읽어오는 함수**이고, **OutPortByte는 프로세서가 컨트롤러에게 값을 보내는 함수**이다.

<br>

```asm
[BITS 64]

SECTION .text

global InPortByte, OutPortByte

InPortByte:
    push rdx
    mov rdx, rdi
    mov rax, 0
    in al, dx

    pop rdx
    ret

OutPortByte:
    push rdx
    push rax

    mov rdx, rdi
    mov rax, rsi

    out dx, al

    pop rax
    pop rdx
    ret
```

<br>

IN 명령어는 포트 I/O 주소를 지정하는데 dx 레지스터를 사용하며, 포트에서 값을 읽어와서 ax 레지스터에 저장한다.

OUT 명령어는 포트 I/O 주소에서 데이터를 출력하며 마찬가지로 dx 레지스터에 포트 주소, al에 값을 넣어서 포트주소에 값을 쓴다.

<br><br>

### IA-32e 모드 함수 호출 규약
---

<br>

보호모드와의 차이점을 중심으로 보면 다음과 같다.

64비트는 알다시피 레지스터에 인자 값을 받아오며 실수 값의 경우에 XMM0 ~ XMM7 레지스터에 값을 받아오고, 왼쪽부터 오른쪽으로 순서대로 받아온다.

보호모드는 오른쪽에서 왼쪽 순으로 스택으로 인자 값을 받아온다.

또한 64비트는 리턴 값을 RAX 또는 XMM0에 받아오고, 보호모드는 EAX 레지스터로 받아온다.

<br><br>

### 키보드 컨트롤러에서 값 읽기
---

<br>

단순히 출력 버퍼에 먼저 값이 있는지 확인한 후, 데이터가 있으면 읽어서 반환해주면 된다.

<br>

![image](https://user-images.githubusercontent.com/52172169/196941794-c07848e3-96b8-4a03-9ea5-e2a673c93b4f.png)

<br><br>

### A20 게이트 활성화와 프로세스 리셋
---

<br>

출력 포트와 관련된 명령은 위의 사진의 컨트롤러 커맨드를 확인해보면 0xD0, 0xD1이 있다.

A20 게이트 비트와 프로세스 리셋 비트는 출력 포트의 비트 1과 0에 있다.

따라서 컨트롤러의 입력 버퍼를 통해 값을 1 또는 0으로 주면 된다.

<br><br>

### 키보드 LED 상태 제어
---

<br>

![image](https://user-images.githubusercontent.com/52172169/196967286-ddbeb2c8-63d5-4013-96df-d488bc2f635a.png)

<br>

키보드 활성화 방법으로 입력 버퍼로 직접 키보드로 전송하는 방식과 똑같이 위의 사진을 참고하여 LED를 활성화 시 0xED 커맨드를 전송한다.

전송하면 ACK를 받고 나서 그 값을 확인하면 된다.

각각의 Lock을 활성화 시 1로, 비활성화 시 0으로 설정해주면 된다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 스캔 코드와 간단한 셸
### 키보드와 스캔코드
---

<br>

키보드마다 고유의 값이 있고, 그 값을 스캔코드라고 하며 이 스캔코드로 무슨 값이 입력됬는지 알 수 있다.

우리는 스캔코드를 아스키코드로 변환하여 값을 출력해줘야 한다.

책에서 스캔코드 테이블을 확인해보면 down, up으로 값이 두 개 있는데, 각각 눌렀을 때와 떨어졌을 때의 값이다.

<br>

up 값은 down 값의 최상위 비트를 1로 설정한 값이며, 0x80을 더한 값이기도 하므로 눌렀을 때의 값에 0x80을 더해주는 방식으로 처리해주면 된다.

즉 ```Up Scan Code == Down Scan Code + 0x80```

<br><br>

### 스캔코드를 ASCII 문자로의 변환
---

<br>

가장 쉬운 방법은 변환 테이블을 만들어주는 것이다.

단, F1 키나 Home 키 같은 아스키 값이 없는 키들은 0x80 이상의 값을 할당해줘서 인식해 줄 것이다.

또한 값을 받을 때 구분해줘야 하는 값들을 그룹화하여 정리해줘야 한다.

<br>

![image](https://user-images.githubusercontent.com/52172169/196978101-732377df-ed84-40c4-aac2-1309b7db050e.png)

<br>

코드는 p.345 참고

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 키보드 디바이스 드라이버의 통합 및 빌드

<br>

02.Kernel64/Source/Keyboard.h와 Keyboard.c 코드가 추가된다.

내용은 위에서 보았던 **키보드 활성화**(키보드 컨트롤러 및 키보드 자체 활성화)를 해주는 것과 **입력버퍼와 출력버퍼에서 각각 값을 쓰고 읽어오는 과정**, **스캔코드를 아스키 코드로 변환**하여 **입력값을 화면에 출력해주는 과정**(**변환 테이블과 알고리즘**을 짜고 화면에 출력해주는 **셸을 구현**)을 했다. 

<br>

![image](https://user-images.githubusercontent.com/52172169/197117146-4f146c7d-bf26-483b-8cf5-00cdebdf0df7.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>
