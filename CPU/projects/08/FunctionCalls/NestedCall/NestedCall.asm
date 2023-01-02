// function Sys.init 0
(Func$Sys.init$)
// push constant 4000	// test THIS and THAT context save
@4000	//
D=A
@SP
A=M
M=D
@SP
M=M+1
// pop pointer 0
@SP
M=M-1
A=M
D=M
@THIS
D=D+A
@SP
A=M
A=M
A=D-A
M=D-A
// push constant 5000
@5000
D=A
@SP
A=M
M=D
@SP
M=M+1
// pop pointer 1
@SP
M=M-1
A=M
D=M
@THAT
D=D+A
@SP
A=M
A=M
A=D-A
M=D-A
// call Sys.main 0
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
@Func$Sys.main$
0;JMP
(RET$0$)
// pop temp 1
@SP
M=M-1
A=M
D=M
@1
D=D+A
@5
D=D+A
@SP
A=M
A=M
A=D-A
M=D-A
// label LOOP
(LOOP)
// goto LOOP
@LOOP
0;JMP
// function Sys.main 5
(Func$Sys.main$)
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
//** initialize local nVars to 0
@2
D=A
@LCL
A=M+D
M=0
//** SP++
@SP
M=M+1
//** initialize local nVars to 0
@3
D=A
@LCL
A=M+D
M=0
//** SP++
@SP
M=M+1
//** initialize local nVars to 0
@4
D=A
@LCL
A=M+D
M=0
//** SP++
@SP
M=M+1
// push constant 4001
@4001
D=A
@SP
A=M
M=D
@SP
M=M+1
// pop pointer 0
@SP
M=M-1
A=M
D=M
@THIS
D=D+A
@SP
A=M
A=M
A=D-A
M=D-A
// push constant 5001
@5001
D=A
@SP
A=M
M=D
@SP
M=M+1
// pop pointer 1
@SP
M=M-1
A=M
D=M
@THAT
D=D+A
@SP
A=M
A=M
A=D-A
M=D-A
// push constant 200
@200
D=A
@SP
A=M
M=D
@SP
M=M+1
// pop local 1
@SP
M=M-1
A=M
D=M
@1
D=D+A
@LCL
D=D+M
@SP
A=M
A=M
A=D-A
M=D-A
// push constant 40
@40
D=A
@SP
A=M
M=D
@SP
M=M+1
// pop local 2
@SP
M=M-1
A=M
D=M
@2
D=D+A
@LCL
D=D+M
@SP
A=M
A=M
A=D-A
M=D-A
// push constant 6
@6
D=A
@SP
A=M
M=D
@SP
M=M+1
// pop local 3
@SP
M=M-1
A=M
D=M
@3
D=D+A
@LCL
D=D+M
@SP
A=M
A=M
A=D-A
M=D-A
// push constant 123
@123
D=A
@SP
A=M
M=D
@SP
M=M+1
// call Sys.add12 1
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
@Func$Sys.add12$
0;JMP
(RET$1$)
// pop temp 0
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
// push local 2
@2
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
// push local 3
@3
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
// push local 4
@4
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
// function Sys.add12 0
(Func$Sys.add12$)
// push constant 4002
@4002
D=A
@SP
A=M
M=D
@SP
M=M+1
// pop pointer 0
@SP
M=M-1
A=M
D=M
@THIS
D=D+A
@SP
A=M
A=M
A=D-A
M=D-A
// push constant 5002
@5002
D=A
@SP
A=M
M=D
@SP
M=M+1
// pop pointer 1
@SP
M=M-1
A=M
D=M
@THAT
D=D+A
@SP
A=M
A=M
A=D-A
M=D-A
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
// push constant 12
@12
D=A
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
