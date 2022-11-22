#include "Task.h"
#include "Descriptor.h"
#include "Utility.h"
#include "Synchronization.h"

static SCHEDULER gs_stScheduler;
static TCBPOOLMANAGER gs_stTCBPoolManager;

static void InitializeTCBPool(void)
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

static TCB* AllocateTCB(void)
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

static void FreeTCB(QWORD qwID)
{
    int i;
    i = GETTCBOFFSET(qwID);

    MemSet(&(gs_stTCBPoolManager.pstStartAddress[i].stContext), 0, sizeof(CONTEXT));

    gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID = i;

    gs_stTCBPoolManager.iUseCount--;
}

TCB *CreateTask(QWORD qwFlags, QWORD qwEntryPointAddress)
{
    TCB *pstTask;
    void *pvStackAddress;
    BOOL bPreviousFlag;

    bPreviousFlag = LockForSystemData();
    pstTask = AllocateTCB();
    
    if (pstTask == NULL)
    {
        UnlockForSystemData(bPreviousFlag);
        return NULL;
    }

    UnlockForSystemData(bPreviousFlag);

    pvStackAddress = (void *)(TASK_STACKPOOLADDRESS + (TASK_STACKSIZE * GETTCBOFFSET(pstTask->stLink.qwID)));

    SetUpTask(pstTask, qwFlags, qwEntryPointAddress, pvStackAddress, TASK_STACKSIZE);


    bPreviousFlag = LockForSystemData();
    
    AddTaskToReadyList(pstTask);
    
    UnlockForSystemData(bPreviousFlag);

    return pstTask;
}

static void SetUpTask(TCB *pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress, void *pvStackAddress, QWORD qwStackSize)
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
    int i;

    InitializeTCBPool();

    for(i=0; i<TASK_MAXREADYLISTCOUNT; i++)
    {
        InitializeList(&(gs_stScheduler.vstReadyList[i]));
        gs_stScheduler.viExecuteCount[i] = 0;
    }

    InitializeList(&(gs_stScheduler.stWaitList));

    gs_stScheduler.pstRunningTask = AllocateTCB();
    gs_stScheduler.pstRunningTask->qwFlags = TASK_FLAGS_HIGHEST;

    gs_stScheduler.qwSpendProcessorTimeInIdleTask = 0;
    gs_stScheduler.qwProcessorLoad = 0;
}


void SetRunningTask(TCB *pstTask)
{
    BOOL bPreviousFlag;

    bPreviousFlag = LockForSystemData();
    
    gs_stScheduler.pstRunningTask = pstTask;

    UnlockForSystemData(bPreviousFlag);
}

TCB *GetRunningTask(void)
{
    BOOL bPreviousFlag;
    TCB *pstRunningTask;

    bPreviousFlag = LockForSystemData();

    pstRunningTask = gs_stScheduler.pstRunningTask;

    UnlockForSystemData(bPreviousFlag);

    return gs_stScheduler.pstRunningTask;
}

static TCB *GetNextTaskToRun(void)
{
    TCB *pstTarget = NULL;
    int iTaskCount, i, j;

    for(j=0; j<2; j++)
    {
        for(i=0; i<TASK_MAXREADYLISTCOUNT; i++)
        {
            iTaskCount = GetListCount(&(gs_stScheduler.vstReadyList[i]));

            if(gs_stScheduler.viExecuteCount[i] < iTaskCount)
            {
                pstTarget = (TCB*)RemoveListFromHeader(&(gs_stScheduler.vstReadyList[i]));
                gs_stScheduler.viExecuteCount[i]++;
                break;
            }
            else
            {
                gs_stScheduler.viExecuteCount[i] = 0;
            }
        }

        if(pstTarget != NULL) break;
    }

    return pstTarget;
}

static BOOL AddTaskToReadyList(TCB *pstTask)
{
    BYTE bPriority;

    bPriority = GETPRIORITY(pstTask->qwFlags);
    if(bPriority >= TASK_MAXREADYLISTCOUNT)
    {
        return FALSE;
    }

    AddListToTail(&(gs_stScheduler.vstReadyList[bPriority]), pstTask);
    return TRUE;
}

