import sys  # sys.argv
import os   # os.scandir
import re   # re.search

# Usage: python JackAnalyzer.py [fileName.jack or folderName]

# analyze jack file, parse code and copy to .xml file
# lexical elements (tokens)
keyword = ['class', 'constructor', 'function', 'method', 'field', 'static', 'var', 
           'int', 'char', 'boolean', 'void', 'true', 'false', 'null', 'this', 'let', 'do', 'if', 'else', 'while', 'return']

symbol = ['{', '}', '(', ')', '[', ']', '.', ',', ';', '+', '-', '*', '/', 
          '&', '|', '<', '>', '=', '~']

# check which close tag must i use before or after(class) parse '}'
classOn = subroutineBodyOn = 0
IfElseWhile = []
isOp = 0    # if op exists, do not add '<term>' tag double

def replace(parsed_code, code):
    return parsed_code.replace('$$', code, 1)

# <, >, ", and & are outputted as &lt;, &gt;, &quot;, and &amp;
def tokenizer(parsed_code, code):
    global classOn, subroutineBodyOn, IfElseWhile

    if code in keyword:
        return replace(parsed_code, "<keyword> {} </keyword>\n$$".format(code))
    elif code in symbol:
        if code == '<':
            return replace(parsed_code, "<symbol> {} </symbol>\n$$".format("&lt;"))
        elif code == '>':
            return replace(parsed_code, "<symbol> {} </symbol>\n$$".format("&gt;"))
        # if symbol is ';' or '}', then line or code ends
        elif code == '}':
            if len(IfElseWhile) != 0:
                statement = IfElseWhile.pop()
                parsed_code = replace(parsed_code, "</statements>\n$$")
                parsed_code = replace(parsed_code, "<symbol> } </symbol>\n$$")

                if statement == 'while':
                    parsed_code = replace(parsed_code, "</whileStatement>\n$$")
                else: # if, else
                    parsed_code = replace(parsed_code, "</ifStatement>\n$$")

            elif subroutineBodyOn:
                subroutineBodyOn = 0
                # add </statements>\n<symbol> } </symbol>\n
                # </subroutineBody>\n</subroutineDec>\n
                parsed_code = replace(parsed_code, "</statements>\n$$")
                parsed_code = replace(parsed_code, "<symbol> } </symbol>\n$$")
                parsed_code = replace(parsed_code, "</subroutineBody>\n$$")
                parsed_code = replace(parsed_code, "</subroutineDec>\n$$")
            
            elif classOn:
                classOn = 0
                parsed_code = replace(parsed_code, "<symbol> } </symbol>\n$$")
                parsed_code = replace(parsed_code, "</class>")

            return parsed_code
        else:
            return replace(parsed_code, "<symbol> {} </symbol>\n$$".format(code))    
    else:
        if code.isnumeric():
            return replace(parsed_code, "<integerConstant> {} </integerConstant>\n$$".format(code))
        else:
            return replace(parsed_code, "<identifier> {} </identifier>\n$$".format(code))

