import sys  # sys.argv
import os   # os.scandir
import CompilationEngine

def analyze(inputFile):
    vmFile = './test/'+inputFile[:-5]+'.vm'
    Compile = CompilationEngine.CompilationEngine(inputFile, vmFile)
    Compile.compileClass()

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