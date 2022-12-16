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
    // out[t-1] or out[t-1] + 1 if inc
    Mux16(a=preout, b=IncPreout, sel=inc, out=IncPreout1);
    // if load, do load else do inc
    Mux16(a=IncPreout1, b=in, sel=load, out=IncOrLoad);
    // if reset, do reset else do load or inc
    Mux16(a=IncOrLoad, b=false, sel=reset, out=NextOut);
    // out
    Register(in=NextOut, load=true, out=out, out=preout);
}
```

<br><br>

+ Ram8.hdl

![image](https://user-images.githubusercontent.com/52172169/207852020-c4285f99-c8b3-4a16-92c7-4a3b235a7e4a.png)

```c
// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/03/a/RAM8.hdl

/**
 * Memory of 8 registers, each 16 bit-wide. Out holds the value
 * stored at the memory location specified by address. If load==1, then 
 * the in value is loaded into the memory location specified by address 
 * (the loaded value will be emitted to out from the next time step onward).
 */

CHIP RAM8 {
    IN in[16], load, address[3];
    OUT out[16];

    PARTS:
    // check address to select register
    DMux8Way(in=load, sel=address, a=load0, b=load1, c=load2, d=load3, e=load4, f=load5, g=load6, h=load7);
    // Register[0] ~ Register[7]
    Register(in=in, load=load0, out=PreOut0);
    Register(in=in, load=load1, out=PreOut1);
    Register(in=in, load=load2, out=PreOut2);
    Register(in=in, load=load3, out=PreOut3);
    Register(in=in, load=load4, out=PreOut4);
    Register(in=in, load=load5, out=PreOut5);
    Register(in=in, load=load6, out=PreOut6);
    Register(in=in, load=load7, out=PreOut7);
    // return out from selected register
    Mux8Way16(a=PreOut0, b=PreOut1, c=PreOut2, d=PreOut3, e=PreOut4, f=PreOut5, g=PreOut6, h=PreOut7, sel=address, out=out);
}
```

<br><br>

+ RAM64.hdl

```c
// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/03/a/RAM64.hdl

/**
 * Memory of 64 registers, each 16 bit-wide. Out holds the value
 * stored at the memory location specified by address. If load==1, then 
 * the in value is loaded into the memory location specified by address 
 * (the loaded value will be emitted to out from the next time step onward).
 */

CHIP RAM64 {
    IN in[16], load, address[6];
    OUT out[16];

    PARTS:
    // select register range
    DMux8Way(in=load, sel=address[3..5], a=ram0to7, b=ram8to15, c=ram16to23, d=ram24to31, e=ram32to39, f=ram40to47, g=ram48to55, h=ram56to63);
    // register 0 ~ 7
    RAM8(in=in, load=ram0to7, address=address[0..2], out=PreOut0to7);
    // register 8 ~ 15
    RAM8(in=in, load=ram8to15, address=address[0..2], out=Preout8to15);
    // register 16 ~ 23
    RAM8(in=in, load=ram16to23, address=address[0..2], out=Preout16to23);
    // register 24 ~ 31
    RAM8(in=in, load=ram24to31, address=address[0..2], out=Preout24to31);
    // register 32 ~ 39
    RAM8(in=in, load=ram32to39, address=address[0..2], out=Preout32to39);
    // register 40 ~ 47
    RAM8(in=in, load=ram40to47, address=address[0..2], out=Preout40to47);
    // register 48 ~ 55
    RAM8(in=in, load=ram48to55, address=address[0..2], out=Preout48to55);
    // register 56 ~ 63
    RAM8(in=in, load=ram56to63, address=address[0..2], out=Preout56to63);
    // return out from selected register
    Mux8Way16(a=PreOut0to7, b=Preout8to15, c=Preout16to23, d=Preout24to31, e=Preout32to39, f=Preout40to47, g=Preout48to55, h=Preout56to63, sel=address[3..5], out=out); 
}
```

<br>

+ RAM512.hdl

```c
// This file is part of the materials accompanying the book 
// "The Elements of Computing Systems" by Nisan and Schocken, 
// MIT Press. Book site: www.idc.ac.il/tecs
// File name: projects/03/b/RAM512.hdl

