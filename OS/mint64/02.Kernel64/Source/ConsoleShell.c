#include "ConsoleShell.h"
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"
#include "PIT.h"
#include "PIC.h"
#include "RTC.h"
#include "AssemblyUtility.h"
#include "Task.h"
#include "Synchronization.h"
#include "DynamicMemory.h"
#include "HardDisk.h"
#include "FileSystem.h"
#include "SerialPort.h"
#include "MPConfigurationTable.h"
#include "LocalAPIC.h"
#include "MultiProcessor.h"
#include "IOAPIC.h"
#include "InterruptHandler.h"

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
        {"createtask", "Create Task ex) createtask 1(type) 10(count), press 'q'", CreateTestTask},
        {"changepriority", "Change Task Priority, ex) changepriority 1(ID) 2(Priority)", ChangeTaskPriority},
        {"tasklist", "Show Task List", ShowTaskList},
        {"killtask", "End Task, ex) killtask 1(ID) or 0xffffffff(All Task)", KillTask},
        {"cpuload", "Show Processor Load", CPULoad},
        {"testmutex", "Test Mutex Function", TestMutex},
        {"testthread", "Test Thread And Process Function", TestThread},
        {"showmatrix", "Show Matrix Screen", ShowMatrix},
        {"testpie", "Test PIE Calculation", TestPIE},
        {"dynamicmeminfo", "Show Dyanmic Memory Information", ShowDyanmicMemoryInformation},
        {"testseqalloc", "Test Sequential Allocation & Free", TestSequentialAllocation},
        {"testranalloc", "Test Random Allocation & Free", TestRandomAllocation},
        {"hddinfo", "Show HDD Information", ShowHDDInformation},
        {"readsector", "Read HDD Sector, ex) readsector 0(LBA) 10(count)", ReadSector},
        {"writesector", "Write HDD Sector, ex) writesector 0(LBA) 10(count)", WriteSector},
        {"mounthdd", "Mount HDD", MountHDD},
        {"formathdd", "Format HDD", FormatHDD},
        {"filesysinfo", "Show File System Information", ShowFileSystemInformation},
        {"touch", "Create File, ex) touch a.txt", CreateFileInRootDirectory},
        {"rm", "Delete File, ex) deletefile a.txt", DeleteFileInRootDirectory},
        {"ls", "Show Directory", ShowRootDirectory},
        {"writefile", "Write Data To File, ex) writefile a.txt", WriteDataToFile},
        {"cat", "Read Data From File, ex) cat a.txt", ReadDataFromFile},
        {"testfileio", "Test File I/O Function", TestFileIO},
        {"testperformance", "Test File Read/Write Performance", TestPerformance},
        {"flush", "Flush File System Cache", FlushCache},
        {"download", "Download Data From Serial, ex) download a.txt", DownloadFile},
        {"showmpinfo", "Show MP Configuration Table Information", ShowMPConfigurationTable},
        {"startap", "Start Application Processor", StartApplicationProcessor},
        {"startsymmetricio", "Start Symmetric I/O Mode", StartSymmetricIOMode},
        {"showirqintinmap", "Show IRQ->INITIN Mapping Table", ShowIRQINTINMappingTable},
        {"showintproccount", "Show Interrupt Processing Count", ShowInterruptProcessingCount},
        {"startintloadbal", "Start Interrupt Load Balancing", StartInterruptLoadBalancing},
};

/**
 * 셸의 메인 루프
*/
void StartConsoleShell(void)
{
    char vcCommandBuffer[CONSOLESHELL_MAXCOMMANDBUFFERCOUNT];
    int iCommandBufferIndex = 0;
    BYTE bKey;
    int iCursorX, iCursorY;

    Printf("\n");
    Printf(CONSOLESHELL_PROMPTMESSAGE); // 셸 프롬프트 출력

    while(1)
    {
        bKey = GetCh(); // 키를 받을 때 까지 대기
        if(bKey == KEY_BACKSPACE) // bacpspace 처리
        {
            if(iCommandBufferIndex > 0)
            {
                GetCursor(&iCursorX, &iCursorY);
                PrintStringXY(iCursorX - 1, iCursorY, " ");
                SetCursor(iCursorX - 1, iCursorY);
                iCommandBufferIndex--;
            }
        }

        else if (bKey == KEY_ENTER) // enter 처리
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
            if(bKey == KEY_TAB)  // TAB은 공백으로 치환
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

/**
 * 명령어 실행해주는 함수
*/
void ExecuteCommand(const char *pcCommandBuffer)
{
    int i, iSpaceIndex;
    int iCommandBufferLength, iCommandLength;
    int iCount;

    // 공백으로 구분된 커맨드를 추출
    iCommandBufferLength = StrLen(pcCommandBuffer);
    for (iSpaceIndex = 0; iSpaceIndex < iCommandBufferLength; iSpaceIndex++)
    {
        if (pcCommandBuffer[iSpaceIndex] == ' ')
        {
            break;
        }
    }
    
    // 커맨드 명과 입력값이 일치하는 명령어 추출 및 검사
    iCount = sizeof(gs_vstCommandTable)/ sizeof(SHELLCOMMANDENTRY);
    for(i=0; i<iCount; i++)
    {
        iCommandLength = StrLen(gs_vstCommandTable[i].pcCommand);
        if((iCommandLength == iSpaceIndex) && (MemCmp(gs_vstCommandTable[i].pcCommand, pcCommandBuffer, iSpaceIndex) == 0))
        {
            // 같으면 함수를 호출
            gs_vstCommandTable[i].pfFunction(pcCommandBuffer + iSpaceIndex + 1);
            break;
        }
    }

    if(i >= iCount)
    {
        Printf("'%s' is not found.\n", pcCommandBuffer);
    }
}

/**
 * 파라미터 초기화 함수
 */
void InitializeParameter(PARAMETERLIST *pstList, const char *pcParameter)
{
    pstList->pcBuffer = pcParameter;
    pstList->iLength = StrLen(pcParameter);
    pstList->iCurrentPosition = 0;

}

/**
 * 공백으로 구분된 파라미터의 내용과 길이를 반환
 */
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

    Printf("=========================================================\n");
    Printf("                           Help                          \n");
    Printf("=========================================================\n");

    // 커맨드 개수 파악
    iCount = sizeof(gs_vstCommandTable) / sizeof(SHELLCOMMANDENTRY);

    // 커맨드 중 가장 긴 커맨드명 길이 가져옴
    for (i = 0; i < iCount; i++)
    {
        iLength = StrLen(gs_vstCommandTable[i].pcCommand);
        if (iLength > iMaxCommandLength)
        {
            iMaxCommandLength = iLength;
        }
    }

    for (i = 0; i < iCount; i++)
    {
        Printf(" %s", gs_vstCommandTable[i].pcCommand);
        GetCursor(&iCursorX, &iCursorY);
        // 글자 짤려서 iMaxCommandLength + 1로 변경
        SetCursor(iMaxCommandLength + 1, iCursorY);
        Printf("  - %s\n", gs_vstCommandTable[i].pcHelp);

        if((i != 0) && ((i % 20) == 0))
        {
            Printf("\n------------ Press any key to continue.... ('q' is exit) : ------------\n");
            if(GetCh() == 'q')
            {
                Printf("\n");
                break;
            }
            Printf("\n");
        }
    }

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

/**
 * 프로세스 리셋하는 함수
*/
static void ShutDownAndReboot(const char *pcParamegerBuffer)
{
    Printf("[*] System Shutdown........\n");
    Printf("[*] Press any key to restart.....");
    GetCh();
    Reboot();
}



/////////////////////////////////////////////////////////////////////////////////
// 시간 관련 함수들

/**
 * PIT 컨트롤러의 카운터 0을 설정하는 함수
 * 물론 디폴트는 Main에서 10ms 1(periodic)으로 설정함
*/
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

/**
 * PIT 컨트롤러를 직접 사용하여 ms 동안 대기
*/
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
        Printf("[*] [!] ex) wait 100(ms)\n" );
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
    
    Printf("[*] %d ms Sleep Complete...\n", lMillisecond );
    
    InitializePIT( MSTOCOUNT( 1 ), TRUE ); // 끝난 후 다시 재설정
}

/**
 * TSC를 읽는 함수
*/
static void ReadTimeStampCounter(const char *pcParameterBuffer)
{
    QWORD qwTSC;
    
    qwTSC = ReadTSC();
    Printf("[*] Time Stamp Counter = %q\n", qwTSC );
}

/**
 * 프로세서의 속도 측정
*/
static void MeasureProcessorSpeed(const char *pcParameterBuffer)
{
    int i;
    QWORD qwLastTSC, qwTotalTSC = 0;
        
    Printf("[*] Measuring speed of CPU");
    DisableInterrupt();

    // 10초 동안 변화한 타임 스탬프 카운터를 이용하여 프로세서의 속도를 간접적으로 측정
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

/**
 * RTC를 읽어서 일자, 시간 정보를 표시하는 함수
*/
static void ShowDateAndTime(const char *pcParameterBuffer)
{
    BYTE bSecond, bMinute, bHour;
    BYTE bDayOfWeek, bDayOfMonth, bMonth;
    WORD wYear;

    ReadRTCTime( &bHour, &bMinute, &bSecond );
    ReadRTCDate( &wYear, &bMonth, &bDayOfMonth, &bDayOfWeek );
    
    Printf("[*] Date: %d/%d/%d %s\n", wYear, bMonth, bDayOfMonth, ConvertDayOfWeekToString( bDayOfWeek ) );
    Printf("[*] Time: %d:%d:%d\n", bHour, bMinute, bSecond );
}



////////////////////////////////////////////////////////////////////////////////////////
// 멀티태스킹 부분 시작

static TCB gs_vstTask[2] = {0, };
static QWORD gs_vstStack[1024] = {0, };

/**
 * 태스크 1
 * 화면 테두리를 돌면서 문자를 출력
*/
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

/**
 * 태스크 2
 * 자신의 ID를 참고하여 특정 위치에 회전하는 바람개비를 출력
*/
static void TestTask2(void)
{
    int i = 0, iOffset;
    CHARACTER *pstScreen = (CHARACTER *)CONSOLE_VIDEOMEMORYADDRESS;
    TCB *pstRunningTask;
    char vcData[4] = {'-', '\\', '|', '/'};

    // 자신의 ID를 얻어서 위치 계산
    pstRunningTask = GetRunningTask();
    iOffset = (pstRunningTask->stLink.qwID & 0xFFFFFFFF) * 2;
    iOffset = CONSOLE_WIDTH * CONSOLE_HEIGHT -
              (iOffset % (CONSOLE_WIDTH * CONSOLE_HEIGHT));

    while (1)
    {
        // 회전하는 바람개비 표시
        pstScreen[iOffset].bCharactor = vcData[i % 4];
        pstScreen[iOffset].bAttribute = (iOffset % 15) + 1;
        i++;
    }
}

/**
 * 태스크를 생성하여 멀티 태스킹 수행
*/
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
            // 낮은 우선순위의 화면 테두리를 돌면서 문자를 출력하는 태스크 생성
            if (CreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, (QWORD) TestTask1) == NULL)
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
            // 낮은 우선순위의 바람개비를 생성하는 태스크 생성
            if (CreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, (QWORD)TestTask2) == NULL)
            {
                break;
            }
        }

        Printf("[*] Task2 %d Created\n", i);
        break;
    }
}

