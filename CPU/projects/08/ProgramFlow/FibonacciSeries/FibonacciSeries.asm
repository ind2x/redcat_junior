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
// pop pointer 1           // that = argument[1]
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
// push constant 0
@0
D=A
@SP
A=M
M=D
@SP
M=M+1
// pop that 0              // first element in the series = 0
@SP
M=M-1
A=M
D=M
@0
D=D+A
@THAT
D=D+M
@SP
A=M
A=M
A=D-A
M=D-A
// push constant 1
@1
D=A
@SP
A=M
M=D
@SP
M=M+1
// pop that 1              // second element in the series = 1
@SP
M=M-1
A=M
D=M
@1
D=D+A
@THAT
D=D+M
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
// pop argument 0          // num_of_elements -= 2 (first 2 elements are set)
@SP
M=M-1
A=M
D=M
@0
D=D+A
@ARG
D=D+M
@SP
A=M
A=M
A=D-A
M=D-A
// label MAIN_LOOP_START
(MAIN_LOOP_START)
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
// if-goto COMPUTE_ELEMENT // if num_of_elements > 0, goto COMPUTE_ELEMENT
@SP
M=M-1
@COMPUTE_ELEMENT
D;JNE
// goto END_PROGRAM        // otherwise, goto END_PROGRAM
@END_PROGRAM
0;JMP
// label COMPUTE_ELEMENT
(COMPUTE_ELEMENT)
// push that 0
@0
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
// push that 1
@1
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
// pop that 2              // that[2] = that[0] + that[1]
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
// push pointer 1
@THAT
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
// pop pointer 1           // that += 1
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
// pop argument 0          // num_of_elements--
@SP
M=M-1
A=M
D=M
@0
D=D+A
@ARG
D=D+M
@SP
A=M
A=M
A=D-A
M=D-A
// goto MAIN_LOOP_START
@MAIN_LOOP_START
0;JMP
// label END_PROGRAM
(END_PROGRAM)
