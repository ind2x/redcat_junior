#ifndef __SYNCHRONIZATION_H__
#define __SYNCHRONIZATION_H__

#include "Types.h"

#pragma pack(push, 1)

// mutex 동기화 자료구조
typedef struct MutexStruct
{
    // volatile은 메모리에서 값을 읽어오도록 설정하는 것
    // 뮤텍스 목적은 태스크의 개수를 제한하는 것이므로 
    // 진입한 태스크 ID, 잠금 여부, 잠금 횟수가 필요함
    volatile QWORD qwTaskID;
    volatile DWORD dwLockCount;
    volatile BOOL bLockFlag;

    // 자료구조 크기를 8바이트 단위로 맞추기 위해 추가 
    BYTE vbPadding[3];
} MUTEX;    // 16바이트

typedef struct SpinLockStruct
{
    volatile DWORD dwLockCount;
    volatile BYTE bAPICID;

    volatile BOOL bLockFlag;

    volatile BOOL bInterruptFlag;

    BYTE vbPadding[1];
} SPINLOCK;

#pragma pack(pop)

#if 0
BOOL LockForSystemData(void);
void UnlockForSystemData(BOOL bInterruptFlag);
#endif

void InitializeMutex(MUTEX *pstMutex);
void Lock(MUTEX *pstMutex);
void Unlock(MUTEX *pstMutex);

void InitializeSpinLock(SPINLOCK *pstSpinLock);
void LockForSpinLock(SPINLOCK *pstSpinLock);
void UnlockForSpinLock(SPINLOCK *pstSpinLock);

#endif /*__SYNCHRONIZATION_H__*/