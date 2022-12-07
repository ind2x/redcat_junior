# 대칭 I/O 모드로 전환해 인터럽트 분산 처리에 대비하자

<br>

멀티코어 환경에서 인터럽트가 여러 코어로 분산되는데 기반이 되는 **대칭 I/O 모드**에 대해 살펴본다.

또한, 멀티코어 환경에서 PIC 컨트롤러를 대체하는 I/O APIC의 구조를 살펴보고 로컬 APIC와 I/O APIC를 설정하여 대칭 I/O 모드로 전환할 것이다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## I/O APIC와 로컬 APIC, 대칭 I/O 모드

<br>

![image](https://user-images.githubusercontent.com/52172169/206085837-88b03fb9-6a66-415b-9518-a81421093824.png)

<br>

대칭 I/O 모드는 PIC 컨트롤러를 사용하지 않고 로컬 APIC와 I/O APIC를 통해 인터럽트를 여러 코어들로 분산하여 처리하는 방식을 뜻한다.

핵심은 I/O APIC이며, 여기에는 I/O 리다이렉션 테이블 레지스터가 있고 이 레지스터의 설정에 따라 인터럽트 전달 경로가 달라진다.

해당 레지스터로 여러 설정을 할 수 있는데 그 중 전달 경로를 설정할 수 있으며, 전달 경로에는 목적지 필드가 있고 이 필드의 값에 따라 특정 로컬 APIC나 전체 로컬 APIC로 인터럽트를 전달할 수 있다.



<br><br>
<hr style="border: 2px solid;">
<br><br>
