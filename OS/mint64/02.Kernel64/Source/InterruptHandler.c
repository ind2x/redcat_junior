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

    if (IsOutputBufferFull() == TRUE)
    {
        bTemp = GetKeyboardScanCode();
        ConvertScanCodeAndPutQueue(bTemp);
    }

    // EOI 전송
    SendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);

    SendEOIToLocalAPIC();
}

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

    ClearTS();

    qwLastFPUTaskID = GetLastFPUUsedTaskID();
    pstCurrentTask = GetRunningTask();

    if (qwLastFPUTaskID == pstCurrentTask->stLink.qwID)
    {
        return;
    }
    else if (qwLastFPUTaskID != TASK_INVALIDID)
    {
        pstFPUTask = GetTCBInTCBPool(GETTCBOFFSET(qwLastFPUTaskID));
        if ((pstFPUTask != NULL) && (pstFPUTask->stLink.qwID == qwLastFPUTaskID))
        {
            SaveFPUContext(pstFPUTask->vqwFPUContext);
        }
    }

    if (pstCurrentTask->bFPUUsed == FALSE)
    {
        InitializeFPU();
        pstCurrentTask->bFPUUsed = TRUE;
    }
    else
    {
        LoadFPUContext(pstCurrentTask->vqwFPUContext);
    }

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