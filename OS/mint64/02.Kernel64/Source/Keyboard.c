#include "Types.h"
#include "AssemblyUtility.h"
#include "Keyboard.h"
#include "Queue.h"
#include "Utility.h"
#include "Synchronization.h"

BOOL IsOutputBufferFull(void)
{
    if (InPortByte(0x64) & 0x01)
    {
        return TRUE;
    }
    return FALSE;
}

BOOL IsInputBufferFull(void)
{
    if (InPortByte(0x64) & 0x02)
    {
        return TRUE;
    }
    return FALSE;
}

/**
 * ACK를 기다리는 동안 다른 스캔 코드는 변환해서 큐에 삽입
*/
BOOL WaitForACKAndPutOtherScanCode(void)
{
    int i, j;
    BYTE bData;
    BOOL bResult = FALSE;

    for (j = 0; j < 100; j++)
    {
        for (i = 0; i < 0xFFFF; i++)
        {
            if (IsOutputBufferFull() == TRUE)
            {
                break;
            }
        }
        bData = InPortByte(0x60);
        if (bData == 0xFA) // ACK이면 종료
        {
            bResult = TRUE;
            break;
        }
        else    // ACK가 아니면 아스키코드로 변환하여 큐에 삽입
        {
            ConvertScanCodeAndPutQueue(bData);
        }
    }
    return bResult;
}

/**
 * 키보드를 활성화 하는 함수
 * 키보드 컨트롤러와 키보드 자체에 값을 전송해야 함
*/
BOOL ActivateKeyboard(void)
{
    int i, j;
    BOOL bPreviousInterrupt;
    BOOL bResult;
    
    // 인터럽트 비활성화 및 이전 인터럽트 상태 저장
    bPreviousInterrupt = SetInterruptFlag(FALSE);

    OutPortByte(0x64, 0xAE);    // 키보드 컨트롤러에 키보드 활성화 값 전송

    for (i = 0; i < 0xFFFF; i++)  // 입력버퍼에 값이 없어졌는지 확인 (없어지면 컨트롤러가 읽은 것)  
    {
        if (IsInputBufferFull() == FALSE)
        {
            break;
        }
    }
    OutPortByte(0x60, 0xF4);    // 키보드에 키보드 활성화 값 전송

    bResult = WaitForACKAndPutOtherScanCode(); // ACK가 오는지 확인

    // 이전 인터럽트 상태 복원
    SetInterruptFlag(bPreviousInterrupt);
    return bResult;
}

/**
 * 출력 버퍼에 값이 있는지 확인 후 있으면 읽어오는 함수
*/
BYTE GetKeyboardScanCode(void)
{
    while (IsOutputBufferFull() == FALSE)
    {
        ;
    }
    return InPortByte(0x60);
}

/**
 * LED 활성화, 0xED를 키보드로 전달 후 ACK로 확인
 * 각각의 Lock을 활성화 시 1로 설정
*/
BOOL ChangeKeyboardLED(BOOL bCapsLockOn, BOOL bNumLockOn, BOOL bScrollLockOn)
{
    int i, j;
    BOOL bPreviousInterrupt;
    BOOL bResult;
    BYTE bData;

    bPreviousInterrupt = SetInterruptFlag(FALSE);

    for (i = 0; i < 0xFFFF; i++)
    {
        if (IsInputBufferFull() == FALSE)
        {
            break;
        }
    }

    OutPortByte(0x60, 0xED);    // LED 활성화 명령 전송
    for (i = 0; i < 0xFFFF; i++)
    {
        if (IsInputBufferFull() == FALSE)
        {
            break;
        }
    }

    bResult = WaitForACKAndPutOtherScanCode();  // ACK 대기

    if (bResult == FALSE)
    {
        SetInterruptFlag(bPreviousInterrupt);
        return FALSE;
    }

    // LED 비트 설정
    OutPortByte(0x60, (bCapsLockOn << 2) | (bNumLockOn << 1) | bScrollLockOn);
    for (i = 0; i < 0xFFFF; i++)
    {
        if (IsInputBufferFull() == FALSE)
        {
            break;
        }
    }

    bResult = WaitForACKAndPutOtherScanCode();  // ACK 대기

    SetInterruptFlag(bPreviousInterrupt);
    return bResult;
}


