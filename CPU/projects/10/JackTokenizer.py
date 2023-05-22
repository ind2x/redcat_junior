import re

COMMENT = "(//.*)|(/\*([^*]|[\r\n]|(\*+([^*/]|[\r\n])))*\*+/)"

KEYWORD_SPACE_PATTERN = re.compile('^\s*(class|constructor|function|method|field|static|var|int|char|boolean|void|true|false|null|this|let|do|if|else|while|return)\s+')
KEYWORD_NONSPACE_PATTERN = re.compile('^\s*(true|false|null|this|return)\s*')
SYMBOL_PATTERN = re.compile('^\s*([{}()[\].,;+\-*/&|<>=~])\s*')
INT_PATTERN = re.compile('^\s*(\d+)\s*')
STR_PATTERN = re.compile('^s*\"(.*)\"\s*')
IDENTIFIER_PATTERN = re.compile('^\s*([a-zA-Z_][a-zA-Z1-9_]*)\s*')

class JackTokenizer:
    #keyword = ['class', 'constructor', 'function', 'method', 'field', 'static', 'var', 'int', 'char', 'boolean', 'void', 'true', 'false', 'null', 'this', 'let', 'do', 'if', 'else', 'while', 'return']

    #symbol = ['{', '}', '(', ')', '[', ']', '.', ',', ';', '+', '-', '*', '/', '&', '|', '<', '>', '=', '~']

    TYPE_KEYWORD = 0
    TYPE_SYMBOL = 1
    TYPE_INT = 2
    TYPE_STR = 3
    TYPE_IDENT = 4

    def __init__(self, JackFile):
        with open(JackFile, 'r') as f:
            self._code = re.sub(COMMENT,"", f.read())
            self._tokenType = None
            self._currentToken = None

    def hasMoreTokens(self):
        if self._code == '': # if code ends, exit
            return False
        else:
            return True
    
    def tokenType(self):
        return self._tokenType
    
    def keyword(self):
        if self.tokenType() == self.TYPE_KEYWORD:
            return self._currentToken
    
    def symbol(self):
        if self.tokenType() == self.TYPE_SYMBOL:
            return self._currentToken
    
    def identifier(self):
        if self.tokenType() == self.TYPE_IDENT:
            return self._currentToken

    def intVal(self):
        if self.tokenType() == self.TYPE_INT:
            return self._currentToken

    def stringVal(self):
        if self.tokenType() == self.TYPE_STR:
            return self._currentToken
    
    def advance(self):
        if self.hasMoreTokens() == True:
            currentToken = KEYWORD_SPACE_PATTERN.match(self._code)
            if currentToken != None:
                self._currentToken = currentToken.group(1)
                self._code = re.sub(KEYWORD_SPACE_PATTERN, '', self._code)
                self._tokenType = self.TYPE_KEYWORD
            else:
                currentToken = KEYWORD_NONSPACE_PATTERN.match(self._code)
                if currentToken != None:
                    self._currentToken = currentToken.group(1)
                    self._code = re.sub(KEYWORD_NONSPACE_PATTERN, '', self._code)
                    self._tokenType = self.TYPE_KEYWORD
                else:
                    currentToken = SYMBOL_PATTERN.match(self._code)
                    if currentToken != None:
                        self._currentToken = currentToken.group(1)
                        self._code = re.sub(SYMBOL_PATTERN, '', self._code)
                        self._tokenType = self.TYPE_SYMBOL
                    else:
                        currentToken = INT_PATTERN.match(self._code)
                        if currentToken != None:
                            self._currentToken = currentToken.group(1)
                            self._code = re.sub(INT_PATTERN, '', self._code)
                            self._tokenType = self.TYPE_INT
                        else:
                            currentToken = STR_PATTERN.match(self._code)
                            if currentToken != None:
                                self._currentToken = currentToken.group(1)
                                self._code = re.sub(STR_PATTERN, '', self._code)
                                self._tokenType = self.TYPE_STR
                            else:
                                currentToken = IDENTIFIER_PATTERN.match(self._code)
                                if currentToken != None:
                                    self._currentToken = currentToken.group(1)
                                    self._code = re.sub(IDENTIFIER_PATTERN, '', self._code)
                                    self._tokenType = self.TYPE_IDENT

'''
a = JackTokenizer('./ArrayTest/Main.jack')
while a.hasMoreTokens():
    a.advance()
    print(a._currentToken)
'''
