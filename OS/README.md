## INFO

<br>

저자의 mint64 OS github : https://github.com/kkamagui/mint64os-examples

MINT64 OS 홈페이지 : http://jsandroidapp.cafe24.com/xe/index

환경 : win11 wsl2 Ubuntu 20.04 / qemu 4.x

내용은 1권까지만 정리하고 2권부터는 정리하기에는 너무 양도 많고 오래걸려서 안하기로 결정. (2권까지 완료)

<br>
<hr style="border: 2px solid;">
<br>

## 오류

<br>

+ 5장 qemu 최신버전으로 인한 변경사항 http://jsandroidapp.cafe24.com/xe/development/12035

+ 10장 ImageMaker.py 프로그램 잘못짜면 무한루프 반복

+ 15장 Printf 함수 실행 중 xmm레지스터로 인한 에러 발생 -> makefile clang 컴파일 옵션으로 ```-msoft-float``` 추가

+ 26장 testfileio 명령어 에러 발생..인가 10번 중에 1번은 명령어가 정상으로 작동을 함 -> 애매함

+ 27장 ls 명령어 오류, 파일 생성 오류 등등 많은 오류 발생.. 자세히는 27장 끝에 오류 부분에 기록해둠 (코드 수정해야 함)

  + 26, 27장 결론은 26장은 무시, 27장에서 RAM 디스크로 변경 후 실행하면 **아주 매우 정말 엄청 스무스하게 빠르게 신속하게 정확하게 문제없이** 잘됨

+ 35장 컴파일 문제 오류 발생
  + 배열에 값을 초기화 해줄 수 없음(memset reference 에러) -->  해당 값이 쓰일 곳에 직접 값을 입력
  + inline 함수에 대한 reference error 발생 --> extern으로 선언해서 해결 가능

<br>
<hr style="border: 2px solid;">
<br>

## MINT64 OS Memory Map

<br>

![memory_map](https://user-images.githubusercontent.com/52172169/203483722-5504a36a-f0ad-4f19-a11b-bd9a587018fa.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/203499023-16f11474-a2cc-4e06-b2f4-01b37b107a70.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/206893844-3f7b6276-145b-4174-9e69-106876640ef9.png)


<br>
<hr style="border: 2px solid;">
<br>
