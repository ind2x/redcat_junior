# 라운드 로빈 스케줄러를 추가하자
## 스케줄러와 리스트

<br>

**스케줄러**는 **태스크의 순서를 정렬하여 실행 순서를 정해주는 역할**을 하는데, 정렬하는 가장 큰 이유는 **시스템 성능 향상이 목적**이다.

대표적으로 라운드 로빈 스케줄러가 있는데 라운드 로빈은 일정 시간마다 돌아가면서 공평하게 실행하는 방법이다.

멀티태스킹의 목적과 가장 부합한 알고리즘이지만, 단점은 태스크가 많아지면 전체적으로 성능이 저하된다.

그래서 **우선순위**를 부여한 라운드 로빈 알고리즘이 나왔고, 이 알고리즘을 이용한 기법에는 **멀티레벨 큐 알고리즘**과 **멀티레벨 피드백 큐 알고리즘**이 있다.

<br>

MINT64 OS에서는 우선순위를 3개의 레벨로 구분하여 부여할 것이며, 각 레벨별로 라운드 로빈을 수행하는 **멀티레벨 큐 스케줄러**를 이용할 것이다.

따라서 먼저 라운드 로빈을 구현해야 한다. 

<br>

라운드 로빈은 리스트라는 자료구조를 이용해서 구현해줄 것이다.

<br>

![image](https://user-images.githubusercontent.com/52172169/202891721-b3cf365e-9884-4eb5-a001-a3c501723558.png)

<br>

리스트에 데이터를 삽입/삭제 하는 과정은 다음과 같다.

<br>

![image](https://user-images.githubusercontent.com/52172169/202892001-f405dd85-b8a0-4be1-ae42-770b2c0b0f52.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/202892008-932dfb61-d565-4c5f-982c-d2399a4a62d8.png)

<br>

Code : https://github.com/kkamagui/mint64os-examples/blob/master/source_code/chap18/02.Kernel64/Source/List.c

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 태스크 풀과 스케줄러

<br>

태스크 풀은 TCB를 모아놓은 영역을 뜻한다.

즉, 태스크 풀에는 최대 개수의 TCB가 있어야 하며, 이 공간은 최대 8MB만큼의 메모리가 필요하다.

우리 MINT64 OS에서는 이 공간을 IST 영역 이후의 공간인 8MB 이후의 영역에 할당할 것이다.

<br>

이제 태스크 풀 영역을 관리할 자료구조를 설정해줘야 한다.

자료구조에는 태스크 풀의 시작 주소, TCB의 최대 개수, 사용한 TCB 개수를 저장해줘야 한다.

<br>

![image](https://user-images.githubusercontent.com/52172169/202892376-2440d419-752a-4ab4-b5d8-f7412ed35929.png)

<br>

태스크 풀은 초기화 함수, 할당 및 해제 함수로 구성된다.

먼저 초기화 하는 함수는 다음과 같다.

<br>

![image](https://user-images.githubusercontent.com/52172169/202892740-5be7e8c1-d7ef-4363-9446-c7ee7610dfeb.png)

<br>

TCB 최대 개수는 MINT 64 OS는 1024개까지 생성할 수 있으므로 1024개로 설정해준다.

태스크를 할당하려면 태스크 풀을 검색하여 해당 TCB가 할당된 상태인지, 해제된 상태인지 확인해야 한다.

이는 TCBPOOLMANAGER 자료구조에 있는 iAllocatedCount 필드와 TCB의 ID 필드를 이용해서 확인할 수 있다.

<br>

iAllocated 값은 항상 1 이상의 값을 가지며, 이 값을 TCB가 할당될 때마다 할당된 TCB의 ID의 상위 32비트 값과 OR 연산하여 저장하고, 빈 TCB를 검색할 때 ID의 상위 32비트의 값이 0인지 확인함으로써 태스크의 할당 여부를 확인한다.

Code : https://github.com/kkamagui/mint64os-examples/blob/master/source_code/chap18/02.Kernel64/Source/Task.c

<br>

이제 라운드 로빈 스케줄러를 구현해준다. 구현 시 필요한 정보를 자료구조로 정의해준다.

<br>

![image](https://user-images.githubusercontent.com/52172169/202893275-7280d5e0-a2a1-4c21-972c-684ad372681c.png)

<br>

스케줄러는 크게 초기화 함수, 현재 수행 중인 태스크와 태스크 리스트를 관리하는 함수, 태스크 전환 함수로 구성된다.





<br><br>
<hr style="border: 2px solid;">
<br><br>
