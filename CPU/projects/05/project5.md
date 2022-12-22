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
    DMux(in=true, sel=address[14], a=RAM, b=SCREEN);
    // if address is in RAM range, read or write in RAM, else SCREEN 
    And(a=load, b=RAM, out=loadRAM);
    And(a=load, b=SCREEN, out=loadSCREEN);
    // read or write in selected area
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

```c

```

<br>

+ ROM32K

![image](https://user-images.githubusercontent.com/52172169/209098345-410a215b-4632-47f5-83a6-a2a02167139e.png)

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