/**
 * 태스크의 우선순위를 변경
*/
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

    // 태스크의 ID를 계산
    if (MemCmp(vcID, "0x", 2) == 0)
    {
        qwID = AToI(vcID + 2, 16);
    }
    else
    {
        qwID = AToI(vcID, 10);
    }

    // 변경하고자 하는 우선순위 설정
    bPriority = AToI(vcPriority, 10);

    Printf("[*] Change Task Priority ID [0x%q] Priority[%d]..... ", qwID, bPriority);
    
    // 인자로 받은 태스크 ID의 우선순위 변경 
    if (ChangePriority(qwID, bPriority) == TRUE)
    {
        Printf("Success\n");
    }
    else
    {
        Printf("Fail\n");
    }
}

/**
 * 현재 생성된 모든 태스크의 정보를 출력 
*/
static void ShowTaskList(const char *pcParameterBuffer)
{
    int i;
    TCB *pstTCB;
    int iCount = 0;

    Printf("\n==================== Task Total Count [%d] ====================\n\n", GetTaskCount());
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

            Printf("[%d] Task ID[0x%Q], Priority[%d], Flags[0x%Q], Thread[%d]\n", 1 + iCount++, pstTCB->stLink.qwID, GETPRIORITY(pstTCB->qwFlags), pstTCB->qwFlags, GetListCount(&(pstTCB->stChildThreadList)));
            
            Printf("    Parent PID[0x%Q], Memory Address[0x%Q], Size[0x%Q]\n\n", pstTCB->qwParentProcessID, pstTCB->pvMemoryAddress, pstTCB->qwMemorySize);
        }
    }
}

/**
 * 태스크를 종료시키는 함수
*/
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
        pstTCB = GetTCBInTCBPool(GETTCBOFFSET(qwID));
        qwID = pstTCB->stLink.qwID;

        if (((qwID >> 32) != 0) && ((pstTCB->qwFlags & TASK_FLAGS_SYSTEM) == 0x00))
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
            Printf("[!] Task does not exist or task is system task....\n");
        }
    }
    else
    {
        for (i = 0; i < TASK_MAXCOUNT; i++)
        {
            pstTCB = GetTCBInTCBPool(i);
            qwID = pstTCB->stLink.qwID;
            
            if (((qwID >> 32) != 0) && ((pstTCB->qwFlags & TASK_FLAGS_SYSTEM) == 0x00))
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

/**
 * 프로세서 부하를 계산함
*/
static void CPULoad(const char *pcParameterBuffer)
{
    Printf("Processor Load : %d%%\n", GetProcessorLoad());
}



///////////////////////////////////////////////////////////////////////
// 뮤텍스 테스트 함수

static MUTEX gs_stMutex;
static volatile QWORD gs_qwAdder;

/**
 * 뮤텍스 테스트를 위한 테스트 함수
 * 1 ~ 5까지의 숫자를 출력하는 함수
*/
static void PrintNumberTask(void)
{
    int i;
    int j;
    QWORD qwTickCount;

    // 50ms 정도 대기하면서 콘솔 셸과 곂치지 않도록 설정
    qwTickCount = GetTickCount();
    while ((GetTickCount() - qwTickCount) < 50)
    {
        Schedule();
    }

    // 루프를 돌면서 숫자를 출력
    for (i = 0; i < 5; i++)
    {   
        // 뮤텍스를 해줘야 하는 이유는 공유되는 변수가 있으며, 이 값이 다른 태스크에 의해 변경되기 때문임
        // 공유 변수는 gs_qwAdder와 Printf의 커서 위치 변수들임

        // 현재 태스크로 뮤텍스 잠금 실행
        // gs_qwAdder와 Printf가 뮤텍스 처리되어 다른 태스크가 접근할 수 없음
        Lock(&(gs_stMutex));

        Printf("[*] Task ID [0x%Q] Value[%d]\n", GetRunningTask()->stLink.qwID, gs_qwAdder);

        gs_qwAdder += 1;

        Unlock(&(gs_stMutex));

        for (j = 0; j < 30000; j++);
    }

    qwTickCount = GetTickCount();
    while ((GetTickCount() - qwTickCount) < 1000)
    {
        Schedule();
    }

    ExitTask();
}

/**
 * 뮤텍스를 테스트하는 태스크 생성
 * 3개의 테스트 태스크를 생성하여 위의 PrintfNumber 함수를 실행시켜서 확인
 * 뮤텍스가 없다면 숫자들이 순서대로 출력되지 않고 뒤죽박죽일 것임
*/
static void TestMutex(const char *pcParameterBuffer)
{
    int i;

    gs_qwAdder = 1;

    InitializeMutex(&gs_stMutex);

    // 3개의 태스크를 생성하여 PrintNumberTask 함수 실행
    for (i = 0; i < 3; i++)
    {
        CreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, (QWORD)PrintNumberTask);
    }
    
    Printf("[*] Wait Util %d Task End.......\n", i);
    // 아무 키나 눌러서 함수 종료
    GetCh();
}



/////////////////////////////////////////////////////
// 쓰레드 관련 기능

/**
 * 낮은 우선순위의 TestTask2 함수를 실행하는 3개의 쓰레드를 생성함
 * TestTask2는 바람개비를 생성하는 태스크
*/
static void CreateThreadTask(void)
{
    int i;

    for (i = 0; i < 3; i++)
    {
        CreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, (QWORD)TestTask2);
    }

    while (1)
    {
        Sleep(1);
    }
}

