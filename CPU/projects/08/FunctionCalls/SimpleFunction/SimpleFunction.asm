// function SimpleFunction.test 2
(Func$SimpleFunction.test$)
//** initialize local nVars to 0
@0
D=A
@LCL
A=M+D
M=0
//** SP++
@SP
M=M+1
//** initialize local nVars to 0
@1
D=A
@LCL
A=M+D
M=0
//** SP++
@SP
M=M+1
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
// push local 1
@1
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
// not
@SP
M=M-1
A=M
M=!M
@SP
M=M+1
// push argument 0
@0
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
// return
//++ retAddr = temp 7(RAM[12])
@5
D=A
@LCL
A=M-D
D=M
@12
M=D
//++ *ARG = return value
@SP
A=M-1
D=M
@ARG
A=M
M=D
//++ SP = ARG + 1
@ARG
D=M
@SP
M=D+1
//++ THAT = *(LCL - 1)
@LCL
A=M-1
D=M
@THAT
M=D
//++ THIS = *(LCL - 2)
@LCL
A=M-1
A=A-1
D=M
@THIS
M=D
//++ ARG = *(LCL - 3)
@LCL
A=M-1
A=A-1
A=A-1
D=M
@ARG
M=D
//++ LCL = *(LCL - 4)
@LCL
A=M-1
A=A-1
A=A-1
A=A-1
D=M
@LCL
M=D
//++ JMP ret
@12
A=M
0;JMP