/**
 * A20 활성화 할 때, 키보드 컨트롤러로 활성화 하는 방법이 있다고 설명했었음.
 * 키보드 컨트롤러로 A20게이트 활성화하는 함수
 * 출력포트 값을 비트 1을 1로 설정
*/
void EnableA20Gate(void)
{
    BYTE bOutputPortData;
    int i;

    OutPortByte(0x64, 0xD0);    // D0는 출력포트 값을 출력버퍼로 복사해오는 것

    for (i = 0; i < 0xFFFF; i++)    // 입력버퍼를 확인하여 값을 가져갔는지 확인
    {
        if (IsOutputBufferFull() == TRUE)
        {
            break;
        }
    }
    bOutputPortData = InPortByte(0x60); // 출력버퍼를 읽어옴

    bOutputPortData |= 0x01;    // A20 활성화 값 설정

    for (i = 0; i < 0xFFFF; i++)    // 입력버퍼에 값이 없으면 전송
    {
        if (IsInputBufferFull() == FALSE)
        {
            break;
        }
    }

    OutPortByte(0x64, 0xD1);    // D1은 입력버퍼의 값을 출력포트로 복사한다는 명령

    OutPortByte(0x60, bOutputPortData); // 입력버퍼에 값을 입력하면 출력포트에 값을 복사함
}

/**
 * 프로세서 리셋 함수
 * 출력포트의 비트 0을 0으로 설정해주면 됨
*/
void Reboot(void)
{
    int i;

    for (i = 0; i < 0xFFFF; i++)
    {
        if (IsInputBufferFull() == FALSE)
        {
            break;
        }
    }

    OutPortByte(0x64, 0xD1);    // 입력버퍼 값을 출력포트로 복사할 것임을 전달
    OutPortByte(0x60, 0x00);    // 입력버퍼에 리셋 값을 써서 출력포트로 복사

    while (1)
    {
        ;
    }
}

static KEYBOARDMANAGER gs_stKeyboardManager; // 키 상태 관리 자료구조 변수

static QUEUE gs_stKeyQueue; // 키 정보를 저장하는 큐
static KEYDATA gs_vstKeyQueueBuffer[KEY_MAXQUEUECOUNT]; // 큐에서 사용할 버퍼