/**
 * 3개의 쓰레드를 테스트하는 태스크 생성
*/
static void TestThread(const char *pcParameterBuffer)
{
    TCB *pstProcess;

    // 프로세스 생성
    pstProcess = CreateTask(TASK_FLAGS_LOW | TASK_FLAGS_PROCESS, (void *)0xEEEEEEEE, 0x1000, (QWORD)CreateThreadTask);
    
    if (pstProcess != NULL)
    {
        Printf("[*] Process [0x%Q] Create Success\n\n", pstProcess->stLink.qwID);
    }
    else
    {
        Printf("[!] Process Create Fail\n\n");
    }
}

// 난수를 발생시키기 위한 변수
static volatile QWORD gs_qwRandomValue = 0;

/**
 * 임의의 난수를 반환
*/
QWORD Random(void)
{
    gs_qwRandomValue = (gs_qwRandomValue * 412153 + 5571031) >> 16;
    
    return gs_qwRandomValue;
}

/**
 * 철자를 흘러내리게 하는 함수
 * 난수를 이용해서 철자를 만드는 것
 * 매트릭스 효과를 위한 함수
*/
static void DropCharactorThread(void)
{
    int iX, iY;
    int i;
    char vcText[2];

    iX = Random() % CONSOLE_WIDTH;

    while (1)
    {
        Sleep(Random() % 20);

        if ((Random() % 20) < 16)
        {
            vcText[0] = ' ';
            for (i = 0; i < CONSOLE_HEIGHT - 1; i++)
            {
                PrintStringXY(iX, i, vcText);
                Sleep(50);
            }
        }
        else
        {
            for (i = 0; i < CONSOLE_HEIGHT - 1; i++)
            {
                vcText[0] = i + Random();
                PrintStringXY(iX, i, vcText);
                Sleep(50);
            }
        }
    }
}

/**
 * 쓰레드를 생성하여 매트릭스 화면처럼 보여주는 프로세스
 * 300개의 위의 함수를 실행하는 쓰레드를 생성
*/
static void MatrixProcess(void)
{
    int i;

    for (i = 0; i < 300; i++)
    {
        if (CreateTask(TASK_FLAGS_THREAD | TASK_FLAGS_LOW, 0, 0, (QWORD)DropCharactorThread) == NULL)
        {
            break;
        }

        Sleep(Random() % 5 + 5);
    }

    Printf("[*] %d Thread is created\n", i);

    // 키가 입력되면 프로세스 종료
    GetCh();
}

/**
 * 
*/
static void ShowMatrix(const char *pcParameterBuffer)
{
    TCB *pstProcess;

    // 매트릭스 쓰레드들의 부모 프로세스 생성
    pstProcess = CreateTask(TASK_FLAGS_PROCESS | TASK_FLAGS_LOW, (void *)0xE00000, 0xE00000,(QWORD)MatrixProcess);
    
    if (pstProcess != NULL)
    {
        Printf("[*] Matrix Process [0x%Q] Create Success\n", pstProcess->stLink.qwID);

        // 상위 32비트 값을 비교해 종료 태스크인지 검사
        // 종료 태스크로 설정되면 종료
        while ((pstProcess->stLink.qwID >> 32) != 0)
        {
            Sleep(100);
        }
    }
    else
    {
        Printf("[!] Matrix Process Create Fail\n");
    }
}


//////////////////////////////////////////////////////
// 실수 연산을 위한 기능

/**
 * 
*/
static void FPUTestTask(void)
{
    double dValue1;
    double dValue2;
    TCB *pstRunningTask;
    QWORD qwCount = 0;
    QWORD qwRandomValue;
    int i;
    int iOffset;
    char vcData[4] = {'-', '\\', '|', '/'};
    CHARACTER *pstScreen = (CHARACTER *)CONSOLE_VIDEOMEMORYADDRESS;

    pstRunningTask = GetRunningTask();

    // 자신의 ID를 얻어서 화면 오프셋으로 사용
    iOffset = (pstRunningTask->stLink.qwID & 0xFFFFFFFF) * 2;
    iOffset = CONSOLE_WIDTH * CONSOLE_HEIGHT -
              (iOffset % (CONSOLE_WIDTH * CONSOLE_HEIGHT));

    // 루프를 무한히 반복하면서 동일한 계산을 수행
    while (1)
    {
        dValue1 = 1;
        dValue2 = 1;

        // 테스트를 위해 동일한 계산을 2번 반복해서 실행
        for (i = 0; i < 10; i++)
        {
            qwRandomValue = Random();
            dValue1 *= (double)qwRandomValue;
            dValue2 *= (double)qwRandomValue;

            Sleep(1);

            qwRandomValue = Random();
            dValue1 /= (double)qwRandomValue;
            dValue2 /= (double)qwRandomValue;
        }

        if (dValue1 != dValue2)
        {
            Printf("\n[!] Value Is Not Same~!!! [%f] != [%f]\n", dValue1, dValue2);
            break;
        }
        qwCount++;

        // 회전하는 바람개비를 표시
        pstScreen[iOffset].bCharactor = vcData[qwCount % 4];

        // 색깔 지정
        pstScreen[iOffset].bAttribute = (iOffset % 15) + 1;
    }
}

/**
 * 파이 소수점 부분을 출력하도록 하는 함수
 * FPUTestTask를 실행하는 쓰레드를 100개 생성
*/
static void TestPIE(const char *pcParameterBuffer)
{
    double dResult;
    int i;

    Printf("\n[*] PIE Calculation Test....\n");
    Printf("[*] Result: 355 / 113 = [ ");
    
    dResult = (double) 355 / 113;
    Printf("%d.%d%d ]\n", (QWORD) dResult, ((QWORD) (dResult * 10) % 10), ((QWORD) (dResult * 100) % 10));

    for(i=0; i<100; i++)
    {
        CreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, (QWORD) FPUTestTask);
    }
}



/////////////////////////////////////////////////////////////////////////////////////////
// 동적 메모리 기능 함수들

/**
 * 동적메모리 관련 아래 정보들을 출력
 * 동적 메모리 시작 주소, 전체 크기
 * 비트맵과 할당된 블록 크기를 저장하는데 사용한 영역
 * 사용된 메모리 영역 
*/
static void ShowDyanmicMemoryInformation(const char *pcParameterBuffer)
{
    QWORD qwStartAddress, qwTotalSize, qwMetaSize, qwUsedSize;

    GetDynamicMemoryInformation(&qwStartAddress, &qwTotalSize, &qwMetaSize, &qwUsedSize);

    Printf("\n============ Dynamic Memory Information ============\n\n");
    
    Printf("[*] Start Address:    [ 0x%Q ]\n", qwStartAddress);

    Printf("[*] Total Size:       [ 0x%Q ] byte,   [ %d ] MB\n", qwTotalSize, qwTotalSize / 1024 / 1024);

    Printf("[*] Meta Size:        [ 0x%Q ] byte,   [ %d ] KB\n", qwMetaSize, qwMetaSize / 1024);

    Printf("[*] Used Size:        [ 0x%Q ] byte,   [ %d ] KB\n", qwUsedSize, qwUsedSize / 1024);
}

