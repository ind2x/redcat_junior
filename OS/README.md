## INFO

<br>

저자의 mint64 OS github : https://github.com/kkamagui/mint64os-examples

MINT64 OS 홈페이지 : http://jsandroidapp.cafe24.com/xe/index

환경 : Ubuntu 20.04 / qemu 4.x

<br>
<hr style="border: 2px solid;">
<br>

## 변경사항

<br>

+ 5장 qemu 최신버전으로 인한 변경사항 http://jsandroidapp.cafe24.com/xe/development/12035

+ 10장 ImageMaker.py 프로그램 잘못짜면 무한루프 반복

+ 15장 Printf 함수 실행 중 xmm레지스터로 인한 에러 발생 -> makefile clang 컴파일 옵션으로 ```-msoft-float``` 추가

+ 26장 testfileio 명령어 에러 발생..인가 10번 중에 1번은 명령어가 정상으로 작동을 함 -> 애매함

+ 27장 ls 명령어 오류 발생.. ReadCluster 부분을 고쳐야 함, 과정은 아래와 같고 자세히는 27장 끝에다 기록해둠
 
  + pstDirectoryBuffer 다음 if문에서 pstDirectory == NULL 구문 ->  why 두 번 검사? -> pstDirectoryBuffer로 변경해줘야 함

    + ReadCluster if문에서 FALSE 임을 확인

    + 해당 함수로 가서 분석 결과 TRUE가 리턴되는데 ReadCluster 함수에서 나올 때 rax 값이 0이 되는 것을 확인함

  + 따라서 ReadCluster에 각각 return을 붙여주었더니 dir 명령어가 정상 작동이 되었음.


<br>
<hr style="border: 2px solid;">
<br>

## MINT64 OS Memory Map

<br>

![memory_map](https://user-images.githubusercontent.com/52172169/203483722-5504a36a-f0ad-4f19-a11b-bd9a587018fa.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/203499023-16f11474-a2cc-4e06-b2f4-01b37b107a70.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/203925329-16d0d4a1-721b-40db-8814-598871adb966.png)

<br>
<hr style="border: 2px solid;">
<br>
