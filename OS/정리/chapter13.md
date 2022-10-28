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

따라서 인터럽트, 예외가 발생하여 처리

<br><br>
<hr style="border: 2px solid;">
<br><br>