/**
 * 블록을 크기순으로 순차적으로 할당하고 검사 후 해제하는 테스트를 수행
*/
static void TestSequentialAllocation(const char *pcParameterBuffer)
{
    DYNAMICMEMORY *pstMemory;
    long i, j, k;
    QWORD *pqwBuffer;

    Printf("\n============ Dynamic Memory Test ============\n");
    pstMemory = GetDynamicMemoryManager();

    for (i = 0; i < pstMemory->iMaxLevelCount; i++)
    {
        Printf("[*] Block List [%d] Test Start\n", i);
        Printf("[*] Allocation And Compare: ");

        for (j = 0; j < (pstMemory->iBlockCountOfSmallestBlock >> i); j++)
        {
            pqwBuffer = AllocateMemory(DYNAMICMEMORY_MIN_SIZE << i);
            if (pqwBuffer == NULL)
            {
                Printf("\nAllocation Fail\n");
                return;
            }

            for (k = 0; k < (DYNAMICMEMORY_MIN_SIZE << i) / 8; k++)
            {
                pqwBuffer[k] = k;
            }

            for (k = 0; k < (DYNAMICMEMORY_MIN_SIZE << i) / 8; k++)
            {
                if (pqwBuffer[k] != k)
                {
                    Printf("\nCompare Fail\n");
                    return;
                }
            }

            Printf(".");
        }

        Printf("\nFree: ");

        for (j = 0; j < (pstMemory->iBlockCountOfSmallestBlock >> i); j++)
        {
            if (FreeMemory((void *)(pstMemory->qwStartAddress + (DYNAMICMEMORY_MIN_SIZE << i) * j)) == FALSE)
            {
                Printf("\nFree Fail\n");
                return;
            }
            Printf(".");
        }
        Printf("\n");
    }
    Printf("[*] Test Completed......!!!\n");
}

/**
 * 임의의 메모리를 할당하고 해제하는 테스트를 10번 진행
*/
static void RandomAllocationTask(void)
{
    TCB *pstTask;
    QWORD qwMemorySize;
    char vcBuffer[200];
    BYTE *pbAllocationBuffer;
    int i, j;
    int iY;

    pstTask = GetRunningTask();
    iY = (pstTask->stLink.qwID) % 15 + 9;

    for (j = 0; j < 10; j++)
    {
        do
        {
            qwMemorySize = ((Random() % (32 * 1024)) + 1) * 1024;
            pbAllocationBuffer = AllocateMemory(qwMemorySize);

            if (pbAllocationBuffer == 0)
            {
                Sleep(1);
            }
        } while (pbAllocationBuffer == 0);

        SPrintf(vcBuffer, "|Address: [0x%Q] Size: [0x%Q] Allocation Success", pbAllocationBuffer, qwMemorySize);

        PrintStringXY(20, iY, vcBuffer);
        Sleep(200);

        SPrintf(vcBuffer, "|Address: [0x%Q] Size: [0x%Q] Data Write...     ", pbAllocationBuffer, qwMemorySize);
        PrintStringXY(20, iY, vcBuffer);
        
        for (i = 0; i < qwMemorySize / 2; i++)
        {
            pbAllocationBuffer[i] = Random() & 0xFF;
            pbAllocationBuffer[i + (qwMemorySize / 2)] = pbAllocationBuffer[i];
        }
        
        Sleep(200);

        SPrintf(vcBuffer, "|Address: [0x%Q] Size: [0x%Q] Data Verify...   ", pbAllocationBuffer, qwMemorySize);
        PrintStringXY(20, iY, vcBuffer);

        for (i = 0; i < qwMemorySize / 2; i++)
        {
            if (pbAllocationBuffer[i] != pbAllocationBuffer[i + (qwMemorySize / 2)])
            {
                Printf("[!] Task ID[0x%Q] Verify Fail\n", pstTask->stLink.qwID);
                ExitTask();
            }
        }
        FreeMemory(pbAllocationBuffer);
        Sleep(200);
    }

    ExitTask();
}

/**
 * 위의 함수를 실행하는 쓰레드(태스크)를 100개 생성
*/
static void TestRandomAllocation(const char *pcParameterBuffer)
{
    int i;

    for (i = 0; i < 100; i++)
    {
        CreateTask(TASK_FLAGS_LOWEST | TASK_FLAGS_THREAD, 0, 0, (QWORD)RandomAllocationTask);
    }
}



///////////////////////////////////////////////////////////////////////////
// 파일 시스템 관련 기능들

/**
 * 하드 디스크의 정보를 출력
*/
static void ShowHDDInformation(const char *pcParameterBuffer)
{
    HDDINFORMATION stHDD;
    char vcBuffer[100];

    // 하드 디스크의 정보를 읽음
    if (GetHDDInformation( &stHDD ) == FALSE)
    {
        Printf("\n[!] HDD Information Read Fail\n");
        return;
    }

    Printf("\n============ Primary Master HDD Information ============\n\n");

    MemCpy(vcBuffer, stHDD.vwModelNumber, sizeof(stHDD.vwModelNumber));
    vcBuffer[sizeof(stHDD.vwModelNumber) - 1] = '\0';
    Printf("[*] Model Number:\t %s\n", vcBuffer);

    MemCpy(vcBuffer, stHDD.vwSerialNumber, sizeof(stHDD.vwSerialNumber));
    vcBuffer[sizeof(stHDD.vwSerialNumber) - 1] = '\0';
    Printf("[*] Serial Number:\t %s\n", vcBuffer);

    Printf("[*] Head Count:\t\t %d\n", stHDD.wNumberOfHead);
    Printf("[*] Cylinder Count:\t %d\n", stHDD.wNumberOfCylinder);
    Printf("[*] Sector Count:\t %d\n", stHDD.wNumberOfSectorPerCylinder);

    Printf("[*] Total Sector:\t %d Sector, %dMB\n", stHDD.dwTotalSectors, stHDD.dwTotalSectors / 2 / 1024);
}

/**
 * 인자로 넘어온 하드 디스크의 LBA 어드레스에서 섹터 수 만큼 읽음 
*/
static void ReadSector(const char *pcParameterBuffer)
{
    PARAMETERLIST stList;
    char vcLBA[50], vcSectorCount[50];
    DWORD dwLBA;
    int iSectorCount;
    char *pcBuffer;
    int i, j;
    BYTE bData;
    BOOL bExit = FALSE;

    InitializeParameter(&stList, pcParameterBuffer);
    if ((GetNextParameter(&stList, vcLBA) == 0) || (GetNextParameter(&stList, vcSectorCount) == 0))
    {
        Printf("\nex) readsector 0(LBA) 10(count)\n");
        return;
    }

    dwLBA = AToI(vcLBA, 10);
    iSectorCount = AToI(vcSectorCount, 10);

    pcBuffer = AllocateMemory(iSectorCount * 512);
    if (ReadHDDSector(TRUE, TRUE, dwLBA, iSectorCount, pcBuffer) == iSectorCount)
    {
        Printf("[*] LBA [%d], [%d] Sector Read Success......\n", dwLBA, iSectorCount);

        for (j = 0; j < iSectorCount; j++)
        {
            for (i = 0; i < 512; i++)
            {
                if (!((j == 0) && (i == 0)) && ((i % 256) == 0))
                {
                    Printf("\n\nPress any key to continue... ('q' is exit) : ");
                    
                    if (GetCh() == 'q')
                    {
                        bExit = TRUE;
                        break;
                    }
                }

                if ((i % 16) == 0)
                {
                    Printf("\n[LBA: %d, Offset: %d]\t| ", dwLBA + j, i);
                }

                bData = pcBuffer[j * 512 + i] & 0xFF;
                if (bData < 16)
                {
                    Printf("0");
                }
                Printf("%X ", bData);
            }

            if (bExit == TRUE)
            {
                break;
            }
        }
        Printf("\n");
    }
    else
    {
        Printf("Read Fail\n");
    }

    FreeMemory(pcBuffer);
}

