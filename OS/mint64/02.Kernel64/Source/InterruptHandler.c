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

/**
 * 예외 발생 시 처리하는 함수
*/
void CommonExceptionHandler(int iVectorNumber, QWORD qwErrorCode)
{
    char vcBuffer[3];

    vcBuffer[0] = '0' + iVectorNumber / 10;
    vcBuffer[1] = '0' + iVectorNumber % 10;

    PrintStringXY(0, 0, "====================================================");
    PrintStringXY(0, 1, "                 Exception Occur~!!!!               ");
    PrintStringXY(0, 2, "                    Vector:                         ");
    PrintStringXY(27, 2, vcBuffer);
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

    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;

    vcBuffer[8] = '0' + g_iCommonInterruptCount;
    g_iCommonInterruptCount = (g_iCommonInterruptCount + 1) % 10;
    PrintStringXY(70, 0, vcBuffer);

    SendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);

    SendEOIToLocalAPIC();
}

/**
 * 키보드 인터럽트 발생 시 처리하는 함수
*/
void KeyboardHandler(int iVectorNumber)
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iKeyboardInterruptCount = 0;
    BYTE bTemp;

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

    // EOI 전송
    SendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);

    SendEOIToLocalAPIC();
}

/**
 * 타이머 인터럽트 발생 시 처리하는 함수
*/
void TimerHandler(int iVectorNumber)
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iTimerInterruptCount = 0;

    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;

    vcBuffer[8] = '0' + g_iTimerInterruptCount;
    g_iTimerInterruptCount = (g_iTimerInterruptCount + 1) % 10;
    
    PrintStringXY(70, 0, vcBuffer);
    
    SendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);

    SendEOIToLocalAPIC();

    g_qwTickCount++;

    DecreaseProcessorTime();
    if (IsProcessorTimeExpired() == TRUE)
    {
        ScheduleInInterrupt();
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

    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;

    vcBuffer[8] = '0' + g_iHDDInterruptCount;
    g_iHDDInterruptCount = (g_iHDDInterruptCount + 1) % 10;

    PrintStringXY(10, 0, vcBuffer);

    if (iVectorNumber - PIC_IRQSTARTVECTOR == 14)
    {
        SetHDDInterruptFlag(TRUE, TRUE);
    }
    else
    {
        SetHDDInterruptFlag(FALSE, TRUE);
    }

    SendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);

    SendEOIToLocalAPIC();
}