static TCB* RemoveTaskFromReadyList(QWORD qwTaskID)
{
    TCB *pstTarget;
    BYTE bPriority;

    if(GETTCBOFFSET(qwTaskID) >= TASK_MAXCOUNT)
    {
        return NULL;
    }

    pstTarget = &(gs_stTCBPoolManager.pstStartAddress[GETTCBOFFSET(qwTaskID)]);
    
    if (pstTarget->stLink.qwID != qwTaskID)
    {
        return NULL;
    }

    bPriority = GETPRIORITY(pstTarget->qwFlags);

    pstTarget = RemoveList(&(gs_stScheduler.vstReadyList[bPriority]),qwTaskID);
    
    return pstTarget;
}

BOOL ChangePriority(QWORD qwTaskID, BYTE bPriority)
{
    TCB *pstTarget;
    BOOL bPreviousFlag;
    
    if (bPriority > TASK_MAXREADYLISTCOUNT)
    {
        return FALSE;
    }

    bPreviousFlag = LockForSystemData();

    pstTarget = gs_stScheduler.pstRunningTask;
    
    if (pstTarget->stLink.qwID == qwTaskID)
    {
        SETPRIORITY(pstTarget->qwFlags, bPriority);
    }
    else
    {
        pstTarget = RemoveTaskFromReadyList(qwTaskID);
        
        if (pstTarget == NULL)
        {
            pstTarget = GetTCBInTCBPool(GETTCBOFFSET(qwTaskID));
            
            if (pstTarget != NULL)
            {
                SETPRIORITY(pstTarget->qwFlags, bPriority);
            }
        }
        else
        {
            SETPRIORITY(pstTarget->qwFlags, bPriority);
            AddTaskToReadyList(pstTarget);
        }
    }

    UnlockForSystemData(bPreviousFlag);
    return TRUE;
}

void Schedule(void)
{
    TCB *pstRunningTask, *pstNextTask;
    BOOL bPreviousFlag;

    if(GetReadyTaskCount() < 1)
    {
        return ;
    }

    bPreviousFlag = LockForSystemData();
    
    pstNextTask = GetNextTaskToRun();
    if (pstNextTask == NULL)
    {
        UnlockForSystemData(bPreviousFlag);
        return;
    }

    pstRunningTask = gs_stScheduler.pstRunningTask;
    gs_stScheduler.pstRunningTask = pstNextTask;

    if ((pstRunningTask->qwFlags & TASK_FLAGS_IDLE) == TASK_FLAGS_IDLE)
    {
        gs_stScheduler.qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME - gs_stScheduler.iProcessorTime;
    }

    if (pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK)
    {
        AddListToTail(&(gs_stScheduler.stWaitList), pstRunningTask);
        SwitchContext(NULL, &(pstNextTask->stContext));
    }
    else
    {
        AddTaskToReadyList(pstRunningTask);
        SwitchContext(&(pstRunningTask->stContext), & (pstNextTask->stContext));
    }

    gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;

    UnlockForSystemData(bPreviousFlag);
}

BOOL ScheduleInInterrupt(void)
{
    TCB *pstRunningTask, *pstNextTask;
    char *pcContextAddress;
    BOOL bPreviousFlag;

    bPreviousFlag = LockForSystemData();

    pstNextTask = GetNextTaskToRun();
    if (pstNextTask == NULL)
    {
        UnlockForSystemData(bPreviousFlag);
        return FALSE;
    }

    pcContextAddress = (char *)IST_STARTADDRESS + IST_SIZE - sizeof(CONTEXT);

    pstRunningTask = gs_stScheduler.pstRunningTask;
    gs_stScheduler.pstRunningTask = pstNextTask;

    if ((pstRunningTask->qwFlags & TASK_FLAGS_IDLE) == TASK_FLAGS_IDLE)
    {
        gs_stScheduler.qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME;
    }

    if (pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK)
    {
        AddListToTail(&(gs_stScheduler.stWaitList), pstRunningTask);
    }
    else
    {
        MemCpy(&(pstRunningTask->stContext), pcContextAddress, sizeof(CONTEXT));
        AddTaskToReadyList(pstRunningTask);
    }

    UnlockForSystemData(bPreviousFlag);

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

BOOL EndTask(QWORD qwTaskID)
{
    TCB *pstTarget;
    BYTE bPriority;
    BOOL bPreviousFlag;

    bPreviousFlag = LockForSystemData();

    pstTarget = gs_stScheduler.pstRunningTask;
    if (pstTarget->stLink.qwID == qwTaskID)
    {
        pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
        SETPRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);

        UnlockForSystemData(bPreviousFlag);
        
        Schedule();

        while (1)
            ;
    }
    else
    {
        pstTarget = RemoveTaskFromReadyList(qwTaskID);
        if (pstTarget == NULL)
        {
            pstTarget = GetTCBInTCBPool(GETTCBOFFSET(qwTaskID));
            if (pstTarget != NULL)
            {
                pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
                SETPRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);
            }

            UnlockForSystemData(bPreviousFlag);
            return TRUE;
        }

        pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
        SETPRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);
        AddListToTail(&(gs_stScheduler.stWaitList), pstTarget);
    }

    UnlockForSystemData(bPreviousFlag);
    return TRUE;
}

