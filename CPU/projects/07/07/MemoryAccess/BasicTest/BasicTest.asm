// push constant 10
@10
D=A
@SP
A=M
M=D
@SP
M=M+1
// pop local 0
@SP
M=M-1
A=M
D=M
@0
D=D+A
@LCL
D=D+M
@SP
A=M
A=M
A=D-A
M=D-A
// push constant 21
@21
D=A
@SP
A=M
M=D
@SP
M=M+1
// push constant 22
@22
D=A
@SP
A=M
M=D
@SP
M=M+1
// pop argument 2
@SP
M=M-1
A=M
D=M
@2
D=D+A
@ARG
D=D+M
@SP
A=M
A=M
A=D-A
M=D-A
// pop argument 1
@SP
M=M-1
A=M
D=M
@1
D=D+A
@ARG
D=D+M
@SP
A=M
A=M
A=D-A
M=D-A
// push constant 36
@36
D=A
@SP
A=M
M=D
@SP
M=M+1
// pop this 6
@SP
M=M-1
A=M
D=M
@6
D=D+A
@THIS
D=D+M
@SP
A=M
A=M
A=D-A
M=D-A
// push constant 42
@42
D=A
@SP
A=M
M=D
@SP
M=M+1
// push constant 45
@45
D=A
@SP
A=M
M=D
@SP
M=M+1
// pop that 5
@SP
M=M-1
A=M
D=M
@5
D=D+A
@THAT
D=D+M
@SP
A=M
A=M
A=D-A
M=D-A
// pop that 2
@SP
M=M-1
A=M
D=M
@2
D=D+A
@THAT
D=D+M
@SP
A=M
A=M
A=D-A
M=D-A
// push constant 510
@510
D=A
@SP
A=M
M=D
@SP
M=M+1
// pop temp 6
@SP
M=M-1
A=M
D=M
@6
D=D+A
@5
D=D+A
@SP
A=M
A=M
A=D-A
M=D-A
// push local 0
@0
D=A
@LCL
D=D+M
A=D
D=M
@SP
A=M
M=D
@SP
M=M+1
// push that 5
@5
D=A
@THAT
D=D+M
A=D
D=M
@SP
A=M
M=D
@SP
M=M+1
// add
@SP
M=M-1
A=M
D=M
@SP
M=M-1
A=M
D=D+M
@SP
A=M
M=D
@SP
M=M+1
// push argument 1
@1
D=A
@ARG
D=D+M
A=D
D=M
@SP
A=M
M=D
@SP
M=M+1
// sub
@SP
M=M-1
A=M
D=M
@SP
M=M-1
A=M
D=M-D
@SP
A=M
M=D
@SP
M=M+1
// push this 6
@6
D=A
@THIS
D=D+M
A=D
D=M
@SP
A=M
M=D
@SP
M=M+1
// push this 6
@6
D=A
@THIS
D=D+M
A=D
D=M
@SP
A=M
M=D
@SP
M=M+1
// add
@SP
M=M-1
A=M
D=M
@SP
M=M-1
A=M
D=D+M
@SP
A=M
M=D
@SP
M=M+1
// sub
@SP
M=M-1
A=M
D=M
@SP
M=M-1
A=M
D=M-D
@SP
A=M
M=D
@SP
M=M+1
// push temp 6
@6
D=A
@5
D=D+A
A=D
D=M
@SP
A=M
M=D
@SP
M=M+1
// add
@SP
M=M-1
A=M
D=M
@SP
M=M-1
A=M
D=D+M
@SP
A=M
M=D
@SP
M=M+1
