.global main

main:
    push rbp
    mov rbp, rsp

    sub rsp, 1024

    # memset
    # rbp - 1024 = buf
    lea rdi, [rbp-1024]
    mov rsi, 0
    mov rdx, 1024
    call cmemset

    mov rdi, 1234567890
    lea rsi, [rbp-1024]
    call citoa

    lea rdi, [rbp-1024]
    call cputs

    leave
    ret


citoa:
    # itoa(value, str)
    push rbp
    mov rbp, rsp

    # rbp - 8 = idx
    # rbp - 16 = max_pos
    sub rsp, 16
    
    # intel 문법에서는 연산을 하려면 대괄호 이용
    # 따라서 rbp-16에 1을 넣는다는 의미
    # 만약 그냥 [rbp] 라면, rbp에 저장된 값의 영역에 1을 넣음
    mov QWORD PTR [rbp-16], 1   # max_pos = 1
    mov QWORD PTR [rbp-8], 0    # idx = 0
    
    # while (value / max_pos >= 10) max_pos *= 10
    .get_max_pos:
        mov rdx, 0
        mov rax, rdi                    # rax = value
        mov rbx, QWORD PTR [rbp-16]     # rbx = max_pos
        div rbx                         # rax = value / max_pos

        cmp rax, 10
        jl .get_max_pos_end

        mov rdx, 0
        mov rax, QWORD PTR [rbp-16]     # rax = max_pos
        mov rcx, 10                     # rcx = 10
        mul rcx                         # rax = max_pos * 10
        mov QWORD PTR [rbp-16], rax     # update max_pos
        
        jmp .get_max_pos

    .get_max_pos_end:

    .itoa_loop:
        # while(max_pos > 0)
        mov rax, QWORD PTR [rbp-16]     # rax = max_pos
        cmp rax, 0
        jle .itoa_end

        mov rbx, rsi                    # rbx = str
        add rbx, QWORD PTR [rbp-8]      # rbx = str[idx]

        # 0x30 + (value / max_pos)
        # value / max_pos
        mov rdx, 0
        mov rax, rdi                    # rax = value
        mov rcx, QWORD PTR [rbp-16]     # rcx = max_pos
        div rcx                         # rax = value / max_pos

        add rax, 0x30                   # rax += 0x30
        mov QWORD PTR [rbx], rax        # str[idx] = rax

        # value -= (value / max_pos * max_pos)
        mov rdx, 0
        mov rax, rdi                    # rax = value
        mov rbx, QWORD PTR [rbp-16]     # rbx = max_pos
        div rbx                         # rax = value / max_pos
        mul rbx                         # rax = value / max_pos * max_pos

        sub rdi, rax                    # value -= rax

        # idx++
        mov rax, QWORD PTR [rbp-8]
        inc rax
        mov QWORD PTR [rbp-8], rax

        # max_pos = max_pos / 10
        mov rdx, 0
        mov rax, QWORD PTR [rbp-16]     # rax = max_pos
        mov rbx, 10                     # rbx = 10
        div rbx                         # rax = max_pos / 10
        mov QWORD PTR [rbp-16], rax     # max_pos = rax
        jmp .itoa_loop

.itoa_end:
    leave
    ret

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

cmemset:
    # memset(src, value, count)
    # rdi = src address, rsi = value, rdx = count

    push rbp
    mov rbp, rsp

    .memset_loop:
        cmp rdx, 0
        je .memset_end

        mov rax, rsi
        mov QWORD PTR [rdi], rax
        inc rdi
        dec rdx
        jmp .memset_loop

.memset_end:
    leave
    ret
