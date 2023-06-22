'''
field -> this i
static -> static i
local -> local i
parameter -> argument i
'''

class VMWriter:
    def __init__(self, vmFile):
        self._vmFile = open(vmFile, 'w+')
    
    def close(self):
        self._vmFile.close()
    
    def writePush(self, segment, index):
        self._vmFile.write('push {} {}\n'.format(segment, index))
    
    def writePop(self, segment, index):
        self._vmFile.write('pop {} {}\n'.format(segment, index))
    
    # add, sub, neg, eq, gt, lt, and, or, not
    def writeArithmetic(self, cmd):
        self._vmFile.write('{}\n'.format(cmd))
    
    def writeLabel(self, label):
        self._vmFile.write('label {}\n'.format(label))

    def writeGoto(self, label):
        self._vmFile.write('goto {}\n'.format(label))
    
    def writeIf(self, label):
        self._vmFile.write('if-goto {}\n'.format(label))
    
    # call functionName nArgs
    def writeCall(self, name, nArgs):
        self._vmFile.write('call {} {}\n'.format(name, nArgs))
    
    # function functionName nArgs
    def writeFunction(self, name, nVars):
        self._vmFile.write('function {} {}\n'.format(name, nVars))
    
    def writeReturn(self):
        self._vmFile.write('return\n')