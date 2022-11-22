#include "ConsoleShell.h"
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"
#include "PIT.h"
#include "RTC.h"
#include "AssemblyUtility.h"
#include "Task.h"
#include "Synchronization.h"

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
    {"createtask", "Create Task ex) createtask 1(type) 10(count), press 'q' to quit", CreateTestTask},
    {"changepriority", "Change Task Priority, ex) changepriority 1(ID) 2(Priority)", ChangeTaskPriority},
    {"tasklist", "Show Task List", ShowTaskList},
    {"killtask", "End Task, ex) killtask 1(ID) or 0xffffffff(All Task)", KillTask},
    {"cpuload", "Show Processor Load", CPULoad},
    {"testmutex", "Test Mutex Function", TestMutex},
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

static void Help(const char *pcParameterBuffer)
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

static void Cls(const char *pcParameterBuffer)
{
    ClearScreen();
    SetCursor(0, 1);
}

static void ShowTotalRAMSize(const char *pcParameterBuffer)
{
    Printf("[*] Total RAM Size = %d MB\n", GetTotalRAMSize());

}

static void StringToDecimalHexTest(const char *pcParameterBuffer)
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

static void ShutDownAndReboot(const char *pcParamegerBuffer)
{
    Printf("[*] System Shutdown........\n");
    Printf("[*] Press any key to restart.....");
    GetCh();
    Reboot();
}

static void SetTimer(const char *pcParameterBuffer)
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