/**
 * Memory of 512 registers, each 16 bit-wide. Out holds the value
 * stored at the memory location specified by address. If load==1, then 
 * the in value is loaded into the memory location specified by address 
 * (the loaded value will be emitted to out from the next time step onward).
 */

CHIP RAM512 {
    IN in[16], load, address[9];
    OUT out[16];

    PARTS:
    // select register
    DMux8Way(in=load, sel=address[6..8], a=ram0, b=ram1, c=ram2, d=ram3, e=ram4, f=ram5, g=ram6, h=ram7);
    // RAM 0 ~ 511, 64ram * 8
    RAM64(in=in, load=ram0, address=address[0..5], out=PreOut0);
    RAM64(in=in, load=ram1, address=address[0..5], out=PreOut1);
    RAM64(in=in, load=ram2, address=address[0..5], out=PreOut2);
    RAM64(in=in, load=ram3, address=address[0..5], out=PreOut3);
    RAM64(in=in, load=ram4, address=address[0..5], out=PreOut4);
    RAM64(in=in, load=ram5, address=address[0..5], out=PreOut5);
    RAM64(in=in, load=ram6, address=address[0..5], out=PreOut6);
    RAM64(in=in, load=ram7, address=address[0..5], out=PreOut7);
    // return out from selected ram
    Mux8Way16(a=PreOut0, b=PreOut1, c=PreOut2, d=PreOut3, e=PreOut4, f=PreOut5, g=PreOut6, h=PreOut7, sel=address[6..8], out=out);
}
```

<br>

+ RAM4K.hdl

```c
// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/03/b/RAM4K.hdl

/**
 * Memory of 4K registers, each 16 bit-wide. Out holds the value
 * stored at the memory location specified by address. If load==1, then 
 * the in value is loaded into the memory location specified by address 
 * (the loaded value will be emitted to out from the next time step onward).
 */

CHIP RAM4K {
    IN in[16], load, address[12];
    OUT out[16];

    PARTS:
    // select register
    DMux8Way(in=load, sel=address[9..11], a=ram0, b=ram1, c=ram2, d=ram3, e=ram4, f=ram5, g=ram6, h=ram7);
    // RAM 0 ~ 4096, ram512 * 8
    RAM512(in=in, load=ram0, address=address[0..8], out=PreOut0);
    RAM512(in=in, load=ram1, address=address[0..8], out=PreOut1);
    RAM512(in=in, load=ram2, address=address[0..8], out=PreOut2);
    RAM512(in=in, load=ram3, address=address[0..8], out=PreOut3);
    RAM512(in=in, load=ram4, address=address[0..8], out=PreOut4);
    RAM512(in=in, load=ram5, address=address[0..8], out=PreOut5);
    RAM512(in=in, load=ram6, address=address[0..8], out=PreOut6);
    RAM512(in=in, load=ram7, address=address[0..8], out=PreOut7);
    // return out from selected ram
    Mux8Way16(a=PreOut0, b=PreOut1, c=PreOut2, d=PreOut3, e=PreOut4, f=PreOut5, g=PreOut6, h=PreOut7, sel=address[9..11], out=out);
}
```

<br>

+ RAM16K.hdl

```c
// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/03/b/RAM16K.hdl

/**
 * Memory of 16K registers, each 16 bit-wide. Out holds the value
 * stored at the memory location specified by address. If load==1, then 
 * the in value is loaded into the memory location specified by address 
 * (the loaded value will be emitted to out from the next time step onward).
 */

CHIP RAM16K {
    IN in[16], load, address[14];
    OUT out[16];

    PARTS:
    // select register
    DMux4Way(in=load, sel=address[12..13], a=ram0, b=ram1, c=ram2, d=ram3);
    // RAM 0 ~ 16K
    RAM4K(in=in, load=ram0, address=address[0..11], out=PreOut0);
    RAM4K(in=in, load=ram1, address=address[0..11], out=PreOut1);
    RAM4K(in=in, load=ram2, address=address[0..11], out=PreOut2);
    RAM4K(in=in, load=ram3, address=address[0..11], out=PreOut3);
    // return out from selected ram
    Mux4Way16(a=PreOut0, b=PreOut1, c=PreOut2, d=PreOut3, sel=address[12..13], out=out);
}
```

<br><br>
<hr style="border: 2px solid;">
<br><br>
