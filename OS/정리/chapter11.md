# 키보드 디바이스 드리이버 추가
## 키보드 컨트롤러의 구조와 기능

<br>

![image](https://user-images.githubusercontent.com/52172169/196896404-7d4f1ff8-2e0c-48a0-81fe-cfe1a99b5ccd.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/196896598-873a84b5-770f-4dc4-9489-7b9a13e5caab.png)

<br>

레지스터의 크기는 모두 **1바이트**이며, 그 중 **상태 레지스터는 가장 중요한 레지스터**로, 키보드 컨트롤러에 값을 읽고 쓰려면 반드시 체크해야 하는 비트를 포함하고 있다.

아래가 그 비트들이다.

<br>

![image](https://user-images.githubusercontent.com/52172169/196896619-96beecdc-d269-4841-93da-7b563a86d37e.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/196896661-d6a483d2-90f4-4391-acd8-074aff22db3f.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 키보드 컨트롤러 제어
### 키보드와 키보드 컨트롤러 활성화

<br>

우선 기본적으로 BIOS에 의해 키보드가 활성화되므로 굳이 해주지 않아도 되는데, 혹시 모르니까 해준다.

키보드를 활성화시켜주려면 커맨드 포트 키보드 디바이스를 활성화한다는 의미의 0xAE 를 보낸다. (위의 키보드 컨트롤러 커맨드 확인)

그러나 이것이 키보드 컨트롤러가 활성화 된 것이지 **키보드가 활성화 된 것은 아니다.**

따라서 키보드 자체에도 활성화한다고 보내야 하는데, 입력 버퍼에 직접 키보드 커맨드를 보내주면 된다. (아래 사진 확인)

키보드는 컨트롤러와 달리 응답 값을 보내주는데, ACK(0xFA)를 보내주므로 이를 통해 확인하여 오지 않았다면 재시도를 하거나 중단해야 한다.

<br>

![image](https://user-images.githubusercontent.com/52172169/196907526-d8d1ff7d-59a4-4f51-9d7e-7c797878b0c2.png)

<br>

그래서 활성화 해주려면 키보드 컨트롤러에 0xAE를 보내주면 되는데, 문제는 컨트롤러는 CPU와 달리 처리속도가 상대적으로 느리다.

우리는 키보드에도 명령을 보내줘야 하므로 **상태 레지스터**를 통해 입력 버퍼와 출력 버퍼의 상태를 확인하여 값이 없으면 보내고 값이 있다면 읽어오도록 처리해줌으써 효율적으로 처리할 수 있다.

<br><br>

### 

<br>



<br><br>

### 

<br>



<br><br>

<br><br>
<hr style="border: 2px solid;">
<br><br>
