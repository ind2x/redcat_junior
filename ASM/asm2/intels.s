.global main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 0x40
    
    # cmemcpy(dest, src, count)
    # lea rdi, [rbp-0x40]
    # lea rsi, pass
    # mov rdx, 5
    # call cmemcpy

    # strcpy(dest, src)
    # lea rdi, [rbp-0x40]
    # lea rsi, pass
    # call cstrcpy

    lea rdi, pass
    call cputs

    lea rdi, second
    call cputs

    leave
    ret


pass:  .asciz "Pass!"     # 5
second: .asciz "Second String Success!"     # 22

cputs:
    # puts(buf) == write(1, buf, count)    -> current rdi = buf
    # count = strlen(buf)
    # buf = copied_buf + "\n" + "\x00"
    # copied_buf = strcpy(buf)

    push rbp
    mov rbp, rsp
    sub rsp, 1040

    # rbp - 1040 <= count
    # rbp - 1032 <= buf
    # rbp - 1024 <= copied_buf

    mov QWORD PTR [rbp-1032], rdi   # rbp - 1032 = buf
                                    # rbp-1032에 저장된 주소값에 rdi 값 저장
    call cstrlen                    # cstrlen(src), rax = count
    mov QWORD PTR [rbp-1040], rax   # rbp - 1040 = count

    lea rdi, [rbp-1024]             # rdi에 rbp-1024 주소 값 저장 => copied_buf
    mov rsi, QWORD PTR [rbp-1032]   # rsi에 문자열 전달 => 문자열이 rbp-1032가 가리키는 주소에 존재
    call cstrcpy                    # cstrcpy(dest, src)

    # buf = copied_buf + "\n" + "\x00"
    lea rax, [rbp-1024]             
    add rax, QWORD PTR [rbp-1040]   # *(copied+strlen(arg)) = '\n'
    mov BYTE PTR [rax], 0xa         # add "\n"
    inc rax                         # add null byte next to "\n"
    mov BYTE PTR [rax], 0x00        # add null byte

    # write(1, buf, count) -> rax = 1, rdi = 1, rsi = buf, rdx = count
    mov rdi, 1                      # rdi = 1
    lea rsi, [rbp-1024]             # rsi = buf
    mov rdx, QWORD PTR [rbp-1040]   # rdx = count
    inc rdx                         # add "\n" count 1
    mov rax, 1                      # rax = 1
    syscall

    leave
    ret

cstrlen:
    # strlen(src), rdi = src
    
    push rbp
    mov rbp, rsp
    mov rax, 0

    .check_len:
        cmp BYTE PTR [rdi], 0x0     # check rdi is null
        je .cstrlen_end

        inc rdi
        inc rax
        jmp .check_len

.cstrlen_end:
    leave
    ret

cmemcpy:
    # memcpy(dest, src, count)  
    # rdi = dest, rsi = src, rdx = count

    push rbp
    mov rbp, rsp

    .copy_string:                   # dest = src + \x00
        cmp rdx, 0                  # check if string is null or end
        mov al, BYTE PTR [rsi]      # add null byte if string is ended
        mov BYTE PTR [rdi], al  
        je .cmemcpy_end             # jmp if equal 0

        dec rdx
        inc rdi
        inc rsi
        jmp .copy_string

.cmemcpy_end:
    leave
    ret

cstrcpy:
    # strcpy(dest, src) == memcpy(dest, src, count)
    # rdi = dest, rsi = src
    # ; strlen(src) == > rdi = src

    push rbp
    mov rbp, rsp

    push rdi        # memcpy에서 rdi, rsi 값이 변화됨
    mov rdi, rsi
    push rsi
    call cstrlen     # rax = count

    mov rdx, rax
    pop rsi
    pop rdi
    call cmemcpy

    leave
    ret
