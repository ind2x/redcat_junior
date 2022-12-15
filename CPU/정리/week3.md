# Memory
## Sequential Logic

<br>

디지털 로직은 크게 combinational logic과 sequential logic으로 구분한다.

기본적으로 모든 로직의 기본 구성은 AND, OR, NOT 게이트이다.

전자는 입력이 변경되었을 때, 출력 값이 저장되지 않고 바로 변경되는 회로를 뜻한다.

후자는 메모리 즉, RAM 같이 값이 저장되는 회로를 뜻하는데 여기에는 **플리플롭**이라는 회로가 있으며 이 회로에 값이 저장되는 것이다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## 1bit register

<br>

![image](https://user-images.githubusercontent.com/52172169/207806478-14745224-ee6a-4a96-a4da-f606619d9be7.png)

<br>

load 값에 따라서 load가 1이라면 값을 불러온다고 생각하면 되는데, 즉 load=1이면 값이 새로 들어오면 다음 타임(tick-tock)에서 값이 변경이 된다. (loading)

load=0이면 값이 변경되지 않고 계속 저장되어 있다. (storing)

<br>

예를 들어, 시뮬레이터에서 load=0으로 하고 in 값을 넣어주고 확인해보면 바뀌지 않는다. (storing)

load=1로 설정 후 해주면 out 값이 in의 값으로 변경된다. (loading)

리셋하고 바로 확인해보면 이전에 저장된 out 값이 나타나는 것을 확인할 수 있다. (storing)

<br>

![image](https://user-images.githubusercontent.com/52172169/207808483-27dce6bd-c586-41b3-8e59-c25f689ab103.png)

<br>

멀티 비트도 마찬가지다.

<br>

![image](https://user-images.githubusercontent.com/52172169/207808649-f29dd882-9984-4fc0-9a88-7307398d5065.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>

## RAM

<br>

![image](https://user-images.githubusercontent.com/52172169/207809349-ec6dad73-d294-4a5f-983e-04e8740e4bc8.png)

<br>

RAM의 개수에 따라 address 크기도 달라지며, n=8인 경우 address에는 0부터 7까지 들어갈 수 있다. (3비트)

마찬가지로 load를 1로 설정 시 RAM에 값이 변경되어 저장되고 load가 0인 경우 값이 유지된다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## Counter

<br>




<br><br>
<hr style="border: 2px solid;">
<br><br>
