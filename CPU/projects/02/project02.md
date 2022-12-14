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

```

<br>



<br><br>
<hr style="border: 2px solid;">
<br><br>
