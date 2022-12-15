## project02

<br>

프로젝트2 진행 시, 우리가 로직 게이트를 만들긴 했지만 BUITIN 칩을 사용하는걸 권장하고 있고, ALU 부분 중 2개의 tst가 있는데 하나는 zr, ng의 값을 무시하는 파일과 zr, ng까지 검사하는 파일이 있다.

여기서 먼저 zr, ng 부분을 빼고 구현을 한 다음 통과하면 그 다음 ng, zr 부분을 구현하는 것을 권장한다.

폴더 내부에 해당 게이트가 없으면 자동으로 tools의 builtin 칩들을 참조한다.

<br>

+ HalfAdder.hdl

```c
// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/02/HalfAdder.hdl

/**
 * Computes the sum of two bits.
 */

CHIP HalfAdder {
    IN a, b;    // 1-bit inputs
    OUT sum,    // Right bit of a + b 
        carry;  // Left bit of a + b

    PARTS:
    Xor(a=a, b=b, out=sum);
    And(a=a, b=b, out=carry);
}

```

<br>

+ FullAdder.hdl

```c
// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/02/FullAdder.hdl

/**
 * Computes the sum of three bits.
 */

CHIP FullAdder {
    IN a, b, c;  // 1-bit inputs
    OUT sum,     // Right bit of a + b + c
        carry;   // Left bit of a + b + c

    PARTS:
    HalfAdder(a=a, b=b, sum=sum1, carry=carry1);
    HalfAdder(a=sum1, b=c, sum=sum, carry=carry2);
    Or(a=carry1, b=carry2, out=carry);
}
```

<br>

+ Add16

```c
// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/02/Adder16.hdl

/**
 * Adds two 16-bit values.
 * The most significant carry bit is ignored.
 */

CHIP Add16 {
    IN a[16], b[16];
    OUT out[16];

    PARTS:
    HalfAdder(a=a[0], b=b[0], sum=out[0], carry=carry1);
    FullAdder(a=a[1], b=b[1], c=carry1, sum=out[1], carry=carry2);
    FullAdder(a=a[2], b=b[2], c=carry2, sum=out[2], carry=carry3);
    FullAdder(a=a[3], b=b[3], c=carry3, sum=out[3], carry=carry4);
    FullAdder(a=a[4], b=b[4], c=carry4, sum=out[4], carry=carry5);
    FullAdder(a=a[5], b=b[5], c=carry5, sum=out[5], carry=carry6);
    FullAdder(a=a[6], b=b[6], c=carry6, sum=out[6], carry=carry7);
    FullAdder(a=a[7], b=b[7], c=carry7, sum=out[7], carry=carry8);
    FullAdder(a=a[8], b=b[8], c=carry8, sum=out[8], carry=carry9);
    FullAdder(a=a[9], b=b[9], c=carry9, sum=out[9], carry=carry10);
    FullAdder(a=a[10], b=b[10], c=carry10, sum=out[10], carry=carry11);
    FullAdder(a=a[11], b=b[11], c=carry11, sum=out[11], carry=carry12);
    FullAdder(a=a[12], b=b[12], c=carry12, sum=out[12], carry=carry13);
    FullAdder(a=a[13], b=b[13], c=carry13, sum=out[13], carry=carry14);
    FullAdder(a=a[14], b=b[14], c=carry14, sum=out[14], carry=carry15);
    FullAdder(a=a[15], b=b[15], c=carry15, sum=out[15], carry=carry);
}
```

<br>

+ Inc16.hdl -> 1과 0은 true, false로 표현함

