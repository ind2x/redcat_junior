.global main

main:
    push rbp
    mov rbp, rsp

    sub rsp, 16
    # rbp - 8 = j
    # rbp - 16 = i

    mov QWORD PTR [rbp-8], 1        # j = 1

    # while ( j < 10)
    .loop_j_start:
        mov rax, QWORD PTR [rbp-8]      # rax = j
        cmp rax, 10
        jge .main_end

        mov QWORD PTR [rbp-16], 2       # i = 2
        
        .loop_i_start:
            # while ( i < 10)
            mov rax, QWORD PTR [rbp-16]     # rax = i
            cmp rax, 10
            jge .loop_i_end

            # printf("%d * %d = %2d    ", i, j, i*j)
            # rdi = format
            # rsi = i, rdx = j, rcx = i*j
            lea rdi, format
            mov rsi, QWORD PTR [rbp-16]     # rsi = i

            mov rdx, 0
            mov rcx, QWORD PTR [rbp-8]      # rcx = j
            mul rcx                         # rax = i * j
            mov rcx, rax

            mov rdx, QWORD PTR [rbp-8]      # rdx = j            
            call printf

            mov rax, QWORD PTR [rbp-16]
            inc rax
            mov QWORD PTR [rbp-16], rax
            jmp .loop_i_start

        .loop_i_end:
            # printf("\n")
            lea rdi, newline
            call printf
        
            # j++
            mov rax, QWORD PTR [rbp-8]
            inc rax
            mov QWORD PTR [rbp-8], rax
            jmp .loop_j_start

.main_end:
    leave
    ret

format : .asciz "%d * %d = %2d    "
newline : .asciz "\n" 
