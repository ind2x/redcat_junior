#include "Task.h"
#include "Descriptor.h"
#include "Utility.h"

static SCHEDULER gs_stScheduler;
static TCBPOOLMANAGER gs_stTCBPoolManager;

void InitializeTCBPool(void)
{
    int i;

    MemSet(&(gs_stTCBPoolManager), 0, sizeof(gs_stTCBPoolManager));

    gs_stTCBPoolManager.pstStartAddress = (TCB *) TASK_TCBPOOLADDRESS;
    MemSet(TASK_TCBPOOLADDRESS, 0, sizeof(TCB) * TASK_MAXCOUNT);

    for(i=0; i<TASK_MAXCOUNT; i++)
    {
        gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID = i;
    }

    gs_stTCBPoolManager.iMaxCount = TASK_MAXCOUNT;
    gs_stTCBPoolManager.iAllocatedCount = 1;
}

TCB* AllocateTCB(void)
{
    TCB * pstEmptyTCB;
    int i;

    if (gs_stTCBPoolManager.iUseCount == gs_stTCBPoolManager.iMaxCount)
    {
        return NULL;
    }

    for (i = 0; i < gs_stTCBPoolManager.iMaxCount; i++)
    {
        if ((gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID >> 32) == 0)
        {
            pstEmptyTCB = &(gs_stTCBPoolManager.pstStartAddress[i]);
            break;
        }
    }

    pstEmptyTCB->stLink.qwID = ((QWORD)gs_stTCBPoolManager.iAllocatedCount << 32) | i;

    gs_stTCBPoolManager.iUseCount++;
    gs_stTCBPoolManager.iAllocatedCount++;
    if (gs_stTCBPoolManager.iAllocatedCount == 0)
    {
        gs_stTCBPoolManager.iAllocatedCount = 1;
    }

    return pstEmptyTCB;
}

void FreeTCB(QWORD qwID)
{
    int i;
    i = qwID && 0xFFFFFFFF;

    MemSet(&(gs_stTCBPoolManager.pstStartAddress[i].stContext), 0, sizeof(CONTEXT));

    gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID = i;

    gs_stTCBPoolManager.iUseCount--;
}

TCB *CreateTask(QWORD qwFlags, QWORD qwEntryPointAddress)
{
    TCB *pstTask;
    void *pvStackAddress;

    pstTask = AllocateTCB();
    if (pstTask == NULL)
    {
        return NULL;
    }

    pvStackAddress = (void *)(TASK_STACKPOOLADDRESS + (TASK_STACKSIZE * (pstTask->stLink.qwID & 0xFFFFFFFF)));

    SetUpTask(pstTask, qwFlags, qwEntryPointAddress, pvStackAddress, TASK_STACKSIZE);
    AddTaskToReadyList(pstTask);

    return pstTask;
}

void SetUpTask(TCB *pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress, void *pvStackAddress, QWORD qwStackSize)
{
    MemSet(pstTCB->stContext.vqRegister, 0, sizeof(pstTCB->stContext.vqRegister));

    pstTCB->stContext.vqRegister[TASK_RSPOFFSET] = (QWORD)pvStackAddress +
                                                   qwStackSize;
    pstTCB->stContext.vqRegister[TASK_RBPOFFSET] = (QWORD)pvStackAddress +
                                                   qwStackSize;

    pstTCB->stContext.vqRegister[TASK_CSOFFSET] = GDT_KERNELCODESEGMENT;
    pstTCB->stContext.vqRegister[TASK_DSOFFSET] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[TASK_ESOFFSET] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[TASK_FSOFFSET] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[TASK_GSOFFSET] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[TASK_SSOFFSET] = GDT_KERNELDATASEGMENT;

    pstTCB->stContext.vqRegister[TASK_RIPOFFSET] = qwEntryPointAddress;

    pstTCB->stContext.vqRegister[TASK_RFLAGSOFFSET] |= 0x0200;

    pstTCB->pvStackAddress = pvStackAddress;
    pstTCB->qwStackSize = qwStackSize;
    pstTCB->qwFlags = qwFlags;
}

void InitializeScheduler(void)
{
    InitializeTCBPool();

    InitializeList(&(gs_stScheduler.stReadyList));

    gs_stScheduler.pstRunningTask = AllocateTCB();
}


void SetRunningTask(TCB *pstTask)
{
    gs_stScheduler.pstRunningTask = pstTask;
}

TCB *GetRunningTask(void)
{
    return gs_stScheduler.pstRunningTask;
}

TCB *GetNextTaskToRun(void)
{
    if (GetListCount(&(gs_stScheduler.stReadyList)) == 0)
    {
        return NULL;
    }

    return (TCB *)RemoveListFromHeader(&(gs_stScheduler.stReadyList));
}


void AddTaskToReadyList(TCB *pstTask)
{
    AddListToTail(&(gs_stScheduler.stReadyList), pstTask);
}

void Schedule(void)
{
    TCB *pstRunningTask, *pstNextTask;
    BOOL bPreviousFlag;

    if (GetListCount(&(gs_stScheduler.stReadyList)) == 0)
    {
        return;
    }

    bPreviousFlag = SetInterruptFlag(FALSE);
    pstNextTask = GetNextTaskToRun();
    if (pstNextTask == NULL)
    {
        SetInterruptFlag(bPreviousFlag);
        return;
    }

    pstRunningTask = gs_stScheduler.pstRunningTask;
    AddTaskToReadyList(pstRunningTask);

    gs_stScheduler.pstRunningTask = pstNextTask;
    SwitchContext(&(pstRunningTask->stContext), &(pstNextTask->stContext));

    gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;

    SetInterruptFlag(bPreviousFlag);
}

BOOL ScheduleInInterrupt(void)
{
    TCB *pstRunningTask, *pstNextTask;
    char *pcContextAddress;

    pstNextTask = GetNextTaskToRun();
    if (pstNextTask == NULL)
    {
        return FALSE;
    }

    pcContextAddress = (char *)IST_STARTADDRESS + IST_SIZE - sizeof(CONTEXT);

    pstRunningTask = gs_stScheduler.pstRunningTask;
    MemCpy(&(pstRunningTask->stContext), pcContextAddress, sizeof(CONTEXT));
    AddTaskToReadyList(pstRunningTask);

    gs_stScheduler.pstRunningTask = pstNextTask;
    MemCpy(pcContextAddress, &(pstNextTask->stContext), sizeof(CONTEXT));

    gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
    return TRUE;
}

void DecreaseProcessorTime(void)
{
    if (gs_stScheduler.iProcessorTime > 0)
    {
        gs_stScheduler.iProcessorTime--;
    }
}


BOOL IsProcessorTimeExpired(void)
{
    if (gs_stScheduler.iProcessorTime <= 0)
    {
        return TRUE;
    }
    return FALSE;
}