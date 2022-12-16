## Project3

<br>

![image](https://user-images.githubusercontent.com/52172169/207836450-8dde974b-e173-44ab-9750-d329bec5e70d.png)

<br>

DFF를 기반으로 Register, 레지스터를 기반으로 RAM과 Counter를 제작한다.

<br>

+ Bit.hdl

![image](https://user-images.githubusercontent.com/52172169/207851852-945599c0-f80d-4cbd-8493-19babf3d3f7c.png)

```c
// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/03/a/Bit.hdl

/**
 * 1-bit register:
 * If load[t] == 1 then out[t+1] = in[t]
 *                 else out does not change (out[t+1] = out[t])
 */

CHIP Bit {
    IN in, load;
    OUT out;

    PARTS:
    Mux(a=preout, b=in, sel=load, out=preout1);
    DFF(in=preout1, out=preout, out=out);
}
```

<br><br>

+ Register.hdl

![image](https://user-images.githubusercontent.com/52172169/207851943-f2d6e038-fd55-45e8-ba42-fb86773f9f22.png)

```c
// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/03/a/Register.hdl

/**
 * 16-bit register:
 * If load[t] == 1 then out[t+1] = in[t]
 * else out does not change
 */

CHIP Register {
    IN in[16], load;
    OUT out[16];

    PARTS:
    Bit(in=in[0], load=load, out=out[0]);
    Bit(in=in[1], load=load, out=out[1]);
    Bit(in=in[2], load=load, out=out[2]);
    Bit(in=in[3], load=load, out=out[3]);
    Bit(in=in[4], load=load, out=out[4]);
    Bit(in=in[5], load=load, out=out[5]);
    Bit(in=in[6], load=load, out=out[6]);
    Bit(in=in[7], load=load, out=out[7]);
    Bit(in=in[8], load=load, out=out[8]);
    Bit(in=in[9], load=load, out=out[9]);
    Bit(in=in[10], load=load, out=out[10]);
    Bit(in=in[11], load=load, out=out[11]);
    Bit(in=in[12], load=load, out=out[12]);
    Bit(in=in[13], load=load, out=out[13]);
    Bit(in=in[14], load=load, out=out[14]);
    Bit(in=in[15], load=load, out=out[15]);
}
```

<br><br>

+ Ram8.hdl

![image](https://user-images.githubusercontent.com/52172169/207852020-c4285f99-c8b3-4a16-92c7-4a3b235a7e4a.png)

```c

```

<br><br>

+ PC.hdl

![image](https://user-images.githubusercontent.com/52172169/207851982-490513f9-5944-4986-b8dc-2f47ae713c76.png)

```c
// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/03/a/PC.hdl

/**
 * A 16-bit counter with load and reset control bits.
 * if      (reset[t] == 1) out[t+1] = 0
 * else if (load[t] == 1)  out[t+1] = in[t]
 * else if (inc[t] == 1)   out[t+1] = out[t] + 1  (integer addition)
 * else                    out[t+1] = out[t]
 */

CHIP PC {
    IN in[16],load,inc,reset;
    OUT out[16];

    PARTS:
    // inc, out[t-1] + 1
    Inc16(in=preout, out=IncPreout);
    Mux16(a=preout, b=IncPreout, sel=inc, out=IncPreout1);
    // if load, do load else do inc
    Mux16(a=IncPreout1, b=in, sel=load, out=IncOrLoad);
    // reset, out[t-1] = 0
    Not(in=reset, out=NotReset);
    And16(a=IncOrLoad, b[0]=NotReset, b[1]=NotReset, b[2]=NotReset, b[3]=NotReset, b[4]=NotReset, b[5]=NotReset, b[6]=NotReset, b[7]=NotReset, b[8]=NotReset, b[9]=NotReset, b[10]=NotReset, b[11]=NotReset, b[12]=NotReset, b[13]=NotReset, b[14]=NotReset, b[15]=NotReset, out=ResetPreout);
    // if reset, do reset else do load or inc
    Mux16(a=IncOrLoad, b=ResetPreout, sel=reset, out=NextOut);
    // out
    Register(in=NextOut, load=true, out=out, out=preout);
}
```

<br><br>

<br><br>
<hr style="border: 2px solid;">
<br><br>
