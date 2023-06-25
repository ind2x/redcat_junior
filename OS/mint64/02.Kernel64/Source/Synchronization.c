#include "Synchronization.h"
#include "Utility.h"
#include "Task.h"
#include "AssemblyUtility.h"
#include "InterruptHandler.h"

#if 0
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
#endif

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
    BYTE bCurrentAPICID;
    BOOL bInterruptFlag;


    bInterruptFlag = SetInterruptFlag(FALSE);
    bCurrentAPICID = GetAPICID();

    // 이미 잠겨 있는 경우
    if (TestAndSet(&(pstMutex->bLockFlag), 0, 1) == FALSE)
    {
        // 현재 태스크가 잠근 경우 (내가 잠근 경우)
        if (pstMutex->qwTaskID == GetRunningTask(bCurrentAPICID)->stLink.qwID)
        {
            SetInterruptFlag(bInterruptFlag);
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
    pstMutex->qwTaskID = GetRunningTask(bCurrentAPICID)->stLink.qwID;
    SetInterruptFlag(bInterruptFlag);
}

/**
 * 잠금 해제 함수
*/
void Unlock(MUTEX *pstMutex)
{
    BOOL bInterruptFlag;

    bInterruptFlag = SetInterruptFlag(FALSE);

    // 잠금이 해제되었거나 뮤텍스를 잠근 태스크가 아닌 경우
    if ((pstMutex->bLockFlag == FALSE) || (pstMutex->qwTaskID != GetRunningTask(GetAPICID())->stLink.qwID))
    {
        SetInterruptFlag(bInterruptFlag);
        return ;
    }

    // 뮤텍스가 중복으로 잠겼으면 잠긴 횟수만 감소
    if(pstMutex->dwLockCount > 1)
    {
        pstMutex->dwLockCount--;
    }
    else
    {
        // 잠금 해제 시 설정 (초기화 할 때랑 똑같음)
        pstMutex->qwTaskID = TASK_INVALIDID;
        pstMutex->dwLockCount = 0;
        pstMutex->bLockFlag = FALSE;
    }

    SetInterruptFlag(bInterruptFlag);
}

void InitializeSpinLock(SPINLOCK *pstSpinLock)
{
    // 잠김 플래그와 횟수, APIC ID, 인터럽트 플래그를 초기화
    pstSpinLock->bLockFlag = FALSE;
    pstSpinLock->dwLockCount = 0;
    pstSpinLock->bAPICID = 0xFF;
    pstSpinLock->bInterruptFlag = FALSE;
}

/**
 *  시스템 전역에서 사용하는 데이터를 위한 잠금 함수
 */
void LockForSpinLock(SPINLOCK *pstSpinLock)
{
    BOOL bInterruptFlag;

    // 인터럽트를 먼저 비활성화
    bInterruptFlag = SetInterruptFlag(FALSE);

    // 이미 잠겨 있다면 내가 잠갔는지 확인하고 그렇다면 잠근 횟수를 증가시킨 뒤 종료
    if (TestAndSet(&(pstSpinLock->bLockFlag), 0, 1) == FALSE)
    {
        // 자신이 잠갔다면 횟수만 증가시킴
        if (pstSpinLock->bAPICID == GetAPICID())
        {
            pstSpinLock->dwLockCount++;
            return;
        }

        // 자신이 아닌 경우는 잠긴 것이 해제될 때까지 대기
        while (TestAndSet(&(pstSpinLock->bLockFlag), 0, 1) == FALSE)
        {
            // TestAndSet() 함수를 계속 호출하여 메모리 버스가 Lock 되는 것을 방지
            while (pstSpinLock->bLockFlag == TRUE)
            {
                Pause();
            }
        }
    }

    // 잠김 설정, 잠김 플래그는 위의 TestAndSet() 함수에서 처리함
    pstSpinLock->dwLockCount = 1;
    pstSpinLock->bAPICID = GetAPICID();

    // 인터럽트 플래그를 저장하여 Unlock 수행 시 복원
    pstSpinLock->bInterruptFlag = bInterruptFlag;
}

/**
 *  시스템 전역에서 사용하는 데이터를 위한 잠금 해제 함수
 */
void UnlockForSpinLock(SPINLOCK *pstSpinLock)
{
    BOOL bInterruptFlag;

    // 인터럽트를 먼저 비활성화
    bInterruptFlag = SetInterruptFlag(FALSE);

    // 스핀락을 잠근 태스크가 아니면 실패
    if ((pstSpinLock->bLockFlag == FALSE) ||
        (pstSpinLock->bAPICID != GetAPICID()))
    {
        SetInterruptFlag(bInterruptFlag);
        return;
    }

    // 스핀락을 중복으로 잠갔으면 잠긴 횟수만 감소
    if (pstSpinLock->dwLockCount > 1)
    {
        pstSpinLock->dwLockCount--;
        return;
    }

    // 스핀락을 해제된 것으로 설정하고 인터럽트 플래그를 복원
    // 인터럽트 플래그는 미리 저장해두었다가 사용
    bInterruptFlag = pstSpinLock->bInterruptFlag;
    pstSpinLock->bAPICID = 0xFF;
    pstSpinLock->dwLockCount = 0;
    pstSpinLock->bInterruptFlag = FALSE;
    pstSpinLock->bLockFlag = FALSE;

    SetInterruptFlag(bInterruptFlag);
}