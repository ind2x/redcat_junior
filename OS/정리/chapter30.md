# 잠자는 코어를 깨우자

<br>

AP를 활성화 하기 위해서 **로컬 APIC에 대해 자세히 알아보고, 로컬 APIC의 ICR 레지스터를 통해 AP를 활성화** 한다.

그리고 **BSP와 AP 별로 자료구조를 생성하여 멀티코어를 처리할 수 있는 기능**을 만들어 줄 것이다.

<br>

29장에서는 싱글코어와 멀티코어 차이점, 로컬 APIC, I/O APIC 같은 시스템 정보를 저장하는 MP 설정 테이블 등을 배웠다.

30장에서는 이 정보를 이용해 AP를 활성화 하는 방법을 알아본다.

단, 활성화만 가능하고 아직 사용은 불가하며, IA-32e 모드로 진입하는 것까지만 해 줄 것이다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 로컬 APIC와 코어 활성화

<br>

로컬 APIC에서 코어를 활성화하는 메시지를 전달하기 때문에 이것부터 보고간다.

<br>

![image](https://user-images.githubusercontent.com/52172169/205827974-18dcfb98-b70d-4b83-98e0-7492b3c25a09.png)

<br>

로컬 APIC의 구조로, 내부와 외부 인터럽트는 특정 인터럽트 3개를 제외하면 벡터 테이블에 할당된 인터럽트 벡터와 프로세서의 우선순위를 비교하여 CPU로 전달한다.

로컬 APIC는 메모리 I/O 형식으로 되어 있어서 특정 범위의 주소 내에 있는 레지스터들의 주소에 값을 읽고 쓰는 방식으로 제어한다.

기준 주소는 프로세서나 코어가 리셋되면 0xFEE00000로 설정되며, 0xFEE00000 ~ 0xFEE003F0 범위 내에 수십 개의 레지스터들이 있다.

자세한 건 p.1337에서 확인

<br>

본론으로 가면 **AP를 활성화하려면 가장 먼저 로컬 APIC를 활성화**해야 한다.

로컬 APIC를 활성화하려면 두 가지 레지스터에 접근해야 한다.

하나는 IA32_APIC_BASE_MSR 레지스터이며, 다른 하나는 Spurious Interrupt Vector Register (의사 인터럽트 벡터 레지스터)이다.

IA32_APIC_BASE_MSR 레지스터는 MSR로 프로세서 모델 별로 정의된 **특수 목적 레지스터**로, **APIC 레지스터의 기준 주소, 모든 APIC 활성화 여부, BSP 여부를 담당**한다.

<br>

![image](https://user-images.githubusercontent.com/52172169/205834328-988874ef-6874-41d3-b3b1-35cfdda8763b.png)

<br>

표를 보면 활성화 필드가 있다. 여기를 1로 설정해줘야 한다.

자세한 내용은 p.1339에서 확인.

<br>

로컬 APIC를 활성화 했다면 그 다음 의사 인터럽트 벡터 레지스터 차례다.

여기에는 **포커스 프로세서 검사 기능 사용 여부, 로컬 APIC 활성화 여부, 의사 벡터**를 저장한다.

<br>

![image](https://user-images.githubusercontent.com/52172169/205839542-612329e6-7990-46d1-8685-fd4b54cc433b.png)

<br>

의사 인터럽트란 하드웨어와 소프트웨어 간의 시간 차로 발생하는 인터럽트이다.

자세한 내용은 p.1342에서 확인.

MINT64 OS에서는 이 레지스터를 로컬 APIC를 임시 활성/비활성 하는데 사용한다.

IA32_APIC_BASE_MSR 레지스터는 모든 로컬 APIC를 활성화 한다면, 의사 인터럽트 벡터 레지스터는 해당 코어의 로컬 APIC에만 적용된다.

따라서 IA32_APIC_BASE_MSR 레지스터로 먼저 활성화 한 뒤, BSP와 AP를 의사 레지스터로 활성화 시켜준다.

<br>

이제 IPI 메시지를 이용해 코어를 깨우기만 하면 되며, 절차는 다음과 같다.

<br>

![image](https://user-images.githubusercontent.com/52172169/205842785-4cc6a022-5da9-4df5-92dc-19f0977e75d5.png)

<br>

IPI(InterProcessor Interrupt)는 프로세서 간 인터럽트이다. (p.1345)

로컬 APIC의 상위, 하위 커맨드 레지스터 (ICR)로 IPI를 생성할 수 있다.

<br>

![image](https://user-images.githubusercontent.com/52172169/205843107-a0c5a844-9be1-42a0-b91f-9961522a2d3c.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/205843160-157d7284-f703-41f5-bd5a-e5a0872d398f.png)

<br>

내용이 방대하고 굳이 외울 필요는 없을 것 같아서 책으로 복습 (p.1345)

<br>

![image](https://user-images.githubusercontent.com/52172169/205843759-60dee1b7-ce47-479c-b4bb-469c4cd17c37.png)

<br>

필드에 전달 모드가 있는데, 우리가 전달할 수 있는 인터럽트는 위의 표와 같다.

여기서 볼 부분은 초기화 IPI, 시작 IPI 부분만 보면 된다.

책으로 계속 복습해주면 될 듯하다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 멀티코어 프로세스용 OS로 업그레이드

<br>

AP를 활성화하면 **보호모드 커널부터 실행**이 될텐데, **IA-32e 모드로 전환하는 과정만 수행**하면 된다.

하지만 **오류**가 나는데, 이유는 **IA-32e 모드 커널 영역 초기화하는 등 BSP용과 AP용 코드가 구분되어 있지 않기 때문**이다.

그 외에도 TSS, IST 또한 구분해줘야 한다. 

이번 절에서는 AP를 IA-32e 모드로 진입하기 위해서 수정해야 될 부분을 살펴본다.

<br>

그러면 BSP인지 AP인지 구분해줘야 하며, 방법은 BSP가 가장 먼저 실행되므로 메모리의 한 영역에 BSP임을 기록해두면 된다.

추후 BSP가 IA-32e 모드로 전환되면 이 값을 AP로 바꾼 뒤, BSP가 AP를 깨우면 이 값은 AP로 되있으므로 구분이 가능하다.

또 다른 방법으로는 IA32_APIC_BASE_MSR 레지스터 값을 확인하는 방법도 있다.

<br>

더 중요한 것은 개별 코어를 구분하는 것인데, 이는 로컬 APIC ID로 정확하게 구분할 수 있다.

그 뒤는 p.1355부터 정독.

<br><br>
<hr style="border: 2px solid;">
<br><br>
