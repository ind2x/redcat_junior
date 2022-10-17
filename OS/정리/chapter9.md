# 페이징 기능을 활성화하여 64비트 전환을 준비하자

<br>

IA-32e 모드 커널을 위해 **64GB까지 매핑하는 페이지 테이블을 생성**하고, **생성된 페이지 테이블을 프로세서에 설정하여 페이징 기능을 활성화**해 줄 것이다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 선형 주소와 4단계 페이징 기법

<br>

![image](https://user-images.githubusercontent.com/52172169/196037348-7be342db-bc4a-41d6-9554-f142bde8ff74.png)

<br>

**4단계 페이징 기법은** 위의 사진의 3단계 페이징에서 **페이지 테이블이 없어지고, 페이지 디렉터리가 직접 페이지 시작 주소를 가리킨다.**

**페이징에 사용하는 각 테이블은 512개의 엔트리로 구성**된다. 

<br>

![image](https://user-images.githubusercontent.com/52172169/196037439-5e0d4258-2fa6-4642-8cc0-541396ddba4a.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/196037856-a99793b1-79e1-4972-8f6c-51d0999ac4a8.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/196037865-2281ef9c-9e35-48bc-871b-a6ee5da9913e.png)

<br>

페이지 엔트리도 위의 사진들을 보듯이 설정하기가 꽤 복잡하지만, 우리가 만들 MINT64 OS에서 필요한 페이징 기능들은 다음과 같다.

<br>

![image](https://user-images.githubusercontent.com/52172169/196037910-3b1df0d6-30f7-49f7-b7c9-9026b67ea153.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 페이지 테이블 구성과 공간 할당

<br>

페이징 기능을 사용하려면 메모리 영역의 정보를 담고 있는 **페이지 테이블**을 생성하여 프로세서에 설정해야 한다.

이번 절에서는 **64GB 메모리 공간을 관리하기 위해 필요한 페이지 테이블 크기**를 알아보고 **페이지 엔트리의 속성 필드를 설정**한다.

<br><br>

### 64GB의 물리 메모리 관리를 위한 메모리 계산
---

<br>

+ 페이지 디렉터리 테이블
  + 위의 엔트리 사진을 통해 디렉터리 테이블 엔트리는 8 바이트
  + 테이블 당 이런 엔트리가 512개가 있다고 위에서 설명
  + 각 엔트리는 2MB 페이지에 대한 정보를 담고 있다
  + 따라서 디렉터리 테이블 하나로 관리할 수 있는 메모리 영역은 2MB * 512 = 1GB
  + 디렉터리 테이블 하나가 차지하는 메모리 영역은 8B * 512 = 4KB
  + 총 64GB를 관리해야 하므로 필요한 디렉터리 테이블은 1GB * 64 = 64개이며, 필요한 메모리는 4KB * 64 = 256KB다.

<br>

+ 페이지 디렉터리 포인터 테이블
  + 엔트리 하나 당 8바이트이고 총 512개의 엔트리가 있다
  + 엔트리 하나 당 하위 페이지 디렉터리의 정보를 담고 있다
    + 우리는 총 64개의 페이지 디렉터리 테이블이 필요하므로 **총 64개의 디렉터리 포인터 테이블 엔트리가 필요**하다
  + 즉, 하나의 페이지 디렉터리 포인터 테이블로 관리할 수 있는 페이지 디렉터리는 512개이므로 우리는 1개의 페이지 디렉터리 포인터 테이블이 필요하다
  + 하나의 페이지 디렉터리 포인터 테이블이 차지하는 메모리 크기는 8B * 512 = 4KB 이다.

<br>

+ 페이지 맵 레벨 4 테이블 (PML4 테이블)
  + 엔트리가 8바이트이며 512개 있다
  + 엔트리 하나 당 하위 페이지 디렉터리 포인터 테이블을 관리하므로 우리는 총 1개의 엔트리만 필요하다
  + 따라서 PML4 테이블 1개로 관리할 수 있으며 차지하는 메모리 크기는 8B * 512 = 4KB이다. 

<br>

따라서 우리의 MINT64 OS에 필요한 테이블은 총 64 + 1 + 1 = 66개이며, 메모리 크기는 256 + 4 + 4 = 264KB이다.

MINT64 OS 이미지는 5KB 크기로, 이 페이지 테이블을 OS 이미지에 포함시킨다면 부팅 시간이 매우 지연될 것이다.

또한 페이지 테이블을 4KB로 정렬해야하는 점을 고려할 때, 메모리 정렬을 위한 불필요한 메모리 낭비가 될 수 있다.

그럼 어디다 할당해야 되는가?

<br><br>

### 페이지 테이블을 위한 공간 할당
---

<br>

IA-32e 커널 영역에 할당하는 것도 별로 좋진 않으므로 이전 챕터에서 보았듯 1MB ~ 2MB 영역을 자료구조 영역으로 활용한다.

따라서 1MB ~ 1MB+264KB 까지가 페이지 테이블 영역이며, 순서는 PML4 -> 페이지 디렉터리 포인터 -> 페이지 디렉터리 순이다.

<br>

![image](https://user-images.githubusercontent.com/52172169/196090913-67a2cbc2-0f98-4bcc-8b39-e4e2e7729c1e.png)


<br><br>

### 공통 속성 필드 설정
---

<br>

각각의 테이블에는 공통된 속성 필드가 있어서 이를 먼저 설정해준다.

속성 필드들은 위의 사진에서 참고

<br>

+ PCD, PWT
  + PCD(Page-Level Cache Disable)은 캐시를 사용할 것이므로 0으로 설정 (1은 비활성화)
  + PWT(Page-Level Write-Through)은 Write-Back 캐시 정책이 더 효율적이므로 0으로 설정

<br>

+ U/S, R/W
  + 지금 단계에서는 유저 레벨과 커널 레벨을 구별할 필요는 없으며, 코드와 데이터 영역도 읽기/쓰기 속성을 부여햘 이유가 없다.
  + 따라서 지금은 모든 페이지를 커널 레벨로 설정하고, 읽기/쓰기가 가능하게 해줄 것이다.
  + U/S(User/Supervisor)은 0으로 설정
  + R/W는 1로 설정

<br>

+ EXB, A, P, Avail
  + EXB(Execute-Disable)은 페이지 내에서 코드 실행을 막을 이유가 없으므로 0으로 설정
  + A(Accessed) 필드는 코드 실행 도중 특정 페이지에 대한 접근 여부를 참조하지 않으므로 0으로 설정
  + P(Present) 필드는 반드시 페이지가 유효함을 나타내줘야 하므로 1로 설정
  + Avail 필드는 필요 없으므로 0으로 설정

<br><br>

### 페이지 디렉터리 엔트리용 속성 설정
---

<br>

페이지 디렉터리 엔트리에는 추가로 PAT, G, D 필드를 설정해줘야 한다.

MINT64 OS에서는 기본 기능만 사용하여 별로 중요하진 않다.

모두 0으로 설정해주면 된다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 페이지 테이블 생성과 기능 활성화
### 페이지 엔트리 자료구조 정의 및 매크로 정의

<br>

![image](https://user-images.githubusercontent.com/52172169/196092925-7748aae4-4d0b-4236-979e-4e9c1c8f6902.png)

<br>

세 종류의 페이지 엔트리 속성은 모두 비슷하므로 하나의 자료구조로 정의하여 사용한다.

보면 알겠지만 8바이트 크기의 자료구조를 4바이트씩 2개로 쪼개주었다.

왜냐하면 이 코드가 실행될때는 아직까지는 보호모드이므로 보호모드에서는 최대 32비트 레지스터(2^32 = 4GB)까지만 사용 가능하다.

<br>

이제 각 속성의 필드 값을 정의해주는데 매크로를 이용해서 정의해준다.

```c
typedef struct kPageTableEntryStruct
{
    // PML4T와 PDPTE의 경우
    // 1 비트 P, RW, US, PWT, PCD, A, 3 비트 Reserved, 3 비트 Avail, 
    // 20 비트 Base Address
    // PDE의 경우
    // 1 비트 P, RW, US, PWT, PCD, A, D, 1, G, 3 비트 Avail, 1 비트 PAT, 8 비트 Avail, 
    // 11 비트 Base Address
    DWORD dwAttributeAndLowerBaseAddress;
    // 8 비트 Upper BaseAddress, 12 비트 Reserved, 11 비트 Avail, 1 비트 EXB 
    DWORD dwUpperBaseAddressAndEXB;
} PML4TENTRY, PDPTENTRY, PDENTRY, PTENTRY;
```

<br><br>

### 페이지 엔트리 및 페이지 테이블 생성
---

<br>

먼저 간단한 PML4 테이블을 만들어준다.

이 부분은 페이지 260을 참고.

<br><br>

### 프로세서의 페이징 기능 활성
---

<br>

CR0 레지스터의 PG비트 CR3 레지스터, CR4 레지스터의 PAE 비트만 1로 설정해주면 된다.

PG비트를 1로 설정하면 그 즉시 프로세서의 페이징 기능이 활성화되므로 그 전에 다른 값들을 먼저 설정해줘야 한다.

자세한 내용은 페이지 262 참고.

<br>

![image](https://user-images.githubusercontent.com/52172169/196099887-5a40ae2c-f57f-44b5-bfae-505d286832f2.png)

<br>

기능을 활성화하진 않고 다음 장에서 IA-32e 모드용 디스크립터를 생성하고 나서 활성화해줄 것이다.

<br>

![image](https://user-images.githubusercontent.com/52172169/196127150-cdf93d0d-8f7d-468f-b048-d5838091db2f.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 보호 모드 커널에 페이지 테이블 생성 기능 추가

<br>

C 보호모드 커널 엔트리 포인터에서 위의 기능들을 C코드로 만들어서 호출하게끔 해주어서 커널의 페이지 테이블 생성 기능을 추가할 것이다.

<br>

![image](https://user-images.githubusercontent.com/52172169/196137642-d07a2ac7-6ef2-41dd-8bba-94a6572b570e.png)

<br>

chapter9 코드 : https://github.com/kkamagui/mint64os-examples/tree/master/source_code/chap09

<br><br>
<hr style="border: 2px solid;">
<br><br>
