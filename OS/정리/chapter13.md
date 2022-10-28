# PIC 컨트롤러와 인터럽트 핸들러를 이용해 인터럽트를 처리하자
## PIC 컨트롤러

<br>

PIC 컨트롤러란 인터럽트 관리를 해주는 컨트롤러로, 1개당 8개의 인터럽트를 처리할 수 있으며 PC에는 2개가 연결되어 있다.

0번부터 7번까지 있으며, 2번은 다른 PIC와 연결되는데 사용되어 총 15개의 인터럽트를 처리할 수 있다.

<br>

![image](https://user-images.githubusercontent.com/52172169/198539470-ee639f55-31d4-4cfc-bfe7-a3421a5da2f2.png)

<br>

PIC 컨트롤러에는 IRR, ISR, IMR 레지스터가 있는데, 레지스터의 각 비트는 8개의 핀에 대한 정보를 나타낸다.

<br>

+ IRR(Interrupt Request Register) 레지스터
  + 인터럽트가 발생된 핀의 정보를 관리하며, 해당 비트를 1로 설정하여 인터럽트 발생 여부를 표시한다.

<br>

+ ISR(In-Service Register) 레지스터
  + 현재 인터럽트 핸들러가 수행중인 인터럽트의 정보를 나타낸다.
  + 특별한 설정이 없다면 IRQ 0에 가까울수록 우선순위가 높으며 

<br>

+ IMR(Interrupt Mask Register) 레지스터
  + 비트가 1로 설정된 인터럽트 핀에서 발생한 인터럽트는 무시한다. 

<br>

또한 INT핀과 /INTA 핀이 있는데, **INT핀은 프로세서에게 인터럽트가 발생했음을 알리는 역할**을 하며 **/INTA핀은 프로세서에 잘 전달되었는지를 PIC 컨트롤러에 알려주는 역할**을 한다.

<br>

![image](https://user-images.githubusercontent.com/52172169/198542812-644bb213-2507-44c0-86ff-17511ab9c24c.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>

## PIC 컨트롤러 제어

<br>

PIC 컨트롤러를 초기화 해 줄 것이다.

우선 PIC 컨트롤러에는 2개의 I/O 포트가 있는데, 마스터 PIC 컨트롤러는 ```0x20, 0x21```이며 슬레이브 PIC 컨트롤러는 ```0xA0, 0xA1```이다.

두 포트 모두 ```읽기/쓰기```가 가능하다.

<br>

![image](https://user-images.githubusercontent.com/52172169/198546108-e4e35b41-1752-4569-8f45-155e7856859d.png)

<br>

커맨드 또한 2개의 타입이 제공되며, 초기화를 하는 ICW 커맨드와 제어와 관련된 OCW가 있다.

여기서는 ICW를 보겠다.

<br>

ICW 커맨드는 1부터 4까지 있으며 자세한건 p.416에서 확인한다.

<br>

![image](https://user-images.githubusercontent.com/52172169/198549006-e8d74653-c68b-4229-ac1f-6abdfb29ad9f.png)

<br>

초기화는 ICW1을 보내주는 것으로부터 시작한다.

이 부분은 책을 통해 확인해야 겠다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 인터럽트, 예외 핸들러, 컨텍스트

<br>

프로세서는 레지스터를 기반으로 코드를 수행하는데, **컨텍스트(Context)**는 **프로세서의 상태와 관련된 레지스터의 집합**이다.

따라서 인터럽트, 예외가 발생하여 처리할 때 **이전의 컨택스트를 저장**해둬야 처리가 완료된 후 복원을 할 수 있다.

컨택스트를 위한 메모리 공간을 할당한 후, 정해진 순서대로 레지스터를 저장하고 복원해주면 된다.

<br>

![image](https://user-images.githubusercontent.com/52172169/198580248-24e080c4-f17a-45a5-a48a-a2c6ae2985fd.png)

<br>

핸들러는 컨텍스트를 저장하는 역할을 하는 어셈블리 코드와 실제 처리를 담당하는 C코드로 작성해준다.

<br>

PIC 컨트롤러가 아무리 프로세서에게 인터럽트를 전달해도 받지 못한다면 소용이 없다.

프로세서의 RFLAGS 레지스터에는 IF 비트가 있는데, 이를 통해 인터럽트를 **활성화/비활성화** 할 수 있다.

이를 직접 관리해줘야 하며, 관리하는 명령어가 있는데 바로 ```STI/CLI``` 명령어로 각각 활성화/비활성화 명령어다.

단, RFLAGS 레지스터의 상태를 확인하는 명령어는 없으나 RFLAGS 레지스터를 스택에 저장하는 명령어인 PUSHF 명령어를 이용해서 확인해 줄 수 있다.

<br>

![image](https://user-images.githubusercontent.com/52172169/198594695-0c787609-5243-4ec9-9cf6-b9387c0d7030.png)

<br>

이제 위의 내용들을 작성하여 통합시켜준다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 코드 통합 및 빌드

<br>





<br><br>
<hr style="border: 2px solid;">
<br><br>
