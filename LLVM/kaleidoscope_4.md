# Kaleidoscope: Adding JIT and Optimizer Support

<br>

이번 챕터에서는 ```adding optimizer support to your language```과 ```adding JIT compiler support```를 해준다고 한다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## Trivial Constant Folding

<br>

LLVM에서는 AST에서 constant folding을 지원하지 않고, LLVM IR에서 자체적으로 constant folding 할 부분이 있는지 찾아서 수행한다.

그래서 아래와 같이 constant folding을 해주는 것 같다.

<br>

```shell
ready> def test(x) 1+2+x;
Read function definition:
define double @test(double %x) {
entry:
        %addtmp = fadd double 3.000000e+00, %x
        ret double %addtmp
}
```

<br>

또한 아래와 같은 LHS = RHS 인 상황도 있다.

<br>

```shell
ready> def test(x) (1+2+x)*(x+(1+2));
ready> Read function definition:
define double @test(double %x) {
entry:
        %addtmp = fadd double 3.000000e+00, %x
        %addtmp1 = fadd double %x, 3.000000e+00
        %multmp = fmul double %addtmp, %addtmp1
        ret double %multmp
}
```

<br>

이 경우 LHS = RHS이기 때문에 굳이 addtmp, addtmp1을 만들어서 multmp를 하는 것이 아닌, addtmp 만으로만 multmp을 표시해주고 싶을 것이다.

그러나 이것은 가능하려면 두 가지 변환이 필요한데, add 명령어에 관한 중복 표현 삭제와 식의 재연결이 필요하다.

LLVM에서는 passes 형태로 광범위한 형태의 최적화를 지원한다고 한다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