/**
 * 인자로 넘어온 하드디스크의 LBA 주소에 섹터 수 만큼 값을 씀 
*/
static void WriteSector(const char *pcParameterBuffer)
{
    PARAMETERLIST stList;
    char vcLBA[50], vcSectorCount[50];
    DWORD dwLBA;
    int iSectorCount;
    char *pcBuffer;
    int i, j;
    BOOL bExit = FALSE;
    BYTE bData;
    static DWORD s_dwWriteCount = 0;

    InitializeParameter(&stList, pcParameterBuffer);
    
    if ((GetNextParameter(&stList, vcLBA) == 0) || (GetNextParameter(&stList, vcSectorCount) == 0))
    {
        Printf("\nex) writesector 0(LBA) 10(count)\n");
        return;
    }
    
    dwLBA = AToI(vcLBA, 10);
    iSectorCount = AToI(vcSectorCount, 10);

    s_dwWriteCount++;

    pcBuffer = AllocateMemory(iSectorCount * 512);
    
    for (j = 0; j < iSectorCount; j++)
    {
        for (i = 0; i < 512; i += 8)
        {
            *(DWORD *)&(pcBuffer[j * 512 + i]) = dwLBA + j;
            *(DWORD *)&(pcBuffer[j * 512 + i + 4]) = s_dwWriteCount;
        }
    }

    if (WriteHDDSector(TRUE, TRUE, dwLBA, iSectorCount, pcBuffer) != iSectorCount)
    {
        Printf("\n[!] Write Fail\n");
        return;
    }
    
    Printf("\n[*] LBA [%d], [%d] Sector Write Success......\n", dwLBA, iSectorCount);

    for (j = 0; j < iSectorCount; j++)
    {
        for (i = 0; i < 512; i++)
        {
            if (!((j == 0) && (i == 0)) && ((i % 256) == 0))
            {
                Printf("\nPress any key to continue... ('q' is exit) : ");
                
                if (GetCh() == 'q')
                {
                    bExit = TRUE;
                    break;
                }
            }

            if ((i % 16) == 0)
            {
                Printf("\n[LBA: %d, Offset: %d]\t| ", dwLBA + j, i);
            }

            bData = pcBuffer[j * 512 + i] & 0xFF;
            
            if (bData < 16)
            {
                Printf("0");
            }
            
            Printf("%X ", bData);
        }

        if (bExit == TRUE)
        {
            break;
        }
    }
    
    Printf("\n");
    
    FreeMemory(pcBuffer);
}

/**
 * MINT 파일 시스템 인식하는 함수
*/
static void MountHDD(const char *pcParameterBuffer)
{
    if (Mount() == FALSE)
    {
        Printf("[!] HDD Mount Fail.......\n");
        return;
    }
    
    Printf("[*] HDD Mount Success.......\n");
}

/**
 * MINT 파일 시스템 생성하는 함수
*/
static void FormatHDD(const char *pcParameterBuffer)
{
    if (Format() == FALSE)
    {
        Printf("[!] HDD Format Fail.......\n");
        return;
    }
    
    Printf("[*] HDD Format Success.......\n");
}

/**
 * 파일 시스템 정보 출력
*/
static void ShowFileSystemInformation(const char *pcParameterBuffer)
{
    FILESYSTEMMANAGER stManager;

    GetFileSystemInformation(&stManager);

    Printf("\n================== File System Information ==================\n\n");
    Printf("[*] Mouted:\t\t\t\t\t %d\n", stManager.bMounted);
    Printf("[*] Reserved Sector Count:\t\t\t %d Sector\n", stManager.dwReservedSectorCount);
    Printf("[*] Cluster Link Table Start Address:\t\t %d Sector\n",
            stManager.dwClusterLinkAreaStartAddress);
    Printf("[*] Cluster Link Table Size:\t\t\t %d Sector\n", stManager.dwClusterLinkAreaSize);
    Printf("[*] Data Area Start Address:\t\t\t %d Sector\n", stManager.dwDataAreaStartAddress);
    Printf("[*] Total Cluster Count:\t\t\t %d Cluster\n", stManager.dwTotalClusterCount);
}

/**
 * 루트 디렉터리에 파일을 생성하는 함수
*/
static void CreateFileInRootDirectory(const char *pcParameterBuffer)
{
    PARAMETERLIST stList;
    char vcFileName[50];
    int iLength;
    DWORD dwCluster;
    DIRECTORYENTRY stEntry;
    int i;
    FILE *pstFile;

    InitializeParameter(&stList, pcParameterBuffer);
    iLength = GetNextParameter(&stList, vcFileName);
    vcFileName[iLength] = '\0';

    if ((iLength > (FILESYSTEM_MAXFILENAMELENGTH - 1)) || (iLength == 0))
    {
        Printf("\n[!] Too Long or Too Short File Name.......\n");
        return;
    }

    // 파일을 쓰기 모드로 생성
    pstFile = fopen(vcFileName, "w");

    if(pstFile == NULL)
    {
        Printf("\n[!] File Create Fail.........\n");
        return;
    }
    
    fclose(pstFile);
    Printf("\n[*] File Create Success...........\n");
    
}

/**
 * 파일 이름을 넘겨받아 파일을 삭제하는 함수
*/
static void DeleteFileInRootDirectory(const char *pcParameterBuffer)
{
    PARAMETERLIST stList;
    char vcFileName[50];
    int iLength;

    InitializeParameter(&stList, pcParameterBuffer);
    
    iLength = GetNextParameter(&stList, vcFileName);
    vcFileName[iLength] = '\0';
    
    if ((iLength > (FILESYSTEM_MAXFILENAMELENGTH - 1)) || (iLength == 0))
    {
        Printf("\n[!] Too Long or Too Short File Name.......\n");
        return;
    }

    if(remove(vcFileName) != 0)
    {
        Printf("\n[!] File Not Found or File Opened........\n");
        return ;
    }

    Printf("\n[*] File delete success..........\n");
}

/**
 * 파일 목록 표시
*/
static void ShowRootDirectory(const char *pcParameterBuffer)
{
    DIR *pstDirectory;
    int i, iCount, iTotalCount;
    struct dirent *pstEntry;
    char vcBuffer[400];
    char vcTempValue[50];
    DWORD dwTotalByte;
    DWORD dwUsedClusterCount;
    FILESYSTEMMANAGER stManager;

    GetFileSystemInformation(&stManager);

    pstDirectory = opendir("/");

    Printf("\n");
    
    if (pstDirectory == NULL)
    {
        Printf("[!] Root Directory Open Fail.........\n");
        return ;
    }

    iTotalCount = 0;
    dwTotalByte = 0;
    dwUsedClusterCount = 0;

    while(1)
    {
        pstEntry = readdir(pstDirectory);
        
        if(pstEntry == NULL)
        {
            break;
        }

        iTotalCount++;
        dwTotalByte += pstEntry->dwFileSize;

        if(pstEntry->dwFileSize == 0)
        {
            dwUsedClusterCount++;
        }
        else
        {
            dwUsedClusterCount += (pstEntry->dwFileSize + (FILESYSTEM_CLUSTERSIZE - 1)) / FILESYSTEM_CLUSTERSIZE;
        }
    }

    rewinddir(pstDirectory);
    iCount = 0;

    while (1)
    {
        pstEntry = readdir(pstDirectory);
        
        if (pstEntry == NULL)
        {
            break;
        }

        MemSet(vcBuffer, ' ', sizeof(vcBuffer) - 1);
        vcBuffer[sizeof(vcBuffer) - 1] = '\0';

        MemCpy(vcBuffer, pstEntry->d_name,
                StrLen(pstEntry->d_name));

        SPrintf(vcTempValue, "%d Byte", pstEntry->dwFileSize);
        MemCpy(vcBuffer + 30, vcTempValue, StrLen(vcTempValue));

        SPrintf(vcTempValue, "0x%X Cluster", pstEntry->dwStartClusterIndex);
        MemCpy(vcBuffer + 55, vcTempValue, StrLen(vcTempValue) + 1);
        Printf("    %s\n", vcBuffer);

        if ((iCount != 0) && ((iCount % 20) == 0))
        {
            Printf("\n----------------- Press any key to continue... ('q' is exit) : -----------------\n\n");
            if (GetCh() == 'q')
            {
                Printf("\n");
                break;
            }
        }
        
        iCount++;
    }

    Printf("\t\tTotal File Count: %d\n", iTotalCount);
    Printf("\t\tTotal File Size: %d KByte (%d Cluster)\n", dwTotalByte, dwUsedClusterCount);

    Printf("\t\tFree Space: %d KByte (%d Cluster)\n", (stManager.dwTotalClusterCount - dwUsedClusterCount) * FILESYSTEM_CLUSTERSIZE / 1024, stManager.dwTotalClusterCount - dwUsedClusterCount);

    closedir(pstDirectory);
}

