#include "ConsoleShell.h"
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"
#include "PIT.h"
#include "RTC.h"
#include "AssemblyUtility.h"

SHELLCOMMANDENTRY gs_vstCommandTable[] =
{
    {"help", "Show Help", Help},
    {"clear", "Clear Screen", Cls},
    {"totalram", "Show Total RAM Size", ShowTotalRAMSize},
    {"strtod", "String To Decimal/Hex Convert", StringToDecimalHexTest},
    {"reboot", "Shutdown And Reboot OS", ShutDownAndReboot},
    {"settimer", "Set PIT Controller Counter0, ex) settimer 10(ms) 1(periodic)", SetTimer},
    {"wait", "Wait ms Using PIT, ex) wait 100(ms)", WaitUsingPIT},
    {"rdtsc", "Read Time Stamp Counter", ReadTimeStampCounter},
    {"cpuspeed", "Measure Processor Speed", MeasureProcessorSpeed},
    {"date", "Show Date And Time", ShowDateAndTime},
};

void StartConsoleShell(void)
{
    char vcCommandBuffer[CONSOLESHELL_MAXCOMMANDBUFFERCOUNT];
    int iCommandBufferIndex = 0;
    BYTE bKey;
    int iCursorX, iCursorY;

    Printf(CONSOLESHELL_PROMPTMESSAGE);

    while(1)
    {
        bKey = GetCh();
        if(bKey == KEY_BACKSPACE)
        {
            if(iCommandBufferIndex > 0)
            {
                GetCursor(&iCursorX, &iCursorY);
                PrintStringXY(iCursorX - 1, iCursorY, " ");
                SetCursor(iCursorX - 1, iCursorY);
                iCommandBufferIndex--;
            }
        }

        else if (bKey == KEY_ENTER)
        {
            Printf("\n");

            if(iCommandBufferIndex > 0)
            {
                vcCommandBuffer[iCommandBufferIndex] = '\0';
                ExecuteCommand(vcCommandBuffer);
            }

            Printf("%s", CONSOLESHELL_PROMPTMESSAGE);
            MemSet(vcCommandBuffer, '\0', CONSOLESHELL_MAXCOMMANDBUFFERCOUNT);
            iCommandBufferIndex = 0;
        }

        else if ((bKey == KEY_LSHIFT) || (bKey == KEY_RSHIFT) || (bKey == KEY_CAPSLOCK) || (bKey == KEY_NUMLOCK) || (bKey == KEY_SCROLLLOCK))
        {
            ;
        }

        else
        {
            if(bKey == KEY_TAB) 
            {
                bKey = ' ';
            }

            if (iCommandBufferIndex < CONSOLESHELL_MAXCOMMANDBUFFERCOUNT)
            {
                vcCommandBuffer[iCommandBufferIndex++] = bKey;
                Printf("%c", bKey);
            }
        }
    }
}

void ExecuteCommand(const char *pcCommandBuffer)
{
    int i, iSpaceIndex;
    int iCommandBufferLength, iCommandLength;
    int iCount;

    iCommandBufferLength = Strlen(pcCommandBuffer);
    for (iSpaceIndex = 0; iSpaceIndex < iCommandBufferLength; iSpaceIndex++)
    {
        if (pcCommandBuffer[iSpaceIndex] == ' ')
        {
            break;
        }
    }

    iCount = sizeof(gs_vstCommandTable)/ sizeof(SHELLCOMMANDENTRY);
    for(i=0; i<iCount; i++)
    {
        iCommandLength = Strlen(gs_vstCommandTable[i].pcCommand);
        if((iCommandLength == iSpaceIndex) && (MemCmp(gs_vstCommandTable[i].pcCommand, pcCommandBuffer, iSpaceIndex) == 0))
        {
            gs_vstCommandTable[i].pfFunction(pcCommandBuffer + iSpaceIndex + 1);
            break;
        }
    }

    if(i >= iCount)
    {
        Printf("'%s' is not found.\n", pcCommandBuffer);
    }
}

void InitializeParameter(PARAMETERLIST *pstList, const char *pcParameter)
{
    pstList->pcBuffer = pcParameter;
    pstList->iLength = Strlen(pcParameter);
    pstList->iCurrentPosition = 0;

}

int GetNextParameter(PARAMETERLIST *pstList, char *pcParameter)
{
    int i, iLength;

    if(pstList->iLength <= pstList->iCurrentPosition) {
        return 0;
    }

    for(i = pstList->iCurrentPosition; i<pstList->iLength; i++)
    {
        if(pstList->pcBuffer[i] == ' ') {
            break;
        }
    }

    MemCpy(pcParameter, pstList->pcBuffer + pstList->iCurrentPosition, i);
    iLength = i - pstList->iCurrentPosition;
    pcParameter[iLength] = '\0';

    pstList->iCurrentPosition += iLength + 1;
    return iLength;

}

void Help(const char *pcParameterBuffer)
{
    int i;
    int iCount;
    int iCursorX, iCursorY;
    int iLength, iMaxCommandLength = 0;

    Printf("\n");
    Printf("=========================================================\n");
    Printf("                           Help                          \n");
    Printf("=========================================================\n");

    iCount = sizeof(gs_vstCommandTable) / sizeof(SHELLCOMMANDENTRY);

    for (i = 0; i < iCount; i++)
    {
        iLength = Strlen(gs_vstCommandTable[i].pcCommand);
        if (iLength > iMaxCommandLength)
        {
            iMaxCommandLength = iLength;
        }
    }

    for (i = 0; i < iCount; i++)
    {
        Printf("%s", gs_vstCommandTable[i].pcCommand);
        GetCursor(&iCursorX, &iCursorY);
        SetCursor(iMaxCommandLength, iCursorY);
        Printf("  - %s\n", gs_vstCommandTable[i].pcHelp);
    }

    Printf("\n");
}

