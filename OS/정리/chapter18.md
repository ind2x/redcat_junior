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



<br><br>
<hr style="border: 2px solid;">
<br><br>
