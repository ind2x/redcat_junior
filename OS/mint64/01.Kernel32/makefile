NASM = nasm
CC = clang -c -m32 -ffreestanding
LD = ld -melf_i386 -T ../elf_i386.x -nostdlib -e Main -Ttext 0x10200
OBJCOPY = objcopy -j .text -j .data -j .rodata -j .bss -S -O binary

OBJECTDIR = Temp
SOURCEDIR = Source

all: prepare Kernel32.bin

prepare:
	mkdir -p $(OBJECTDIR)

$(OBJECTDIR)/EntryPoint.bin: $(SOURCEDIR)/EntryPoint.s
	$(NASM) -o $@ $<

dep:
	@echo === Make Dependancy File ===
	make -C $(OBJECTDIR) -f ../makefile InternalDependency
	@echo === Dependancy Search Complete ===

ExecuteInternalBuild: dep
	make -C $(OBJECTDIR) -f ../makefile Kernel32.elf

$(OBJECTDIR)/Kernel32.elf.bin: ExecuteInternalBuild
	$(OBJCOPY) $(OBJECTDIR)/Kernel32.elf $@

Kernel32.bin: $(OBJECTDIR)/EntryPoint.bin $(OBJECTDIR)/Kernel32.elf.bin
	cat $^ > $@

clean:
	rm -f *.bin
	rm -f $(OBJECTDIR)/*.*


## for InternalDependency
CENTRYPOINTOBJECTFILE = Main.o
CSOURCEFILES = $(wildcard ../$(SOURCEDIR)/*.c)
ASSEMBLYSOURCEFILES = $(wildcard ../$(SOURCEDIR)/*.asm)
COBJECTFILES = $(subst Main.o, , $(notdir $(patsubst %.c,%.o,$(CSOURCEFILES))))
ASSEMBLYOBJECTFILES = $(notdir $(patsubst %.asm,%.o,$(ASSEMBLYSOURCEFILES)))

%.o: ../$(SOURCEDIR)/%.c
	$(CC) -c $<

%.o: ../$(SOURCEDIR)/%.asm
	$(NASM) -f elf32 -o $@ $<

InternalDependency:
	$(CC) -MM $(CSOURCEFILES) > Dependency.dep

Kernel32.elf: $(CENTRYPOINTOBJECTFILE) $(COBJECTFILES) $(ASSEMBLYOBJECTFILES)
	$(LD) -o $@ $^

ifeq (Dependency.dep, $(wildcard Dependency.dep))
include Dependency.dep
endif
