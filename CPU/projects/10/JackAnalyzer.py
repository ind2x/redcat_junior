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

# compile expressionList
def expressionList(parsed_code, code):
    # expressionList
    parsed_code = replace(parsed_code, "<expressionList>\n$$")
        
    # if parameters exist --> expressionList
    if code != '':
        # if multiple parameters
        if ',' in code:
            exprl = code.split(', ')
            for idx, par in enumerate(exprl):
                parsed_code = replace(parsed_code, "<expression>\n$$")
                parsed_code = expression(parsed_code, par)
                parsed_code = replace(parsed_code, "</expression>\n$$")
                if idx == len(exprl)-1:
                    continue
                parsed_code = tokenizer(parsed_code, ',')
        # if one parameter
        else:
            parsed_code = replace(parsed_code, "<expression>\n$$")
            parsed_code = expression(parsed_code, code)
            parsed_code = replace(parsed_code, "</expression>\n$$")

    parsed_code = replace(parsed_code, "</expressionList>\n$$")

    return parsed_code

# compile subroutineCall if not expression
def subroutineCall(parsed_code, code, type):
    # subroutineCall - subroutineName '(' expressionList ')'
    if type == 1:
        code = re.search('(.+)\((.?|.+)\)', code)
        
        parsed_code = tokenizer(parsed_code, code.group(1))
        parsed_code = tokenizer(parsed_code, '(')
        parsed_code = expressionList(parsed_code, code.group(2))
        parsed_code = tokenizer(parsed_code, ')')
    
    # subroutineCall - (className|varName)'.'subroutineName '(' expressionList ')'
    if type == 2:
        code = re.search('(.+)\.(.+)\((.?|.+)\)', code)

        parsed_code = tokenizer(parsed_code, code.group(1))
        parsed_code = tokenizer(parsed_code, '.')
        parsed_code = tokenizer(parsed_code, code.group(2))
        parsed_code = tokenizer(parsed_code, '(')
        # expressionList
        parsed_code = expressionList(parsed_code, code.group(3))
        parsed_code = tokenizer(parsed_code, ')')
    
    return parsed_code

