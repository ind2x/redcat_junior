# asm 2강
## hello world 출력하기

<br>

```asm
# AT&T syntax

.globl main
main:
    push %rbp
    mov %rsp, %rbp

    mov $1, %rax
    mov $1, %rdi
    mov $hello_string, %rsi
    mov $14, %rdx
    syscall

    
    leave
    ret

hello_string: .asciz "Hello, World!\n"
```

<br>

```asm
# intel syntax

.globl main
main:
    push rbp
    mov rbp, rsp

    mov rax, 1
    mov rdi, 1
    lea rsi, hello_string   # not mov rsi, hello_string
    mov rdx, 14
    syscall

    
    leave
    ret

hello_string: .asciz "Hello, World!\n"
``

<br>

처음엔 ```mov rsi, hello_string```으로 하였는데, 이렇게 하면 gdb로 분석해보면 ```mov rsi, DWORD PTR ds:0x40112d```으로 나온다.

이는 rsi에 문자열의 주소를 넣는 것이 아니라, 문자열의 주소에 저장된 값을 rsi에 저장시키는 것이여서 아무런 값이 출력되지 않는 것이었다.

<br><br>
<hr style="border: 2px solid;">
<br><br>

## strlen, memcpy, strcpy 

<br>

```asm
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
```

<br>

위의 내용을 참고하여 인텔 형식으로  

+ intel syntax

```asm
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
```
