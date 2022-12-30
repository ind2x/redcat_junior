## Project 7

<br>

High-level 언어에서 컴파일을 거치면 VM code (바이트 코드)가 나온다.

이 VM 코드를 Machine Language로 변환해주는 VM Translator를 이번 장과 8장에 걸쳐서 만든다.

이번 장에서는 논리적/산술 연산자까지만 변환해준다.

또한 POP 연산자는 push 연산자에 비해 복잡한 구조여서 블로그를 참조하였다.

<br>

pop implementation: https://evoniuk.github.io/posts/nand.html 

<br>

```python
import sys 

'''
SP = RAM[0], LCL = RAM[1], ARG = RAM[2], THIS = RAM[3], THAT = RAM[4]
stack = RAM[256] ~ 
static = RAM[16] ~ RAM[255]

command: 
push/pop segment i, (i >= 0)
add, sub, neg
eq, gt, lt
and, or, not

label, goto, if-goto
Function, Call, return

memory segment:
local, argument, this, that, constant, static, pointer, temp
'''

with open(sys.argv[1], 'r') as f:
    lines = f.readlines()
    SP = 256    # stack starts at RAM[256]

    with open('./'+sys.argv[1].replace('.vm', '.asm'), 'w') as res:
        MemorySegment = {'local':'LCL', "argument":"ARG", "this":"THIS", "that":"THAT", "static":16, "temp":5}
        Pointer = ["THIS","THAT"]
        # count how many ['eq', 'gt', 'lt'] is used 
        ElseExist = 0

        for line in lines:
            instruction = line.strip().split(' ')
            if instruction[0] == "//" or instruction[0] == '':
                continue
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
                    if instruction[1] == "temp" or instruction[1] == "static":
                        res.write("D=D+A\n")
                        res.write("A=D\n")
                        res.write("D=M\n")
                    else:
                        res.write("D=D+M\n")    # D = i + LCL, ... ~
                        res.write("A=D\n")      # A = i + LCL, ... ~
                        res.write("D=M\n")      # D = RAM[i+LCL] 

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
                    if instruction[1] == "temp" or instruction[1] == "static":
                        res.write("D=D+A\n")    # D = RAM[SP] + i + 5(temp)
                    else:
                        res.write("D=D+M\n")    # D = RAM[SP] + i + LCL, ..., ~
                    
                elif instruction[1] == "pointer":
                    res.write("@{}\n".format(Pointer[int(instruction[2])]))
                    res.write("D=D+A\n")    # D = RAM[THIS] + RAM[SP] 

                res.write("@SP\n")
                res.write("A=M\n")      # A = SP
                res.write("A=M\n")      # A = RAM[SP]
                res.write("A=D-A\n")    # A = i + LCL
                res.write("M=D-A\n")    # RAM[i+LCL] = RAM[SP]

            else:
                res.write("@SP\n")
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
                
                res.write("D=M\n")      # D = RAM[SP]
                
                res.write("@SP\n")
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
```

<br><br>
<hr style="border: 2px solid;">
<br><br>