# compile expressionList
def expressionList(parsed_code, code):
    global isOp
    preisOp = isOp
    if preisOp == 1:
        isOp = 0

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

    if preisOp == 1:
        isOp = 1
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
    global isOp

    # '(' expression ')'
    if code[0] == '(':
        code = re.search('\((.+)\)', code)

        parsed_code = tokenizer(parsed_code, '(')
        
        # expression
        parsed_code = replace(parsed_code, "<expression>\n$$")
        parsed_code = expression(parsed_code, code.group(1))
        parsed_code = replace(parsed_code, "</expression>\n$$")
        
        parsed_code = tokenizer(parsed_code, ')')
    
    # if op exists
    elif '+' in code or '-' in code or '*' in code or '/' in code or '&' in code or '|' in code or '<' in code or '>' in code or '=' in code:
        # if unaryOp term
        if code[0] == '-' or code[0] == '~':
            code = re.search('(-|~)(.+)', code)

            # <term> unaryOp term </term>
            parsed_code = replace(parsed_code, "<term>\n$$")
            
            # unaryOp
            parsed_code = tokenizer(parsed_code, code.group(1))
            # term
            parsed_code = replace(parsed_code, "<term>\n$$")
            parsed_code = tokenizer(parsed_code, code.group(2))
            parsed_code = replace(parsed_code, "</term>\n$$")

            parsed_code = replace(parsed_code, "</term>\n$$")
        else: 
            isOp = 1

            # split to "term (op term)"
            expr = re.search('(.+)\s(.?)\s(.+)', code)
            
            # expression term1
            parsed_code = replace(parsed_code, "<term>\n$$")
            parsed_code = expression(parsed_code, expr.group(1))
            parsed_code = replace(parsed_code, "</term>\n$$")

            # parse op
            parsed_code = tokenizer(parsed_code, expr.group(2))
            
            # expression term2
            parsed_code = replace(parsed_code, "<term>\n$$")
            parsed_code = expression(parsed_code, expr.group(3))
            parsed_code = replace(parsed_code, "</term>\n$$")

            isOp = 0
    
    # integerConstant
    elif code.isnumeric():
        if isOp == 0:
            parsed_code = replace(parsed_code, "<term>\n<integerConstant> {} </integerConstant>\n</term>\n$$".format(code))
        else:
            parsed_code = replace(parsed_code, "<integerConstant> {} </integerConstant>\n$$".format(code))

    # stringConstant
    elif code[0] == '"':
        parsed_code = replace(parsed_code, "<term>\n<stringConstant> {} </stringConstant>\n</term>\n$$".format(code[1:-1]))
        
    # subroutineCall - (className|varName)'.'subroutineName '(' expressionList ')'
    elif re.search('(.+)(\.)(.+)\((.?|.+)\)', code):
        if isOp == 0:
            # add <term>
            parsed_code = replace(parsed_code, "<term>\n$$")
            parsed_code = subroutineCall(parsed_code, code, 2)
            parsed_code = replace(parsed_code, "</term>\n$$")
        else:
            parsed_code = subroutineCall(parsed_code, code, 2)
    
    # subroutineCall - subroutineName '(' expressionList ')'
    elif re.search('(.+)\((.?|.+)\)', code):
        if isOp == 0:
            parsed_code = replace(parsed_code, "<term>\n$$")
            parsed_code = subroutineCall(parsed_code, code, 1)
            parsed_code = replace(parsed_code, "</term>\n$$")
        else:
            parsed_code = subroutineCall(parsed_code, code, 1)
    
    # varName'[' expression ']'
    elif '[' in code:
        preisOp = isOp
        # split to varName, indexName
        code = re.search('(.+?)\[(.+?)\]', code)

        # add <term>
        parsed_code = replace(parsed_code, "<term>\n$$")

        # parse varName
        parsed_code = tokenizer(parsed_code, code.group(1))

        # parse '[', 'expr', ']'
        parsed_code  = tokenizer(parsed_code, '[')

        # add <expression> --> varName[expression]
        parsed_code = replace(parsed_code, "<expression>\n$$")
        if preisOp == 1:
            isOp = 0
        parsed_code = expression(parsed_code, code.group(2))
        if preisOp == 1:
            isOp = 1
        parsed_code = replace(parsed_code, "</expression>\n$$")

        parsed_code  = tokenizer(parsed_code, ']')

        parsed_code = replace(parsed_code, "</term>\n$$")

    # varName, true, flase, this, null
    else:
        if isOp == 0:
            parsed_code = replace(parsed_code, "<term>\n$$")
            parsed_code = tokenizer(parsed_code, code)
            parsed_code = replace(parsed_code, "</term>\n$$")
        else:
            parsed_code = tokenizer(parsed_code, code)

    return parsed_code

# compile classVarDec, varDec
def classVarDecOrvarDec(parsed_code, code):
    # split to keyword, type varName*, ';'
    code = re.search('(.+?)\s(.+?)\s(.+)(;)', code)

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

    return parsed_code

