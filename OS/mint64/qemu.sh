#!/bin/sh

make;

qemu-system-x86_64 -L . -m 64 -fda ./Disk.img -M pc -localtime;

: << 'END'
usage : -d in_asm

Log items (comma separated):
out_asm         show generated host assembly code for each compiled TB
in_asm          show target assembly code for each compiled TB
op              show micro ops for each compiled TB
op_opt          show micro ops after optimization
op_ind          show micro ops before indirect lowering
int             show interrupts/exceptions in short format
exec            show trace before each executed TB (lots of logs)
cpu             show CPU registers before entering a TB (lots of logs)
mmu             log MMU-related activities
pcall           x86 only: show protected mode far calls/returns/exceptions
cpu_reset       show CPU state before CPU resets
unimp           log unimplemented functionality
guest_errors    log when the guest OS does something invalid (eg accessing a
non-existent register)
page            dump pages at beginning of user mode emulation
nochain         do not chain compiled TBs so that "exec" and "cpu" show
complete traces
trace:PATTERN   enable trace events

Use "-d trace:help" to get a list of trace events.
END