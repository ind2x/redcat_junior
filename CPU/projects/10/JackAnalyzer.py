import sys  # sys.argv
import os   # os.scandir
import re

# Usage: python JackAnalyzer.py [fileName.jack or folderName]

# analyze jack file, parse code and copy to .xml file
# lexical elements (tokens)
keyword = ['class', 'constructor', 'function', 'method', 'field', 'static', 'var', 
           'int', 'char', 'boolean', 'void', 'true', 'false', 'null', 'this', 'let', 'do', 'if', 'else', 'while', 'return']

symbol = ['{', '}', '(', ')', '[', ']', '.', ',', ';', '+', '-', '*', '/', '&', 
          '|', '<', '>', '=', '~'] 

def replace(parsed_code, code):
    return parsed_code.replace('$$', code, 1)

# <, >, ", and & are outputted as &lt;, &gt;, &quot;, and &amp;
def tokenizer(parsed_code, code):
    if code in keyword:
        return replace(parsed_code, "<keyword> {} </keyword>\n$$".format(code))
    elif code in symbol:
        if code == '<':
            return replace(parsed_code, "<symbol> {} </symbol>\n$$".format("&lt;"))
        elif code == '>':
            return replace(parsed_code, "<symbol> {} </symbol>\n$$".format("&gt;"))
        # if symbol is ';' or '}', then line or code ends
        elif code == '}':
            return replace(parsed_code, "<symbol> {} </symbol>\n".format(code))
        else:
            return replace(parsed_code, "<symbol> {} </symbol>\n$$".format(code))
    else:
        if code.isnumeric():
            return replace(parsed_code, "<integerConstant> {} </integerConstant>\n$$".format(code))
        else:
            return replace(parsed_code, "<identifier> {} </identifier>\n$$".format(code))

# compile expressions
#def expression(parsed_code, code):
    # expression == term (op term)*



def analyze(jackFile):
    global keyword, symbol
    parsed_code = ''    # save parsed codes

    with open(jackFile, 'r') as f:
        lines = f.readlines()

        for line in lines:
            code = line.strip().split('\n')
            if "/" in code[0] or code[0] == '' :
                continue
            
            # compile class structure
            if code[0].find('class') == 0:
                code = ''.join(code).split(' ')
                parsed_code += "<class>\n"
                parsed_code += "$$\n"
                parsed_code += "</class>"
                for token in code:
                    parsed_code = tokenizer(parsed_code, token)

            # compile subroutineDec structure
            elif code[0].find('constructor') == 0 or code[0].find('function') == 0 or code[0].find('method') == 0:
                parsed_code = replace(parsed_code, "<subroutineDec>\n$$</subroutineDec>")

                # extract keyword, type
                code = re.search('(.+?)\s(.+?)\s(.+?)\s({)', code[0])
                parsed_code = tokenizer(parsed_code, code.group(1))
                parsed_code = tokenizer(parsed_code, code.group(2))

                # subroutineName, par
                etc = re.search('(.+?)\(|(.+?)\)', code.group(3))
                
                subroutineName = etc.group(1)
                parsed_code = tokenizer(parsed_code, '(')
                parsed_code = tokenizer(parsed_code, subroutineName)

                # add <parameterList>
                parsed_code = replace(parsed_code, "<parameterList>\n$$")
                par = etc.group(2)

                # if multiple parameters
                # if no any par
                if par == None:
                    pass
                elif ',' in par:
                    par = par.strip().split(',')
                    for idx, token in enumerate(par):
                        parsed_code = tokenizer(parsed_code, token)
                        if idx == len(par)-1:
                            continue
                        parsed_code = tokenizer(parsed_code, ',')
                else:
                    parsed_code = tokenizer(parsed_code, par)
                
                parsed_code = replace(parsed_code, "</parameterList>\n$$")
                parsed_code = tokenizer(parsed_code, ')')

                # add <subroutineBody>
                parsed_code = replace(parsed_code, "<subroutineBody>\n$$</subroutineBody>\n")

                # '{'
                parsed_code = tokenizer(parsed_code, code[1].split()[0])

            # classVarDec and varDec
            elif code[0].find('static') == 0 or code[0].find('field') == 0 or code[0].find('var') == 0:
                varOn = 0   # check if static, field or var
                if code[0].find('static') == 0 or code[0].find('field') == 0:
                    parsed_code = replace(parsed_code, "<classVarDec>\n$$")
                else:
                    parsed_code = replace(parsed_code, "<varDec>\n$$")
                    varOn = 1
                
                # extract keyword, type
                code = re.search('(.+?)\s(.+?)\s(.+?)(;)', code[0])
                parsed_code = tokenizer(parsed_code, code.group(1))
                parsed_code = tokenizer(parsed_code, code.group(2))

                # if there is multiple varNames
                if ',' in code.group(3):
                    par = code.group(3).strip().split(',')
                    for idx, token in enumerate(par):
                        parsed_code = tokenizer(parsed_code, token)
                        if idx == len(par)-1:
                            continue
                        parsed_code = tokenizer(parsed_code, ',')
                else:
                    parsed_code = tokenizer(parsed_code, code.group(3))

                parsed_code = tokenizer(parsed_code, code.group(4))

                if varOn == 1:
                    parsed_code = replace(parsed_code, "</varDec>\n$$")
                else:
                    parsed_code = replace(parsed_code, "</classVarDec>\n$$")
            
            # compile statements
            if code[0].find('let') == 0 or code[0].find('if') == 0 or code[0].find('while') == 0 or code[0].find('do') == 0 or code[0].find('return') == 0 :
                parsed_code = replace(parsed_code, "<statements>\n$$")

                # compile let statement
                if code[0].find('let') == 0:
                    code = re.search('(.+?)\s(.+?|\[.+?\])\s(=)\s(.+?)(;)', code[0])
                    
                    # parse keyword, varName
                    parsed_code = tokenizer(parsed_code, code.group(1))
                    if '[' in code.group(2):
                        parsed_code  = tokenizer(parsed_code, '[')
                        indexName = re.search('\[(.+?)\]', code.group(2))
                        parsed_code = tokenizer(parsed_code, indexName)
                        parsed_code  = tokenizer(parsed_code, ']')
                    
                    # parse '='
                    parsed_code = tokenizer(parsed_code, code.group(3))

                    # parse expression
                    #parsed_code = expression()

    # make file "fileName_res.xml" and copy parsed code to result file
    with open(jackFile[:-5]+'_res.xml', 'w') as res:
        res.write(parsed_code)


# if argv[1] is file, translate
if ".jack" in sys.argv[1]:
    analyze(sys.argv[1])
else:
    # Search jack files in directory
    JackFiles = []
    with os.scandir(sys.argv[1]) as files:
        for file in files:
            if file.is_file() and file.name[-5:] == ".jack":
                JackFiles.append(file.name)

    for File in JackFiles:
        analyze(sys.argv[1] + File)