// NormalCode, CombinedCode
static KEYMAPPINGENTRY gs_vstKeyMappingTable[KEY_MAPPINGTABLEMAXCOUNT] =
    {
        /*  0   */ {KEY_NONE, KEY_NONE},
        /*  1   */ {KEY_ESC, KEY_ESC},
        /*  2   */ {'1', '!'},
        /*  3   */ {'2', '@'},
        /*  4   */ {'3', '#'},
        /*  5   */ {'4', '$'},
        /*  6   */ {'5', '%'},
        /*  7   */ {'6', '^'},
        /*  8   */ {'7', '&'},
        /*  9   */ {'8', '*'},
        /*  10  */ {'9', '('},
        /*  11  */ {'0', ')'},
        /*  12  */ {'-', '_'},
        /*  13  */ {'=', '+'},
        /*  14  */ {KEY_BACKSPACE, KEY_BACKSPACE},
        /*  15  */ {KEY_TAB, KEY_TAB},
        /*  16  */ {'q', 'Q'},
        /*  17  */ {'w', 'W'},
        /*  18  */ {'e', 'E'},
        /*  19  */ {'r', 'R'},
        /*  20  */ {'t', 'T'},
        /*  21  */ {'y', 'Y'},
        /*  22  */ {'u', 'U'},
        /*  23  */ {'i', 'I'},
        /*  24  */ {'o', 'O'},
        /*  25  */ {'p', 'P'},
        /*  26  */ {'[', '{'},
        /*  27  */ {']', '}'},
        /*  28  */ {'\n', '\n'},
        /*  29  */ {KEY_CTRL, KEY_CTRL},
        /*  30  */ {'a', 'A'},
        /*  31  */ {'s', 'S'},
        /*  32  */ {'d', 'D'},
        /*  33  */ {'f', 'F'},
        /*  34  */ {'g', 'G'},
        /*  35  */ {'h', 'H'},
        /*  36  */ {'j', 'J'},
        /*  37  */ {'k', 'K'},
        /*  38  */ {'l', 'L'},
        /*  39  */ {';', ':'},
        /*  40  */ {'\'', '\"'},
        /*  41  */ {'`', '~'},
        /*  42  */ {KEY_LSHIFT, KEY_LSHIFT},
        /*  43  */ {'\\', '|'},
        /*  44  */ {'z', 'Z'},
        /*  45  */ {'x', 'X'},
        /*  46  */ {'c', 'C'},
        /*  47  */ {'v', 'V'},
        /*  48  */ {'b', 'B'},
        /*  49  */ {'n', 'N'},
        /*  50  */ {'m', 'M'},
        /*  51  */ {',', '<'},
        /*  52  */ {'.', '>'},
        /*  53  */ {'/', '?'},
        /*  54  */ {KEY_RSHIFT, KEY_RSHIFT},
        /*  55  */ {'*', '*'},
        /*  56  */ {KEY_LALT, KEY_LALT},
        /*  57  */ {' ', ' '},
        /*  58  */ {KEY_CAPSLOCK, KEY_CAPSLOCK},
        /*  59  */ {KEY_F1, KEY_F1},
        /*  60  */ {KEY_F2, KEY_F2},
        /*  61  */ {KEY_F3, KEY_F3},
        /*  62  */ {KEY_F4, KEY_F4},
        /*  63  */ {KEY_F5, KEY_F5},
        /*  64  */ {KEY_F6, KEY_F6},
        /*  65  */ {KEY_F7, KEY_F7},
        /*  66  */ {KEY_F8, KEY_F8},
        /*  67  */ {KEY_F9, KEY_F9},
        /*  68  */ {KEY_F10, KEY_F10},
        /*  69  */ {KEY_NUMLOCK, KEY_NUMLOCK},
        /*  70  */ {KEY_SCROLLLOCK, KEY_SCROLLLOCK},

        /*  71  */ {KEY_HOME, '7'},
        /*  72  */ {KEY_UP, '8'},
        /*  73  */ {KEY_PAGEUP, '9'},
        /*  74  */ {'-', '-'},
        /*  75  */ {KEY_LEFT, '4'},
        /*  76  */ {KEY_CENTER, '5'},
        /*  77  */ {KEY_RIGHT, '6'},
        /*  78  */ {'+', '+'},
        /*  79  */ {KEY_END, '1'},
        /*  80  */ {KEY_DOWN, '2'},
        /*  81  */ {KEY_PAGEDOWN, '3'},
        /*  82  */ {KEY_INS, '0'},
        /*  83  */ {KEY_DEL, '.'},
        /*  84  */ {KEY_NONE, KEY_NONE},
        /*  85  */ {KEY_NONE, KEY_NONE},
        /*  86  */ {KEY_NONE, KEY_NONE},
        /*  87  */ {KEY_F11, KEY_F11},
        /*  88  */ {KEY_F12, KEY_F12}
    };

/**
 * 입력 값이 알파벳인지 확인하는 함수
*/
BOOL IsAlphabetScanCode(BYTE bScanCode)
{
    if (('a' <= gs_vstKeyMappingTable[bScanCode].bNormalCode) &&
        (gs_vstKeyMappingTable[bScanCode].bNormalCode <= 'z'))
    {
        return TRUE;
    }
    return FALSE;
}

/**
 * 숫자 또는 기호 범위인지 확인하는 함수
*/
BOOL IsNumberOrSymbolScanCode(BYTE bScanCode)
{
    // 스캔코드 2 ~ 53 사이에서 영문자가 아니면 숫자 또는 기호라고 함
    if ((2 <= bScanCode) && (bScanCode <= 53) &&
        (IsAlphabetScanCode(bScanCode) == FALSE))
    {
        return TRUE;
    }

    return FALSE;
}

