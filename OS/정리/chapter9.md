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

페이징에 사용하는 각 테이블은 512개의 엔트리로 구성된다. 

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




<br><br>

### 
---

<br>


<br><br>


<br><br>
<hr style="border: 2px solid;">
<br><br>
