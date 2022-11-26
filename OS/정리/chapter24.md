# 하드 디스크 디바이스 드라이버를 추가하자

<br>

이번 장에서는 하드 디스크의 구조, 하드 디스크 컨트롤러를 제어하여 디스크의 정보 추출 방법 및 디스크 섹터 읽기/쓰기 방법에 대해 살펴본다.

또한, 하드 디스크 인터럽트를 활성화하여 인터럽트 기반 하드 디스크 디바이스 드라이버를 구현한다.

수많은 저장매체 중 하드 디스크를 사용할 것이고, 이를 지원하기 위해선 하드 디스크 디바이스 드라이버를 제작해야한다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 하드 디스크와 하드 디스크 컨트롤러의 구조와 기능

<br>

하드 디스크 구조 : https://ind2x.github.io/posts/디스크관리/

<br>

소프트웨어 입장에서는 이 물리적인 하드디스크의 구조를 연속된 순서인 0번 섹터에서부터 x번 섹터로 표현하는 것이 접근하기에 훨씬 편리하다.

이렇게 해주려면 디스크 구성에 맞춰서 디스크 어드레스로 변환을 해줘야 하는데 이를 ```CHS(Cylinder, Head, Sector) 어드레스 방식``` 이라고 한다.

순서는 섹터 -> 헤드 -> 실린더 순으로 논리적인 번호를 증가시키며 번호를 줄 것이며 아래 표를 통해 확인.

<br>

![image](https://user-images.githubusercontent.com/52172169/204076314-d0c69727-ebd2-4490-97e6-e932866460e3.png)

<br>

섹터를 다 할당해줬다면 헤드를 증가시키고 다시 1번부터 부여, 헤드를 다 할당해줬다면 실린더를 증가시키고 헤드는 0, 섹터는 1부터 다시 시작한다.

하지만 현재는 하드 디스크 구성에 관계없이 논리적인 번호를 부여하는 LBA(Logical Block Addressing) 방식을 사용하여 우리도 LBA 방식을 이용한다.

<br>

요즘은 SATA 방식의 디바이스 드라이버를 사용하나 우리는 PATA 라는 방식을 사용하며 PATA 방식은 직접 프로세서의 I/O 포트를 통해 컨트롤러에 접근할 수 있으며 구현이 쉽다.

이 부분은 그냥 읽고 넘기면 될 듯. p.929

<br>

![image](https://user-images.githubusercontent.com/52172169/204077060-720d332f-639b-4376-86ca-4e7d30726053.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/204077066-795ccc82-d2a9-467d-bdc0-b04fb982628f.png)

![image](https://user-images.githubusercontent.com/52172169/204077082-5473f6dc-4d10-44a3-95ba-be51531d635e.png)

<br>

+ 커맨드 레지스터
  + 하드 디스크로 전송할 커맨드 저장하는 역할
  + 다른 모든 설정들을 모두 마무리 한 뒤 가장 마지막에 사용해야 
  + 1바이트 크기, 수 많은 커맨드 중 MINT64 OS는 3가지 커맨드 사용
  + 작업이 완료되면 인터럽트를 통해 알 수 있으며, 이 때 디지털 출력 레지스터를 이용

<br>

![image](https://user-images.githubusercontent.com/52172169/204077132-fbf709d7-abe2-41e7-b6e0-573049109624.png)

<br>

+ 데이터 레지스터
  + 커맨드 수행이 완료되었을 때, 하드 디스크로부터 값을 전송하거나 읽어올 때 사용
  + 2바이트 크기이므로 WORD 단위 레지스터 사용 (AX, BX 등등)

<br>n

나머지 레지스터는 읽어서 복습.


<br><br>
<hr style="border: 2px solid;">
<br><br>

## 드라이버 설계, 구현

<br>

p.936부터 읽으면서 복습


<br><br>
<hr style="border: 2px solid;">
<br><br>

## 빌드

<br>




<br><br>
<hr style="border: 2px solid;">
<br><br>
