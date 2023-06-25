[ORG 0x00]
[BITS 16]

SECTION .text

jmp 0x07C0:START

TOTALSECTORCOUNT: dw 0x02  ; 보호모드 커널 + C커널
KERNEL32SECTORCOUNT: dw 0x02    ; 보호모드 커널 총 섹터 수
BOOTSTRAPPROCESSOR: db 0x01     ; 0x7C09, BSP = 1, AP = 0으로 구분
STARTGRAPHICMODE: db 0x01       ; 그래픽 모드로 시작하는지 여부

START:
    mov ax, 0x07C0
    mov ds, ax

    mov ax, 0xB800
    mov es, ax

    mov ax, 0x0000
    mov ss, ax
    mov sp, 0xFFFE
    mov bp, 0xFFFE

    mov si, 0

.SCREENCLEARLOOP:                       ; 화면에서 문자만 0으로 초기화 해줌
                                        ; 속성 값은 그대로 설정
    mov byte [ es: si ], 0
    mov byte [ es: si + 1], 0x0A        ; 속성 값 = 검정 배경에 초록색 글씨

    add si, 2
    cmp si, 80*25*2                     ; 가로 80, 세로 25, 문자 당 2바이트

    jl .SCREENCLEARLOOP

    mov si, 0
    mov di, 0

    push MESSAGE1
    push 0
    push 0
    call PRINTMESSAGE
    add sp, 6

    push OSLOADINGMESSAGE
    push 1
    push 0
    call PRINTMESSAGE
    add sp, 6


RESETDISK:
    mov ax, 0
    mov dl, 0
    int 0x13
    jc HANDLEDISKERROR

    mov si, 0x1000
    mov es, si
    mov bx, 0x0000

    mov di, word [ TOTALSECTORCOUNT ]

READDATA:
    ; 플로피 디스크는 섹터 -> 헤더 -> 트랙 순으로 증가됨
    ; 섹터는 현 버전 qemu에서는 1 ~ 36
    ; 헤더는 0 ~ 1
    ; 트랙은 0 ~ 79

    cmp di, 0
    je READEND
    sub di, 0x1

    mov ah, 0x2
    mov al, 0x1
    mov ch, byte [ TRACKNUMBER ]
    mov cl, byte [ SECTORNUMBER ]
    mov dh, byte [ HEADNUMBER ]
    mov dl, 0x00
    int 0x13
    jc HANDLEDISKERROR

    add si, 0x0020                  ; 세그먼트에 넣을 것이므로 512바이트(0x200)만큼 넘어가려면
    mov es, si                      ; 0x0020을 es 세그먼트 레지스터에 저장하면 0x200이 됨

    mov al, byte [ SECTORNUMBER ]
    add al, 0x01
    mov byte [ SECTORNUMBER ], al
    cmp al, 37                      ; qemu 최신버전은 섹터 개수가 18 -> 36개로 변경되었음
    jl READDATA                     ; 따라서 37과 비교하는 것

    xor byte [ HEADNUMBER ], 0x01   ; 헤더는 0 -> 1, 1 -> 0 밖에 없어서 xor로 간단히 구현
    mov byte [ SECTORNUMBER ], 0x01

    cmp byte [ HEADNUMBER ], 0x00
    jne READDATA

    add byte [ TRACKNUMBER ], 0x01
    jmp READDATA

READEND:
    push LOADINGCOMPLETEMESSAGE
    push 1
    push 20
    call PRINTMESSAGE
    add sp, 6

    ; VBE 기능 번호 0x4F01을 호출하여 그래픽 모드에 대한 모드 정보 블록 구함
    mov ax, 0x4F01
    mov cx, 0x117
    mov bx, 0x07E0
    mov es, bx
    mov di, 0x00

    int 0x10
    cmp ax, 0x004F
    jne VBEERROR

    ; VBE 기능 번호 0x4F02 호출, 그래픽 모드 전환
    cmp byte [STARTGRAPHICMODE], 0x00
    je JUMPTOPROTECTEDMODE

    mov ax, 0x4F02
    mov bx, 0x4117

    int 0x10
    cmp ax, 0x004F
    jne VBEERROR

    jmp JUMPTOPROTECTEDMODE

VBEERROR:
    push CHANGEGRAPHICMODEFAIL
    push 2
    push 0
    call PRINTMESSAGE
    add sp, 6
    jmp $

JUMPTOPROTECTEDMODE:
    jmp 0x1000:0x0000

HANDLEDISKERROR:
    push DISKERRORMESSAGE
    push 1
    push 20
    call PRINTMESSAGE

    jmp $

PRINTMESSAGE:
    push bp
    mov bp, sp

    push es
    push si
    push di
    push ax
    push cx
    push dx

    mov ax, 0xB800
    mov es, ax

    mov ax, word [ bp + 6 ]
    mov si, 160
    mul si
    mov di, ax

    mov ax, word [ bp + 4 ]
    mov si, 2
    mul si
    add di, ax

    mov si, word [ bp + 8 ]

.MESSAGELOOP:
    mov cl, byte [ si ]
    cmp cl, 0
    je .MESSAGEEND

    mov byte [ es: di ], cl

    add si, 1
    add di, 2

    jmp .MESSAGELOOP

.MESSAGEEND:
    pop dx
    pop cx
    pop ax
    pop di
    pop si
    pop es
    pop bp
    ret

MESSAGE1: db '[*] MINT64 OS Boot Loader Start~!!', 0

DISKERRORMESSAGE: db '[*] DISK Error~!!', 0
OSLOADINGMESSAGE: db '[*] OS Image Loading...', 0
LOADINGCOMPLETEMESSAGE: db 'Complete~!!', 0
CHANGEGRAPHICMODEFAIL: db 'Change Graphic Mode Fail~!!', 0

SECTORNUMBER: db 0x02
HEADNUMBER: db 0x00
TRACKNUMBER: db 0x00

times 510 - ( $ - $$ ) db 0x00  ; $ = 현재 라인의 주소
                                ; $$ = 현재 섹션의 시작 주소
                                ; $ - $$ = 현재 라인에서 시작 주소를 뺀 것이므로 오프셋을 구한 것
                                ; 510 - ($ - $$)는 현재부터 510 라인까지 반복한다는 것
                                ; 즉, 현재부터 510 라인까지 0으로 초기화

db 0x55                         ; 부트로더는 마지막 2바이트를 0x55 0xAA로 구분하므로 설정한 것 
db 0xAA