/**
 * 숫자 패드 범위인지 확인하는 함수
*/
BOOL IsNumberPadScanCode(BYTE bScanCode)
{
    // 숫자 패드는 스캔코드 71 ~ 83 사이에 있다고 함
    if ((71 <= bScanCode) && (bScanCode <= 83))
    {
        return TRUE;
    }

    return FALSE;
}

/**
 * 조합키를 사용해야 하는지 확인하는 함수
*/
BOOL IsUseCombinedCode(BOOL bScanCode)
{
    BYTE bDownScanCode;
    BOOL bUseCombinedKey;

    bDownScanCode = bScanCode & 0x7F; // 스캔코드 값을 키가 눌렸을 때 값으로 통일하는 것
    // UpScanCode = DownScanCode에서 최상위 비트를 1로 설정한 값
    // 다르게는 0x80을 더한 값
    // 7F = 0111 1111

    // 알파벳 키이면 Shift와 Caps Lock에 영향을 받으므로 해당 플래그 설정
    if (IsAlphabetScanCode(bDownScanCode) == TRUE)
    {
        if (gs_stKeyboardManager.bShiftDown ^ gs_stKeyboardManager.bCapsLockOn)
        {
            bUseCombinedKey = TRUE;
        }
        else
        {
            bUseCombinedKey = FALSE;
        }
    }
    // 숫자 또는 기호면은 Shift 영향을 받음 (기호가 되기 때문)
    else if (IsNumberOrSymbolScanCode(bDownScanCode) == TRUE)
    {
        if (gs_stKeyboardManager.bShiftDown == TRUE)
        {
            bUseCombinedKey = TRUE;
        }
        else
        {
            bUseCombinedKey = FALSE;
        }
    }
    // 숫자 패드 키라면 Num Lock 영향을 받음
    else if ((IsNumberPadScanCode(bDownScanCode) == TRUE) && (gs_stKeyboardManager.bExtendedCodeIn == FALSE))
    {
        if (gs_stKeyboardManager.bNumLockOn == TRUE)
        {
            bUseCombinedKey = TRUE;
        }
        else
        {
            bUseCombinedKey = FALSE;
        }
    }

    return bUseCombinedKey;
}

/**
 * 조합키 상태와 LED 상태를 갱신하는 함수
*/
void UpdateCombinationKeyStatusAndLED(BYTE bScanCode)
{
    BOOL bDown;
    BYTE bDownScanCode;
    BOOL bLEDStatusChanged = FALSE;

    if (bScanCode & 0x80) // UpScanCode인지 확인
    {
        bDown = FALSE;
        bDownScanCode = bScanCode & 0x7F;
    }
    else
    {
        bDown = TRUE;
        bDownScanCode = bScanCode;
    }

    // Shift라면 Shift가 눌렸다고 설정
    if ((bDownScanCode == 42) || (bDownScanCode == 54))
    {
        gs_stKeyboardManager.bShiftDown = bDown;
    }
    // Caps Lock 설정 후 LED 상태 변경
    else if ((bDownScanCode == 58) && (bDown == TRUE))
    {
        gs_stKeyboardManager.bCapsLockOn ^= TRUE;
        bLEDStatusChanged = TRUE;
    }
    // Num Lock 설정 후 LED 상태 변경
    else if ((bDownScanCode == 69) && (bDown == TRUE))
    {
        gs_stKeyboardManager.bNumLockOn ^= TRUE;
        bLEDStatusChanged = TRUE;
    }
    // Scroll Lock 설정 후 LED 상태 변경
    else if ((bDownScanCode == 70) && (bDown == TRUE))
    {
        gs_stKeyboardManager.bScrollLockOn ^= TRUE;
        bLEDStatusChanged = TRUE;
    }

    // LED 상태가 변경되었으면 키보드 커맨드를 전송하여 LED 변경
    if (bLEDStatusChanged == TRUE)
    {
        ChangeKeyboardLED(gs_stKeyboardManager.bCapsLockOn, gs_stKeyboardManager.bNumLockOn, gs_stKeyboardManager.bScrollLockOn);
    }
}

