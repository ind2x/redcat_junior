# GDT, IDT 테이블, TSS 세그먼트를 추가해 인터럽트에 대비하자

<br>

인터럽트와 예외처리의 기본이 되는 **TSS 세그먼트와 IDT 테이블을 생성하는 방법**에 대해 알아보고 임시 핸들러를 통해 확인해 볼 것이다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 인터럽트와 예외

<br>

인터럽트(외부 디바이스가 발생시킴)나 예외(프로세서가 발생시킴)가 발생하면 벡터 테이블로 가서 처리함수를 통해 처리를 한다.

보호모드와 IA-32e 모드에서는 이것을 IDT 벡터 테이블이 담당한다.

<br>

**IDTR 레지스터를 이용**해서 **프로세서에 IDT 테이블의 정보를 설정**할 수 있다.

IDT 테이블에는 **IDT 게이트 디스크립터**가 있으며 **최대 256개의 엔트리(디스크립터)**를 포함할 수 있고,  **IA-32e 모드에서는 16바이트**이다.

<br>

![image](https://user-images.githubusercontent.com/52172169/198018371-45233159-2b9c-40a2-9450-35bc3ee7f84c.png)

<br>

아래는 디스크립터의 구조와 필드의 의미이다.

<br>

![image](https://user-images.githubusercontent.com/52172169/198018445-2df2be60-d5c4-4dd7-b8a3-b56a0a573c3a.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/198018479-82b0cc9e-583a-4e27-9dcc-f10031a2d591.png)

<br>







<br><br>
<hr style="border: 2px solid;">
<br><br>