void Cls(const char *pcParameterBuffer)
{
    ClearScreen();
    SetCursor(0, 1);
}

void ShowTotalRAMSize(const char *pcParameterBuffer)
{
    Printf("[*] Total RAM Size = %d MB\n", GetTotalRAMSize());

}

void StringToDecimalHexTest(const char *pcParameterBuffer)
{
    char vcParameter[100];
    int iLength;
    PARAMETERLIST stList;
    int iCount = 0;
    long lValue;

    // 파라미터 초기화
    InitializeParameter(&stList, pcParameterBuffer);

    while (1)
    {
        // 다음 파라미터를 구함, 파라미터의 길이가 0이면 파라미터가 없는 것이므로
        // 종료
        iLength = GetNextParameter(&stList, vcParameter);
        if (iLength == 0)
        {
            break;
        }

        // 파라미터에 대한 정보를 출력하고 16진수인지 10진수인지 판단하여 변환한 후
        // 결과를 printf로 출력
        Printf("Param %d = '%s', Length = %d, ", iCount + 1,
                vcParameter, iLength);

        // 0x로 시작하면 16진수, 그외는 10진수로 판단
        if (MemCmp(vcParameter, "0x", 2) == 0)
        {
            lValue = AToI(vcParameter + 2, 16);
            Printf("HEX Value = %q\n", lValue);
        }
        else
        {
            lValue = AToI(vcParameter, 10);
            Printf("Decimal Value = %d\n", lValue);
        }

        iCount++;
    }
}

void ShutDownAndReboot(const char *pcParamegerBuffer)
{
    Printf("[*] System Shutdown........\n");
    Printf("[*] Press any key to restart.....");
    GetCh();
    Reboot();
}

void SetTimer(const char *pcParameterBuffer)
{
    char vcParameter[100];
    PARAMETERLIST stList;
    long lValue;
    BOOL bPeriodic;

    InitializeParameter(&stList, pcParameterBuffer);

    if(GetNextParameter(&stList, vcParameter) == 0)
    {
        Printf("[!] ex) settimer 10(ms) 1(periodic)\n");
        return;
    }
    lValue = AToI(vcParameter, 10);

    if(GetNextParameter(&stList, vcParameter) == 0)
    {
        Printf("[!] ex) settimer 10(ms) 1(periodic)\n");
        return;
    }
    bPeriodic = AToI(vcParameter, 10);

    InitializePIT(MSTOCOUNT(lValue), bPeriodic);
    Printf("[*] Time = %d ms, Periodic = %d Change Complete..\n", lValue, bPeriodic);
}

void WaitUsingPIT(const char *pcParameterBuffer)
{
    char vcParameter[100];
    int iLength;
    PARAMETERLIST stList;
    long lMillisecond;
    int i;

    InitializeParameter(&stList, pcParameterBuffer);
    if( GetNextParameter( &stList, vcParameter ) == 0 )
    {
        Printf( "[!] ex)wait 100(ms)\n" );
        return ;
    }

    lMillisecond = AToI(pcParameterBuffer, 10);
    Printf("[*] %d ms Sleep Start....\n", lMillisecond);

    DisableInterrupt();
    for(i=0; i<lMillisecond/30; i++)
    {
        WaitUsingDirectPIT(MSTOCOUNT(30));
    }
    WaitUsingDirectPIT( MSTOCOUNT( lMillisecond % 30 ) );   
    EnableInterrupt();
    Printf( "[*] %d ms Sleep Complete...\n", lMillisecond );
    
    InitializePIT( MSTOCOUNT( 1 ), TRUE );
}

void ReadTimeStampCounter(const char *pcParameterBuffer)
{
    QWORD qwTSC;
    
    qwTSC = ReadTSC();
    Printf( "[*] Time Stamp Counter = %q\n", qwTSC );
}

void MeasureProcessorSpeed(const char *pcParameterBuffer)
{
    int i;
    QWORD qwLastTSC, qwTotalTSC = 0;
        
    Printf("[*] Measuring speed of CPU");
    DisableInterrupt();

    for(i=1; i<=200; i++)
    {
        qwLastTSC = ReadTSC();
        WaitUsingDirectPIT(MSTOCOUNT(50));
        qwTotalTSC += ReadTSC() - qwLastTSC;

        Printf(".");
    }

    InitializePIT(MSTOCOUNT(1), TRUE);
    EnableInterrupt();

    Printf("\n[*] CPU Speed = %d MHz\n", qwTotalTSC / 10 / 1000 / 1000);
}

void ShowDateAndTime(const char *pcParameterBuffer)
{
    BYTE bSecond, bMinute, bHour;
    BYTE bDayOfWeek, bDayOfMonth, bMonth;
    WORD wYear;

    ReadRTCTime( &bHour, &bMinute, &bSecond );
    ReadRTCDate( &wYear, &bMonth, &bDayOfMonth, &bDayOfWeek );
    
    Printf( "[*] Date: %d/%d/%d %s\n", wYear, bMonth, bDayOfMonth, ConvertDayOfWeekToString( bDayOfWeek ) );
    Printf( "[*] Time: %d:%d:%d\n", bHour, bMinute, bSecond );
}