#include "Synchronization.h"
#include "Utility.h"
#include "Task.h"
#include "AssemblyUtility.h"

BOOL LockForSystemData(void)
{
    return SetInterruptFlag(FALSE);
}

void UnlockForSystemData(BOOL bInterruptFlag)
{
    SetInterruptFlag(bInterruptFlag);
}

void InitializeMutex(MUTEX *pstMutex)
{
    pstMutex->bLockFlag = FALSE;
    pstMutex->dwLockCount = 0;
    pstMutex->qwTaskID = TASK_INVALIDID;
}

void Lock(MUTEX *pstMutex)
{
    if (TestAndSet(&(pstMutex->bLockFlag), 0, 1) == FALSE)
    {
        if (pstMutex->qwTaskID == GetRunningTask()->stLink.qwID)
        {
            pstMutex->dwLockCount++;
            return ;
        }

        while(TestAndSet(&(pstMutex->bLockFlag), 0, 1) == FALSE)
        {
            Schedule();
        }
    }

    pstMutex->dwLockCount = 1;
    pstMutex->qwTaskID = GetRunningTask()->stLink.qwID;
}

void Unlock(MUTEX *pstMutex)
{
    if ((pstMutex->bLockFlag == FALSE) || (pstMutex->qwTaskID != GetRunningTask()->stLink.qwID))
    {
        return ;
    }

    if(pstMutex->dwLockCount > 1)
    {
        pstMutex->dwLockCount--;
        return ;
    }

    pstMutex->qwTaskID = TASK_INVALIDID;
    pstMutex->dwLockCount = 0;
    pstMutex->bLockFlag = FALSE;
}