def analyze(jackFile):
    global keyword, symbol
    global classOn, subroutineBodyOn

    subroutineDecOn = 0 # use when compile subroutineBody
    parsed_code = ''    # save parsed codes

    with open(jackFile, 'r') as f:
        lines = f.readlines()

        for line in lines:
            code = line.strip().split('\n')
            if code[0] == '' or code[0][0] == '/' :
                continue
            if '//' in code[0]:
                code = code[0].split('//')[0].strip().split('\n')
            
            # if '}'
            if code[0] == '}':
                parsed_code = tokenizer(parsed_code, code[0])

            # compile class structure
            if code[0][:5] == 'class':
                # split to keyword, className, '{'
                code = ''.join(code).split(' ')
                parsed_code += "<class>\n$$"

                # add </class> after parse '}'
                classOn = 1
                for token in code:
                    parsed_code = tokenizer(parsed_code, token)

            # compile classVarDec
            elif code[0][:6] == 'static' or code[0][:5] == 'field':
                parsed_code = replace(parsed_code, "<classVarDec>\n$$")
                parsed_code = classVarDecOrvarDec(parsed_code, code[0])
                parsed_code = replace(parsed_code, "</classVarDec>\n$$")

            # compile subroutineDec structure
            elif code[0][:11] == 'constructor' or code[0][:8] == 'function' or code[0][:6] == 'method':
                parsed_code = replace(parsed_code, "<subroutineDec>\n$$")

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

                subroutineDecOn = 1
                # add <subroutineBody>
                parsed_code = replace(parsed_code, "<subroutineBody>\n$$")
                # '{'
                parsed_code = tokenizer(parsed_code, '{')
                subroutineBodyOn = 1

            # compile varDec
            elif code[0][:3] == "var":
                
                parsed_code = replace(parsed_code, "<varDec>\n$$")
                parsed_code = classVarDecOrvarDec(parsed_code, code[0])
                parsed_code = replace(parsed_code, "</varDec>\n$$")

            # if no varDec, add <statements>
            elif subroutineDecOn == 1:
                parsed_code = replace(parsed_code, "<statements>\n$$")
                subroutineDecOn = 0

            # compile let statement
            if code[0][:3] == 'let':
                parsed_code = replace(parsed_code, "<letStatement>\n$$")
                
                code = re.search('(.+)\s(.+)\s(=)\s(.+?)(;)', code[0])

                # parse 'let', varName
                parsed_code = tokenizer(parsed_code, code.group(1))
                
                # if varName[indexName] = 'expr'
                if '[' in code.group(2):
                    expr = re.search('(.+?)\[(.+?)\]', code.group(2))

                    # varName, '[' expr ']'
                    parsed_code = tokenizer(parsed_code, expr.group(1))
                    parsed_code = tokenizer(parsed_code, '[')
                    parsed_code = replace(parsed_code, "<expression>\n$$")
                    parsed_code = expression(parsed_code, expr.group(2))
                    parsed_code = replace(parsed_code, "</expression>\n$$")
                    parsed_code = tokenizer(parsed_code, ']')
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
            
            # compile if statement 
            # code[0] == if () {
            elif code[0][:2] == 'if':
                IfElseWhile.append('if')
                
                # add <ifstatement>
                parsed_code = replace(parsed_code, "<ifStatement>\n$$")
                parsed_code = tokenizer(parsed_code, 'if')
                
                # get expression
                code = re.search('if\s(\(.+?\))\s{', code[0])
                
                # '(' expression ')'
                parsed_code = expression(parsed_code, code.group(1))
                
                # '{' statements '}'
                parsed_code = tokenizer(parsed_code, "{")
                parsed_code = replace(parsed_code, "<statements>\n$$")
            
            # compile else
            # else {
            elif code[0][:4] == 'else':
                IfElseWhile.append('else')
                # replace "</ifstatement>" to "<keyword> else </keyword>"
                parsed_code = parsed_code[:-17] + "<keyword> else </keyword>\n$$"
                # parse '{'
                parsed_code = tokenizer(parsed_code, "{")
                # add <statements>
                parsed_code = replace(parsed_code, "<statements>\n$$")
            
            # compile while statement
            elif code[0][:5] == 'while':
                IfElseWhile.append('while')
                
                parsed_code = replace(parsed_code, "<whileStatement>\n$$")
                parsed_code = tokenizer(parsed_code, 'while')
                
                # get expression
                code = re.search('while\s(\(.+?\))\s{', code[0])
                
                # '(' expression ')'
                parsed_code = expression(parsed_code, code.group(1))
                parsed_code = tokenizer(parsed_code, '{')
                parsed_code = replace(parsed_code, "<statements>\n$$")
            
            # compile do statement
            elif code[0][:2] == 'do':
                parsed_code = replace(parsed_code, "<doStatement>\n$$")
                
                # ['do', 'subroutineCall', 'parameters', ';']
                code = re.search('do\s(.+)(;)', code[0])
                
                # parse 'do'
                parsed_code = tokenizer(parsed_code, "do")
                # subroutineCall - (className|varName)'.'subroutineName '(' expressionList ')'
                if re.search('(.+)(\.)(.+)\((.?|.+)\)', code.group(1)):
                    parsed_code = subroutineCall(parsed_code, code.group(1), 2)
                else:
                    parsed_code = subroutineCall(parsed_code, code.group(1), 1)
                # parse ';'
                parsed_code = tokenizer(parsed_code, code.group(2))
                # add </doStatement>
                parsed_code = replace(parsed_code, "</doStatement>\n$$")
                
            # compile return statement
            elif code[0][:6] == 'return':
                # add <returnStatement>
                parsed_code = replace(parsed_code, "<returnStatement>\n$$")
                
                # parse 'return'
                parsed_code = tokenizer(parsed_code, 'return')
                # if not return;
                if ' ' in code[0]:
                    code = re.search('return\s(.+?);', code[0])
                    parsed_code = expression(parsed_code, code.group(1))
                # parse ';'
                parsed_code = tokenizer(parsed_code, ';')
                # add </returnStatement>
                parsed_code = replace(parsed_code, "</returnStatement>\n$$")

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
