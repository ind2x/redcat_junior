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
```

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
    call puts

    leave
    ret

hello_string: .asciz "Hello, World!"

second_string: .asciz "This is the second string!"

puts:
    # write(1, buf+"\n", strlen(buf)), rax=1
    # buf = arg_copy + "\n"
    # arg_copy = strcpy(arg)

    push %rbp
    mov %rsp, %rbp

    sub $1040, %rsp
    # rbp - 1040 = strlen(arg)
    # rbp - 1032 == rdi(arg)
    # rbp - 1024 = copied
    mov %rdi, -1032(%rbp)
    call strlen
    mov %rax, -1040(%rbp)

    lea -1024(%rbp), %rdi
    mov -1032(%rbp), %rsi
    call strcpy

    lea -1024(%rbp), %rax
    add -1040(%rbp), %rax
    movb $0xa, (%rax)
    inc %rax
    movb $0x00, (%rax)

    mov $1, %rdi
    lea -1024(%rbp), %rsi
    mov -1040(%rbp), %rdx
    inc %rdx
    mov $1, %rax
    syscall

    leave
    ret

strlen:
    # strlen(src)

    push %rbp
    mov %rsp, %rbp
    mov $0, %rax

    .check_length:
        cmpb $0, (%rdi)
        je .strlen_END
        inc %rax
        inc %rdi
        jmp .check_length

.strlen_END:
    leave
    ret

memcpy:
    # memcpy(dest, src, count)
    push %rbp
    mov %rsp, %rbp
    
.copy_memory:
    cmp $0, %rdx        # stack에 쓰레기값이 들어있을 수 있는데 널바이트
                        # 전까지만 복사를 해준다면 끝에 그 쓰레기값이 추가됨
                        # 따라서 널바이트라면 널바이트 복사해주고 끝내면 됨
    movb (%rsi), %al    
    movb %al, (%rdi)    
    je .memcpy_END
        
    # movb (%rsi), %al
    # movb %al, (%rdi)
    inc %rdi
    inc %rsi
    dec %rdx
    jmp .copy_memory

.memcpy_END:
    leave
    ret

strcpy:
    # strcpy(dest, src) == memcpy(dest, src, strlen(src))
    push %rbp
    mov %rsp, %rbp
    
    push %rdi
    mov %rsi, %rdi
    push %rsi
    call strlen     # rax = strlen(src)

    mov %rax, %rdx
    
    pop %rsi         # rsi 먼저 해줘야 함 -> FILO
    pop %rdi
    call memcpy

    jmp .strcpy_END

.strcpy_END:
    leave
    ret
```

<br>