/**
 * 스캔코드를 아스키코드로 변경하는 함수
*/
BOOL ConvertScanCodeToASCIICode(BYTE bScanCode, BYTE *pbASCIICode, BOOL *pbFlags)
{
    BOOL bUseCombinedKey;

    if (gs_stKeyboardManager.iSkipCountForPause > 0) // Pause 수신여부 확인
    {
        gs_stKeyboardManager.iSkipCountForPause--;
        return FALSE;
    }

    if (bScanCode == 0xE1) // Pause 키인지 확인, 플래그 설정 후 종료
    {
        *pbASCIICode = KEY_PAUSE;
        *pbFlags = KEY_FLAGS_DOWN;
        gs_stKeyboardManager.iSkipCountForPause = KEY_SKIPCOUNTFORPAUSE;
        return TRUE;
    }
    // 확장 키인지 확인, 실제 키 값은 다음에 들어오니까 플래그만 설정 후 종료
    else if (bScanCode == 0xE0)
    {
        gs_stKeyboardManager.bExtendedCodeIn = TRUE;
        return FALSE;
    }

    bUseCombinedKey = IsUseCombinedCode(bScanCode); // 스캔코드가 조합키인지 확인

    if (bUseCombinedKey == TRUE) // 조합 키면은 조합 키 값으로 설정
    {
        *pbASCIICode = gs_vstKeyMappingTable[bScanCode & 0x7F].bCombinedCode;
    }
    else
    {
        *pbASCIICode = gs_vstKeyMappingTable[bScanCode & 0x7F].bNormalCode;
    }

    // 확장 키이면 확장 키임을 설정
    if (gs_stKeyboardManager.bExtendedCodeIn == TRUE)
    {
        *pbFlags = KEY_FLAGS_EXTENDEDKEY;
        gs_stKeyboardManager.bExtendedCodeIn = FALSE;
    }
    else
    {
        *pbFlags = 0;
    }

    if ((bScanCode & 0x80) == 0) // 키보드가 눌러짐 또는 떨어짐 유무 설정
    {
        *pbFlags |= KEY_FLAGS_DOWN;
    }

    UpdateCombinationKeyStatusAndLED(bScanCode); // 키보드 상태 갱신
    return TRUE;
}

/**
 * 키보드 초기화 하는 함수
*/
BOOL InitializeKeyboard(void)
{
    InitializeQueue(&gs_stKeyQueue, gs_vstKeyQueueBuffer, KEY_MAXQUEUECOUNT, sizeof(KEYDATA));

    return ActivateKeyboard();
}

/**
 * 스캔코드를 아스키로 변환 후 큐에 삽입
 * 큐에 삽입 시 인터럽트 비활성화
*/
BOOL ConvertScanCodeAndPutQueue(BYTE bScanCode)
{
    KEYDATA stData;
    BOOL bResult = FALSE;
    BOOL bPreviousInterrupt;

    stData.bScanCode = bScanCode;

    if (ConvertScanCodeToASCIICode(bScanCode, &(stData.bASCIICode), &(stData.bFlags)) == TRUE) // 변환에 성공했으면 큐에 삽입
    {
        bPreviousInterrupt = LockForSystemData();
        
        // 큐에 stData에 저장된 값을 삽입
        bResult = PutQueue(&gs_stKeyQueue, &stData); 

        UnlockForSystemData(bPreviousInterrupt);
    }

    return bResult;
}

/**
 * 큐에서 키를 가져옴
 * 큐에서 값 삭제 시 인터럽트 비활성화
*/
BOOL GetKeyFromKeyQueue(KEYDATA *pstData)
{
    BOOL bResult;
    BOOL bPreviousInterrupt;

    if (IsQueueEmpty(&gs_stKeyQueue) == TRUE)
    {
        return FALSE;
    }

    bPreviousInterrupt = LockForSystemData();

    bResult = GetQueue(&gs_stKeyQueue, pstData);

    UnlockForSystemData(bPreviousInterrupt);
    
    return bResult;
}