static void WaitUsingPIT(const char *pcParameterBuffer)
{
    char vcParameter[100];
    int iLength;
    PARAMETERLIST stList;
    long lMillisecond;
    int i;

    InitializeParameter(&stList, pcParameterBuffer);
    if( GetNextParameter( &stList, vcParameter ) == 0 )
    {
        Printf( "[!] ex) wait 100(ms)\n" );
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

static void ReadTimeStampCounter(const char *pcParameterBuffer)
{
    QWORD qwTSC;
    
    qwTSC = ReadTSC();
    Printf( "[*] Time Stamp Counter = %q\n", qwTSC );
}

static void MeasureProcessorSpeed(const char *pcParameterBuffer)
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

static void ShowDateAndTime(const char *pcParameterBuffer)
{
    BYTE bSecond, bMinute, bHour;
    BYTE bDayOfWeek, bDayOfMonth, bMonth;
    WORD wYear;

    ReadRTCTime( &bHour, &bMinute, &bSecond );
    ReadRTCDate( &wYear, &bMonth, &bDayOfMonth, &bDayOfWeek );
    
    Printf( "[*] Date: %d/%d/%d %s\n", wYear, bMonth, bDayOfMonth, ConvertDayOfWeekToString( bDayOfWeek ) );
    Printf( "[*] Time: %d:%d:%d\n", bHour, bMinute, bSecond );
}

static TCB gs_vstTask[2] = {0, };
static QWORD gs_vstStack[1024] = {0, };

static void TestTask1(void)
{
    BYTE bData;
    int i = 0, iX = 0, iY = 0, iMargin, j;
    CHARACTER *pstScreen = (CHARACTER *)CONSOLE_VIDEOMEMORYADDRESS;
    TCB *pstRunningTask;

    // 자신의 ID를 얻어서 화면 오프셋으로 사용
    pstRunningTask = GetRunningTask();
    iMargin = (pstRunningTask->stLink.qwID & 0xFFFFFFFF) % 10;

    // 화면 네 귀퉁이를 돌면서 문자 출력
    for(j=0; j<2000; j++)
    {
        switch (i)
        {
        case 0:
            iX++;
            if (iX >= (CONSOLE_WIDTH - iMargin))
            {
                i = 1;
            }
            break;

        case 1:
            iY++;
            if (iY >= (CONSOLE_HEIGHT - iMargin))
            {
                i = 2;
            }
            break;

        case 2:
            iX--;
            if (iX < iMargin)
            {
                i = 3;
            }
            break;

        case 3:
            iY--;
            if (iY < iMargin)
            {
                i = 0;
            }
            break;
        }

        pstScreen[iY * CONSOLE_WIDTH + iX].bCharactor = bData;
        pstScreen[iY * CONSOLE_WIDTH + iX].bAttribute = bData & 0x0F;
        bData++;

        //Schedule();
    }

    ExitTask();
}

static void TestTask2(void)
{
    int i = 0, iOffset;
    CHARACTER *pstScreen = (CHARACTER *)CONSOLE_VIDEOMEMORYADDRESS;
    TCB *pstRunningTask;
    char vcData[4] = {'-', '\\', '|', '/'};

    pstRunningTask = GetRunningTask();
    iOffset = (pstRunningTask->stLink.qwID & 0xFFFFFFFF) * 2;
    iOffset = CONSOLE_WIDTH * CONSOLE_HEIGHT -
              (iOffset % (CONSOLE_WIDTH * CONSOLE_HEIGHT));

    while (1)
    {
        pstScreen[iOffset].bCharactor = vcData[i % 4];
        pstScreen[iOffset].bAttribute = (iOffset % 15) + 1;
        i++;

        //Schedule();
    }
}

static void CreateTestTask(const char *pcParameterBuffer)
{
    PARAMETERLIST stList;
    char vcType[30];
    char vcCount[30];
    int i;

    InitializeParameter(&stList, pcParameterBuffer);
    GetNextParameter(&stList, vcType);
    GetNextParameter(&stList, vcCount);

    switch (AToI(vcType, 10))
    {
    case 1:
        for (i = 0; i < AToI(vcCount, 10); i++)
        {
            if (CreateTask(TASK_FLAGS_LOW, (QWORD)TestTask1) == NULL)
            {
                break;
            }
        }

        Printf("[*] Task1 %d Created\n", i);
        break;

    case 2:
    default:
        for (i = 0; i < AToI(vcCount, 10); i++)
        {
            if (CreateTask(TASK_FLAGS_LOW, (QWORD)TestTask2) == NULL)
            {
                break;
            }
        }

        Printf("[*] Task2 %d Created\n", i);
        break;
    }
}

static void ChangeTaskPriority(const char *pcParameterBuffer)
{
    PARAMETERLIST stList;
    char vcID[30];
    char vcPriority[30];
    QWORD qwID;
    BYTE bPriority;

    InitializeParameter(&stList, pcParameterBuffer);
    GetNextParameter(&stList, vcID);
    GetNextParameter(&stList, vcPriority);

    if (MemCmp(vcID, "0x", 2) == 0)
    {
        qwID = AToI(vcID + 2, 16);
    }
    else
    {
        qwID = AToI(vcID, 10);
    }

    bPriority = AToI(vcPriority, 10);

    Printf("[*] Change Task Priority ID [0x%q] Priority[%d]..... ", qwID, bPriority);
    if (ChangePriority(qwID, bPriority) == TRUE)
    {
        Printf("Success\n");
    }
    else
    {
        Printf("Fail\n");
    }
}


static void ShowTaskList(const char *pcParameterBuffer)
{
    int i;
    TCB *pstTCB;
    int iCount = 0;

    Printf("=========== Task Total Count [%d] ===========\n", GetTaskCount());
    for (i = 0; i < TASK_MAXCOUNT; i++)
    {
        pstTCB = GetTCBInTCBPool(i);
        if ((pstTCB->stLink.qwID >> 32) != 0)
        {

            if ((iCount != 0) && ((iCount % 10) == 0))
            {
                Printf("Press any key to continue... ('q' is exit) : ");
                if (GetCh() == 'q')
                {
                    Printf("\n");
                    break;
                }
                Printf("\n");
            }

            Printf("[%d] Task ID[0x%Q], Priority[%d], Flags[0x%Q]\n", 1 + iCount++, pstTCB->stLink.qwID, GETPRIORITY(pstTCB->qwFlags),pstTCB->qwFlags);
        }
    }
}

static void KillTask(const char *pcParameterBuffer)
{
    PARAMETERLIST stList;
    char vcID[30];
    QWORD qwID;
    TCB *pstTCB;
    int i;

    InitializeParameter(&stList, pcParameterBuffer);
    GetNextParameter(&stList, vcID);

    if (MemCmp(vcID, "0x", 2) == 0)
    {
        qwID = AToI(vcID + 2, 16);
    }
    else
    {
        qwID = AToI(vcID, 10);
    }

    if(qwID != 0xFFFFFFFFF)
    {
        Printf("[*] Kill Task ID [0x%q] ", qwID);
        if (EndTask(qwID) == TRUE)
        {
            Printf("Success\n");
        }
        else
        {
            Printf("Fail\n");
        }
    }
    else
    {
        for (i = 2; i < TASK_MAXCOUNT; i++)
        {
            pstTCB = GetTCBInTCBPool(i);
            qwID = pstTCB->stLink.qwID;
            
            if ((qwID >> 32) != 0)
            {
                Printf("[*] Kill Task ID [0x%q] ", qwID);
                if (EndTask(qwID) == TRUE)
                {
                    Printf("Success\n");
                }
                else
                {
                    Printf("Fail\n");
                }
            }
        }
    }
}

static void CPULoad(const char *pcParameterBuffer)
{
    Printf("Processor Load : %d%%\n", GetProcessorLoad());
}

static MUTEX gs_stMutex;
static volatile QWORD gs_qwAdder;

static void PrintNumberTask(void)
{
    int i;
    int j;
    QWORD qwTickCount;

    qwTickCount = GetTickCount();
    while ((GetTickCount() - qwTickCount) < 50)
    {
        Schedule();
    }

    for (i = 0; i < 5; i++)
    {
        Lock(&(gs_stMutex));
        Printf("[*] Task ID [0x%Q] Value[%d]\n", GetRunningTask()->stLink.qwID, gs_qwAdder);

        gs_qwAdder += 1;
        Unlock(&(gs_stMutex));

        for (j = 0; j < 30000; j++)
            ;
    }

    qwTickCount = GetTickCount();
    while ((GetTickCount() - qwTickCount) < 1000)
    {
        Schedule();
    }

    ExitTask();
}

static void TestMutex(const char *pcParameterBuffer)
{
    int i;

    gs_qwAdder = 1;

    InitializeMutex(&gs_stMutex);

    for (i = 0; i < 3; i++)
    {
        CreateTask(TASK_FLAGS_LOW, (QWORD)PrintNumberTask);
    }
    
    Printf("[*] Wait Util %d Task End.......\n", i);
    GetCh();
}