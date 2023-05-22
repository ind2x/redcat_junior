import JackTokenizer
import SymbolTable
import VMWriter

class CompilationEngine:
    def __init__(self, inputFile, outputFile):
        self._tokenizer = JackTokenizer.JackTokenizer(inputFile)
        self._classSymbolTable = SymbolTable.SymbolTable()
        self._subSymbolTable = SymbolTable.SymbolTable()
        self._vmFile = VMWriter.VMWriter(outputFile)
        
    CLASS_NAME = SUB_NAME = NAME = TYPE = KIND = ''
    
    UNARY_OP = ['-', '~']
    OP = ['+', '-', '*', '/', '&', '|', '<', '>', '=']
    
    UNARY_VM = {'-':'neg', '~':'not'}
    OP_VM = {'+':'add', '-':'sub', '&':'and', '|':'or', '<':'lt', '>':'gt', '=':'eq'} 
    
    # nVars == the number of local variables in subroutine(constructor, method, function)
    # ifCount, whileCount == the index of each label
    nVars = nFields = ifCount = whileCount = 0
    
    def advance(self):
        self._tokenizer.advance()

    def getKeyword(self):
        return self._tokenizer.keyword()
    
    def getSymbol(self):
        return self._tokenizer.symbol()
    
    def getInt(self):
        return self._tokenizer.intVal()
    
    def getStr(self):
        return self._tokenizer.stringVal()

    def getIdentifier(self):
        return self._tokenizer.identifier()

    def write_type_and_varName(self, isSubroutine):
        # the number of local variables
        count = 0

        # write type
        if self._tokenizer.tokenType() == self._tokenizer.TYPE_KEYWORD:
            self.TYPE = self.getKeyword()    # int, char, boolean
        else:
            self.TYPE = self.getIdentifier()  # className
        self.advance()
        
        # write varName until meet ';'
        while 1:
            # varName
            self.NAME = self.getIdentifier() 
            self.advance()
            count += 1
            
            # write SymbolTable
            if isSubroutine == 1:
                self._subSymbolTable.define(self.NAME, self.TYPE, self.KIND)
            else:
                self._classSymbolTable.define(self.NAME, self.TYPE, self.KIND)

            if self._tokenizer.tokenType() == self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() != ',':
                break
            
            # ','
            self.advance()
        
        return count

    def compileClass(self):
        if self._tokenizer.hasMoreTokens() == True:
            # start compile
            self.advance()

            # class start
            self.advance()
            
            # className
            self.CLASS_NAME = self._tokenizer.identifier()
            self.advance()
            
            # '{'
            self.advance()
            
            # classVarDec --> make class SymbolTable
            while self._tokenizer.keyword() == 'static' or self._tokenizer.keyword() == 'field':
                self.compileClassVarDec()
            
            #print(self._classSymbolTable._symbolTable)

            # subroutineDec --> make subroutine SymbolTable
            while self._tokenizer.keyword() == 'method' or self._tokenizer.keyword() == 'function' or self._tokenizer.keyword() == 'constructor':
                self.compileSubroutineDec()
                self._subSymbolTable.reset()

            # '}' --> class end
            self._vmFile.close()
            
    def compileClassVarDec(self):
        # write static/field
        self.KIND = self.getKeyword()
        self.advance()
        
        # write type and varName
        if self.KIND == 'field':
            self.nFields += self.write_type_and_varName(0)
        else:
            self.write_type_and_varName(0)
        
        # ';'
        self.advance()

    def compileSubroutineDec(self):
        # initialize label count for every function or method
        self.ifCount = self.whileCount = 0

        # keyword (function or method or constructor)
        self.whichKeyword = self.getKeyword()
        self.advance()        
        
        # void or type
        self.advance()
        
        # subroutineName
        self.SUB_NAME = self.getIdentifier()
        self.advance()        
        
        # '('
        self.advance()
        # parameterList
        self.compileParameterList()
        # ')'
        self.advance()
        
        # write subRoutineBody
        self.compileSubroutineBody()

    def compileParameterList(self):
        self.KIND = 'argument'
        
        # if method, then add 'this = argument 0' to symbolTable 
        if self.whichKeyword == 'method':
            self._subSymbolTable.define('this', self.CLASS_NAME, self.KIND)
        
        if self._tokenizer.tokenType() != self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() != ')':
            while 1:
                # type
                if self._tokenizer.tokenType() == self._tokenizer.TYPE_KEYWORD:
                    self.TYPE = self.getKeyword()     # int, char, boolean
                else:
                    self.TYPE = self.getIdentifier()  # className
                self.advance()
                
                # varName
                self.NAME = self.getIdentifier()
                self.advance()
                
                # write SymbolTable
                self._subSymbolTable.define(self.NAME, self.TYPE, self.KIND)

                if self._tokenizer.tokenType() == self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() != ',':
                    break
                
                # ','
                self.advance()
        
    def compileSubroutineBody(self):
        # '{'
        self.advance()

        # varDec
        while self._tokenizer.tokenType() == self._tokenizer.TYPE_KEYWORD and self._tokenizer.keyword() == 'var':
            self.KIND = 'local'
            self.compileVarDec()


        #print(self._subSymbolTable._symbolTable)
        # 여기서 function vm코드 작성
        # function ClassName.FunctionName nVars, nVars는 local variable 개수
        self._vmFile.writeFunction(self.CLASS_NAME + '.' + self.SUB_NAME, self.nVars)

        # if constructor --> create object
        # Memory.alloc(n), n is the number of fields in the class
        if self.whichKeyword == 'constructor':
            self._vmFile.writePush('constant', self.nFields)
            self._vmFile.writeCall('Memory.alloc', '1')
            self._vmFile.writePop('pointer', '0')
        elif self.whichKeyword == 'method':
            self._vmFile.writePush('argument', '0')
            self._vmFile.writePop('pointer', '0')

        # statements
        self.compileStatements()

        # '}'
        self.advance()

    def compileVarDec(self):
        # 'var'
        self.advance()

        # write type and varName
        self.nVars += self.write_type_and_varName(1)
        
        # ';'
        self.advance()

    def compileStatements(self):
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

    def compileLet(self):
        # 'let'
        self.advance()

        # varName
        varName = self.getIdentifier()
        self.advance()

        isArray = 0
        # if varName['expression']
        if self._tokenizer.tokenType() == self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() == '[':
            isArray = 1 # check if manipulating array element
            
            # '['
            self.advance()
            # expression
            self.compileExpression()
            # ']'
            self.advance()

            # push varName
            if self._subSymbolTable.kindOf(varName) != 'NONE':
                self._vmFile.writePush(self._subSymbolTable.kindOf(varName), self._subSymbolTable.indexOf(varName))
            elif self._classSymbolTable.kindOf(varName) != 'NONE':
                self._vmFile.writePush(self._classSymbolTable.kindOf(varName), self._classSymbolTable.indexOf(varName))

            # write add
            self._vmFile.writeArithmetic('add')

        # write '='
        self.advance()

        # write 'expression'
        self.compileExpression()

        # write ';'
        self.advance()

        if isArray == 1:
            self._vmFile.writePop('temp', '0')
            self._vmFile.writePop('pointer', '1')
            self._vmFile.writePush('temp', '0')
            self._vmFile.writePop('that', '0')
        else:
            # write pop commmand
            if self._subSymbolTable.kindOf(varName) != 'NONE':
                self._vmFile.writePop(self._subSymbolTable.kindOf(varName), self._subSymbolTable.indexOf(varName))
            elif self._classSymbolTable.kindOf(varName) != 'NONE':
                self._vmFile.writePop(self._classSymbolTable.kindOf(varName), self._classSymbolTable.indexOf(varName))

    def compileIf(self):
        # 'if'
        self.advance()

        # write '('
        self.advance()

        # write 'expression'
        self.compileExpression()

        # write ')'
        self.advance()

        # make label
        IfEndLabel = 'IF_END' + str(self.ifCount)
        IfFalseLabel = 'IF_FALSE' + str(self.ifCount)
        self.ifCount += 1

        # write not command
        self._vmFile.writeArithmetic('not')

        # write if-goto whether 'else' exists or not
        self._vmFile.writeIf(IfFalseLabel)
        
        # write '{'
        self.advance()
        # statements
        self.compileStatements()
        # write '}'
        self.advance()

        # write goto END 
        self._vmFile.writeGoto(IfEndLabel)

        # write False Label whether it exists or not
        self._vmFile.writeLabel(IfFalseLabel)

        # if 'else' exists
        if self._tokenizer.tokenType() == self._tokenizer.TYPE_KEYWORD and self._tokenizer.keyword() == 'else':
            # 'else'
            self.advance()
            # '{'
            self.advance()
            # statements
            self.compileStatements()
            # '}'
            self.advance()

        self._vmFile.writeLabel(IfEndLabel)
              
    def compileWhile(self):
        # make label
        WhileStartLabel = 'WHILE_EXP' + str(self.whileCount)
        WhileEndLabel = 'WHILE_END' + str(self.whileCount)
        self.whileCount += 1

        # 'while'
        self.advance()
        # '('
        self.advance()
       
        # write Start Label
        self._vmFile.writeLabel(WhileStartLabel)
        # expression
        self.compileExpression()
        
        # ')'
        self.advance()

        # write not
        self._vmFile.writeArithmetic('not')
        # write if-goto
        self._vmFile.writeIf(WhileEndLabel)

        # '{'
        self.advance()
        # statements
        self.compileStatements()
        # '}'
        self.advance()

        # write goto
        self._vmFile.writeGoto(WhileStartLabel)

        # write End Label
        self._vmFile.writeLabel(WhileEndLabel)

    def compileDo(self):
        # 'do'
        self.advance()

        # subroutineCall
        self.compileTerm()

        # ';'
        self.advance()

        # write pop temp 0
        self._vmFile.writePop('temp', '0')

    def compileReturn(self): 
        # 'return'
        self.advance()

        # if expression exist
        if self._tokenizer.symbol() != ';':
            self.compileExpression()
        else:
            self._vmFile.writePush('constant', '0')
        
        # write ';'
        self.advance()

        # write return command
        self._vmFile.writeReturn()

    # Expression = term op term
    # translate infix to postfix (term term op)
    def compileExpression(self):
        # compile term
        self.compileTerm()
        
        while self._tokenizer.tokenType() == self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() in self.OP:
            # save op to make postfix notation
            op = self.getSymbol()
            self.advance()
            
            # compile term
            self.compileTerm()

            # write op vm code
            if op == '*':         # call Math.multiply
                self._vmFile.writeCall('Math.multiply', '2')
            elif op == '/':       # call Math.divide
                self._vmFile.writeCall('Math.divide', '2')
            else:
                self._vmFile.writeArithmetic(self.OP_VM[op])

    def compileTerm(self):
        # if integerConstant
        if self._tokenizer.tokenType() == self._tokenizer.TYPE_INT:
            self._vmFile.writePush('constant', self.getInt())
            self.advance()
        
        # if stringConstant
        elif self._tokenizer.tokenType() == self._tokenizer.TYPE_STR:
            string = self.getStr()
            
            # call String.new(length)
            self._vmFile.writePush('constant', len(string))
            self._vmFile.writeCall('String.new', '1')

            # call String.appendChar(c), if x = "ccc....ccc"
            for c in string:
                self._vmFile.writePush('constant', ord(c))
                self._vmFile.writeCall('String.appendChar', '2')

            self.advance()
        
        # if keywordConstant --> true, false, null, this
        elif self._tokenizer.tokenType() == self._tokenizer.TYPE_KEYWORD:
            self.keywordConstant = self.getKeyword()
            if self.keywordConstant == 'true':
                self._vmFile.writePush('constant', '1')
                self._vmFile.writeArithmetic('neg')
            elif self.keywordConstant == 'false' or self.keywordConstant == 'null':
                self._vmFile.writePush('constant', '0')
            elif self.keywordConstant == 'this':
                self._vmFile.writePush('pointer', '0')
            self.advance()

        # if identifier -> there are 5 possibilities
        # foo
        # foo[expression]
        # varName.subroutineName(expressionList)
        # className.subroutineName(expressionList)
        # subroutineName(expressionList)
        elif self._tokenizer.tokenType() == self._tokenizer.TYPE_IDENT:
            # write varName or subroutineName
            name = self.getIdentifier()
            self.advance()

            # if varName[expression]
            if self._tokenizer.tokenType() == self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() == '[':
                # '['
                self.advance()
                # write expression
                self.compileExpression()
                # ']'
                self.advance()
                
                # push varName
                if self._subSymbolTable.kindOf(name) != 'NONE':
                    self._vmFile.writePush(self._subSymbolTable.kindOf(name), self._subSymbolTable.indexOf(name))
                elif self._classSymbolTable.kindOf(name) != 'NONE':
                    self._vmFile.writePush(self._classSymbolTable.kindOf(name), self._classSymbolTable.indexOf(name))

                # write add
                self._vmFile.writeArithmetic('add')

                # write pop pointer 1, push that 0
                self._vmFile.writePop('pointer', '1')
                self._vmFile.writePush('that', '0')
            
            # if subroutineName(expressionList)
            elif self._tokenizer.tokenType() == self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() == '(':
                nArgs = self.compileExpressionList()
                
                # if method call --> push obj (pointer 0)
                self._vmFile.writePush('pointer', '0')

                # if method call --> ClassName.subName nArgs+1
                self._vmFile.writeCall(self.CLASS_NAME + '.'+ name, nArgs+1)

            # if (className | varName).subroutineName(expressionList)
            elif self._tokenizer.tokenType() == self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() == '.':
                # push varName if it is variable, not Class
                if self._subSymbolTable.kindOf(name) != 'NONE':
                    self._vmFile.writePush(self._subSymbolTable.kindOf(name), self._subSymbolTable.indexOf(name))
                
                elif self._classSymbolTable.kindOf(name) != 'NONE':
                    self._vmFile.writePush(self._classSymbolTable.kindOf(name), self._classSymbolTable.indexOf(name))

                # '.'
                self.advance()

                # subroutineName
                subName = self.getIdentifier()
                self.advance()

                # expressionList
                nArgs = self.compileExpressionList()

                # if method call --> call ClassName.subName nArgs+1           
                # if name == varName, then write its type(className)
                if self._subSymbolTable.kindOf(name) != 'NONE':
                    self._vmFile.writeCall(self._subSymbolTable.typeOf(name) + '.' + subName, nArgs+1)
                elif self._classSymbolTable.kindOf(name) != 'NONE':
                    self._vmFile.writeCall(self._classSymbolTable.typeOf(name) + '.' + subName, nArgs+1)
                else:
                    # if constructor or function call --> call ClassName.subName nArgs
                    self._vmFile.writeCall(name + '.' + subName, nArgs)
            
            # if varName only --> push
            else:
                if self._subSymbolTable.kindOf(name) != 'NONE':
                    self._vmFile.writePush(self._subSymbolTable.kindOf(name), self._subSymbolTable.indexOf(name))
                elif self._classSymbolTable.kindOf(name) != 'NONE':
                    self._vmFile.writePush(self._classSymbolTable.kindOf(name), self._classSymbolTable.indexOf(name))
                
        # if ( expression )
        elif self._tokenizer.tokenType() == self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() == '(':
            # '('
            self.advance()
            # just compile expression only
            self.compileExpression()
            # ')'
            self.advance()

        # if unaryOp term
        elif self._tokenizer.tokenType() == self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() in self.UNARY_OP:
            # write unaryOp
            unary = self.getSymbol()
            self.advance()

            # compile term
            self.compileTerm()

            # output unaryOp
            self._vmFile.writeArithmetic(self.UNARY_VM[unary])

    def compileExpressionList(self):
        # '('
        self.advance()
        
        count = 0 # the number of expressions in the list

        # if expressions exist,
        if self._tokenizer.symbol() != ')':
            while 1:
                # expressionList
                count += 1
                
                self.compileExpression()
                
                if self._tokenizer.tokenType() == self._tokenizer.TYPE_SYMBOL and self._tokenizer.symbol() != ',':
                    break
                
                # ','
                self.advance()

        # ')'
        self.advance()
        
        return count