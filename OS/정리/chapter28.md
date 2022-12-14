# 시리얼 포트 디바이스 드라이버를 추가해 외부와 연결하자

<br>

PC의 시리얼 포트 드라이버를 제어하는 디바이스 드리아버를 작성하고 이를 이용해 PC와 가상머신 간의 데이터 송수신을 테스트한다.

이를 확장하여 PC와 PC 간의 데이터 송수신도 살펴본다.

<br>

시리얼 포트는 데이터 라인을 통해 비트를 순차적으로 전달하며, PC와 외부장치 또는 다른 PC 사이의 데이터 송수신에 주로 이용된다.

시리얼 포트 또한 I/O 포트에 연결된 PC 주변 장치이므로 I/O 포트를 이용해 제어를 할 것이다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 시리얼 포트와 컨트롤러의 구조와 기능

<br>

![image](https://user-images.githubusercontent.com/52172169/205480245-50748233-5f93-4649-86ec-5278ffab17af.png)

<br>

시리얼 포트를 보면 데이터 송신 담당 TxD 핀과 수신 담당 RxD 핀이 있는데, 이를 통해 시리얼 포트는 전이중(양방향) 통신을 가능케한다는 것을 알 수 있다.

또한 송수신 여부를 제어할 수 있는 RTS, CTS 핀도 있다.

<br>

우리의 목적은 모뎀이 아닌 PC와 연결하여 데이터 통신을 하는 것이기 때문에, 9핀 모두가 필요하지 않고 3개의 핀만 사용하면 된다.

PC와의 데이터 통신을 위한 최소한의 구성으로는 TxD/RxD 핀과 신호의 기준점을 잡는데 필요한 GxD핀이 되겠다.

<br>

우리는 qemu와 PC사이 통신을 할 것이므로 굳이 시리얼 케이블이 필요하진 않고 이를 TCP/IP 네트워크 포트로 대체할 수 있다.

시리얼 포트의 자세한 내용은 이제 p.1223부터 읽어야 된다.

<br>

![image](https://user-images.githubusercontent.com/52172169/205480997-7dff52bc-b1af-4524-9b1d-154ec14a0bd0.png)

<br>

COM 포트 별로 12개의 레지스터가 개별적으로 있으며 IRQ 3, 4번을 공유해서 사용한다.

<br>

![image](https://user-images.githubusercontent.com/52172169/205481013-c562eb82-781d-4e61-94ae-e0fe4c013187.png)

<br>

우리가 사용할 레지스터에 대해서는 책으로 읽기

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 빌드 및 구현

<br>

![image](https://user-images.githubusercontent.com/52172169/205488587-0d3bc761-a7a8-4fb1-99f8-63b0b320dd37.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/205488596-8be29892-e8e4-41ba-b45e-fa2a66c48548.png)

<br>

이번 장은 무사히 아무 오류없이 끝냈다. 

내용 또한 어렵진 않고 책으로 복습해주면 될 것 같다.

<br><br>
<hr style="border: 2px solid;">
<br><br>
