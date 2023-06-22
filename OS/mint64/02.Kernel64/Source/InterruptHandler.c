#include "InterruptHandler.h"
#include "PIC.h"
#include "Keyboard.h"
#include "Console.h"
#include "Utility.h"
#include "Task.h"
#include "Descriptor.h"
#include "AssemblyUtility.h"
#include "HardDisk.h"
#include "LocalAPIC.h"


static INTERRUPTMANAGER gs_stInterruptManager;

void InitializeHandler(void)
{
    MemSet(&gs_stInterruptManager, 0, sizeof(gs_stInterruptManager));
}

void SetSymmetricIOMode(BOOL bSymmetricIOMode)
{
    gs_stInterruptManager.bSymmetricIOMode = bSymmetricIOMode;
}

void SetInterruptLoadBalancing(BOOL bUseLoadBalancing)
{
    gs_stInterruptManager.bUseLoadBalancing = bUseLoadBalancing;
}

void IncreaseInterruptCount(int iIRQ)
{
    gs_stInterruptManager.vvqwCoreInterruptCount[GetAPICID()][iIRQ]++;
}

void SendEOI(int iIRQ)
{
    if (gs_stInterruptManager.bSymmetricIOMode == FALSE)
    {
        SendEOIToPIC(iIRQ);
    }
    else
    {
        SendEOIToLocalAPIC();
    }
}

INTERRUPTMANAGER* GetInterruptManager(void)
{
    return &gs_stInterruptManager;
}

void ProcessLoadBalancing(int iIRQ)
{
    QWORD qwMinCount = 0xFFFFFFFFFFFFFFFF;
    int iMinCountCoreIndex;
    int iCoreCount;
    int i;
    BOOL bResetCount = FALSE;
    BYTE bAPICID;

    bAPICID = GetAPICID();

    if ( (gs_stInterruptManager.vvqwCoreInterruptCount[bAPICID][iIRQ] == 0) || ( (gs_stInterruptManager.vvqwCoreInterruptCount[bAPICID][iIRQ] % INTERRUPT_LOADBALANCINGIVIDOR) != 0 ) || (gs_stInterruptManager.bUseLoadBalancing == FALSE) )
    {
        return;
    }

    iMinCountCoreIndex = 0;
    iCoreCount = GetProcessorCount();
    for(i=0; i < iCoreCount; i++)
    {
        if ( (gs_stInterruptManager.vvqwCoreInterruptCount[i][iIRQ] < qwMinCount) )
        {
            qwMinCount = gs_stInterruptManager.vvqwCoreInterruptCount[i][iIRQ];
            iMinCountCoreIndex = i;
        }

        else if (gs_stInterruptManager.vvqwCoreInterruptCount[i][iIRQ] >= 0xFFFFFFFFFFFFFFFE)
        {
            bResetCount = TRUE;
        }
    }

    RoutingIRQToAPICID(iIRQ, iMinCountCoreIndex);

    if(bResetCount == TRUE)
    {
        for(i=0; i<iCoreCount; i++)
        {
            gs_stInterruptManager.vvqwCoreInterruptCount[i][iIRQ] = 0;
        }
    }
}

/**
 * 예외 발생 시 처리하는 함수
*/
void CommonExceptionHandler(int iVectorNumber, QWORD qwErrorCode)
{
    char vcBuffer[3];

    PrintStringXY(0, 0, "====================================================");
    PrintStringXY(0, 1, "                 Exception Occur~!!!!               ");
    PrintStringXY(0, 2, "              Vector:           Core ID:            ");

    // 예외 벡터를 화면 오른쪽 위에 2자리 정수로 출력
    vcBuffer[0] = '0' + iVectorNumber / 10;
    vcBuffer[1] = '0' + iVectorNumber % 10;

    PrintStringXY(21, 2, vcBuffer);
    SPrintf(vcBuffer, "0x%X", GetAPICID());
    PrintStringXY(40, 2, vcBuffer);
    PrintStringXY(0, 3, "====================================================");

    while (1)
        ;
}

/**
 * 인터럽트 발생 시 처리하는 함수
*/
void CommonInterruptHandler(int iVectorNumber)
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iCommonInterruptCount = 0;
    int iIRQ;

    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;

    vcBuffer[8] = '0' + g_iCommonInterruptCount;
    g_iCommonInterruptCount = (g_iCommonInterruptCount + 1) % 10;
    PrintStringXY(70, 0, vcBuffer);

    iIRQ = iVectorNumber - PIC_IRQSTARTVECTOR;

    // EOI 전송
    SendEOI(iIRQ);

    // 인터럽트 발생 횟수를 업데이트
    IncreaseInterruptCount(iIRQ);

    // 부하 분산(Load Balancing) 처리
    ProcessLoadBalancing(iIRQ);
}

/**
 * 키보드 인터럽트 발생 시 처리하는 함수
*/
void KeyboardHandler(int iVectorNumber)
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iKeyboardInterruptCount = 0;
    BYTE bTemp;
    int iIRQ;

    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;

    vcBuffer[8] = '0' + g_iKeyboardInterruptCount;
    g_iKeyboardInterruptCount = (g_iKeyboardInterruptCount + 1) % 10;
    PrintStringXY(0, 0, vcBuffer);

    if (IsOutputBufferFull() == TRUE) // 출력버퍼에 값이 있다면 값을 읽음
    {
        bTemp = GetKeyboardScanCode(); // 값을 가져와서
        ConvertScanCodeAndPutQueue(bTemp); // 변환하여 큐에 저장
    }

    iIRQ = iVectorNumber - PIC_IRQSTARTVECTOR;

    // EOI 전송
    SendEOI(iIRQ);

    // 인터럽트 발생 횟수를 업데이트
    IncreaseInterruptCount(iIRQ);

    // 부하 분산(Load Balancing) 처리
    ProcessLoadBalancing(iIRQ);
}

