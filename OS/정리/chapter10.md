# 64비트 모드로 전환하자

<br>

IA-32e 모드 커널을 작성하고 페이징 기능을 활성화시켜주어 64비트 모드로 전환할 것이다.

마무리 단계여서 할게 많다고 한다. 

<br>

보호모드에서 IA-32e 모드로 전환하기 위해서는 7단계를 거처야 한다.

<br>

![image](https://user-images.githubusercontent.com/52172169/196426550-990ce9ba-347b-4663-a3ce-05dcfb98947a.png)

<br>

그 전에 옛날 책이라서 이 때 당시에는 IA-32e 모드가 나온지 별로 안된 듯 하다.

그래서 프로세스가 이 모드를 지원하는지 검사하는 부분이 있다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 프로세서의 제조사와 IA-32e 지원 여부 검사

<br>

CPUID 명령어를 통해 검사할 수 있다고 한다.

CPUID를 통해 기본정보 또는 확장정보를 받을 수 있는데, EAX 값에 따라 달라지며, 결과 값은 범용 레지스터들로 받아온다.

<br>

![image](https://user-images.githubusercontent.com/52172169/196430776-f30c8b90-d6d0-42c9-8656-65241de646a6.png)

<br>

우리의 목적은 64비트 모드를 지원하는지 알아내는 것이므로 EDX의 비트 29 값을 확인해주면 되겠다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## IA-32e 모드용 세그먼트 디스크립터 추가

<br>

리얼모드에서 보호모드로 전환할 때도 필요했듯이, IA-32e 모드로 전환할 때도 IA-32e 모드 세그먼트 디스크립터가 필요하다.

보호 모드 세그먼트 디스크립터와 거의 비슷하므로 보호모드 커널 엔트리 포인터 코드에 추가를 해줄 것이고, 1MB 이상의 영역에 대한 64비트로 전환한 후에 생성해 줄 것이다.

<br>

![image](https://user-images.githubusercontent.com/52172169/196443488-782185ea-5f87-4e16-bbb1-c49676de465d.png)

<br>

보호모드와의 차이점은 L비트가 호환 모드 또는 64비트 모드를 선택하는데 이용된다. (챕터 6 참고)

디스크립터의 크기는 8바이트로 IA-32e 모드 디스크립터가 추가가 되어서 (코드, 데이터 총 2개) 16바이트를 추가하여 ```jmp dword 0x18```로 수정해준다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## IA-32e 모드 전환과 1차 정리

<br>



<br><br>
<hr style="border: 2px solid;">
<br><br>
