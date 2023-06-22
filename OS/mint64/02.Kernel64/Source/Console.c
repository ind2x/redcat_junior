#include <stdarg.h>
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"
#include "AssemblyUtility.h"

CONSOLEMANAGER gs_stConsoleManager;

void InitializeConsole(int iX, int iY)
{
    MemSet(&gs_stConsoleManager, 0, sizeof(gs_stConsoleManager));
    
    SetCursor(iX, iY);
}

/**
 * 커서의 위치를 설정하는 함수
*/
void SetCursor(int iX, int iY)
{
    int iLinearValue;

    iLinearValue = iY * CONSOLE_WIDTH + iX;
    
    // 주소 레지스터의 상위 커서 레지스터 지정
    OutPortByte(VGA_PORT_INDEX, VGA_INDEX_UPPERCURSOR);
    // 데이터 레지스터의 상위 바이트에 커서 위치 전달
    OutPortByte(VGA_PORT_DATA, iLinearValue >> 8);

    // 주소 레지스터의 하위 커서 레지스터 지정
    OutPortByte(VGA_PORT_INDEX, VGA_INDEX_LOWERCURSOR);
    // 데이터 레지스터의 하위 바이트에 커서 위치 전달
    OutPortByte(VGA_PORT_DATA, iLinearValue & 0xFF);

    // 문자를 출력할 위치 업데이트
    gs_stConsoleManager.iCurrentPrintOffset = iLinearValue;
}

/**
 * 현재 커서의 위치 반환하는 함수
*/
void GetCursor(int *piX, int *piY)
{
    *piX = gs_stConsoleManager.iCurrentPrintOffset % CONSOLE_WIDTH;
    *piY = gs_stConsoleManager.iCurrentPrintOffset / CONSOLE_WIDTH;
}

/**
 * VSPrintf 함수를 이용한 printf 함수 구현
*/
void Printf(const char *pcFormatString, ...)
{
    va_list ap;
    char vcBuffer[1024];
    int iNextPrintOffset;

    // 가변 인자 사용
    va_start(ap, pcFormatString);
    VSPrintf(vcBuffer, pcFormatString, ap);
    va_end(ap);

    // 포맷 문자열을 화면에 출력
    iNextPrintOffset = ConsolePrintString(vcBuffer);

    // 커서의 위치를 업데이트
    SetCursor(iNextPrintOffset % CONSOLE_WIDTH, iNextPrintOffset / CONSOLE_WIDTH);
}

/**
 * 문자를 출력한 후, 화면상의 다음 출력 위치를 반환하는 함수
*/
int ConsolePrintString(const char *pcBuffer)
{
    CHARACTER *pstScreen = (CHARACTER *)CONSOLE_VIDEOMEMORYADDRESS;

    int i, j, iLength, iPrintOffset;

    iPrintOffset = gs_stConsoleManager.iCurrentPrintOffset;

    iLength = StrLen(pcBuffer);
    for(i=0; i<iLength; i++)    // 문자열 길이만큼 화면에 출력
    {
        if(pcBuffer[i] == '\n')
        {
            iPrintOffset += (CONSOLE_WIDTH - (iPrintOffset % CONSOLE_WIDTH));
        }
        else if(pcBuffer[i] == '\t')
        {
            iPrintOffset += (8 - (iPrintOffset % 8));
        }
        else
        {
            pstScreen[iPrintOffset].bCharactor = pcBuffer[i];
            pstScreen[iPrintOffset].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
            iPrintOffset++;
        }

        // 출력위치가 화면의 최대값을 넘어가면 스크롤 효과처럼 화면을 한칸 씩 올림 
        if(iPrintOffset >= (CONSOLE_WIDTH * CONSOLE_HEIGHT))
        {   
            // 가장 윗줄을 제외한 나머지를 한 줄 씩 올림
            MemCpy(CONSOLE_VIDEOMEMORYADDRESS, CONSOLE_VIDEOMEMORYADDRESS + CONSOLE_WIDTH * sizeof(CHARACTER), (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH * sizeof(CHARACTER));

            // 가장 마지막 라인은 공백으로 설정
            for (j = (CONSOLE_HEIGHT - 1) * (CONSOLE_WIDTH); j < (CONSOLE_HEIGHT * CONSOLE_WIDTH); j++)
            {
                // 공백 출력
                pstScreen[j].bCharactor = ' ';
                pstScreen[j].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
            }

            // 출력할 라인을 마지막 라인으로 업데이트 후 반환
            iPrintOffset = (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH;
        }
    }
    return iPrintOffset;
}

/**
 * 화면을 삭제하는 함수
*/
void ClearScreen(void)
{
    CHARACTER *pstScreen = (CHARACTER *) CONSOLE_VIDEOMEMORYADDRESS;
    int i;

    // 화면 전체를 공백으로 초기화
    for(i=0; i<CONSOLE_WIDTH * CONSOLE_HEIGHT; i++)
    {
        pstScreen[i].bCharactor = ' ';
        pstScreen[i].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
    }

    // 공백으로 초기화 후 커서 위치를 가장 첫 부분으로 업데이트
    SetCursor(0,0);
}

/**
 * 셸에서 바로 키를 출력할 수 있도록 큐에서 키 값을 가져오는 함수
*/
BYTE GetCh(void)
{
    KEYDATA stData;

    while(1)
    {
        while(GetKeyFromKeyQueue(&stData) == FALSE)
        {
            Schedule();
        }
        
        if(stData.bFlags & KEY_FLAGS_DOWN) 
        {
            return stData.bASCIICode;
        }
    }
}

void PrintStringXY(int iX, int iY, const char *pcString)
{
    CHARACTER *pstScreen = (CHARACTER *)CONSOLE_VIDEOMEMORYADDRESS;
    int i;

    pstScreen += (iY * CONSOLE_WIDTH) + iX;

    for(i=0; pcString[i] != 0; i++)
    {
        pstScreen[i].bCharactor = pcString[i];
        pstScreen[i].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
    }
}