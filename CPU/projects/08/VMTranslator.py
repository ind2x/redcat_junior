import sys 

'''
SP = RAM[0] = 256
LCL = RAM[1]
ARG = RAM[2]
THIS = RAM[3]
THAT = RAM[4]
temp = RAM[5] ~ RAM[12]
static = RAM[16] ~ RAM[255]
stack = RAM[256] ~ 

command: 
// project 7
push/pop segment i, (i >= 0)
add, sub, neg
eq, gt, lt
and, or, not

// project 8
label, goto, if-goto
Function, Call, return

memory segment:
local, argument, this, that, constant, static, pointer, temp
'''
'''
코드 변경..
프로젝트 8 같이 여러 코드를 변환하여 하나의 파일로 만들어야 하는 경우
1. 디렉터리 명과 저장할 ouput 파일명을 인자로 받음
2. 인자로 받은 디렉터리에서 .vm 코드 검색
2-1. Sys.vm이 있다면 이 파일부터 변환
3. output 파일을 생성하여 저장
'''
# Usage: python VMTranslator [SearchDirectory] [output.vm]
with open(sys.argv[1], 'r') as f:
    lines = f.readlines()

    with open('./'+sys.argv[1].replace('.vm', '.asm'), 'w') as res:
        MemorySegment = {'local':'LCL', "argument":"ARG", "this":"THIS", "that":"THAT", "temp":5}
        Pointer = ["THIS","THAT"]
        # count the number of ['eq', 'gt', 'lt'] is used 
        ElseExist = 0
        # for label of function return address
        Return = 0

        for line in lines:
            instruction = line.strip().split(' ')
            if instruction[0] == "//" or instruction[0] == '':
                continue
            #print(instruction)
            res.write("// {}\n".format(' '.join(instruction)))  # for debug
            if instruction[0] == "push":
                # if push constant i
                if instruction[1] == "constant":
                    res.write("@{}\n".format(instruction[2]))
                    res.write("D=A\n") 
                
                # if push local, argument, this, that i
                elif instruction[1] in MemorySegment:
                    res.write("@{}\n".format(instruction[2]))
                    res.write("D=A\n")
                    res.write("@{}\n".format(MemorySegment[instruction[1]]))
                    if instruction[1] == "temp":
                        res.write("D=D+A\n")
                        res.write("A=D\n")
                        res.write("D=M\n")
                    else:
                        res.write("D=D+M\n")    # D = i + LCL, ... ~
                        res.write("A=D\n")      # A = i + LCL, ... ~
                        res.write("D=M\n")      # D = RAM[i+LCL] 

                # if push static 0 -> push FileName.0
                elif instruction[1] == "static":


                # if push pointer 0 -> push THIS
                # if push pointer 1 -> push THAT
                elif instruction[1] == "pointer":
                    res.write("@{}\n".format(Pointer[int(instruction[2])]))
                    res.write("D=M\n")      # D = RAM[THIS] or RAM[THAT]

                res.write("@SP\n")
                res.write("A=M\n")      # A = SP
                res.write("M=D\n")      # RAM[SP] = D
                res.write("@SP\n")
                res.write("M=M+1\n")
            
            # pop implementation: https://evoniuk.github.io/posts/nand.html
            elif instruction[0] == "pop":
                res.write("@SP\n")      # SP--
                res.write("M=M-1\n")
                res.write("A=M\n")
                res.write("D=M\n")      # D = RAM[SP]
                
                if instruction[1] in MemorySegment:
                    res.write("@{}\n".format(instruction[2]))
                    res.write("D=D+A\n")    # D = RAM[SP] + i
                    res.write("@{}\n".format(MemorySegment[instruction[1]]))
                    if instruction[1] == "temp":
                        res.write("D=D+A\n")    # D = RAM[SP] + i + 5(temp)
                    else:
                        res.write("D=D+M\n")    # D = RAM[SP] + i + LCL, ..., ~
                
                elif instruction[1] == "static":


                elif instruction[1] == "pointer":
                    res.write("@{}\n".format(Pointer[int(instruction[2])]))
                    res.write("D=D+A\n")    # D = RAM[THIS] + RAM[SP] 

                res.write("@SP\n")
                res.write("A=M\n")      # A = SP
                res.write("A=M\n")      # A = RAM[SP]
                res.write("A=D-A\n")    # A = i + LCL
                res.write("M=D-A\n")    # RAM[i+LCL] = RAM[SP]

            # if Branching commands - label, goto, if-goto
            elif instruction[0] == "label":
                res.write("({})\n".format(instruction[1]))
            elif instruction[0] == "goto":
                res.write("@{}\n".format(instruction[1]))
                res.write("0;JMP\n")
            elif instruction[0] == "if-goto":
                res.write("@SP\n")      # SP--
                res.write("M=M-1\n")
                res.write("@{}\n".format(instruction[1])) 
                res.write("D;JNE\n")    # D = -1 -> true

            # if Function commands - call, function, return
            # if call -> call [function] [nArguments]
            elif instruction[0] == "call":
                res.write("//** push returnAddress\n")
                res.write("@{}\n".format("RET$"+str(Return)+"$"))
                res.write("D=A\n")
                res.write("@SP\n")
                res.write("A=M\n")
                res.write("M=D\n")
                res.write("@SP\n")
                res.write("M=M+1\n")
                res.write("//** push LCL\n")
                res.write("@LCL\n")     # push LCL
                res.write("D=M\n")
                res.write("@SP\n")
                res.write("A=M\n")
                res.write("M=D\n")
                res.write("@SP\n")
                res.write("M=M+1\n")
                res.write("//** push ARG\n")
                res.write("@ARG\n")     # push ARG
                res.write("D=M\n")
                res.write("@SP\n")
                res.write("A=M\n")
                res.write("M=D\n")
                res.write("@SP\n")
                res.write("M=M+1\n")
                res.write("//** push THIS\n")
                res.write("@THIS\n")    # push THIS
                res.write("D=M\n")
                res.write("@SP\n")
                res.write("A=M\n")
                res.write("M=D\n")
                res.write("@SP\n")
                res.write("M=M+1\n")
                res.write("//** push THAT\n")
                res.write("@THAT\n")    # push THAT
                res.write("D=M\n")
                res.write("@SP\n")
                res.write("A=M\n")
                res.write("M=D\n")
                res.write("@SP\n")
                res.write("M=M+1\n")
                res.write("//** ARG = SP - 5 - nArgs\n")
                res.write("@SP\n")      # ARG = SP - 5 - nArgs
                res.write("D=M\n")      
                res.write("@{}\n".format(instruction[2]))
                res.write("D=D-A\n")    
                res.write("@5\n")
                res.write("D=D-A\n")    
                res.write("@ARG\n")
                res.write("M=D\n")      
                res.write("//** LCL = SP\n")
                res.write("@SP\n")      # LCL = SP
                res.write("D=M\n")
                res.write("@LCL\n")
                res.write("M=D\n")
                res.write("//** goto FunctionName\n")
                res.write("@{}\n".format("Func$"+instruction[1]+"$"))
                res.write("0;JMP\n")    # goto FunctionName
                res.write("({})\n".format("RET$"+str(Return)+"$"))
                Return += 1
            
            # if function FunctionName NVars
            elif instruction[0] == "function":  
                res.write("(Func${}$)\n".format(instruction[1]))
                # initialize N Vars to 0
                for i in range(int(instruction[2])):
                    res.write("//** initialize local nVars to 0\n")
                    res.write("@{}\n".format(i))
                    res.write("D=A\n")
                    res.write("@LCL\n")     # *(LCL+i)
                    res.write("A=M+D\n")
                    res.write("M=0\n")
                    res.write("//** SP++\n")
                    res.write("@SP\n")
                    res.write("M=M+1\n")
            
            elif instruction[0] == "return":
                res.write("//++ retAddr = temp 7(RAM[12])\n")
                res.write("@5\n")       # ret = *(LCL-5)
                res.write("D=A\n")
                res.write("@LCL\n")
                res.write("A=M-D\n")    # A = LCL-5
                res.write("D=M\n")      # D = ret Address
                res.write("@12\n")      # RAM[12] = ret address
                res.write("M=D\n")
                res.write("//++ *ARG = return value\n")
                res.write("@SP\n")      # *ARG = return value
                res.write("A=M-1\n")    
                res.write("D=M\n")
                res.write("@ARG\n")
                res.write("A=M\n")
                res.write("M=D\n")
                res.write("//++ SP = ARG + 1\n")
                res.write("@ARG\n")     # SP = ARG + 1
                res.write("D=M\n")
                res.write("@SP\n")      
                res.write("M=D+1\n")
                res.write("//++ THAT = *(LCL - 1)\n")
                res.write("@LCL\n")     # THAT = *(LCL - 1)
                res.write("A=M-1\n")    
                res.write("D=M\n")
                res.write("@THAT\n")
                res.write("M=D\n")
                res.write("//++ THIS = *(LCL - 2)\n")
                res.write("@LCL\n")     # THIS = *(LCL - 2)
                res.write("A=M-1\n")
                res.write("A=A-1\n")
                res.write("D=M\n")
                res.write("@THIS\n")
                res.write("M=D\n")
                res.write("//++ ARG = *(LCL - 3)\n")
                res.write("@LCL\n")     # ARG = *(LCL - 3)
                res.write("A=M-1\n")
                res.write("A=A-1\n")
                res.write("A=A-1\n")
                res.write("D=M\n")
                res.write("@ARG\n")
                res.write("M=D\n")
                res.write("//++ LCL = *(LCL - 4)\n")
                res.write("@LCL\n")     # LCL = *(LCL - 4)
                res.write("A=M-1\n")
                res.write("A=A-1\n")
                res.write("A=A-1\n")
                res.write("A=A-1\n")
                res.write("D=M\n")      # D = RAM[LCL-4]
                res.write("@LCL\n")
                res.write("M=D\n")
                res.write("//++ JMP ret\n")
                res.write("@12\n")      # RAM[12] = ret Address
                res.write("A=M\n")
                res.write("0;JMP\n")
            
            # if Logical and Arithmetic Commands
            else:
                res.write("@SP\n")      # D = RAM[SP-1]
                res.write("M=M-1\n")    # SP--
                res.write("A=M\n")
                
                if instruction[0] == "neg":
                    res.write("M=-M\n")
                    res.write("@SP\n")      # SP++
                    res.write("M=M+1\n")
                    continue
                elif instruction[0] == "not":
                    res.write("M=!M\n")
                    res.write("@SP\n")      # SP++
                    res.write("M=M+1\n")
                    continue
                
                res.write("D=M\n")      # D = RAM[SP-1]
                
                res.write("@SP\n")      # RAM[SP-2] -> M
                res.write("M=M-1\n")    # SP--
                res.write("A=M\n")
                
                if instruction[0] == "add":
                    res.write("D=D+M\n")
                elif instruction[0] == "sub":
                    res.write("D=M-D\n")    # D = x - y
                elif instruction[0] == "and":
                    res.write("D=D&M\n")
                elif instruction[0] == "or":
                    res.write("D=D|M\n")
                # if lt, gt
                else:
                    res.write("D=D-M\n")    # D = y-x
                    res.write("@TRUE{}\n".format(ElseExist))
                    if instruction[0] == "eq":      # D = 0
                        res.write("D;JEQ\n")
                    elif instruction[0] == "gt":    # if x > y, 0 > y-x = D < 0
                        res.write("D;JLT\n")
                    elif instruction[0] == "lt":    # if x < y, y-x > 0 = D > 0
                        res.write("D;JGT\n")
                    res.write("D=0\n")
                    res.write("@END{}\n".format(ElseExist))
                    res.write("0;JMP\n")
                    res.write("(TRUE{})\n".format(ElseExist))
                    res.write("    D=-1\n")
                    res.write("(END{})\n".format(ElseExist))
                    res.write("    @SP\n")
                    res.write("    A=M\n")
                    res.write("    M=D\n")      # RAM[SP] = D
                    res.write("    @SP\n")      # SP++
                    res.write("    M=M+1\n")
                    ElseExist += 1
                    continue

                res.write("@SP\n")
                res.write("A=M\n")
                res.write("M=D\n")      # RAM[SP] = D
                res.write("@SP\n")      # SP++
                res.write("M=M+1\n")
