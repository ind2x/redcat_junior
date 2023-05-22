'''
field -> field i (class-level)
static -> static i (class-level)

parameter -> argument i (subroutine-level)
var -> local i (subroutine-level)
'''
# TABLE = {'name': [], 'type': [], 'kind': [], 'index': []}

class SymbolTable:
    def __init__(self):
        self._kind = {'static': 0, 'this': 0, 'argument': 0, 'local': 0}
        self._symbolTable = {'name': [], 'type': [], 'kind': [], 'index': []}

    def reset(self):
        self._kind = {'static': 0, 'this': 0, 'argument': 0, 'local': 0}
        self._symbolTable = {'name': [], 'type': [], 'kind': [], 'index': []}

    def varCount(self, kind):
        return self._kind[kind]

    def kindOf(self, name):
        try:
            return self._symbolTable['kind'][self._symbolTable['name'].index(name)]
        except:
            return 'NONE'

    def typeOf(self, name):
        return self._symbolTable['type'][self._symbolTable['name'].index(name)]

    def indexOf(self, name):
        return self._symbolTable['index'][self._symbolTable['name'].index(name)]

    def define(self, name, type, kind):
        self._symbolTable['name'].append(name)
        self._symbolTable['type'].append(type)
        if kind == 'field':
            kind = 'this'
        self._symbolTable['kind'].append(kind)
        self._symbolTable['index'].append(self.varCount(kind))
        self._kind[kind] += 1