/**
 * 파일을 생성하여 내용을 입력
 * 엔터 3번치면 종료
*/
static void WriteDataToFile(const char *pcParameterBuffer)
{
    PARAMETERLIST stList;
    char vcFileName[50];
    int iLength;
    FILE *fp;
    int iEnterCount;
    BYTE bKey;

    InitializeParameter(&stList, pcParameterBuffer);
    iLength = GetNextParameter(&stList, vcFileName);
    vcFileName[iLength] = '\0';
    if ((iLength > (FILESYSTEM_MAXFILENAMELENGTH - 1)) || (iLength == 0))
    {
        Printf("\n[!] Too Long or Too Short File Name.......\n");
        return;
    }

    fp = fopen(vcFileName, "w");
    if (fp == NULL)
    {
        Printf("\n[!] %s File Open Fail.........\n", vcFileName);
        return;
    }

    iEnterCount = 0;
    while (1)
    {
        bKey = GetCh();
        if (bKey == KEY_ENTER)
        {
            iEnterCount++;
            if (iEnterCount >= 3)
            {
                break;
            }
        }
        else
        {
            iEnterCount = 0;
        }

        Printf("%c", bKey);
        if (fwrite(&bKey, 1, 1, fp) != 1)
        {
            Printf("\n[!] File Write Fail...........\n");
            break;
        }
    }

    Printf("\n[*] File Create Success..............\n");
    fclose(fp);
}

/**
 * 파일을 열어서 데이터를 읽음
*/
static void ReadDataFromFile(const char *pcParameterBuffer)
{
    PARAMETERLIST stList;
    char vcFileName[50];
    int iLength;
    FILE *fp;
    int iEnterCount;
    BYTE bKey;

    InitializeParameter(&stList, pcParameterBuffer);
    iLength = GetNextParameter(&stList, vcFileName);
    vcFileName[iLength] = '\0';
    if ((iLength > (FILESYSTEM_MAXFILENAMELENGTH - 1)) || (iLength == 0))
    {
        Printf("\n[!] Too Long or Too Short File Name\n");
        return;
    }

    fp = fopen(vcFileName, "r");
    if (fp == NULL)
    {
        Printf("\n[!] %s File Open Fail\n", vcFileName);
        return;
    }

    iEnterCount = 0;
    while (1)
    {
        if (fread(&bKey, 1, 1, fp) != 1)
        {
            break;
        }
        Printf("%c", bKey);

        if (bKey == KEY_ENTER)
        {
            iEnterCount++;

            if ((iEnterCount != 0) && ((iEnterCount % 20) == 0))
            {
                Printf("\n-------------- Press any key to continue... ('q' is exit) : --------------\n");
                if (GetCh() == 'q')
                {
                    Printf("\n");
                    break;
                }
                Printf("\n");
                iEnterCount = 0;
            }
        }
    }
    fclose(fp);
}

/**
 * 파일 I/O 기능 테스트
 * 하드디스크로 했을 때 매우 잘 안됨
 * 램 디스크로 해야 잘 됨
*/
static void TestFileIO(const char *pcParameterBuffer)
{
    FILE *pstFile;
    BYTE *pbBuffer;
    int i;
    int j;
    DWORD dwRandomOffset;
    DWORD dwByteCount;
    BYTE vbTempBuffer[1024];
    DWORD dwMaxFileSize;

    Printf("\n================== File I/O Function Test ==================\n");

    dwMaxFileSize = 4 * 1024 * 1024;
    pbBuffer = AllocateMemory(dwMaxFileSize);
    if (pbBuffer == NULL)
    {
        Printf("\n[!] Memory Allocation Fail........\n");
        return;
    }
    else
    {
        Printf("\n[*] Memory Allocation Success.........\n");
    }

    remove("testfileio.bin");

    Printf("\n[*] 1. File Open Fail Test...");
    
    pstFile = fopen("testfileio.bin", "r");
    if (pstFile == NULL)
    {
        Printf("[Pass]\n");
    }
    else
    {
        Printf("[Fail]\n");
        fclose(pstFile);
    }

    Printf("\n[*] 2. File Create Test...");
    pstFile = fopen("testfileio.bin", "w");
    if (pstFile != NULL)
    {
        Printf("[Pass]\n");
        Printf("    File Handle [0x%Q]\n", pstFile);
    }
    else
    {
        Printf("[Fail]\n");
    }

    Printf("\n[*] 3. Sequential Write Test(Cluster Size)...");
    for (i = 0; i < 100; i++)
    {
        MemSet(pbBuffer, i, FILESYSTEM_CLUSTERSIZE);
        if (fwrite(pbBuffer, 1, FILESYSTEM_CLUSTERSIZE, pstFile) !=
            FILESYSTEM_CLUSTERSIZE)
        {
            Printf("[Fail]\n");
            Printf("    %d Cluster Error\n", i);
            break;
        }
    }
    if (i >= 100)
    {
        Printf("[Pass]\n");
    }

    Printf("\n[*] 4. Sequential Read And Verify Test(Cluster Size)...");
    fseek(pstFile, -100 * FILESYSTEM_CLUSTERSIZE, SEEK_END);

    for (i = 0; i < 100; i++)
    {
        if (fread(pbBuffer, 1, FILESYSTEM_CLUSTERSIZE, pstFile) !=
            FILESYSTEM_CLUSTERSIZE)
        {
            Printf("[Fail]\n");
            return;
        }

        for (j = 0; j < FILESYSTEM_CLUSTERSIZE; j++)
        {
            if (pbBuffer[j] != (BYTE)i)
            {
                Printf("[Fail]\n");
                Printf("    %d Cluster Error. [%X] != [%X]\n", i, pbBuffer[j],
                        (BYTE)i);
                break;
            }
        }
    }
    if (i >= 100)
    {
        Printf("[Pass]\n");
    }

    Printf("\n[*] 5. Random Write Test...\n");

    MemSet(pbBuffer, 0, dwMaxFileSize);

    fseek(pstFile, -100 * FILESYSTEM_CLUSTERSIZE, SEEK_CUR);
    fread(pbBuffer, 1, dwMaxFileSize, pstFile);

    for (i = 0; i < 100; i++)
    {
        dwByteCount = (Random() % (sizeof(vbTempBuffer) - 1)) + 1;
        dwRandomOffset = Random() % (dwMaxFileSize - dwByteCount);

        Printf("    [%d] Offset [%d] Byte [%d]...", i, dwRandomOffset,
                dwByteCount);

        fseek(pstFile, dwRandomOffset, SEEK_SET);
        MemSet(vbTempBuffer, i, dwByteCount);

        if (fwrite(vbTempBuffer, 1, dwByteCount, pstFile) != dwByteCount)
        {
            Printf("[Fail]\n");
            break;
        }
        else
        {
            Printf("[Pass]\n");
        }

        MemSet(pbBuffer + dwRandomOffset, i, dwByteCount);
    }

    fseek(pstFile, dwMaxFileSize - 1, SEEK_SET);
    fwrite(&i, 1, 1, pstFile);
    pbBuffer[dwMaxFileSize - 1] = (BYTE)i;

    Printf("\n[*] 6. Random Read And Verify Test...\n");
    
    for (i = 0; i < 100; i++)
    {
        dwByteCount = (Random() % (sizeof(vbTempBuffer) - 1)) + 1;
        dwRandomOffset = Random() % ((dwMaxFileSize)-dwByteCount);

        Printf("    [%d] Offset [%d] Byte [%d]...", i, dwRandomOffset,
                dwByteCount);

        fseek(pstFile, dwRandomOffset, SEEK_SET);

        if (fread(vbTempBuffer, 1, dwByteCount, pstFile) != dwByteCount)
        {
            Printf("[Fail]\n");
            Printf("    Read Fail\n", dwRandomOffset);
            break;
        }

        if (MemCmp(pbBuffer + dwRandomOffset, vbTempBuffer, dwByteCount) != 0)
        {
            Printf("[Fail]\n");
            Printf("    Compare Fail\n", dwRandomOffset);
            break;
        }

        Printf("[Pass]\n");
    }

    Printf("\n[*] 7. Sequential Write, Read And Verify Test(1024 Byte)...\n");

    fseek(pstFile, -dwMaxFileSize, SEEK_CUR);

    for (i = 0; i < (2 * 1024 * 1024 / 1024); i++)
    {
        Printf("    [%d] Offset [%d] Byte [%d] Write...", i, i * 1024, 1024);

        if (fwrite(pbBuffer + (i * 1024), 1, 1024, pstFile) != 1024)
        {
            Printf("[Fail]\n");
            return;
        }
        else
        {
            Printf("[Pass]\n");
        }
    }

    fseek(pstFile, -dwMaxFileSize, SEEK_SET);

    for (i = 0; i < (dwMaxFileSize / 1024); i++)
    {
        Printf("    [%d] Offset [%d] Byte [%d] Read And Verify...", i,
                i * 1024, 1024);

        if (fread(vbTempBuffer, 1, 1024, pstFile) != 1024)
        {
            Printf("[Fail]\n");
            return;
        }

        if (MemCmp(pbBuffer + (i * 1024), vbTempBuffer, 1024) != 0)
        {
            Printf("[Fail]\n");
            break;
        }
        else
        {
            Printf("[Pass]\n");
        }
    }

    Printf("\n[*] 8. File Delete Fail Test...  ");
    
    if (remove("testfileio.bin") != 0)
    {
        Printf("[Pass]\n");
    }
    else
    {
        Printf("[Fail]\n");
    }

    Printf("\n[*] 9. File Close Test...  ");

    if (fclose(pstFile) == 0)
    {
        Printf("[Pass]\n");
    }
    else
    {
        Printf("[Fail]\n");
    }

    Printf("\n[*] 10. File Delete Test...  ");

    if (remove("testfileio.bin") == 0)
    {
        Printf("[Pass]\n");
    }
    else
    {
        Printf("[Fail]\n");
    }

    FreeMemory(pbBuffer);
}

