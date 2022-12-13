## Boolean Logic

<br>

OR는 더하기(+), AND는 곱하기(x) 라고 보면 된다.

<br>

+ 교환법칙 (commutative)
  + x and y = y and x
  
  + x or y = y or x

<br>

+ 결합법칙 (associative)
  + x and (y and z) = (x and y) and z

  + x or (y or z) = (x or y) or z

<br>

+ 분배법칙 (distributive)
  + x and (y or z) = (x and y) or (x and z)

  + x or (y and z) = (x or y) and (x or z)

<br>

+ De morgan
  + NOT(x and y) = NOT(x) OR NOT(y)
  
  + NOT(x OR y) = NOT(x) AND NOT(y) 

<br><br>
<hr style="border: 2px solid;">
<br><br>

## Hardware Descriptive Language

<br>

칩 즉, 전자회로를 정밀하게 기술하는데 사용하는 언어이다.

<br>

![image](https://user-images.githubusercontent.com/52172169/207237615-84f389e2-9053-4cad-9fd7-560a9a1b8e83.png)

<br>

위의 사진처럼 Xor 칩을 만들 때, HDL 언어를 이용해서 정밀하게 기술할 수 있다.

강의에서는 이제 Tool을 제공해주는데, 간단한 하드웨어 시뮬레이터이다.

방법은 hdl 파일을 로드시켜주고, 테스트를 위한 tst 파일을 만들어서 로드시켜주면 세미콜론을 기준으로 구분해서 테스트를 진행한다.

테스트 후 출력파일을 만들어주는데, 이 파일과 강의에서 제공해준 결과 파일이 있는데 둘을 비교해보면 되겠다.

파일은 시뮬레이터에서 수정하지 못하므로 에디터를 이용해 사전에 코드를 작성하고 로드해줘야 한다.

아래는 예시 파일이다.

<br>

+ Xor.hdl

```c
CHIP Xor {
		IN a, b;
		OUT out;
	
		PARTS:
		Not (in=a, out=nota);
		Not (in=b, out=notb);
		And (a=a, b=notb, out=aAndNotb);
		And (a=nota, b=b, out=noaAndb);
		Or  (a=aAndNotb, b=notaAndb, out=out);
}
```

<br>

+ Xor.tst

```c
load Xor.hdl,            // hdl을 로드함
output-file Xor.out,     // Xor.out에 출력 값 저장
compare-to Xor.cmp,      // Xor.cmp 파일과 결과 비교
output-list a b out;     // a, b, out 값의 결과를 저장

set a 0, set b 0, eval, output;
set a 0, set b 1, eval, output;
set a 1, set b 0, eval, output;
set a 1, set b 1, eval, output;
```

<br><br>
<hr style="border: 2px solid;">
<br><br>

## Basic Logic gate

<br>

논리 연산자에 대해 AND, OR, NOT 이외 연산자들을 더 살펴본다. 

<br>

+ Mux (Multiple xor)
	+ 

<br><br>
<hr style="border: 2px solid;">
<br><br>
