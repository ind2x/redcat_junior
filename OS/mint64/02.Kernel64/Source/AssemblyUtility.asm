[BITS 64]

SECTION .text

global InPortByte, OutPortByte, LoadGDTR, LoadTR, LoadIDTR
global EnableInterrupt, DisableInterrupt, ReadRFLAGS
global ReadTSC
global SwitchContext, Hlt

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

EnableInterrupt:
    sti
    ret

DisableInterrupt:
    cli
    ret

ReadRFLAGS:
    pushfq
    pop rax
    ret

ReadTSC:
    push rdx

    rdtsc

    shl rdx, 32
    or rax, rdx

    pop rdx
    ret

%macro KSAVECONTEXT 0       ; 파라미터를 전달받지 않는 KSAVECONTEXT 매크로 정의
    ; RBP 레지스터부터 GS 세그먼트 셀렉터까지 모두 스택에 삽입
    push rbp
    push rax
    push rbx
    push rcx
    push rdx
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    
    mov ax, ds      ; DS 세그먼트 셀렉터와 ES 세그먼트 셀렉터는 스택에 직접
    push rax        ; 삽입할 수 없으므로, RAX 레지스터에 저장한 후 스택에 삽입
    mov ax, es
    push rax
    push fs
    push gs 
%endmacro       ; 매크로 끝


; 콘텍스트를 복원하는 매크로
%macro KLOADCONTEXT 0   ; 파라미터를 전달받지 않는 KSAVECONTEXT 매크로 정의
    ; GS 세그먼트 셀렉터부터 RBP 레지스터까지 모두 스택에서 꺼내 복원
    pop gs
    pop fs
    pop rax
    mov es, ax      ; ES 세그먼트 셀렉터와 DS 세그먼트 셀렉터는 스택에서 직접
    pop rax         ; 꺼내 복원할 수 없으므로, RAX 레지스터에 저장한 뒤에 복원
    mov ds, ax
    
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    pop rbp        
%endmacro       ; 매크로 끝

SwitchContext:
    push rbp
    mov rbp, rsp

    pushfq
    cmp rdi, 0
    je .LoadContext
    popfq

    push rax
    mov ax, ss
    mov qword[ rdi + ( 23 * 8 ) ], rax

    mov rax, rbp                       
    add rax, 16                        
    mov qword[ rdi + ( 22 * 8 ) ], rax  
    
    pushfq        
    pop rax
    mov qword[ rdi + ( 21 * 8 ) ], rax

    mov ax, cs 
    mov qword[ rdi + ( 20 * 8 ) ], rax
    
    mov rax, qword[ rbp + 8 ]
    mov qword[ rdi + ( 19 * 8 ) ], rax  
    pop rax
    pop rbp
    
    add rdi, ( 19 * 8 )
    mov rsp, rdi
    sub rdi, ( 19 * 8 )
    
    KSAVECONTEXT

.LoadContext:
    mov rsp, rsi
    
    KLOADCONTEXT
    iretq

Hlt:
    hlt
    hlt
    ret