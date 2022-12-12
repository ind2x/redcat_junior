#include "Synchronization.h"
#include "Utility.h"
#include "Task.h"
#include "AssemblyUtility.h"

/**
 * 인터럽트 제어
*/
BOOL LockForSystemData(void)
{
    return SetInterruptFlag(FALSE);
}

/**
 * 이전 인터럽트 상태 복원
*/
void UnlockForSystemData(BOOL bInterruptFlag)
{
    SetInterruptFlag(bInterruptFlag);
}

/**
 * 뮤텍스 초기화 함수
*/
void InitializeMutex(MUTEX *pstMutex)
{
    // 잠금 여부는 FALSE
    pstMutex->bLockFlag = FALSE;
    // 잠금 횟수는 0
    pstMutex->dwLockCount = 0;
    // 진입한 태스크 ID는 곂치지 않게 아예 유효하지 않는 ID로 설정
    pstMutex->qwTaskID = TASK_INVALIDID;
}

/**
 * 태스크 간 동기화 함수
*/
void Lock(MUTEX *pstMutex)
{
    // 이미 잠겨 있는 경우
    if (TestAndSet(&(pstMutex->bLockFlag), 0, 1) == FALSE)
    {
        // 현재 태스크가 잠근 경우 (내가 잠근 경우)
        if (pstMutex->qwTaskID == GetRunningTask()->stLink.qwID)
        {
            // 잠금 횟수만 증가
            pstMutex->dwLockCount++;
            return ;
        }

        // 아닌 경우 잠금이 해제될 때 까지 대기
        while(TestAndSet(&(pstMutex->bLockFlag), 0, 1) == FALSE)
        {
            // 대기하는 동안에 스케줄링
            Schedule();
        }
    }

    // 처음 잠그는 경우
    pstMutex->dwLockCount = 1;
    pstMutex->qwTaskID = GetRunningTask()->stLink.qwID;
}

/**
 * 잠금 해제 함수
*/
void Unlock(MUTEX *pstMutex)
{
    // 잠금이 해제되었거나 뮤텍스를 잠근 태스크가 아닌 경우
    if ((pstMutex->bLockFlag == FALSE) || (pstMutex->qwTaskID != GetRunningTask()->stLink.qwID))
    {
        return ;
    }

    // 뮤텍스가 중복으로 잠겼으면 잠긴 횟수만 감소
    if(pstMutex->dwLockCount > 1)
    {
        pstMutex->dwLockCount--;
        return ;
    }

    // 잠금 해제 시 설정 (초기화 할 때랑 똑같음)
    pstMutex->qwTaskID = TASK_INVALIDID;
    pstMutex->dwLockCount = 0;
    pstMutex->bLockFlag = FALSE;
}
