# 캐시와 램 디스크를 추가해 속도를 높이자
## 파일 시스템 캐시 설계와 구현

<br>

캐시는 매번 메모리에 접근하여 데이터를 읽고 하는 것은 너무 느리기 때문에 더 빠른 속도를 위해 **작은 버퍼**를 만들어 거기에 **자주 사용하는** 데이터를 저장하고 사용하는 공간이다.

즉, 효율적으로 사용하려면 **자주 사용하는 데이터**여야 하며, 단점으로는 공간이 작으며 대상의 정보가 변경되었을 때 캐시의 값도 같이 변경해줘야 하는 동기화 문제가 있다.

<br>

![image](https://user-images.githubusercontent.com/52172169/204444288-ae63ee46-897a-410a-b3e6-fabf7e42a588.png)

<br>

이런 식으로 하드 디스크와 MINT 파일 시스템 사이에 캐시를 만들 것이고, 캐시에는 자주 사용하는, 중요한 데이터들이 저장이 될텐데 이 데이터들은 시간에 따라 중요도가 변경이 될 수 있으며 새로운 캐시를 추가하거나 캐시가 가득찼을 때 비우는 등 기능들이 있다.

이 때, 중요한 부분인 캐시를 비우는 부분에서 사용되는 알고리즘으로 LRU(Least Recently Used) 알고리즘이 있다. (다른 os 책에서 배운 내용)

<br>

![image](https://user-images.githubusercontent.com/52172169/204444768-105f589e-28d4-43ef-89ac-ed7f2fc74f62.png)

<br>

이런 식으로 변경해주고 데이터에 접근 시 접근 시간을 업데이트 해준다.

자료구조 설계 및 정의하는 부분은 p.1142부터 읽는다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 램 디스크 설계와 구현

<br>

램은 HDD와 달리 휘발성 저장매체로, 전원이 꺼지면 데이터가 날라간다.

하지만 처리 속도가 빠르다는 장점이 있고, 디스크가 없는 PC에서도 데이터를 저장할 수 있다.

그래서 SATA 형식의 PC에서는 부팅했을 때 하드디스크 인식이 안되는데, 이 때 램 디스크를 생성한다면 파일 시스템을 사용할 수 있다.

<br>

![image](https://user-images.githubusercontent.com/52172169/204462913-ba85c433-7414-4216-86d5-00273bf1db08.png)

<br>

p.1172부터 읽으면 된다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 오류 해결
### ls(dir) 명령어 오류
---

<br>

ls(dir) 명령어 오류 해결 과정은 다음과 같다.

<br>

![image](https://user-images.githubusercontent.com/52172169/205440511-a8084033-9cf8-43e1-a79c-3716ba2e9bda.png)

<br>

왼쪽이 수정된 것이고 오른쪽이 기존 코드인데, 수정된 곳은 다음과 같다.

<br>

```c
pstDirectoryBuffer = (DIRECTORYENTRY *)AllocateMemory(FILESYSTEM_CLUSTERSIZE);
    
if (pstDirectory == NULL) 를 ---> if(pstDirectoryBuffer == NULL)로 변
```

<br>

그 다음 실질적인 문제점인 Readcluster가 TRUE임에도 rax 값이 0으로 변경되어 FALSE가 되는 부분은 다음과 같이 수정해준다.

<br>

![image](https://user-images.githubusercontent.com/52172169/205440669-8b00743a-14a1-46b3-83dd-0f672e777554.png)

<br>

해당 코드를 수정해주면 ls(dir) 명령어가 정상적으로 작동한다.

다른 코드들도 마찬가지로 Write나 WriteClusterLink나 등등 그런 것들도 수정해주자..

<br><br>


### file create 오류
---

위의 ls 명령어를 해결해주고 추가로 나머지 return이 안붙은 write 쪽이나 등등에도 return을 붙여줘야 함. 

안붙여주면 어차피 붙여줘야 다음으로 넘어갈 수 있기 때문에.

<br>

붙여줬다면 이제 createfile을 해주면 에러가 나는데 우선 Consoleshell.c의 CreateFileInRootDirectory 함수에 break를 걸어준다.

걸면 FileSystem.c의 OpenFile 함수로 넘어가는데 w모드로 넘어가므로 w 모드를 처리해주는 코드를 보면 가장 마지막 부분 if문이 있다.

<br>

![image](https://user-images.githubusercontent.com/52172169/205443015-6b0bce1b-8b17-4b3a-aaeb-ca81fe1bd5ae.png)

<br>

저 if문에서 FALSE가 되어 null이 리턴되어 파일 생성에 실패한다. 해당 함수로 들어가서 문제점을 살펴본다.

해당 함수로 가보면 마지막 if문인 kWriteCluster 함수에서 FALSE가 되어 FALSE가 리턴된다.

다시 이 함수로 들어간다.

<br>

확인해보니.. ls 명령어 해결할 때 같이 해결할 수 있는 부분인데, return을 꼼꼼히 붙여줘야 한다..

안붙여준 부분이 있어서(WriteCluster) 붙여준 뒤, formathdd와 mounthdd를 해준 다음 확인해줘야 한다. (안해주면 에러 남)

<br><br>

### RAM디스크에서 hddinfo 에러
---

ConsoleShell.c 코드에서 ShowHDDInformation 부분에서 GetHDDInformation 부분 코드를 바꿔주지 않아서 에러가 발생했다.

저자 코드처럼 바꿔주면 정상 작동한다. 

단, 너무 저자 코드를 신뢰해선 안될 것 같고 추후 1권이 끝난 다음에 코드를 분석해가면서 이해도 할겸 다 뜯어고쳐봐야겠다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 빌드 실행

<br>

일단 RAM 오류를 뺀 2개의 오류를 해결한 다음 26장과 마찬가지로 한번에 명령어가 딱 되진 않았지만 어찌됬든 실행은 된다.

<br>

![image](https://user-images.githubusercontent.com/52172169/205445704-c0ad5768-7005-4f97-b112-03b83f8f3790.png)

<br>

이제 RAM으로 실행해보면 아래처럼 에러가 난다.

처음엔 RAM이 아예 안되는 줄 알았는데 시간이 좀 걸린 다음에서야 활성화가 되었고, 다른 에러가 발생했다.

<br>

![image](https://user-images.githubusercontent.com/52172169/205445801-7c2e8bdb-7785-48b5-8fa5-ee1880e12a9e.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/205447055-e8065950-4396-43e5-85d5-70a23232fe6c.png)

<br>

testfileio 명령어도 잘 된다. 그냥 하드디스크가 ㅈ같은거였다.

<br>

![image](https://user-images.githubusercontent.com/52172169/205447079-3ce9fc9d-1b3c-4fdf-8348-b9402d7c1e0a.png)