void ExitTask(void)
{
    EndTask(gs_stScheduler.pstRunningTask->stLink.qwID);
}

int GetReadyTaskCount(void)
{
    int iTotalCount = 0;
    int i;
    BOOL bPreviousFlag;

    bPreviousFlag = LockForSystemData();

    for (i = 0; i < TASK_MAXREADYLISTCOUNT; i++)
    {
        iTotalCount += GetListCount(&(gs_stScheduler.vstReadyList[i]));
    }

    UnlockForSystemData(bPreviousFlag);
    return iTotalCount;
}

int GetTaskCount(void)
{
    int iTotalCount;
    BOOL bPreviousFlag;

    iTotalCount = GetReadyTaskCount();

    bPreviousFlag = LockForSystemData();

    iTotalCount += GetListCount(&(gs_stScheduler.stWaitList)) + 1;

    UnlockForSystemData(bPreviousFlag);
    return iTotalCount;
}

TCB *GetTCBInTCBPool(int iOffset)
{
    if ((iOffset < -1) && (iOffset > TASK_MAXCOUNT))
    {
        return NULL;
    }

    return &(gs_stTCBPoolManager.pstStartAddress[iOffset]);
}

BOOL IsTaskExist(QWORD qwID)
{
    TCB *pstTCB;

    pstTCB = GetTCBInTCBPool(GETTCBOFFSET(qwID));

    if ((pstTCB == NULL) || (pstTCB->stLink.qwID != qwID))
    {
        return FALSE;
    }
    return TRUE;
}


QWORD GetProcessorLoad(void)
{
    return gs_stScheduler.qwProcessorLoad;
}

void IdleTask(void)
{
    TCB *pstTask;
    QWORD qwLastMeasureTickCount, qwLastSpendTickInIdleTask;
    QWORD qwCurrentMeasureTickCount, qwCurrentSpendTickInIdleTask;

    BOOL bPreviousFlag;
    QWORD qwTaskID;

    qwLastSpendTickInIdleTask = gs_stScheduler.qwSpendProcessorTimeInIdleTask;
    qwLastMeasureTickCount = GetTickCount();

    while (1)
    {
        qwCurrentMeasureTickCount = GetTickCount();
        qwCurrentSpendTickInIdleTask = gs_stScheduler.qwSpendProcessorTimeInIdleTask;

        if (qwCurrentMeasureTickCount - qwLastMeasureTickCount == 0)
        {
            gs_stScheduler.qwProcessorLoad = 0;
        }
        else
        {
            gs_stScheduler.qwProcessorLoad = 100 - (qwCurrentSpendTickInIdleTask - qwLastSpendTickInIdleTask) * 100 / (qwCurrentMeasureTickCount - qwLastMeasureTickCount);
        }

        qwLastMeasureTickCount = qwCurrentMeasureTickCount;
        qwLastSpendTickInIdleTask = qwCurrentSpendTickInIdleTask;

        HaltProcessorByLoad();

        if (GetListCount(&(gs_stScheduler.stWaitList)) >= 0)
        {
            while (1)
            {
                bPreviousFlag = LockForSystemData();
                
                pstTask = RemoveListFromHeader(&(gs_stScheduler.stWaitList));
                if (pstTask == NULL)
                {
                    UnlockForSystemData(bPreviousFlag);
                    break;
                }
    
                qwTaskID = pstTask->stLink.qwID;
                FreeTCB(pstTask->stLink.qwID);

                UnlockForSystemData(bPreviousFlag);

                Printf("[*] IDLE: Task ID[0x%q] is completely ended.\n", qwTaskID);
                
            }
        }

        Schedule();
    }
}

void HaltProcessorByLoad(void)
{
    if (gs_stScheduler.qwProcessorLoad < 40)
    {
        Hlt();
        Hlt();
        Hlt();
    }
    else if (gs_stScheduler.qwProcessorLoad < 80)
    {
        Hlt();
        Hlt();
    }
    else if (gs_stScheduler.qwProcessorLoad < 95)
    {
        Hlt();
    }
}