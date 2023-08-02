#!/bin/sh

#make;

#qemu-system-x86_64 -L . -m 64 -fda ./Disk.img -hda ./HDD.img -boot a -M pc -curses;
# fucking error HDD

# qemu-system-x86_64 -L . -m 64 -fda ./Disk.img -boot a -M pc -rtc base=localtime -curses;
# use RAM Disk, no error, fast 

qemu-system-x86_64 -L . -m 64 -fda ./DiskWithPackage.img -boot a -M pc -rtc base=localtime -serial tcp::4444,server,nowait -smp 2 #-gdb tcp::1234 -S; 
# Use RAM Disk, N Processor(MUlti Processor) with "-smp N" option



: << 'END'

-L path ==> set the directory for the BIOS, VGA BIOS and keymaps
-m 64 ==> 가상머신의 메모리를 64MB로 설정
-fda ./Disk.img ==> 플로피 디스크의 이미지 설정
-boot a ==> boot floppy (a)
-M pc ==> 가상머신을 일반 PC환경으로 설정


debug 방법 :
1. clang 옵션으로 -g 설정
2. qemu 옵션으로 -gdb tcp::1234 -S 설정
3. gdb -> target remote:1234 -> file 02 ~~ /Kernel64.elf -> 원하는 곳에 브레이크 후 continue
END