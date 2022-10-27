[BITS 64]

SECTION .text

global InPortByte, OutPortByte, LoadGDTR, LoadTR, LoadIDTR

InPortByte:
    push rdx
    mov rdx, rdi
    mov rax, 0
    in al, dx

    pop rdx
    ret

OutPortByte:
    push rdx
    push rax

    mov rdx, rdi
    mov rax, rsi

    out dx, al

    pop rax
    pop rdx
    ret

LoadGDTR:
    lgdt [rdi]
    ret

LoadTR:
    ltr di
    ret

LoadIDTR:
    lidt [rdi]
    ret