.global main
main:
    push %rbp
    mov %rsp, %rbp
    sub $0x40, %rsp

    # memcpy(%rbp-0x40, hello_string, 13)
    # mov $26, %rdx
    # mov $second_string, %rsi
    # lea -0x40(%rbp), %rdi
    # call memcpy

    # strcpy(rbp-0x40, hello_string)
    # mov $hello_string, %rsi
    # lea -0x40(%rbp), %rdi
    # call strcpy

    mov $hello_string, %rdi
    call kputs

    mov $second_string, %rdi
    call kputs

    leave
    ret

hello_string: .asciz "Hello, World!"

second_string: .asciz "This is the second string!"

kputs:
    # write(1, buf+"\n", strlen(buf)), rax=1
    # buf = arg_copy + "\n"
    # arg_copy = strcpy(arg)

    push %rbp
    mov %rsp, %rbp

    sub $1040, %rsp
    # rbp - 1040 = strlen(arg)
    # rbp - 1032 == rdi(arg)
    # rbp - 1024 = copied
    mov %rdi, -1032(%rbp)       # mov QWORD PTR [rbp-1032], rdi
    call kstrlen
    mov %rax, -1040(%rbp)       # mov QWORD PTR [rbp-1040], rax

    lea -1024(%rbp), %rdi       # lea rdi, [rbp-1024]
    mov -1032(%rbp), %rsi       # mov rsi, QWORD PTR [rbp-1032]
    call kstrcpy

    lea -1024(%rbp), %rax       # lea rax, [rbp-1024]
    add -1040(%rbp), %rax       # add rax, QWORD PTR [rbp-1040]
    movb $0xa, (%rax)           # mov BYTE PTR [rax], 0xa
    inc %rax
    movb $0x00, (%rax)          # mov BYTE PTR [rax], 0x0

    mov $1, %rdi
    lea -1024(%rbp), %rsi       # lea rsi, [rbp-0x400]
    mov -1040(%rbp), %rdx       # mov rdx, QWORD PTR [rbp-1040]
    inc %rdx
    mov $1, %rax
    syscall

    leave
    ret

kstrlen:
    # strlen(src)

    push %rbp
    mov %rsp, %rbp
    mov $0, %rax

    .check_length:
        cmpb $0, (%rdi)     # cmp BYTE PTR [rdi], 0x0
        je .kstrlen_END
        inc %rax
        inc %rdi
        jmp .check_length

.kstrlen_END:
    leave
    ret

kmemcpy:
    # memcpy(dest, src, count)
    push %rbp
    mov %rsp, %rbp
    
    .copy_memory:
        cmp $0, %rdx        # stack에 쓰레기값이 들어있을 수 있는데 널바이트
                            # 전까지만 복사를 해준다면 끝에 그 쓰레기값이 추가됨
                            # 따라서 널바이트라면 널바이트 복사해주고 끝내면 됨
        movb (%rsi), %al    # mov al, BYTE PTR [rsi]
        movb %al, (%rdi)    # mov BYTE PTR [rdi], al
        je .kmemcpy_END
        
        # movb (%rsi), %al
        # movb %al, (%rdi)
        inc %rdi
        inc %rsi
        dec %rdx
        jmp .copy_memory

.kmemcpy_END:
    leave
    ret

kstrcpy:
    # strcpy(dest, src) == memcpy(dest, src, strlen(src))
    push %rbp
    mov %rsp, %rbp
    
    push %rdi
    mov %rsi, %rdi
    push %rsi
    call kstrlen     # rax = strlen(src)

    mov %rax, %rdx
    
    pop %rsi         # rsi 먼저 해줘야 함 -> FILO
    pop %rdi
    call kmemcpy

    jmp .kstrcpy_END

.kstrcpy_END:
    leave
    ret
