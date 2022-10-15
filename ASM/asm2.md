## asm 2강

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

<br>