static void TestPerformance(const char *pcParameterBuffer)
{
    FILE *pstFile;
    DWORD dwClusterTestFileSize;
    DWORD dwOneByteTestFileSize;
    QWORD qwLastTickCount;
    DWORD i;
    BYTE *pbBuffer;

    dwClusterTestFileSize = 1024 * 1024;
    dwOneByteTestFileSize = 16 * 1024;

    pbBuffer = AllocateMemory(dwClusterTestFileSize);
    if (pbBuffer == NULL)
    {
        Printf("\n[!] Memory Allocate Fail\n");
        return;
    }

    Printf("\n[*] 0. Memory Allocate Success......\n");

    MemSet(pbBuffer, 0, FILESYSTEM_CLUSTERSIZE);

    Printf("\n================== File I/O Performance Test ==================\n");

    Printf("[*] 1.Sequential Read/Write Test(Cluster Size)\n");

    remove("performance.txt");
    pstFile = fopen("performance.txt", "w");
    if (pstFile == NULL)
    {
        Printf("\t[!] File Open Fail.......\n");
        FreeMemory(pbBuffer);
        return;
    }

    qwLastTickCount = GetTickCount();
    for (i = 0; i < (dwClusterTestFileSize / FILESYSTEM_CLUSTERSIZE); i++)
    {
        if (fwrite(pbBuffer, 1, FILESYSTEM_CLUSTERSIZE, pstFile) !=
            FILESYSTEM_CLUSTERSIZE)
        {
            Printf("\t[!] Write Fail.......\n");
            fclose(pstFile);
            FreeMemory(pbBuffer);
            return;
        }
    }
    Printf("   Sequential Write(Cluster Size): %d ms\n", GetTickCount() -
                                                              qwLastTickCount);

    fseek(pstFile, 0, SEEK_SET);

    qwLastTickCount = GetTickCount();
    for (i = 0; i < (dwClusterTestFileSize / FILESYSTEM_CLUSTERSIZE); i++)
    {
        if (fread(pbBuffer, 1, FILESYSTEM_CLUSTERSIZE, pstFile) !=
            FILESYSTEM_CLUSTERSIZE)
        {
            Printf("\t[!] Read Fail..........\n");
            fclose(pstFile);
            FreeMemory(pbBuffer);
            return;
        }
    }
    Printf("   Sequential Read(Cluster Size): %d ms\n", GetTickCount() -
                                                             qwLastTickCount);

    Printf("[*] 2.Sequential Read/Write Test(1 Byte)\n");

    remove("performance.txt");
    pstFile = fopen("performance.txt", "w");
    if (pstFile == NULL)
    {
        Printf("\t[!] File Open Fail\n");
        FreeMemory(pbBuffer);
        return;
    }

    qwLastTickCount = GetTickCount();
    for (i = 0; i < dwOneByteTestFileSize; i++)
    {
        if (fwrite(pbBuffer, 1, 1, pstFile) != 1)
        {
            Printf("\t[!] Write Fail...............\n");
            fclose(pstFile);
            FreeMemory(pbBuffer);
            return;
        }
    }
    
    Printf("   Sequential Write(1 Byte): %d ms\n", GetTickCount() -
                                                        qwLastTickCount);

    fseek(pstFile, 0, SEEK_SET);

    qwLastTickCount = GetTickCount();
    for (i = 0; i < dwOneByteTestFileSize; i++)
    {
        if (fread(pbBuffer, 1, 1, pstFile) != 1)
        {
            Printf("\t[!] Read Fail................\n");
            fclose(pstFile);
            FreeMemory(pbBuffer);
            return;
        }
    }
    Printf("   Sequential Read(1 Byte): %d ms\n", GetTickCount() -  qwLastTickCount);

    fclose(pstFile);
    FreeMemory(pbBuffer);
}

/**
 * 캐시 버퍼를 비움
*/
static void FlushCache(const char *pcParameterBuffer)
{
    QWORD qwTickCount;

    qwTickCount = GetTickCount();
    
    Printf("\n[*] Cache Flush...... ");
    
    if (FlushFileSystemCache() == TRUE)
    {
        Printf("Pass\n");
    }
    else
    {
        Printf("Fail\n");
    }
    
    Printf("[*] Total Time = %d ms\n", GetTickCount() - qwTickCount);
}

