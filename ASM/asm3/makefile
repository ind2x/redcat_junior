all : att intel

att : att.s
	clang -o att att.s

intel : intel.s
	clang -o intel intel.s -masm=intel 

clean:
	rm att intel
