import sys      # sys.argv
import string   # isnumeric, islower, index

'''
Need to handle: 
1. White space == Empty lines, Comments, Indentation -> ignore
2. Instruction == A or C
3. Symbols == predefined symbols, Label, variables
'''

# Computation Specification       
def comp(comp):
    comp_bit = ['101010', '111111', '111010', '001100', '110000', '001101', '110001', '001111', 
                '110011', '011111', '110111', '001110', '110010', '000010', '010011', 
                '000111', '000000', '010101']
    
    a0 = ['0', '1', '-1', 'D', 'A', '!D', '!A', '-D', '-A', 'D+1', 
          'A+1', 'D-1', 'A-1', 'D+A', 'D-A', 'A-D', 'D&A', 'D|A']
    
    a1 = {'M':4, '!M':6, '-M':8, 'M+1':10, 'M-1':12, 'D+M':13, 
          'D-M':14, 'M-D':15, 'D&M':16, 'D|M':17}

    if comp in a0:
        return '0' + comp_bit[a0.index(comp)]
    else :
        return '1' + comp_bit[a1[comp]]

# Destination Specification
def dest(dest):
    dest_field = ['M', 'D', 'MD', 'A', 'AM', 'AD', 'AMD']
    return '{0:03b}'.format(dest_field.index(dest)+1)

# Jump Specificaiton
def jump(jump):
    jump_field = ['JGT', 'JEQ', 'JGE', 'JLT', 'JNE', 'JLE', 'JMP']
    return '{0:03b}'.format(jump_field.index(jump)+1)

# predefined symbols
symbol_table = {'R0':0, 'R1':1, 'R2':2, 'R3':3, 'R4':4, 'R5':5, 'R6':6, 'R7':7, 'R8':8, 
                'R9':9, 'R10':10, 'R11':11, 'R12':12, 'R13':13, 'R14':14, 'R15':15, 
                'SCREEN':16384, 'KBD':24576, 
                'SP':0, 'LCL':1, 'ARG':2, 'THIS':3, 'THAT':4}

# label이 있는 경우 label의 instruction 위치 값을 저장
label = 0
# 변수는 instruction 16부터 시작
variable = 16

with open(sys.argv[1],'r') as f:
    lines = f.readlines()
    instructions=[]     # instructions
    n=0     # program line

    # read the program lines, one by one focusing only on declarations
    # add the found labels to the symbol table
    for line in lines:
        line = line.strip().split(' ')[0]
        # if White Space -> ignore
        if line[0:2] == "//" or line == '\n' or line == '':   
            continue
        
        # if variable, add on symbol table
        if line[0] == '@':
            if not line[1:].isnumeric() and line[1:].islower() and line[1:] not in symbol_table:
                symbol_table[line[1:]] = variable
                variable += 1
        
        # if lable, add on symbol table and not append in instructions
        if line[0] == '(':
            symbol_table[line[1:-1]] = n
            continue
        
        n += 1
        instructions.append(line)
        #print(line)
    
    with open("./"+ sys.argv[1].replace('asm', 'hack'), 'w') as res:
        # translate instructions to 16-bit binary code
        for cmd in instructions:
            # if A-instruction or symbol
            bit='0'
            if cmd[0] == '@':
                # if A-instruction, translate it to 16-bit
                if cmd[1:].isnumeric():
                    bit += '{0:015b}'.format(int(cmd[1:]))
                # if variable or label, find it in symbol table
                else:
                    bit += '{0:015b}'.format(symbol_table[cmd[1:]])
                
            # if C-instruction
            else:
                bit = '111'
                # if ';' is omitted
                if '=' in cmd:
                    d = dest(cmd.split('=')[0])
                    c = comp(cmd.split('=')[1])
                    j = '000'
                else :
                    d = '000'
                    c = comp(cmd.split(';')[0])
                    j = jump(cmd.split(';')[1])

                bit += c + d + j
            
            res.write(bit+'\n')