/**
 * 타이머 인터럽트 발생 시 처리하는 함수
*/
void TimerHandler(int iVectorNumber)
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iTimerInterruptCount = 0;
    int iIRQ;

    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;

    vcBuffer[8] = '0' + g_iTimerInterruptCount;
    g_iTimerInterruptCount = (g_iTimerInterruptCount + 1) % 10;
    
    PrintStringXY(70, 0, vcBuffer);

    // 인터럽트 벡터에서 IRQ 번호 추출
    iIRQ = iVectorNumber - PIC_IRQSTARTVECTOR;

    // EOI 전송
    SendEOI(iIRQ);

    // 인터럽트 발생 횟수를 업데이트
    IncreaseInterruptCount(iIRQ);

    // IRQ 0 인터럽트 처리는 Bootstrap Processor만 처리
    if (GetAPICID() == 0)
    {
        // 타이머 발생 횟수를 증가
        g_qwTickCount++;

        // 태스크가 사용한 프로세서의 시간을 줄임
        DecreaseProcessorTime();
        // 프로세서가 사용할 수 있는 시간을 다 썼다면 태스크 전환 수행
        if (IsProcessorTimeExpired() == TRUE)
        {
            ScheduleInInterrupt();
        }
    }
}

/**
 * FPU 사용 시 예외 7번 발생
 * 예외 7번이 발생하면 FPU 사용을 위한 설정을 해주는 예외 7번 핸들러
*/
void DeviceNotAvailableHandler(int iVectorNumber)
{
    TCB *pstFPUTask, *pstCurrentTask;
    QWORD qwLastFPUTaskID;

    char vcBuffer[] = "[EXC:  , ]";
    static int g_iFPUInterruptCount = 0;

    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;

    vcBuffer[8] = '0' + g_iFPUInterruptCount;
    g_iFPUInterruptCount = (g_iFPUInterruptCount + 1) % 10;
    PrintStringXY(0, 0, vcBuffer);

    // FPU를 사용한다면 예외 7번이 발생되고 이 함수가 실행이 됨
    // 실행되면 TS를 0으로 설정
    ClearTS();

    // 마지막으로 FPU를 사용한 태스크 확인
    qwLastFPUTaskID = GetLastFPUUsedTaskID();
    pstCurrentTask = GetRunningTask();

    // 마지막으로 FPU를 사용한 태스크가 현재 태스크이면 컨텍스트 전환이 필요없음
    if (qwLastFPUTaskID == pstCurrentTask->stLink.qwID)
    {
        return;
    }
    // FPU를 사용한 태스크가 있다면 컨텍스트를 해당 태스크에 저장시켜줘야 함
    else if (qwLastFPUTaskID != TASK_INVALIDID)
    {   
        // 마지막으로 사용한 태스크를 검색
        pstFPUTask = GetTCBInTCBPool(GETTCBOFFSET(qwLastFPUTaskID));

        // 찾았다면 해당 태스크에 FPU 컨텍스트 저장
        if ((pstFPUTask != NULL) && (pstFPUTask->stLink.qwID == qwLastFPUTaskID))
        {
            SaveFPUContext(pstFPUTask->vqwFPUContext);
        }
    }

    // 현재 태스크가 FPU를 사용한 적이 없다면 초기화
    if (pstCurrentTask->bFPUUsed == FALSE)
    {
        InitializeFPU();
        pstCurrentTask->bFPUUsed = TRUE;
    }
    // 사용한 적이 있다면 자신의 FPU 컨텍스트에서 값을 가져옴
    else
    {
        LoadFPUContext(pstCurrentTask->vqwFPUContext);
    }

    // 마지막으로 FPU를 사용한 태스크로 자신을 등록
    SetLastFPUUsedTaskID(pstCurrentTask->stLink.qwID);
}

void HDDHandler(int iVectorNumber)
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iHDDInterruptCount = 0;
    BYTE bTemp;
    int iIRQ;

    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;

    vcBuffer[8] = '0' + g_iHDDInterruptCount;
    g_iHDDInterruptCount = (g_iHDDInterruptCount + 1) % 10;

    PrintStringXY(10, 0, vcBuffer);

    /* 책이랑 저자 소스코드랑 다른 부분 (뒤에서 바꾸나봄)
    if (iVectorNumber - PIC_IRQSTARTVECTOR == 14)
    {
        SetHDDInterruptFlag(TRUE, TRUE);
    }
    else
    {
        SetHDDInterruptFlag(FALSE, TRUE);
    }
    */

    // 첫 번째 PATA 포트의 인터럽트 발생 여부를 TRUE로 설정
    SetHDDInterruptFlag(TRUE, TRUE);

    // 인터럽트 벡터에서 IRQ 번호 추출
    iIRQ = iVectorNumber - PIC_IRQSTARTVECTOR;

    // EOI 전송
    SendEOI(iIRQ);

    // 인터럽트 발생 횟수를 업데이트
    IncreaseInterruptCount(iIRQ);

    // 부하 분산(Load Balancing) 처리
    ProcessLoadBalancing(iIRQ);
}