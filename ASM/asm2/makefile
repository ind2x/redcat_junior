all : intel att

intel : intel.s
	clang -o intel intel.s -no-pie -masm=intel

att : att.s
	clang -o att att.s -no-pie

clean:
	rm att intel
