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

```

<br><br>

<br><br>
<hr style="border: 2px solid;">
<br><br>
