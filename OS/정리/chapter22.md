# 실수 연산 기능 추가

<br>

현재 시스템은 정수형만 다루고 double형 즉, 실수형은 다루지 않는다.

따라서 원주율 같이 소숫점 값이 필요한 값들을 출력하고자 한다면 예외 6번(Invalid Opcode ISR)가 발생한다.

우리는 이번 장에서 소수 연산 기능을 추가하여 testpie 콘솔 함수를 작성하여 테스트 해 볼 것이다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 실수 연산 장치와 프로세서

<br>

실수 연산에 사용하는 레지스터는 3개로 ```FPU 레지스터, XMM 레지스터, MMX 레지스터```가 있다.

<br>

![image](https://user-images.githubusercontent.com/52172169/203735466-b063ccc1-3a3b-4a60-99e3-7d6cbb0264ca.png)

<br>

FPU 레지스터는 초기에 사용되고 XMM, MMX 레지스터는 SIMD 기능이 추가된 레지스터라고 한다.

결론은 이 레지스터를 사용하려면 먼저 초기화해줘야 한다.

FPU 레지스터의 경우, 초기화 자체는 간단하지만 실수 연산 관련 레지스터는 태스크 **컨텍스트**와 관련이 있으므로 **멀티태스킹 시에 함께 저장하고 복원**해야 한다. 

<br>

FPU 레지스터를 초기화, 저장, 복원하는 명령어는 ```FINIT, FXSAVE, FXRSTOR```가 있다.

이 부분은 p.827를 통해 복습

<br>

```FXSAVE, FXRSTOR``` 명령어는 FPU, XMM, MMX 레지스터들을 512바이 크기의 메모리에 저장하고 복원한다.

이 때 **메모리 주소는 반드시 16바이트로 정렬**되어야 한다.

<br>

FPU 레지스터를 관리하는 가장 간단한 방법은 태스크 전환 시 FPU 레지스터를 저장하고 복원해주는 것이나, 512바이트의 크기를 매번 저장/복원한다는 점과 별로 사용하지 않는다는 점에서 비효율적이다.

가장 효과적인 방법은 **FPU 레지스터를 사용하는 시점에 초기화나 저장, 복원**을 해주면 되며 이는 예외 7번(Device not available)을 이용해서 처리해준다.

그에 대한 구체적인 방법은 p.830에서 참고한다.

요약하면 CR0의 TS비트(Task Switched bit)는 태스크 전환이 되었음을 알려주는 비트로, 태스크 전환 시 TS비트를 1로 설정하여 태스크 전환이 되었음을 알리고 만약 FPU 명령어가 실행되면 예외 7번을 발생시킨다.

<br>

이를 통해 FPU 사용 시점을 알 수 있다면 우리는 FPU를 마지막으로 사용한 태스크 ID를 알아두면 FPU 레지스터의 저장/복원을 할 수 있게 된다.

<br>

![image](https://user-images.githubusercontent.com/52172169/203815040-f202b77e-48f5-42f5-86be-cc7d812f9795.png)

<br>

위의 과정은 처음 FPU 레지스터 사용을 위한 초기화 단계를 나타낸다.

예를 들어, 태스크 1과 태스크 2가 FPU 레지스터를 사용한다고 가정한다. 우선 CR0 레지스터의 TS 비트를 1로 설정한다.

태스크 1이 FPU를 사용한다면 예외 7번이 발생하고 TS를 0으로 설정한 뒤 초기화를 진행한다.

그 다음 태스크 1이 실행 중에 태스크 전환이 발생하여 태스크 2로 전환되면, 다시 TS=1로 설정되고 태스크 2에서 초기화 명령어를 사용하면 TS=0으로 설정되는데 여기서 마지막으로 FPU를 사용한 태스크 1의 FPU 상태를 태스크 1의 컨텍스트에 저장시켜준 다음 초기화를 진행해주는 것이다.

초기화 이후에는 다음과 같다.

<br>

![image](https://user-images.githubusercontent.com/52172169/203815832-e877868f-d9fa-4d4d-ac11-07d944cbe810.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>

## FPU 처리를 위한 모듈 업데이트

<br>

태스크 별로 FPU 컨텍스트를 관리하려면 TCB 자료구조에 FPU 컨텍스트 공간을 추가해줘야 하며, FPU 사용유무에 대한 필드도 추가해야 한다.

자료구조를 설정하는 부분에서 FPU 컨텍스트 공간은 16바이트로 정렬되어야 하는 특징이 있어서 이 부분은 p.834부터 복습

<br>

지금까지 만든 FPU 처리 함수들은 모두 예외 7번(Device not available) 핸들러라 봐도 무방하다.

따라서 아예 핸들러 함수를 작성해 줄 것이다. 이 부분 또한 p.844부터 복습.

<br>

내용 자체는 어려운 부분이 없어서 그냥 복습할 때 책으로 복습해주면 될 것 같다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 빌드

<br>

testpie 함수는 355/113을 수행하여 결과 값을 구한 뒤, 태스크 100개를 생성해 이를 반복한다.

계산을 두 번 반복해 서로 다른 변수 2개에 저장하고 비교를 할 것이며, 연산 도중 태스크 전환이 발생하더라고 컨텍스트 저장/복원이 제대로 처리된다면 두 변수의 값이 같을 것이므로 두 변수의 값을 비교하는 것으로 실수 연산을 테스트한다.

<br>

![image](https://user-images.githubusercontent.com/52172169/203916450-f9ef38e0-3a07-4727-86a2-370fe5073892.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>
