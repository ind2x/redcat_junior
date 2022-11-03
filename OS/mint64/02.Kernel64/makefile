NASM = nasm -f elf64
CC = clang -g -m64 -ffreestanding -msoft-float
LD = ld -melf_x86_64 -T ../elf_x86_64.x -nostdlib -e main -Ttext 0x200000
OBJCOPY = objcopy -j .text -j .data -j .rodata -j .bss -S -O binary

OBJECTDIR = Temp
SOURCEDIR = Source

all: prepare Kernel64.bin

prepare:
	mkdir -p $(OBJECTDIR)

dep:
	@echo === Make Dependancy File ===
	make -C $(OBJECTDIR) -f ../makefile InternalDependency
	@echo === Dependancy Search Complete ===

ExecuteInternalBuild: dep
	make -C $(OBJECTDIR) -f ../makefile Kernel64.elf


Kernel64.bin: ExecuteInternalBuild
	$(OBJCOPY) $(OBJECTDIR)/Kernel64.elf $@

clean:
	rm -f *.bin
	rm -f $(OBJECTDIR)/*.*


## for InternalDependency
ENTRYPOINTSOURCEFILE = ../$(SOURCEDIR)/EntryPoint.s
ENTRYPOINTOBJECTFILE = EntryPoint.o
CSOURCEFILES = $(wildcard ../$(SOURCEDIR)/*.c)
ASSEMBLYSOURCEFILES = $(wildcard ../$(SOURCEDIR)/*.asm)
COBJECTFILES = $(notdir $(patsubst %.c,%.o,$(CSOURCEFILES)))
ASSEMBLYOBJECTFILES = $(notdir $(patsubst %.asm,%.o,$(ASSEMBLYSOURCEFILES)))

$(ENTRYPOINTOBJECTFILE) : $(ENTRYPOINTSOURCEFILE)
	$(NASM) -o $@ $<

%.o: ../$(SOURCEDIR)/%.c
	$(CC) -c $<

%.o: ../$(SOURCEDIR)/%.asm
	$(NASM) -o $@ $<

InternalDependency:
	$(CC) -MM $(CSOURCEFILES) > Dependency.dep

Kernel64.elf: $(ENTRYPOINTOBJECTFILE) $(COBJECTFILES) $(ASSEMBLYOBJECTFILES)
	$(LD) -o $@ $^

ifeq (Dependency.dep, $(wildcard Dependency.dep))
include Dependency.dep
endif
