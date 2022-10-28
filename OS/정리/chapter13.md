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

## 
