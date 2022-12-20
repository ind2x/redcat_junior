## Project 4

<br>

![image](https://user-images.githubusercontent.com/52172169/208603151-72a08c21-51ba-406a-9179-7c6e0b706dbb.png)

<br>

Hack Computer가 CPU 에뮬레이터가 되고 어셈블리 명령어를 이용해서 **곱셈을 하는 프로그램**과 **컴퓨터와 상호작용 하는 프로그램**을 만든다.

내용 정리 : https://ccss17.netlify.app/computer/nand2tetris/#4-machine-language

<br>

+ Mul.asm

```asm
// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)
//
// This program only needs to handle arguments that satisfy
// R0 >= 0, R1 >= 0, and R0*R1 < 32768.

@i
M=0
@sum
M=0

(LOOP)      // i=0; i<R1; i++ { sum += x; }
    @i
    D=M
    @R1
    D=D-M   // i-y = 0
    @END
    D;JEQ
    @i
    M=M+1

    @R0
    D=M
    @sum
    M=M+D
    @LOOP
    0;JMP

(END)
    @sum
    D=M
    @R2
    M=D // R2 = sum
```

<br>

좀 더 효율적으로 하고 싶어서 삼항연산자처럼 더 큰 값을 더해주는 식으로 더 적게 반복되게끔 했는데 일정 시간안에 값을 출력되게 해야하는지 정상적인 값이 나오는데 첫 번째에서 PC가 20에 머물러서 실패가 뜬다. --> 강의자료에 ```비효율적이여도 목적에 맞게 짜면 OK```라고 써있다..

<br>

+ Fill.asm
  + 참고 : https://ccss17.netlify.app/computer/nand2tetris/#inputoutput-handling 

```asm

```

<br><br>
<hr style="border: 2px solid;">
<br><br>