# compile expressions
def expression(parsed_code, code):
    
    # integerConstant
    if code.isnumeric():
        parsed_code = replace(parsed_code, "<term>\n<integerConstant> {} </integerConstant>\n</term>\n$$".format(code))
    
    # keywordConstant
    elif code in ['true', 'false', 'null', 'this']:
        parsed_code = replace(parsed_code, "<term>\n<keywordConstant> {} </keywordConstant>\n</term>\n$$".format(code))

    # stringConstant
    elif code[0] == '"':
        parsed_code = replace(parsed_code, "<term>\n<stringConstant> {} </stringConstant>\n</term>\n$$".format(code[1:-1]))
    
    # '(' expression ')'
    elif code[0] == '(':
        code = re.search('\((.+)\)', code)
        parsed_code = tokenizer(parsed_code, '(')

        parsed_code = replace(parsed_code, "<expression>\n$$")
        # expression
        parsed_code = expression(parsed_code, code.group(1))
        parsed_code = replace(parsed_code, "</expression>\n$$")
        
        parsed_code = tokenizer(parsed_code, ')')

    # subroutineCall - subroutineName '(' expressionList ')'
    elif re.search('(.+)\((.?|.+)\)', code):
        parsed_code = replace(parsed_code, "<term>\n$$")
        parsed_code = subroutineCall(parsed_code, code, 1)
        parsed_code = replace(parsed_code, "</term>\n$$")
        
    # subroutineCall - (className|varName)'.'subroutineName '(' expressionList ')'
    elif re.search('(.+)\.(.+)\((.?|.+)\)', code):
        # add <term>
        parsed_code = replace(parsed_code, "<term>\n$$")
        parsed_code = subroutineCall(parsed_code, code, 2)
        parsed_code = replace(parsed_code, "</term>\n$$")
    
    # varName'[' expression ']'
    elif '[' in code:
        # split to varName, indexName
        code = re.search('(.+?)\[(.+?)\]', code)
        # parse varName
        parsed_code = tokenizer(parsed_code, code.group(1))

        # parse '[', 'expr', ']'
        parsed_code  = tokenizer(parsed_code, '[')

        # add <expression> --> varName[expression]
        parsed_code = replace(parsed_code, "<expression>\n$$")
        parsed_code = expression(parsed_code, code.group(2))
        parsed_code = replace(parsed_code, "</expression>\n$$")

        parsed_code  = tokenizer(parsed_code, ']')

    # if op exists
    elif code in ['+', '-', '*', '/', '&', '|', '<', '>', '=']:
        # split to "term (op term)"
        expr = re.search('(.+?)\s(.+?)\s(\(.+?\))', code)
        
        # expression term1
        parsed_code = expression(parsed_code, expr.group(1))
        # parse op
        parsed_code = tokenizer(parsed_code, expr.group(2))
        # expression term2
        parsed_code = expression(parsed_code, expr.group(3))

    # varName
    else:
        parsed_code = replace(parsed_code, "<term>\n$$")
        parsed_code = tokenizer(parsed_code, code)
        parsed_code = replace(parsed_code, "</term>\n$$")

    return parsed_code

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

                # split to keyword, type, subroutineName, parameter, '{'
                code = re.search('(.+)\s(.+)\s(.+)\((.?|.+)\)\s({)', code[0])
                # parse keyword, type, subroutineName
                parsed_code = tokenizer(parsed_code, code.group(1))
                parsed_code = tokenizer(parsed_code, code.group(2))
                parsed_code = tokenizer(parsed_code, code.group(3))
                parsed_code = tokenizer(parsed_code, '(')

                # add <parameterList>
                parsed_code = replace(parsed_code, "<parameterList>\n$$")
                
                # check if parameters exist or not
                par = code.group(4)
                # if parameters exist
                if par != '':
                    # if multiple parameters
                    if ',' in par:
                        par = par.split(', ')
                        for idx, token in enumerate(par):
                            # split to type, varName
                            token = token.split(' ')
                            parsed_code = tokenizer(parsed_code, token[0])
                            parsed_code = tokenizer(parsed_code, token[1])

                            if idx == len(par)-1:
                                continue
                            parsed_code = tokenizer(parsed_code, ',')        
                    # if one parameter
                    else:
                        par = par.split(' ')
                        parsed_code = tokenizer(parsed_code, par[0])
                        parsed_code = tokenizer(parsed_code, par[1])
                
                parsed_code = replace(parsed_code, "</parameterList>\n$$")
                parsed_code = tokenizer(parsed_code, ')')

                # add <subroutineBody>
                parsed_code = replace(parsed_code, "<subroutineBody>\n$$</subroutineBody>\n$$")

                # '{'
                parsed_code = tokenizer(parsed_code, code.group(5))

            # classVarDec and varDec
            elif code[0].find('static') == 0 or code[0].find('field') == 0 or code[0].find('var') == 0:
                varOn = 0   # check if static, field or var
                if code[0].find('static') == 0 or code[0].find('field') == 0:
                    parsed_code = replace(parsed_code, "<classVarDec>\n$$")
                else:
                    parsed_code = replace(parsed_code, "<varDec>\n$$")
                    varOn = 1
                
                # extract keyword, type
                code = re.search('(.+?)\s(.+?)\s(.+)(;)', code[0])
                parsed_code = tokenizer(parsed_code, code.group(1))
                parsed_code = tokenizer(parsed_code, code.group(2))

                # if there is multiple varNames
                if ',' in code.group(3):
                    par = code.group(3).split(', ')
                    for idx, token in enumerate(par):
                        parsed_code = tokenizer(parsed_code, token)
                        if idx == len(par)-1:
                            continue
                        parsed_code = tokenizer(parsed_code, ',')
                else:
                    parsed_code = tokenizer(parsed_code, code.group(3))

                # parse ';'
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
                    parsed_code = replace(parsed_code, "<letStatement>\n$$")
                    code = re.search('(.+)\s(.+)\s(=)\s(.+?)(;)', code[0])
                    
                    # parse keyword, varName
                    parsed_code = tokenizer(parsed_code, code.group(1))
                    
                    # if varName[indexName] = 'expr'
                    if '[' in code.group(2):
                        parsed_code = expression(parsed_code, code.group(2))
                    else:
                        parsed_code = tokenizer(parsed_code, code.group(2))
                    
                    # parse '='
                    parsed_code = tokenizer(parsed_code, code.group(3))

                    # add and parse <expression>
                    parsed_code = replace(parsed_code, "<expression>\n$$")
                    parsed_code = expression(parsed_code, code.group(4))
                    parsed_code = replace(parsed_code, "</expression>\n$$")

                    # parse ';'
                    parsed_code = tokenizer(parsed_code, code.group(5))
                    parsed_code = replace(parsed_code, "</letStatement>\n$$")
                '''
                # if statement 
                elif code[0].find('if') == 0:
                    # add <ifstatement>
                    parsed_code = replace(parsed_code, "<ifStatement>\n$$")

                    expr = re.search('if\((.+)\)', code[0])

                    # parse if and '('
                    parsed_code = tokenizer(parsed_code, 'if')
                    parsed_code = tokenizer(parsed_code, '(')

                    # expression
                    parsed_code = expression(parsed_code, expr)

                # while statement
                elif code[0].find('while') == 0:
                    parsed_code = replace(parsed_code, "<whileStatement>\n$$")
                    code = re.search('(while)', code[0])

                # if do statement
                elif code[0].find('do') == 0:
                    parsed_code = replace(parsed_code, "<doStatement>\n$$")
                    # ['do', 'subroutineCall', 'parameters', ';']
                    code = re.search('(do)\s(.+)\((.?|.+)\)(;)', code[0])

                    # parse 'do'
                    parsed_code = tokenizer(parsed_code, code.group(1))
                    parsed_code = expression
'''

                parsed_code = replace(parsed_code, "</statements>\n$$")

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
