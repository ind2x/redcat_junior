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



<br><br>
<hr style="border: 2px solid;">
<br><br>