/**
 * 파일을 외부에서 다운로드함
*/
static void DownloadFile( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcFileName[ 50 ];
    int iFileNameLength;
    DWORD dwDataLength;
    FILE* fp;
    DWORD dwReceivedSize;
    DWORD dwTempSize;
    BYTE vbDataBuffer[ SERIAL_FIFOMAXSIZE ];
    QWORD qwLastReceivedTickCount;
    
    InitializeParameter( &stList, pcParameterBuffer );
    iFileNameLength = GetNextParameter( &stList, vcFileName );
    vcFileName[ iFileNameLength ] = '\0';
    
    if( ( iFileNameLength > ( FILESYSTEM_MAXFILENAMELENGTH - 1 ) ) || ( iFileNameLength == 0 ) )
    {
        Printf("[*] \nToo Long or Too Short File Name\n" );
        Printf("[*] ex) download a.txt\n" );
        
        return ;
    }
    
    ClearSerialFIFO();
    
    Printf("[*] \n[*] Waiting For Data Length......" );
    dwReceivedSize = 0;
    qwLastReceivedTickCount = GetTickCount();
    
    while( dwReceivedSize < 4 )
    {
        dwTempSize = ReceiveSerialData( ( ( BYTE* ) &dwDataLength ) + dwReceivedSize, 4 - dwReceivedSize );
        
        dwReceivedSize += dwTempSize;
        
        if( dwTempSize == 0 )
        {
            Sleep( 0 );
            
            if( ( GetTickCount() - qwLastReceivedTickCount ) > 30000 )
            {
                Printf("[*]  Time Out Occur......\n" );
                return ;
            }
        }
        else
        {
            qwLastReceivedTickCount = GetTickCount();
        }
    }
    Printf("[*] [%d] Byte\n", dwDataLength );

    SendSerialData( "A", 1 );

    fp = fopen( vcFileName, "w" );
    if( fp == NULL )
    {
        Printf("[*] %s File Open Fail\n", vcFileName );
        return ;
    }
    
    Printf("[*] Data Receive Start: " );
    dwReceivedSize = 0;
    qwLastReceivedTickCount = GetTickCount();
    
    while( dwReceivedSize < dwDataLength )
    {
        dwTempSize = ReceiveSerialData( vbDataBuffer, SERIAL_FIFOMAXSIZE );
        dwReceivedSize += dwTempSize;

        if( dwTempSize != 0 ) 
        {

            if( ( ( dwReceivedSize % SERIAL_FIFOMAXSIZE ) == 0 ) ||
                ( ( dwReceivedSize == dwDataLength ) ) )
            {
                SendSerialData( "A", 1 );
                Printf("[*] #" );
            }
            
            if( fwrite( vbDataBuffer, 1, dwTempSize, fp ) != dwTempSize )
            {
                Printf("[*] [!] File Write Error Occur\n" );
                break;
            }
            
            qwLastReceivedTickCount = GetTickCount();
        }
        else
        {
            Sleep( 0 );
            
            if( ( GetTickCount() - qwLastReceivedTickCount ) > 10000 )
            {
                Printf("[*] Time Out Occur~!!\n" );
                break;
            }
        }
    }   

    if( dwReceivedSize != dwDataLength )
    {
        Printf("[*] \nError Occur. Total Size [%d] Received Size [%d]\n", 
                 dwReceivedSize, dwDataLength );
    }
    else
    {
        Printf("[*] \nReceive Complete. Total Size [%d] Byte\n", dwReceivedSize );
    }
    
    fclose( fp );
    FlushFileSystemCache();
}

/**
 * MP테이블 보여줌
*/
static void ShowMPConfigurationTable( const char* pcParameterBuffer )
{
    PrintMPConfigurationTable();
}

/**
 * AP 코어 활성화
*/
static void StartApplicationProcessor( const char* pcParameterBuffer )
{
    if( StartUpApplicationProcessor() == FALSE )
    {
        Printf("\n[!] Application Processor Start Fail\n" );
        return ;
    }
    Printf("\n[*] Application Processor Start Success.....\n" );
    
    Printf("[*] Bootstrap Processor [APIC ID: %d] Start Application Processor.....\n", GetAPICID() );
}

/**
 * 대칭 I/O 모드 활성화
*/
static void StartSymmetricIOMode( const char* pcParameterBuffer )
{
    MPCONFIGURATIONMANAGER* pstMPManager;
    BOOL bInterruptFlag;

    if( AnalysisMPConfigurationTable() == FALSE )
    {
        Printf("\n[!] Analyze MP Configuration Table Fail\n" );
        return ;
    }

    pstMPManager = GetMPConfigurationManager();
    
    if( pstMPManager->bUsePICMode == TRUE )
    {

        OutPortByte( 0x22, 0x70 );
        OutPortByte( 0x23, 0x01 );
    }

    Printf("\n[*] Mask All PIC Controller Interrupt\n" );
    MaskPICInterrupt( 0xFFFF );

    Printf("[*] Enable Global Local APIC\n" );
    EnableGlobalLocalAPIC();
    
    Printf("[*] Enable Software Local APIC\n" );
    EnableSoftwareLocalAPIC();

    Printf("[*] Disable CPU Interrupt Flag\n" );
    bInterruptFlag = SetInterruptFlag( FALSE );
    
    SetTaskPriority( 0 );

    InitializeLocalVectorTable();

    // 대칭 I/O 모드로 변경되었음을 설정
    SetSymmetricIOMode(TRUE);
    
    Printf("[*] Initialize IO Redirection Table\n" );
    InitializeIORedirectionTable();
        
    // 이전 인터럽트 플래그를 복원
    Printf("[*] Restore CPU Interrupt Flag\n" );
    SetInterruptFlag( bInterruptFlag );
        
    Printf("[*] Change Symmetric I/O Mode Complete\n" );
}

static void ShowIRQINTINMappingTable( const char* pcParameterBuffer )
{
    PrintIRQToINTINMap();
}

/**
 *  코어 별로 인터럽트를 처리한 횟수를 출력
 */
static void ShowInterruptProcessingCount(const char *pcParameterBuffer)
{
    INTERRUPTMANAGER *pstInterruptManager;
    int i;
    int j;
    int iProcessCount;
    char vcBuffer[20];
    int iRemainLength;
    int iLineCount;

    Printf("========================== Interrupt Count ==========================\n");

    // MP 설정 테이블에 저장된 코어의 개수를 읽음
    iProcessCount = GetProcessorCount();

    //==========================================================================
    //  Column 출력
    //==========================================================================
    // 프로세서의 수만큼 Column을 출력
    // 한 줄에 코어 4개씩 출력하고 한 Column당 15칸을 할당함
    for (i = 0; i < iProcessCount; i++)
    {
        if (i == 0)
        {
            Printf("IRQ Num\t\t");
        }
        else if ((i % 4) == 0)
        {
            Printf("\n       \t\t");
        }
        SPrintf(vcBuffer, "Core %d", i);
        Printf(vcBuffer);

        // 출력하고 남은 공간을 모두 스페이스로 채움
        iRemainLength = 15 - StrLen(vcBuffer);
        MemSet(vcBuffer, ' ', iRemainLength);
        vcBuffer[iRemainLength] = '\0';
        Printf(vcBuffer);
    }
    Printf("\n");

    //==========================================================================
    //  Row와 인터럽트 처리 횟수 출력
    //==========================================================================
    // 총 인터럽트 횟수와 코어 별 인터럽트 처리 횟수를 출력
    iLineCount = 0;
    pstInterruptManager = GetInterruptManager();
    for (i = 0; i < INTERRUPT_MAXVECTORCOUNT; i++)
    {
        for (j = 0; j < iProcessCount; j++)
        {
            // Row를 출력, 한 줄에 코어 4개씩 출력하고 한 Column당 15칸을 할당
            if (j == 0)
            {
                // 20 라인마다 화면 정지
                if ((iLineCount != 0) && (iLineCount > 10))
                {
                    Printf("\nPress any key to continue... ('q' is exit) : ");
                    if (GetCh() == 'q')
                    {
                        Printf("\n");
                        return;
                    }
                    iLineCount = 0;
                    Printf("\n");
                }
                Printf("---------------------------------------------------------------------\n");
                Printf("IRQ %d\t\t", i);
                iLineCount += 2;
            }
            else if ((j % 4) == 0)
            {
                Printf("\n      \t\t");
                iLineCount++;
            }

            SPrintf(vcBuffer, "0x%Q", pstInterruptManager->vvqwCoreInterruptCount[j][i]);
            // 출력하고 남은 영역을 모두 스페이스로 채움
            Printf(vcBuffer);
            iRemainLength = 15 - StrLen(vcBuffer);
            MemSet(vcBuffer, ' ', iRemainLength);
            vcBuffer[iRemainLength] = '\0';
            Printf(vcBuffer);
        }
        Printf("\n");
    }
}

/**
 *  인터럽트 부하 분산 기능을 시작
 */
static void StartInterruptLoadBalancing(const char *pcParameterBuffer)
{
    Printf("Start Interrupt Load Balancing\n");
    SetInterruptLoadBalancing(TRUE);
}