[BITS 32]

global ReadCPUID, SwitchAndExecute64bitKernel

SECTION .text

ReadCPUID:
    ; ReadCPUID(DWORD dwEAX, DWORD *pdwEAX, DWORD *pdwEBX, DWORD *pdwECX, DWORD *pdwEDX)
    push ebp
    mov ebp, esp
    push eax
    push ebx
    push ecx
    push edx
    push esi

    ; ebp + 4 는 ret 주소임
    mov eax, dword [ ebp + 8 ]  ; 파라미터 1 dwEAX
    cpuid

    mov esi, dword [ ebp + 12 ]
    mov dword [ esi ], eax

    mov esi, dword [ ebp + 16 ]
    mov dword [ esi ], ebx

    mov esi, dword [ ebp + 20 ]
    mov dword [ esi ], ecx

    mov esi, dword [ ebp + 24 ]
    mov dword [ esi ], edx

    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax
    pop ebp
    ret

SwitchAndExecute64bitKernel:
    ; cr4 ==> PAE 기능 활성화
    ; cr3 ==> PML4 주소 저장
    ; IA32_EFER ==> LME 활성화(IA-32e 모드 전환 기능)
    ; cr0 ==> 캐시 활성화(페이지 캐시보다 더 우선순위 높은 캐시가 있음)
    mov eax, cr4    
    or eax, 0x620
    mov cr4, eax

    mov eax, 0x100000   ; PML4 = 1MB부터 시작함
    mov cr3, eax

    mov ecx, 0xC0000080     ; IA32_EFER 레지스터 주소
    rdmsr                   ; MSR 특수명령어, 읽기
    or eax, 0x0100          ; LME 활성화
    wrmsr                   ; 다시 쓰기

    mov eax, cr0
    or eax, 0xE000000E      
    xor eax, 0x60000004
    mov cr0, eax

    jmp 0x08:0x200000       ; IA-32 Code Descriptor 위치 설정, 
                            ; 64비트 커널로 이동

    jmp $