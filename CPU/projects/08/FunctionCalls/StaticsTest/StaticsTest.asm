// function Sys.init 0
(Func$Sys.init$)
// push constant 6
@6
D=A
@SP
A=M
M=D
@SP
M=M+1
// push constant 8
@8
D=A
@SP
A=M
M=D
@SP
M=M+1
// call Class1.set 2
//** push returnAddress
@RET$0$
D=A
@SP
A=M
M=D
@SP
M=M+1
//** push LCL
@LCL
D=M
@SP
A=M
M=D
@SP
M=M+1
//** push ARG
@ARG
D=M
@SP
A=M
M=D
@SP
M=M+1
//** push THIS
@THIS
D=M
@SP
A=M
M=D
@SP
M=M+1
//** push THAT
@THAT
D=M
@SP
A=M
M=D
@SP
M=M+1
//** ARG = SP - 5 - nArgs
@SP
D=M
@2
D=D-A
@5
D=D-A
@ARG
M=D
//** LCL = SP
@SP
D=M
@LCL
M=D
//** goto FunctionName
@Func$Class1.set$
0;JMP
(RET$0$)
// pop temp 0 // Dumps the return value
@SP
M=M-1
A=M
D=M
@0
D=D+A
@5
D=D+A
@SP
A=M
A=M
A=D-A
M=D-A
// push constant 23
@23
D=A
@SP
A=M
M=D
@SP
M=M+1
// push constant 15
@15
D=A
@SP
A=M
M=D
@SP
M=M+1
// call Class2.set 2
//** push returnAddress
@RET$1$
D=A
@SP
A=M
M=D
@SP
M=M+1
//** push LCL
@LCL
D=M
@SP
A=M
M=D
@SP
M=M+1
//** push ARG
@ARG
D=M
@SP
A=M
M=D
@SP
M=M+1
//** push THIS
@THIS
D=M
@SP
A=M
M=D
@SP
M=M+1
//** push THAT
@THAT
D=M
@SP
A=M
M=D
@SP
M=M+1
//** ARG = SP - 5 - nArgs
@SP
D=M
@2
D=D-A
@5
D=D-A
@ARG
M=D
//** LCL = SP
@SP
D=M
@LCL
M=D
//** goto FunctionName
@Func$Class2.set$
0;JMP
(RET$1$)
// pop temp 0 // Dumps the return value
@SP
M=M-1
A=M
D=M
@0
D=D+A
@5
D=D+A
@SP
A=M
A=M
A=D-A
M=D-A
// call Class1.get 0
//** push returnAddress
@RET$2$
D=A
@SP
A=M
M=D
@SP
M=M+1
//** push LCL
@LCL
D=M
@SP
A=M
M=D
@SP
M=M+1
//** push ARG
@ARG
D=M
@SP
A=M
M=D
@SP
M=M+1
//** push THIS
@THIS
D=M
@SP
A=M
M=D
@SP
M=M+1
//** push THAT
@THAT
D=M
@SP
A=M
M=D
@SP
M=M+1
//** ARG = SP - 5 - nArgs
@SP
D=M
@0
D=D-A
@5
D=D-A
@ARG
M=D
//** LCL = SP
@SP
D=M
@LCL
M=D
//** goto FunctionName
@Func$Class1.get$
0;JMP
(RET$2$)
// call Class2.get 0
//** push returnAddress
@RET$3$
D=A
@SP
A=M
M=D
@SP
M=M+1
//** push LCL
@LCL
D=M
@SP
A=M
M=D
@SP
M=M+1
//** push ARG
@ARG
D=M
@SP
A=M
M=D
@SP
M=M+1
//** push THIS
@THIS
D=M
@SP
A=M
M=D
@SP
M=M+1
//** push THAT
@THAT
D=M
@SP
A=M
M=D
@SP
M=M+1
//** ARG = SP - 5 - nArgs
@SP
D=M
@0
D=D-A
@5
D=D-A
@ARG
M=D
//** LCL = SP
@SP
D=M
@LCL
M=D
//** goto FunctionName
@Func$Class2.get$
0;JMP
(RET$3$)
// label WHILE
(WHILE)
// goto WHILE
@WHILE
0;JMP
// function Class1.set 0
(Func$Class1.set$)
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
// pop static 0
@SP
M=M-1
A=M
D=M
@Class1.0
D=D+A
@SP
A=M
A=M
A=D-A
M=D-A
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
// pop static 1
@SP
M=M-1
A=M
D=M
@Class1.1
D=D+A
@SP
A=M
A=M
A=D-A
M=D-A
// push constant 0
@0
D=A
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
// function Class1.get 0
(Func$Class1.get$)
// push static 0
@Class1.0
D=M
@SP
A=M
M=D
@SP
M=M+1
// push static 1
@Class1.1
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
// function Class2.set 0
(Func$Class2.set$)
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
// pop static 0
@SP
M=M-1
A=M
D=M
@Class2.0
D=D+A
@SP
A=M
A=M
A=D-A
M=D-A
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
// pop static 1
@SP
M=M-1
A=M
D=M
@Class2.1
D=D+A
@SP
A=M
A=M
A=D-A
M=D-A
// push constant 0
@0
D=A
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
// function Class2.get 0
(Func$Class2.get$)
// push static 0
@Class2.0
D=M
@SP
A=M
M=D
@SP
M=M+1
// push static 1
@Class2.1
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
