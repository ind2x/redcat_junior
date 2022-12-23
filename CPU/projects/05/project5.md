## Project 5

<br>

5장에서는 Hack Computer를 만들어야하고 여태까지 만든 칩들을 이용해서 만든다.

<br>

![image](https://user-images.githubusercontent.com/52172169/209097501-3324681c-4219-411d-b162-895862d3574c.png)

<br>

Hack Computer는 Memory, CPU, ROM(instruction memory)로 구성된다.

만드는 순서는 Memory -> CPU -> ROM -> Computer 순으로 만드면 좋다고 한다.

<br>

+ Memory

![image](https://user-images.githubusercontent.com/52172169/209097548-1e282910-8826-4db9-81e1-262fdc9909f2.png)

```c
// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/05/Memory.hdl

/**
 * The complete address space of the Hack computer's memory,
 * including RAM and memory-mapped I/O. 
 * The chip facilitates read and write operations, as follows:
 *     Read:  out(t) = Memory[address(t)](t)
 *     Write: if load(t-1) then Memory[address(t-1)](t) = in(t-1)
 * In words: the chip always outputs the value stored at the memory 
 * location specified by address. If load==1, the in value is loaded 
 * into the memory location specified by address. This value becomes 
 * available through the out output from the next time step onward.
 * Address space rules:
 * Only the upper 16K+8K+1 words of the Memory chip are used. 
 * Access to address>0x6000 is invalid. Access to any address in 
 * the range 0x4000-0x5FFF results in accessing the screen memory 
 * map. Access to address 0x6000 results in accessing the keyboard 
 * memory map. The behavior in these addresses is described in the 
 * Screen and Keyboard chip specifications given in the book.
 */

CHIP Memory {
    IN in[16], load, address[15];
    OUT out[16];

    PARTS:
    // select address to load
    DMux(in=true, sel=address[14], a=loadRAM, b=loadSCREEN);
    // if address is in RAM range, read or write in RAM, else SCREEN 
    RAM16K(in=in, load=loadRAM, address=address[0..13], out=DataOut);
    Screen(in=in, load=loadSCREEN, address=address[0..12], out=ScreenOut);
    Keyboard(out=KeyboardOut);
    // return out from selected address via address[13,14]
    // if address[13,14] == 00 or 01, return RAM output
    // else if 10, return SCREEN output
    // else 11, return KEYBOARD output
    Mux4Way16(a=DataOut, b=DataOut, c=ScreenOut, d=KeyboardOut, sel=address[13..14], out=out);
}
```

<br>

+ CPU

![image](https://user-images.githubusercontent.com/52172169/209097696-4b14ae25-dd39-4cea-bd1d-700978b0f266.png)

<br>

![image](https://user-images.githubusercontent.com/52172169/209253373-7416d5c8-a54b-4800-8795-f9c9923a3814.png)

<br>

점프 로직에 대해 살펴봐야 한다.

점프 비트가 있고, 점프 비트와 ALU의 output 중 ng, zr 비트를 통해 점프 로직을 이용해 판단하여 조건이 맞으면 점프를 할 수 있게 설정해줘야 한다.

우선 점프문에 따른 조건을 정리해보면 다음과 같다.

<br>

| jump | j-bit |       condition       |
|:----:|:-----:|:---------------------:|
| null |  000  |        no jump        |
|  JGT |  001  |       zr=0, ng=0      |
|  JEQ |  010  |       zr=1, ng=0      |
|  JGE |  011  | (zr=0, ng=0) OR (zr=1, ng=0) |
|  JLT |  100  |       zr=0, ng=1      |
|  JNE |  101  | (zr=0, ng=0) OR (zr=0, ng=1) |
|  JLE |  110  | (zr=0, ng=1) OR (zr=1, ng=0) |
|  JMP |  111  |          jump         |

<br>

가만히 보다보면 특징이 보인다. 이 특징을 이용해서 로직을 설계해야 한다..

+ ```zr=1```이 되는 경우는 j2 비트가 1인 경우다.

+ ```ng=1```이 되는 경우는 j1 비트가 1인 경우다.

+ ```zr=0, ng=0```이 되는 경우는 j3 비트가 1인 경우다.

위 세가지를 조합했을 때 ```(J1 AND ng) OR (J2 AND zr) OR (J3 AND NOT((zr OR ng)))``` 라는 로직이 설계된다.

즉, ng=1이거나 J1이 1일 때 1이 나와야 하는데 둘 중 하나라도 0인 값이 있다면 조건이 틀린 경우로 0이 나오게 된다.

예를 들어, JGE를 살펴보자면 ```zr=1, ng=0```인 경우에는 ```(0 AND 0) OR (1 AND 1) OR (1 AND NOT(1 OR 0)) = 0 OR 1 OR 0 = 1```이 되어 조건이 맞으므로 1로 설정된다.

그러나 조건이 ```zr=0, ng=1```이었다면 0이 나와야 되는데, 살펴보자면 ```(0 AND 1) OR (1 AND 0) OR (1 AND NOT(0 OR 1)) = 0 OR 0 OR 0 = 0```이 되어 정상적으로 0이 나온다. 

<br>

```c

```

<br>

+ Computer

![image](https://user-images.githubusercontent.com/52172169/209098412-02354efc-87bb-465c-bdda-30b7e5ee98ee.png)

```c

```

<br><br>
<hr style="border: 2px solid;">
<br><br>