```c
// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/02/Inc16.hdl

/**
 * 16-bit incrementer:
 * out = in + 1 (arithmetic addition)
 */

CHIP Inc16 {
    IN in[16];
    OUT out[16];

    PARTS:
    HalfAdder(a=in[0], b=true, sum=out[0], carry=carry1);
    FullAdder(a=in[1], b=false, c=carry1, sum=out[1], carry=carry2);
    FullAdder(a=in[2], b=false, c=carry2, sum=out[2], carry=carry3);
    FullAdder(a=in[3], b=false, c=carry3, sum=out[3], carry=carry4);
    FullAdder(a=in[4], b=false, c=carry4, sum=out[4], carry=carry5);
    FullAdder(a=in[5], b=false, c=carry5, sum=out[5], carry=carry6);
    FullAdder(a=in[6], b=false, c=carry6, sum=out[6], carry=carry7);
    FullAdder(a=in[7], b=false, c=carry7, sum=out[7], carry=carry8);
    FullAdder(a=in[8], b=false, c=carry8, sum=out[8], carry=carry9);
    FullAdder(a=in[9], b=false, c=carry9, sum=out[9], carry=carry10);
    FullAdder(a=in[10], b=false, c=carry10, sum=out[10], carry=carry11);
    FullAdder(a=in[11], b=false, c=carry11, sum=out[11], carry=carry12);
    FullAdder(a=in[12], b=false, c=carry12, sum=out[12], carry=carry13);
    FullAdder(a=in[13], b=false, c=carry13, sum=out[13], carry=carry14);
    FullAdder(a=in[14], b=false, c=carry14, sum=out[14], carry=carry15);
    FullAdder(a=in[15], b=false, c=carry15, sum=out[15], carry=carry);
}
```

<br>

+ ALU.hdl

```c
// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/02/ALU.hdl

/**
 * The ALU (Arithmetic Logic Unit).
 * Computes one of the following functions:
 * x+y, x-y, y-x, 0, 1, -1, x, y, -x, -y, !x, !y,
 * x+1, y+1, x-1, y-1, x&y, x|y on two 16-bit inputs, 
 * according to 6 input bits denoted zx,nx,zy,ny,f,no.
 * In addition, the ALU computes two 1-bit outputs:
 * if the ALU output == 0, zr is set to 1; otherwise zr is set to 0;
 * if the ALU output < 0, ng is set to 1; otherwise ng is set to 0.
 */

// Implementation: the ALU logic manipulates the x and y inputs
// and operates on the resulting values, as follows:
// if (zx == 1) set x = 0        // 16-bit constant
// if (nx == 1) set x = !x       // bitwise not
// if (zy == 1) set y = 0        // 16-bit constant
// if (ny == 1) set y = !y       // bitwise not
// if (f == 1)  set out = x + y  // integer 2's complement addition
// if (f == 0)  set out = x & y  // bitwise and
// if (no == 1) set out = !out   // bitwise not
// if (out == 0) set zr = 1
// if (out < 0) set ng = 1

CHIP ALU {
    IN  
        x[16], y[16],  // 16-bit inputs        
        zx, // zero the x input?
        nx, // negate the x input?
        zy, // zero the y input?
        ny, // negate the y input?
        f,  // compute out = x + y (if 1) or x & y (if 0)
        no; // negate the out output?

    OUT 
        out[16], // 16-bit output
        zr, // 1 if (out == 0), 0 otherwise
        ng; // 1 if (out < 0),  0 otherwise

    PARTS:
    // zx
    Mux16(a=true, b=false, sel=zx, out=NotZx);
    And16(a=NotZx, b=x, out=ZxX);
    // nx
    Not16(in=ZxX, out=Not16X);
    Mux16(a=ZxX, b=Not16X, sel=nx, out=ZxNxX);
    // zy
    Mux16(a=true, b=false, sel=zy, out=NotZy);
    And16(a=y, b=NotZy, out=ZyY);
    // ny
    Not16(in=ZyY, out=Not16Y);
    Mux16(a=ZyY, b=Not16Y, sel=ny, out=ZyNyY);
    // f
    Add16(a=ZxNxX, b=ZyNyY, out=f1);
    And16(a=ZxNxX, b=ZyNyY, out=f0);
    Mux16(a=f0, b=f1, sel=f, out=fResult);
    // no
    Not16(in=fResult, out=Not16fResult);
    Mux16(a=fResult, b=Not16fResult, sel=no, out=out, out[0..7]=out0to7, out[8..15]=out8to15, out[15]=ng);
    // zr
    Or8Way(in=out0to7, out=zr1);
    Or8Way(in=out8to15, out=zr2);
    Or(a=zr1, b=zr2, out=zrResult);
    Not(in=zrResult, out=zr);
}
```

<br><br>
<hr style="border: 2px solid;">
<br><br>
