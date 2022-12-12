[ORG 0x00]
[BITS 16]

SECTION .text

START:              ; 0x10000부터 OS 시작, 보호모드로 전환되는데
    mov ax, 0x1000
    mov ds, ax
    mov es, ax

    cmp byte [es:0x7C09], 0x00          ; BSP인지 AP인지 확인하는 것
    je .APPLICATIONPROCESSORSTARTPOINT

    ; BIOS 시스템 서비스 인터럽트 벡터 (0x15) 중 A20 게이트 관련 기능
    mov ax, 0x2401          ; A20 게이트 활성화 (BIOS를 통한 활성화)
    int 0x15                ; BIOS 시스템 서비스 인터럽트 벡터

    jc .A20GATEERROR
    jmp .A20GATESUCCESS

.A20GATEERROR:              
    ; BIOS로 활성화하는 방법이 실패하면 시스템 컨트롤 포트로 활성화 시도
    ; 포트는 0x92이며, in, out 명령어 이용해서 접근 가능
    
    in al, 0x92
    or al, 0x02
    and al, 0xFE
    out 0x92, al

.A20GATESUCCESS:
.APPLICATIONPROCESSORSTARTPOINT:
    cli                             ; 인터럽트 비활성화
    lgdt [ GDTR ]                   ; GDTR 테이블 로드

    mov eax, 0x4000003B             ; CR0 레지스터 설정 (보호모드 전환 시 필요)
    mov cr0, eax

    jmp dword 0x18: ( PROTECTEDMODE - $$ + 0x10000 )
    ; 보호모드 코드 디스크립터가 있는 0x18 (디스크립터 크기는 8바이트)로 CS 설정
    ; 오른쪽 값은 EIP의 값을 설정한 것
    ; 커널 코드 세그먼트가 기준주소가 0x00으로 되어 있어서 그에 맞춰서 EIP를 설정

[BITS 32]
PROTECTEDMODE:
    mov ax, 0x20
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ss, ax
    mov esp, 0xFFFE
    mov ebp, 0xFFFE

    cmp BYTE [0x7C09], 0x00
    je .APPLICATIONPROCESSORSTARTPOINT

    push ( SWITCHSUCCESSMESSAGE - $$ + 0x10000 )
    push 2
    push 0
    call PRINTMESSAGE
    add esp, 12

.APPLICATIONPROCESSORSTARTPOINT:
    ; jmp $ -> modify
    jmp dword 0x18: 0x10200         ; jmp to 32bit C kernel(main.c)

PRINTMESSAGE:
    push ebp
    mov ebp, esp
    push esi
    push edi
    push eax
    push ecx
    push edx

    mov eax, dword [ ebp + 12 ]
    mov esi, 160
    mul esi
    mov edi, eax

    mov eax, dword [ ebp + 8 ]
    mov esi, 2
    mul esi
    add edi, eax

    mov esi, dword [ ebp + 16 ]

.MESSAGELOOP:
    mov cl, byte [ esi ]
    
    cmp cl, 0
    je .MESSAGEEND

    mov byte [ edi + 0xB8000 ], cl
    
    add esi, 1
    add edi, 2

    jmp .MESSAGELOOP

.MESSAGEEND:
    pop edx
    pop ecx
    pop eax
    pop edi
    pop esi
    pop ebp
    ret

align 8, db 0

dw 0x0000

GDTR:
    dw GDTEND - GDT - 1
    dd ( GDT - $$ + 0x10000 ) ; $$ = 현재 섹션의 시작 주소 
                              ; 즉, GDT - $$는 GDT 주소에서 현재 코드 실행 주소를 빼서
                              ; GDT의 오프셋을 구하는 것임

GDT:
    NULLDescriptor:           ; 예약된 곳으로 GDT의 첫 부분은 반드시 널 디스크립터
        dw 0x0000
        dw 0x0000
        db 0x00
        db 0x00
        db 0x00
        db 0x00

    IA_32eCODEDESCRIPTOR:  ; 0x08
        dw 0xFFFF
        dw 0x0000
        db 0x00
        db 0x9A
        db 0xAF
        db 0x00
    
    IA_32eDATADESCRIPTOR: ; 0x10
        dw 0xFFFF
        dw 0x0000
        db 0x00
        db 0x92
        db 0xAF
        db 0x00

    CODEDESCRIPTOR: ; 0x18
        dw 0xFFFF
        dw 0x0000
        db 0x00
        db 0x9A
        db 0xCF
        db 0x00

    DATADESCRIPTOR:
        dw 0xFFFF
        dw 0x0000
        db 0x00
        db 0x92
        db 0xCF
        db 0x00
GDTEND:

SWITCHSUCCESSMESSAGE: db '[*] Switch To Protected Mode Success~!!', 0

times 512 - ( $ - $$ )  db 0x00