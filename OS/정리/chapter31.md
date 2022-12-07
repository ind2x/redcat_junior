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

<br>

![image](https://user-images.githubusercontent.com/52172169/206087919-77852352-6a37-4a6d-b6fa-6da64cd801f4.png)

<br>

위의 사진은 I/O APIC 구조이다.

메모리 맵 I/O 방식으로 특정 메모리 주소에 값을 읽고 쓰는 방식으로 제어, 부팅 후 기준 주소는 ```0xFEC0000```

<br>

![image](https://user-images.githubusercontent.com/52172169/206087994-e6f6b5e7-6d36-4835-8daf-ecefa6850864.png)

<br>

단, 특정 레지스터 2개에만 메모리 주소를 부여했으며, 2개의 레지스터 중 하나인 ````I/O 레지스터 선택 레지스터```로 나머지 숨긴 레지스터를 지정할 수 있도록 하였다.

숨긴 레지스터들은 인덱스가 부여되어있다. 아래가 나머지 레지스터들이다.

<br>

![image](https://user-images.githubusercontent.com/52172169/206088050-2779371b-8e53-48e4-9dbf-9bbc71f7b350.png)

<br>

자세한 내용들은 p.1399부터 읽기.

<br>

미리 말했듯이 중요한 부분은 목적지를 설정하는 리다이렉션 테이블 레지스터이다.

다른 레지스터들은 하드웨어적이나 BIOS에서 설정을 해줄 것이다.

<br>

![image](https://user-images.githubusercontent.com/52172169/206088912-6f7587ac-3fb9-47cf-8bf8-268cbd226440.png)

<br>

이 또한 p.1404, 1405를 읽으면 된다. 대체로 ICR 레지스터와 비슷하며 몇 가지만 보면 된다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 대칭모드 구현과 전환

<br>

구현 부분은 p.1406부터 

<br>

![image](https://user-images.githubusercontent.com/52172169/206126383-540c8060-c88a-417e-b830-49fcd07fae77.png)


<br><br>
<hr style="border: 2px solid;">
<br><br>
