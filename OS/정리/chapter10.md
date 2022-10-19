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

이제 9장에서 보았던 물리 메모리 확장 (CR4 레지스터의 PAE 비트 활성화) 및 페이지 테이블 설정 작업을 해주면 된다.

<br>

![image](https://user-images.githubusercontent.com/52172169/196450758-5ef73c1a-4327-41dd-b079-69a6ab7ffe76.png)

<br>

남은 작업은 IA32_EFER 레지스터를 설정해주면 된다.

우리는 LME 비트를 활성화해주어야 하는데, 이 비트가 비활성화 되어 있다면 32비트 모드로 동작하게 된다.

이 레지스터는 MSR이라는 특수 용도 레지스터로, 특수 명령어를 통해 값을 읽고 쓸 수 있다.

읽기는 RDMSR 명령어로 ECX 레지스터에 IA32_EFER 레지스터 주소를 넘겨주면 EDX:EAX 형식으로 IA32_EFER 레지스터의 값을 받아온다.

쓰기는 WRMSR 명령어로 ECX 레지스터에 IA32_EFER 레지스터 주소를 넘겨주면, EDX:EAX 형식으로 IA32_EFER 레지스터에 값을 쓴다.

우리가 쓸 IA32_EFER 레지스터는 0xC0000080에 있다.

<br>

![image](https://user-images.githubusercontent.com/52172169/196453347-07de1784-e15f-44a4-b805-33d2186641b8.png)

<br>

LME 비트를 활성화 해주면 된다.

<br>

![image](https://user-images.githubusercontent.com/52172169/196453529-16b10082-c68f-4ca2-831e-81af2142daa9.png)

<br>

마지막으로 CR0에 있는 캐시 비트를 활성화하여 캐시를 사용할 것이다.

9장에서 각 페이지 필드들의 공통된 속성으로 캐시를 사용해야하므로 캐시 비트를 활성화 시켜주는 코드를 작성하였다.

근데, 페이지 캐시보다 더 우선순위가 높은 캐시 비트가 있는데 바로 CR0의 NW와 CD비트이다.

각각 Not Write-Through와 Cache Disable라는 뜻으로 캐시를 사용하려면 두 비트 모드 0으로 설정해줘야 한다.

<br>

![image](https://user-images.githubusercontent.com/52172169/196596734-7f890bdd-cc7c-4b8a-9c48-6ef510d37632.png)

<br>

우리는 IA-32e 모드용 세그먼트 디스크립터 자료구조인 GDT를 0x08, 0x10 위치에 설정해줬으므로 거기로 점프를 해준다.

그 다음 사용할 세그먼트 레지스터들을 초기화해주는 과정이다.

<br>

![image](https://user-images.githubusercontent.com/52172169/196596874-d473f2f6-4f6e-4340-baf3-e147d6a27472.png)

<br>

이제 남은 작업은 IA-32e 모드 커널을 작성하고 OS 이미지에 통합시켜주면 된다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 

<br>



<br><br>
<hr style="border: 2px solid;">
<br><br>
