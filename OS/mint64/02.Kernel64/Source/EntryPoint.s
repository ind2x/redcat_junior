[BITS 64]

SECTION .text

extern main

START:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ss, ax
    mov rsp, 0x6FFFF8
    mov rbp, 0x6FFFF8

    call main
    jmp $