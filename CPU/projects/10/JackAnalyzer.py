import sys  # sys.argv
import os   # os.scandir

# Usage: python JackAnalyzer.py [fileName.jack or folderName]

# analyze jack file, parse code and copy to .xml file
keyword = ['int', 'char', 'boolean', 'void', 'true', 'false', 'null', 
           'this']

# except '(', ')' to process in detail
symbol = ['{', '}', '[', ']', '.', ',', ';', '+', '-', '*', '/', '&', 
          '|', '<', '>', '=', '~', ')', '('] 

def replace(parsed_code, code):
    return parsed_code.replace('{}', code)

def analyze(jackFile):
    global keyword, symbol
    parsed_code = ''    # save parsed codes
    parentheses = 0     # 0 is default (non)
                        # 1 is 'used in subroutineDec'
    
    varOn = 0           # 1인 경우 var 안에 진입한 것

    with open(jackFile, 'r') as f:
        lines = f.readlines()

        for line in lines:
            code = line.strip().split('\n')
            if "/" in code[0] or code[0] == '' :
                continue
            
            print(code)
            """
            for parse in code:
                # tokenizer
                # keyword
                if parse in keyword:
                    # must add {} for next parsed code
                    parsed_code = replace(parsed_code, "\t<keyword> " + parse + " </keyword>\n" + "{}")
                
                # symbol
                elif parse in symbol:
                    parsed_code = replace(parsed_code, "\t<symbol> " + parse + " </symbol>\n" + "{}")

                # process program structure
                # class
                elif parse == "class":
                    parsed_code += "<class>\n"
                    parsed_code += "\t<keyword> class </keyword>\n"
                    parsed_code += "{}\n"
                    parsed_code += "</class>"
                
                # classVarDec
                elif parse in ['static', 'field']:
                    parsed_code = replace(parsed_code, "\t<classVarDec>\n" + "\t<keyword> " + parse + " </keyword>\n" + "{}\n" + "</classVarDec>")
                
                # subroutineDec
                elif parse in ['constructor', 'function', 'method']:
                    parsed_code = replace(parsed_code, "\t<subroutineDec>\n" + "\t<keyword> " + parse + " </keyword>\n" + "{}\n" + "</subroutineDec>")
                
                # varDec
                elif parse == 'var':
                    if varOn == 1:
                        parsed_code = replace(parsed_code, "</varDec>\n" + "{}")
                    parsed_code = replace(parsed_code, "\t<varDec>\n" + "\t<keyword> var </keyword>\n" + "{}")
                
                # identifier, integerConstant, StringConstant
                else:
                    if parse.isalpha() == True:
                        parsed_code = replace(parsed_code, "\t<identifier> " + parse + " </identifier>\n" + "{}")
                    elif parse.isnumeric() == True:
                        parsed_code = replace(parsed_code, "\t<integerConstant> " + parse + " </integerConstant>\n" + "{}")
                
"""
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
