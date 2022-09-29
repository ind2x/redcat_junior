# 내 PC를 부팅하자

## 부팅과 부트로더

<br>

모든 OS는 512바이트의 작은 코드로부터 시작되는데, 이를 **부트로더**라고 한다.

부팅은 PC가 켜지고 OS가 실행되기 전까지 수행되는 일련의 작업 과정이다.

부팅 과정에서 프로세서 초기화, 외부 디바이스 검사 및 초기화, 부트 로더를 메모리에 적재, OS 시작 등을 수행한다.

BIOS는 Basic Input/Output System이라고 하며, 부팅과정 중 하드웨어와 관련된 작업을 수행하고 이를 POST라고 한다.

전원이 켜짐과 동시에 프로세서가 가장 먼저 실행하는 코드이며, OS가 실행될 환경을 만든다.

<br>

![image](https://user-images.githubusercontent.com/52172169/192775175-5bbd25dc-ef59-475c-be44-95ef304cddf2.png)

<br>

BIOS가 수행하는 작업 중 **가장 중요한 작업**은 **부트로더 이미지를 메모리로 복사하는 과정**이다.

부트로더는 Bootstrap 코드라고도 불리며, 우리가 BIOS로부터 처음으로 제어권을 넘겨받는 과정이다.

부트로더 코드는 저장 매체의 가장 앞부분에 존재를 하며, PC는 다양한 장치로 부팅이 가능하기 때문에 BIOS는 POST 작업 후 여러 저장매체를 검사하여, 부트로더가 있으면 코드를 ```0x7C00``` 주소로 복사한 뒤 프로세스가 ```0x7C00``` 주소부터 실행하도록 한다.

만약, 부트로더를 찾을 수 없다면 에러가 발생하며 작업을 중단한다.

**정상적으로 되었다면**, BIOS에 의해 PC가 정상적으로 실행되었음을 의미한다.

즉, **우리가 만든 OS를 메모리에 올려서 실행할 준비가 된 것**이다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 부트로더의 역할과 구성

<br>

부트로더는 외부 저장 매체 (디스크 등)에 있으며, 가장 첫 번째 섹터(MBR, Master Boot Record)에 있는 작은 프로그램이다.

섹터는 디스크를 구성하는 데이터 단위로, 512바이트이다.

크기가 너무 작아 여러 기능을 구현할 수 없으므로, **부트로더의 역할은 OS 실행에 필요한 환경 설정 및 OS 이미지를 메모리로 복사하는 것**뿐이다.

<br>

BIOS가 저장 매체의 MBR이 부트로더인지 판단하는 방법은 MBR 512바이트의 끝의 2바이트 값이 ```0x55, 0xAA```로 끝나는지 확인하는 것이다.

이 값이 아니라면 데이터라고 판단하고 부팅을 중단한다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 부트로더 제작 준비

<br>

![image](https://user-images.githubusercontent.com/52172169/192780024-c1e88a97-a44d-42ed-bd5e-87248d632c79.png)

<br>

우리가 만들 OS의 디렉토리는 이렇게 생겼다.

<br>

간단한 부트로더를 만든 결과

<br>

![image](https://user-images.githubusercontent.com/52172169/192797676-5015e4b5-0722-434f-a1d5-5d544b7de7c2.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 화면버퍼와 화면제어

<br>

화면에 문자를 출력하려면 현재 동작중인 화면과 관련된 비디오 메모리 어드레스를 알아야 한다.

비디오 메모리는 화면 출력과 관계된 메모리로, 모드별로 정해진 형식에 따라 데이터를 채우면 화면에 원하는 문자나 그림을 출력해줄 수 있다.

PC 부팅 후 디폴트 화면 모드는 텍스트 모드로, 화면 크기는 가로 80문자에 세로 25문자이고, 비디오 메모리 주소는 **0xB8000**에서 시작한다.

<br>

문자 하나 당 문자값 1바이트 + 속성값 1바이트 = 2바이트이며, 총 메모리 크기는 ```80 * 25 * 2 = 4000byte```이다.

<br>

![image](https://user-images.githubusercontent.com/52172169/192921399-a37cba49-a33a-4c78-9773-f2e30de76c44.png)

<br>

속성 1바이트는 세부적으로는 하위 4비트의 전경색과 상위 4비트의 배경색으로 구성되어 있다.

더 나아가 각각 최상위 비트는 화면깜빡임이나 강조 효과를 나타내는데 사용하며, 나머지 3비트는 색을 나타낸다.

속성값의 최상위 비트(배경색 강조 또는 깜빡임)는 비디오 컨트롤러의 속성 모드 제어 레지스터의 값에 따라 결정된다.

레지스터 값이 1이면 깜빡임을, 0이면 배경색 강조 효과가 된다. qemu는 기본적으로 0으로 설정되어 있다고 한다.

<br>

![image](https://user-images.githubusercontent.com/52172169/192924060-b0b62625-b765-48da-aa61-d46009c8e11f.png)

<br>

리얼모드에서의 메모리 관리 기법을 생각해보면 여기서는 ```물리주소 = 세그먼트 레지스터 값(기준 주소)*16 + 범용레지스터```가 된다.

따라서 세그먼트 레지스터의 기준 주소를 ```0xB800```으로 설정을 해주고 범용 레지스터 값이 0이라면 물리 주소는 곧 비디오 메모리 주소인 ```0xB8000```이 될 것이다.

<br>

```asm
mov ax, 0xB800
mov ds, ax
```

<br>

위의 속성값 표를 참조하여 첫 번째 글자를 빨간색 배경에 녹색으로 표현해주고 싶다고 하면 최상위 비트는 어차피 qemu에서는 1이므로 신경쓰지 말고 나머지 각각 하위 3비트만 신경써주면 된다.

따라서 상위 3비트는 빨간색이 되려면 2진수로 ```100(4)```이 되어야 하고, 하위 3비트는 녹색이 되려먼 2진수로 ```010(2)```이 되어야 한다.

이 값은 1바이트 값으로 ```0x4A```가 된다

<br>

```asm
mov byte [0x00], 'M'
mov byte [0x01], 0x4A
```

<br>

![image](https://user-images.githubusercontent.com/52172169/192928281-1332dc10-e791-44b2-8b12-f6bb5cb1318c.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 세그먼트 레지스터 초기화 

<br>

문자를 출력하는 코드 이전에 세그먼트 레지스터를 초기화해줘야 한다.

초기화 하지 않으면 이전에 BIOS가 쓰던 값이 들어있으므로 엉뚱한 주소에 접근하게 될 수 있다.

<br>

BIOS가 부트로더를 메모리로 복사하는데, 그 주소는 ```0x7C00```이며, 부트로더의 CS와 DS는 ```0x7C00```에서부터 512바이트 범위에 존재한다.

그래서 세그먼트 레지스터 값을 ```0x7C0```으로 초기화 해줄 것이다. (같은 이유)

단, CS 세그먼트 레지스터는 mov 명령어로는 불가능하므로 jmp 구문을 통해 해줘야 한다.

저자는 CS,DS 레지스터는 ```0x7C00```으로 초기화해주고, ES 레지스터는 화면 출력 레지스터로 사용하고자 비디오 메모리 주소로 초기화하였다.

<br>

```asm
[ORG 0x00]
[BITS 16]

SECTION .text

jmp 0x07C0:START

START:
    mov ax, 0x07C0
    mov ds, ax

    mov ax, 0xB800
    mov es, ax

mov byte [ es: 0x00 ], 'M'
mov byte [ es: 0x01 ], 0x4A

jmp $

times 510 - ($ - $$)    db  0x00

db 0x55
db 0xAA
```

<br>

우리는 es 레지스터를 화면 출력 관련 레지스로 사용할 것이므로 es 레지스터를 사용한다고 명시를 해준 것이다.

별다른 명시가 없다면 ds 레지스터를 사용하게 된다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 화면정리 및 부팅메시지 출력

<br>

가장 간단한 방법은 ```0xB8000```부터 ```80 * 25 * 2 = 4000 byte```를 모두 0으로 채우는 것이지만, 다음에 화면에 출력해줄 땐 속성값을 지정해줘야 하므로 문자값만 0으로 바꿔줘야 한다.

저자는 검은 배경에 강조된 녹색 글씨로 표현하고자 해서 ```0x0A (0000 1010)```로 설정해준다.

<br>

핵심은 화면정리를 해줄 때는 문자값만 0으로 초기화해주면 된다는 점과 속성값은 사용자 맘대로 원하는 값으로 해주면 된다.

또한 부팅메시지를 출력하고자 하면 화면정리를 한 상태에서 문자값에 내가 출력해줄 문자를 넣어주면 되는 것이다.

물론 일일이 해주기에는 힘드니까 먼저 C로 코드를 짠 다음, 어셈블리어로 된 object 파일로 변환하기 위해 컴파일을 해준 후 (```gcc -c a.c -o a.o -O2```) objdump로 체크하면 된다.

<br>

최종적인 화면정리 어셈블리 코드와 문자 출력 어셈블리 코드는 아래와 같이 나온다.

<br>

```asm
[ORG 0x00]
[BITS 16]

SECTION .text

jmp 0x07C0:START

START:
    mov ax, 0x07C0
    mov ds, ax

    mov ax, 0xB800
    mov es, ax

mov si, 0

.SCREENCLEARLOOP:
    mov byte [ es: si ], 0
    mov byte [ es: si + 1], 0x0A

    add si, 2
    cmp si, 80*25*2

    jl .SCREENCLEARLOOP

    mov si, 0
    mov di, 0

.MESSAGEPRINTLOOP:
    mov cl, byte [ si + MESSAGE1 ]
    cmp cl, 0
    je .MESSAGEEND

    mov byte [ es: di ], cl
    add si, 1
    add di, 2

    jmp .MESSAGEPRINTLOOP

.MESSAGEEND:
    jmp $

MESSAGE1:
    db 'MINT64 OS Boot Loader Start~!! Success!!', 0

times 510 - ( $ - $$ ) db 0x00

db 0x55
db 0xAA
```

<br>

![image](https://user-images.githubusercontent.com/52172169/192952578-d856caf3-ba90-4648-927a-4b11fe04c32b.png)


<br><br>
<hr style="border: 2px solid;">
<br><br>
