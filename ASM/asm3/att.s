.global main

main:
    push %rbp
    mov %rsp, %rbp
    
    sub $1024, %rsp

    lea -1024(%rbp), %rdi
    mov $0, %rsi
    mov $1024, %rdx
    call kmemset

    lea -1024(%rbp), %rsi
    mov $12344321, %rdi
    call kitoa

    lea -1024(%rbp), %rdi
    call kputs

    leave
    ret

kitoa:
    # itoa(value, str)
    # rdi = value, rsi = str
    
    push %rbp
    mov %rsp, %rbp

    sub $16, %rsp        # max_pos, idx
    movq $0, -16(%rbp)   # rbp-16 = idx
    movq $1, -8(%rbp)    # rbp-8 = max_pos   

    .get_max_pos_start:
        mov $0, %rdx        # rdx:rax = value
        mov %rdi, %rax   
        mov -8(%rbp), %rbx  # rbx = max_pos
        div %rbx            # rax = 몫

        cmp $10, %rax
        jl .get_max_pos_end

        mov -8(%rbp), %rbx
        mov $10, %rax
        mul %rbx            # rax * rbx = rdx:rax
        mov %rax, -8(%rbp)
        jmp .get_max_pos_start
    
    .get_max_pos_end:

    .itoa_main_loop:
        mov -8(%rbp), %rbx  # rbx = max_pos
        cmp $0, %rbx        
        jle .itoa_end

        mov %rsi, %rbx
        add -16(%rbp), %rbx  # rbx = str[idx]

        mov $0, %rdx
        mov %rdi, %rax
        mov -8(%rbp), %rcx  # rcx = max_pos
        div %rcx            # rax = value / max_pos

        add $0x30, %rax
        mov %al, (%rbx)

        mov $0, %rdx
        mov %rdi, %rax      # rax = value
        
        mov -8(%rbp), %rbx  # rbx = max_pos
        div %rbx            # rax = value / max_pos
        mul %rbx            # rax = value/max_pos * max_pos
        sub %rax, %rdi      # update [value]

        incq -16(%rbp)        # idx++
        
        mov $0, %rdx
        mov -8(%rbp), %rax  # rax = max_pos
        mov $10, %rbx   
        div %rbx            # rax = max_pos / 10
        mov %rax, -8(%rbp)
        jmp .itoa_main_loop

.itoa_end:
    leave
    ret

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

kmemset:
    # memset(src, value, count)
    # rdi = src, rsi = value, rdx = count
    # rdi = rbp-1024, rsi = 0, rdx=1024

    push %rbp
    mov %rsp, %rbp

    .memset_loop:
        cmp $0, %rdx
        je .memset_end

        mov %rsi, %rax
        mov %rax, (%rdi)
        inc %rdi
        dec %rdx
        jmp .memset_loop

.memset_end:
    leave
    ret
