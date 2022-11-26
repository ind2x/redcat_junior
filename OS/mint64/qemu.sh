#!/bin/sh

make;

qemu-system-x86_64 -L . -m 64 -fda ./Disk.img -hda ./HDD.img -boot a -M pc -rtc base=localtime;

: << 'END'
usage : -d in_asm

 -gdb tcp::1234 -S -> gdb -> target remote:1234 -> file 02 ~~ /Kernel64.elf -> b main -> c
END
