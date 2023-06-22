[BITS 64]

SECTION .text

global InPortByte, OutPortByte, InPortWord, OutPortWord, 
global LoadGDTR, LoadTR, LoadIDTR
global EnableInterrupt, DisableInterrupt, ReadRFLAGS
global ReadTSC
global SwitchContext, Hlt, TestAndSet, Pause
global InitializeFPU, SaveFPUContext, LoadFPUContext, SetTS, ClearTS
global EnableGlobalLocalAPIC

InPortByte:
    ; 출력포트의 값을 읽어와서 값을 읽어오는 함수
    push rdx
    mov rdx, rdi
    mov rax, 0
    in al, dx

    pop rdx
    ret

OutPortByte:
    ; 입력포트에 값을 쓰는 함수
    push rdx
    push rax

    mov rdx, rdi
    mov rax, rsi

    out dx, al

    pop rax
    pop rdx
    ret

InPortWord:
    ; 2바이트 씩 읽는 것임
    push rdx       
    
    mov rdx, rdi    
    mov rax, 0      
    in ax, dx       

    
    pop rdx         
    ret             
    
OutPortWord:
    ; 2바이트 씩 쓰는 것임
    push rdx        
    push rax        
    
    mov rdx, rdi
    mov rax, rsi
    out dx, ax  
     
    pop rax     
    pop rdx
    ret         

LoadGDTR:
    lgdt [rdi] ; GDTR에 GDT 정보 설정
    ret

LoadTR:
    ltr di ; 프로세서에 TSS 정보 로드
    ret

LoadIDTR:
    lidt [rdi] ; IDTR에 IDT 정보 로드
    ret

EnableInterrupt:
    sti
    ret

DisableInterrupt:
    cli
    ret

ReadRFLAGS:
    pushfq  ; RFLAGS 레지스터의 IF 비트를 통해 인터럽트를 활성화 비활성화 가능
    pop rax
    ret

ReadTSC:
    ; 타임 스탬프 카운터(TSC)를 읽는 함수
    push rdx

    rdtsc

    shl rdx, 32
    or rax, rdx

    pop rdx
    ret

;///////////////////////////////////////////////////////////////////////////////
; 컨텍스트 저장, 복원, 전환 기능

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

;/////////////////////////////////////////////////////////////////////

Hlt:
    ; 프로세서를 대기 상태로 변경하는 명령어
    ; 인터럽트나 예외가 발생하기 전까지 대기
    hlt
    hlt
    ret

TestAndSet:
    ; p.744 참고
    mov rax, rsi

    lock cmpxchg byte [rdi], dl
    je .SUCCESS

.NOTSAME:
    mov rax, 0x00
    ret

.SUCCESS:
    mov rax, 0x01
    ret

Pause:
    pause
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; FPU 관련 명령어

InitializeFPU:
    finit
    ret

SaveFPUContext:
    fxsave [rdi]
    ret

LoadFPUContext:
    fxrstor [rdi]
    ret

SetTS:
    ; CR0의 TS 비트를 1로 설정
    push rax
    
    mov rax, cr0
    or rax, 0x08
    mov cr0, rax

    pop rax
    ret

ClearTS:
    ; CR0의 TS 비트를 0으로 설정
    clts
    ret


;////////////////////////////////////////////////////////////////////////
; 

EnableGlobalLocalAPIC:
    push rax           
    push rcx
    push rdx
    
    mov rcx, 27         
    rdmsr               
    
    or eax, 0x0800      
    wrmsr               

    pop rdx            
    pop rcx
    pop rax
    ret