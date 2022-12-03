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

첫 번째 오류는 무엇보다 Utility.c 코드에서 MemSet, MemCpy 등의 함수들이 변경되었음에도 책에서 언급이 없었음. 이 부분부터 코드를 찾아서 확인해가면서 고쳐줘야 함.

Mem 종류와 VSPrintf에서 case f 부분이 변경되었던 걸로 기억.

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

### 
---



<br><br>
<br><br>
<hr style="border: 2px solid;">
<br><br>
