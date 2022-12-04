#!/bin/sh

make;

#qemu-system-x86_64 -L . -m 64 -fda ./Disk.img -hda ./HDD.img -boot a -M pc -curses; --> fucking error

# qemu-system-x86_64 -L . -m 64 -fda ./Disk.img -boot a -M pc -rtc base=localtime -curses; --> use RAM Disk, no error, fast 

qemu-system-x86_64 -L . -m 64 -fda ./Disk.img -boot a -M pc -rtc base=localtime -serial tcp::4444,server,nowait -curses;

: << 'END'

qemu-img create HDD.img 20M

usage : -d in_asm

 -gdb tcp::1234 -S -> gdb -> target remote:1234 -> file 02 ~~ /Kernel64.elf -> b main -> c
END
