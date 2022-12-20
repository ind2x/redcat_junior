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
// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

// screen = 16bit word 32 column per raw, total 256 raw
@SCREEN
D=A
@base   // base = 16384(SCREEN)
M=D

(MAIN)
    @i
    M=0
    @KBD
    D=M
    // blacken if KBD != 0, else clear
    @BLACKEN
    D;JNE
    @CLEAR
    0;JMP

(BLACKEN)   // 16384 ~ 24575
    @8192
    D=A
    @i
    D=D-M
    @MAIN
    D;JEQ

    @base
    D=M
    @i
    A=D+M  // RAM[16384+i]
    M=-1
    @i
    M=M+1
    @BLACKEN
    0;JMP

(CLEAR)     // 16384 ~ 24575
    @8192
    D=A
    @i
    D=D-M
    @MAIN
    D;JEQ
    
    @base
    D=M
    @i
    A=D+M  // RAM[16384+i]
    M=0
    @i
    M=M+1
    @CLEAR
    0;JMP
```

<br>

코드가 중복되는 부분이 너무 많아서 고쳐보았다.. 

<br>

```asm
@SCREEN
D=A
@base   // base = 16384(SCREEN)
M=D

(MAIN)
    @i
    M=0
    @LOOP
    0;JMP

(BLACKEN)
    @addr
    A=M
    M=-1
    @LOOP2
    0;JMP

(LOOP)   // 16384 ~ 24575
    @8192
    D=A
    @i
    D=D-M
    @MAIN
    D;JEQ

    @base
    D=M
    @i
    D=D+M  // RAM[16384+i]
    A=D
    M=0

    @addr
    M=D
    @KBD
    D=M
    // blacken if KBD != 0, else clear
    @BLACKEN
    D;JNE

    @LOOP2
    0;JMP

(LOOP2)
    @i
    M=M+1
    @LOOP
    0;JMP
```

<br><br>
<hr style="border: 2px solid;">
<br><br>
