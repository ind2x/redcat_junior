import JackTokenizer

class CompilationEngine:
    def __init__(self, inputF, outputFile):
        self._tokenizer = JackTokenizer.JackTokenizer(inputF)
        self._xmlFile = open(outputFile, 'w+')
        
    UNARY_OP = ['-', '~']
    OP = ['+', '-', '*', '/', '&', '|', '<', '>', '=']
    
    def write(self, code):
        self._xmlFile.write(code)
    
    def advance(self):
        self._tokenizer.advance()
    
    def writeKeyword(self):
        self.write("<keyword> {} </keyword>\n".format(self._tokenizer.keyword()))
    
    def writeSymbol(self):
        symbol = self._tokenizer.symbol()
        if symbol == '<':
            symbol = "&lt;"
        elif symbol == '>':
            symbol = "&gt;"
        elif symbol == '&':
            symbol = "&amp;"
        self.write("<symbol> {} </symbol>\n".format(symbol))
    
    def writeInt(self):
        self.write("<integerConstant> {} </integerConstant>\n".format(self._tokenizer.intVal()))
    
    def writeStr(self):
        self.write("<stringConstant> {} </stringConstant>\n".format(self._tokenizer.stringVal()))

    def writeIdentifier(self):
        self.write("<identifier> {} </identifier>\n".format(self._tokenizer.identifier()))

    def write_type_and_varName(self):
        # write type
        if self._tokenizer.tokenType() == self._tokenizer.TYPE_KEYWORD:
            self.writeKeyword()     # int, char, boolean
        else:
            self.writeIdentifier()  # className
        self.advance()
        
        # write varName until meet ';'
        while 1:
            self.writeIdentifier() # write varName
            self.advance()
            
            if self._tokenizer.tokenType() == self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() != ',':
                break
            
            self.writeSymbol()
            self.advance()

    def compileClass(self):
        if self._tokenizer.hasMoreTokens() == True:
            self.advance()
            self.write("<class>\n")

            # write class
            self.writeKeyword()
            self.advance()
            # write className
            self.writeIdentifier()
            self.advance()
            # write '{'
            self.writeSymbol()
            self.advance()
            
            # classVarDec
            while self._tokenizer.keyword() == 'static' or self._tokenizer.keyword() == 'field':
                self.compileClassVarDec()
            # subroutineDec
            while self._tokenizer.keyword() == 'method' or self._tokenizer.keyword() == 'function' or self._tokenizer.keyword() == 'constructor': 
                self.compileSubroutineDec()
            
            # write '}'
            self.writeSymbol()
            
            # write </class>
            self.write('</class>')
            
            # complete parse
            self._xmlFile.close()

    def compileClassVarDec(self):
        self.write("<classVarDec>\n")
        
        # write static/field
        self.writeKeyword()
        self.advance()
        
        # write type and varName
        self.write_type_and_varName()
        
        # write ';'
        self.writeSymbol()
        self.advance()

        self.write("</classVarDec>\n")

    def compileSubroutineDec(self):
        self.write('<subroutineDec>\n')
        
        # write keyword
        self.writeKeyword()
        self.advance()
        
        # write void or type
        if self._tokenizer.tokenType() == self._tokenizer.TYPE_KEYWORD:
            self.writeKeyword()     # if void, int, char, boolean
        else:
            self.writeIdentifier()  # if className
        self.advance()
        
        # write subroutineName
        self.writeIdentifier()
        self.advance()
        
        # write '('
        self.writeSymbol()
        self.advance()
        
        # write parameterList
        self.compileParameterList()
        
        # write ')'
        self.writeSymbol()
        self.advance()
        
        # write subRoutineBody
        self.compileSubroutineBody()

        self.write('</subroutineDec>\n')

    def compileParameterList(self):
        self.write("<parameterList>\n")
        
        if self._tokenizer.tokenType() != self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() != ')':
            while 1:
                # write type
                if self._tokenizer.tokenType() == self._tokenizer.TYPE_KEYWORD:
                    self.writeKeyword()     # int, char, boolean
                else:
                    self.writeIdentifier()  # className
                self.advance()
                
                # write varName
                self.writeIdentifier() 
                self.advance()
                
                if self._tokenizer.tokenType() == self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() != ',':
                    break
                
                self.writeSymbol()
                self.advance()
        
        self.write("</parameterList>\n")

    def compileSubroutineBody(self):
        self.write("<subroutineBody>\n")

        # write '{'
        self.writeSymbol()
        self.advance()

        # write varDec
        while self._tokenizer.tokenType() == self._tokenizer.TYPE_KEYWORD and self._tokenizer.keyword() == 'var':
            self.compileVarDec()

        # write statements
        self.compileStatements()

        # write '}'
        self.writeSymbol()
        self.advance()

        self.write("</subroutineBody>\n")
    
    def compileVarDec(self):
        self.write("<varDec>\n")
        
        # write 'var'
        self.writeKeyword()
        self.advance()

        # write type and varName
        self.write_type_and_varName()
        
        # write ';'
        self.writeSymbol() 
        self.advance()

        self.write("</varDec>\n")

    def compileStatements(self):
        self.write("<statements>\n")
        
        while self._tokenizer.tokenType() == self._tokenizer.TYPE_KEYWORD:
            if self._tokenizer.keyword() == 'let':
                self.compileLet()
            
            elif self._tokenizer.keyword() == 'if':
                self.compileIf()
            
            elif self._tokenizer.keyword() == 'while':
                self.compileWhile()
            
            elif self._tokenizer.keyword() == 'do':
                self.compileDo()
            
            elif self._tokenizer.keyword() == 'return':
                self.compileReturn()

        self.write("</statements>\n")

    def compileLet(self):
        self.write("<letStatement>\n")

        # write 'let'
        self.writeKeyword()
        self.advance()

        # write varName
        self.writeIdentifier()
        self.advance()

        # if varName['expression']
        if self._tokenizer.tokenType() == self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() == '[':
            # write '['
            self.writeSymbol()
            self.advance()

            # write expression
            self.compileExpression()

            # write ']'
            self.writeSymbol()
            self.advance()

        # write '='
        self.writeSymbol()
        self.advance()

        # write 'expression'
        self.compileExpression()

        # write ';'
        self.writeSymbol()
        self.advance()

        self.write("</letStatement>\n")

    def compileIf(self):
        self.write("<ifStatement>\n")

        # write 'if'
        self.writeKeyword()
        self.advance()

        # write '('
        self.writeSymbol()
        self.advance()

        # write 'expression'
        self.compileExpression()

        # write ')'
        self.writeSymbol()
        self.advance()

        # write '{'
        self.writeSymbol()
        self.advance()

        # statements
        self.compileStatements()

        # write '}'
        self.writeSymbol()
        self.advance()

        # if 'else' exists
        if self._tokenizer.tokenType() == self._tokenizer.TYPE_KEYWORD and self._tokenizer.keyword() == 'else':
            # write 'else'
            self.writeKeyword()
            self.advance()

            # write '{'
            self.writeSymbol()
            self.advance()

            # statements
            self.compileStatements()

            # write '}'
            self.writeSymbol()
            self.advance()

        self.write("</ifStatement>\n")

    def compileWhile(self):
        self.write("<whileStatement>\n")
        
        # write 'while'
        self.writeKeyword()
        self.advance()

        # write '('
        self.writeSymbol()
        self.advance()

        # expression
        self.compileExpression()

        # write ')'
        self.writeSymbol()
        self.advance()

        # write '{'
        self.writeSymbol()
        self.advance()
        
        # statements
        self.compileStatements()
        
        # write '}'
        self.writeSymbol()
        self.advance()

        self.write("</whileStatement>\n")

    def compileDo(self):
        self.write('<doStatement>\n')

        # write 'do'
        self.writeKeyword()
        self.advance()

        # subroutineCall
        # write subroutineName or varName, className
        self.writeIdentifier()
        self.advance()
        
        # if subroutineName(expressionList)
        if self._tokenizer.tokenType() == self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() == '(':
            self.compileExpressionList()

        # if foo.bar or Foo.bar --> subroutineCall
        elif self._tokenizer.tokenType() == self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() == '.':
            # write '.'
            self.writeSymbol()
            self.advance()
            
            # write subroutineName
            self.writeIdentifier()
            self.advance()
            
            # expressionList
            self.compileExpressionList()

        # write ';'
        self.writeSymbol()
        self.advance()

        self.write('</doStatement>\n')
    
    def compileReturn(self):
        self.write('<returnStatement>\n')
        
        # write 'return'
        self.writeKeyword()
        self.advance()

        # if expression exist
        if self._tokenizer.symbol() != ';':
            self.compileExpression()

        # write ';'
        self.writeSymbol()
        self.advance()

        self.write('</returnStatement>\n')

    # Expression = term op term
    def compileExpression(self):
        self.write('<expression>\n')

        self.compileTerm()
        while self._tokenizer.tokenType() == self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() in self.OP:
            # write op
            self.writeSymbol()
            self.advance()
            # write term
            self.compileTerm()

        self.write('</expression>\n')

    def compileTerm(self):
        # if integerConstant
        if self._tokenizer.tokenType() == self._tokenizer.TYPE_INT:
            self.write("<term>\n")
            self.writeInt()
            self.advance()
            self.write("</term>\n")
        
        # if stringConstant
        elif self._tokenizer.tokenType() == self._tokenizer.TYPE_STR:
            self.write("<term>\n")
            self.writeStr()
            self.advance()
            self.write("</term>\n")
        
        # if keywordConstant --> true, false, null, this
        elif self._tokenizer.tokenType() == self._tokenizer.TYPE_KEYWORD:
            self.write("<term>\n")
            self.writeKeyword()
            self.advance()
            self.write("</term>\n")

        # if identifier -> there are 5 possibilities
        # foo
        # foo[expression]
        # foo.bar(expressionList)
        # Foo.bar(expressionList)
        # foo(expressionList)
        # save the current token and advance to get the next one
        elif self._tokenizer.tokenType() == self._tokenizer.TYPE_IDENT:
            # write varName or subroutineName
            self.write("<term>\n")
            self.writeIdentifier()
            self.advance()

            # if varName[expression]
            if self._tokenizer.tokenType() == self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() == '[':
                # write '['
                self.writeSymbol()
                self.advance()

                # write expression
                self.compileExpression()

                # write ']'
                self.writeSymbol()
                self.advance()
            
            # if subroutineName(expressionList)
            elif self._tokenizer.tokenType() == self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() == '(':
                self.compileExpressionList()

            # if foo.bar or Foo.bar --> subroutineCall
            elif self._tokenizer.tokenType() == self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() == '.':
                # write '.'
                self.writeSymbol()
                self.advance()

                # write subroutineName
                self.writeIdentifier()
                self.advance()

                # expressionList
                self.compileExpressionList()

            self.write("</term>\n")

        # if ( expression )
        elif self._tokenizer.tokenType() == self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() == '(':
            self.write("<term>\n")

            # write '('
            self.writeSymbol()
            self.advance()

            # expression
            self.compileExpression()

            # write ')'
            self.writeSymbol()
            self.advance()

            self.write("</term>\n")

        
        # if unaryOp term
        elif self._tokenizer.tokenType() == self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() in self.UNARY_OP:
            self.write("<term>\n")

            # write unaryOp
            self.writeSymbol()
            self.advance()

            # write term
            self.compileTerm()

            self.write("</term>\n")

    def compileExpressionList(self):
        # write '('
        self.writeSymbol()
        self.advance()
        
        self.write("<expressionList>\n")
        
        count = 0 # the number of expressions in the list

        # if expressions exist,
        if self._tokenizer.symbol() != ')':
            while 1:
                # expressionList
                count += 1
                self.compileExpression()
                
                if self._tokenizer.tokenType() == self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() != ',':
                    break
                
                # write ','
                self.writeSymbol()
                self.advance()

        self.write("</expressionList>\n")
        
        # write ')'
        self.writeSymbol()
        self.advance()

        return count