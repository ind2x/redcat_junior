# Boolean Arithmetic and the ALU
## ALU

<br>

이진수의 음수는 2의 보수로, 2의 보수는 ```1의 보수 + 1``` 형태이다.

1의 보수는 NOT 연산을 한 것과 같으며, 다르게 말하면 예를 들어, 4비트라면 1111에서 값을 빼면 1의 보수가 된다.

여기에 1을 더해 준 뒤, 4비트가 넘어간다면 맨 앞의 비트를 버려주면 된다.

예를 들어, 7을 음수로 나타낸다면 ```0111 -> 1의 보수 = 1000 -> 1001```

<br>

ALU는 Arithmetic Logical Unit으로, CPU 내에 있는 연산 장치다.

<br>

![image](https://user-images.githubusercontent.com/52172169/207499403-165eeac4-8672-4798-86b3-540991749a14.png)

<br>

ALU에는 16비트의 입력 값 2개와 컨트롤 비트 6개가 인풋으로 들어오며, 컨트롤 비트의 값에 따라 16비트의 output 값이 달라진다.

또한 다음 장에서 배우는데, zr, ng 값도 아웃풋으로 나온다.

<br>

![image](https://user-images.githubusercontent.com/52172169/207511282-106108ef-5a85-40e3-9191-652029a0c25f.png)

<br>

이번 프로젝트2에서는 1에서 만든 로직 게이트 칩들을 이용해서 연산 칩을 만들 것이다.

<br>

![image](https://user-images.githubusercontent.com/52172169/207511987-5ccd3643-1f32-4e82-a7fe-1c88edddb56c.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>

## projects

<br>

+ HalfAdder

![image](https://user-images.githubusercontent.com/52172169/207512679-03ddc94e-f96a-42ed-963b-29d91bd94040.png)

<br>

+ FullAdder

![image](https://user-images.githubusercontent.com/52172169/207512847-7fe0dc91-92e9-435b-87c5-3b26bf8ceb38.png)

<br>

+ Add16

![image](https://user-images.githubusercontent.com/52172169/207512870-86689a21-b9e9-4de5-b0d1-ee7b6c7016df.png)

<br>

+ inc16
  + 너무 간단해서 패스. 플러스 일

<br>

+ ALU

![image](https://user-images.githubusercontent.com/52172169/207513684-137eb938-8243-41e9-a12a-884f98523f01.png)

<br><br>
<hr style="border: 2px solid;">
<br><br>
