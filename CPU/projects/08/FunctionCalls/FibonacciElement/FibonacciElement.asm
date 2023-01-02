// function Sys.init 0
(Func$Sys.init$)
// push constant 4
@4
D=A
@SP
A=M
M=D
@SP
M=M+1
// call Main.fibonacci 1   // computes the 4'th fibonacci element
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
@1
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
@Func$Main.fibonacci$
0;JMP
(RET$0$)
// label WHILE
(WHILE)
// goto WHILE              // loops infinitely
@WHILE
0;JMP
// function Main.fibonacci 0
(Func$Main.fibonacci$)
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
// push constant 2
@2
D=A
@SP
A=M
M=D
@SP
M=M+1
// lt                     // checks if n<2
@SP
M=M-1
A=M
D=M
@SP
M=M-1
A=M
D=D-M
@TRUE0
D;JGT
D=0
@END0
0;JMP
(TRUE0)
    D=-1
(END0)
    @SP
    A=M
    M=D
    @SP
    M=M+1
// if-goto IF_TRUE
@SP
M=M-1
@IF_TRUE
D;JNE
// goto IF_FALSE
@IF_FALSE
0;JMP
// label IF_TRUE          // if n<2, return n
(IF_TRUE)
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
// label IF_FALSE         // if n>=2, returns fib(n-2)+fib(n-1)
(IF_FALSE)
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
// push constant 2
@2
D=A
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
// call Main.fibonacci 1  // computes fib(n-2)
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
@1
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
@Func$Main.fibonacci$
0;JMP
(RET$1$)
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
// push constant 1
@1
D=A
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
// call Main.fibonacci 1  // computes fib(n-1)
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
@1
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
@Func$Main.fibonacci$
0;JMP
(RET$2$)
// add                    // returns fib(n-1) + fib(n-2)
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
