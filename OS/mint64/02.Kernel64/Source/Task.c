#include "Task.h"
#include "Descriptor.h"
#include "Utility.h"
#include "Synchronization.h"
#include "AssemblyUtility.h"
#include "Console.h"
#include "MultiProcessor.h"
#include "MPConfigurationTable.h"

static SCHEDULER gs_vstScheduler[MAXPROCESSORCOUNT];
static TCBPOOLMANAGER gs_stTCBPoolManager; // TCB풀 관리하는 변수

/**
 * TCB풀 초기화 함수
*/
static void InitializeTCBPool(void)
{
    int i;

    // 자료구조 초기화
    MemSet(&(gs_stTCBPoolManager), 0, sizeof(gs_stTCBPoolManager));

    // TCB풀 시작 주소는 8MB
    gs_stTCBPoolManager.pstStartAddress = (TCB *) TASK_TCBPOOLADDRESS;
    // 8MB 영역부터 0으로 초기화
    MemSet(TASK_TCBPOOLADDRESS, 0, sizeof(TCB) * TASK_MAXCOUNT);

    // 태스크 최대 개수만큼 각각의 TCB 데이터의 ID를 순서대로 저장
    for(i=0; i<TASK_MAXCOUNT; i++)
    {
        gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID = i;
    }

    gs_stTCBPoolManager.iMaxCount = TASK_MAXCOUNT;
    gs_stTCBPoolManager.iAllocatedCount = 1;

    InitializeSpinLock(&gs_stTCBPoolManager.stSpinLock);
}

/**
 * TCB를 할당하고 반환하는 함수
*/
static TCB* AllocateTCB(void)
{
    TCB *pstEmptyTCB;   // 할당해줄 빈 TCB
    int i;

    LockForSpinLock(&gs_stTCBPoolManager.stSpinLock);

    // 최대로 할당할 수 있는 개수와 현재까지 할당해준 TCB 개수가 같은 경우
    if (gs_stTCBPoolManager.iUseCount == gs_stTCBPoolManager.iMaxCount)
    {
        UnlockForSpinLock(&gs_stTCBPoolManager.stSpinLock);
        return NULL;
    }

    // TCB풀을 검색하여 빈 공간을 검색
    for (i = 0; i < gs_stTCBPoolManager.iMaxCount; i++)
    {
        // 할당 여부는 ID의 상위 32비트 값을 통해 판별
        // ID의 상위 32비트와 iAllocate 값과 OR 연산을 하여 ID에 저장함
        // ID의 상위 32비트가 0인 경우 비어있는 공간
        if ((gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID >> 32) == 0)
        {
            // 비어있는 경우 해당 공간을 TCB에 할당
            pstEmptyTCB = &(gs_stTCBPoolManager.pstStartAddress[i]);
            break;
        }
    }

    // 할당이 되었으므로 할당된 TCB의 ID의 상위 32비트를 0이 아닌 값으로 설정
    // iAllocate 값과 ID의 상위 32비트를 OR 연산 후 ID에 저장
    pstEmptyTCB->stLink.qwID = ((QWORD)gs_stTCBPoolManager.iAllocatedCount << 32) | i;

    // 할당해준 개수와 사용한 개수 추가
    gs_stTCBPoolManager.iUseCount++;
    gs_stTCBPoolManager.iAllocatedCount++;
    // 처음 할당해준 경우 1로 설정
    if (gs_stTCBPoolManager.iAllocatedCount == 0)
    {
        gs_stTCBPoolManager.iAllocatedCount = 1;
    }

    UnlockForSpinLock(&gs_stTCBPoolManager.stSpinLock);
    // 할당한 TCB 반환
    return pstEmptyTCB;
}

/**
 * TCB를 해제하는 함수
*/
static void FreeTCB(QWORD qwID)
{
    int i;
    // TCB의 ID는 상위 32비트는 할당 여부
    // 하위 32비트는 인덱스 값
    i = GETTCBOFFSET(qwID); // 태스크의 오프셋을 가져옴

    // 태스크의 컨텍스트를 초기화
    MemSet(&(gs_stTCBPoolManager.pstStartAddress[i].stContext), 0, sizeof(CONTEXT));

    LockForSpinLock(&gs_stTCBPoolManager.stSpinLock);

    // TCB가 할당되었던 공간을 빈 공간으로 설정
    // 즉, 상위 32비트 값이 0이 되는 것
    gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID = i;

    gs_stTCBPoolManager.iUseCount--;

    UnlockForSpinLock(&gs_stTCBPoolManager.stSpinLock);
}

/**
 * 태스크를 생성하는 함수
*/
TCB *CreateTask(QWORD qwFlags, void *pvMemoryAddress, QWORD qwMemorySize, QWORD qwEntryPointAddress, BYTE bAffinity)
{
    TCB *pstTask, *pstProcess;
    void *pvStackAddress;
    BYTE bCurrentAPICID;

    // 임계 영역이므로 잠금
    bCurrentAPICID = GetAPICID();

    // TCB 할당
    pstTask = AllocateTCB();
    
    // 할당에 실패하면 리턴
    if (pstTask == NULL)
    {
        return NULL;
    }

    LockForSpinLock(&(gs_vstScheduler[bCurrentAPICID].stSpinLock));

    // 현재 프로세스 또는 쓰레드가 속한 프로세스 검색
    pstProcess = GetProcessByThread(GetRunningTask(bCurrentAPICID));

    // 프로세스가 없다면 아무런 작업도 하지 않음
    if(pstProcess == NULL)
    {
        FreeTCB(pstTask->stLink.qwID);
        UnlockForSpinLock(&(gs_vstScheduler[bCurrentAPICID].stSpinLock));
        return NULL;
    }

    // 자신이 쓰레드인 경우
    // 부모 프로세스를 등록해줘야 하고
    // 부모 프로세스와 쓰레드와 메모리를 공유하므로 프로세스의 메모리로 설정
    if(qwFlags & TASK_FLAGS_THREAD)
    {
        pstTask->qwParentProcessID = pstProcess->stLink.qwID;
        pstTask->pvMemoryAddress = pstProcess->pvMemoryAddress;
        pstTask->qwMemorySize = pstProcess->qwMemorySize;

        // 쓰레드이므로 프로세스의 자식 쓰레드 리스트에 추가
        AddListToTail(&(pstProcess->stChildThreadList), &(pstTask->stThreadLink));
    }
    // 프로세스라면 인자 값 그대로 설정
    else
    {
        pstTask->qwParentProcessID = pstProcess->stLink.qwID;
        pstTask->pvMemoryAddress = pvMemoryAddress;
        pstTask->qwMemorySize = qwMemorySize;
    }

    // 생성한 태스크의 쓰레드 ID를 자신의 ID로 설정
    pstTask->stThreadLink.qwID = pstTask->stLink.qwID;

    UnlockForSpinLock(&(gs_vstScheduler[bCurrentAPICID].stSpinLock));

    // 태스크의 스택 주소 설정
    pvStackAddress = (void *)(TASK_STACKPOOLADDRESS + (TASK_STACKSIZE * GETTCBOFFSET(pstTask->stLink.qwID)));

    // 태스크에 태스크 속성, 시작 주소, 스택 주소, 스택 크기 설정
    SetUpTask(pstTask, qwFlags, qwEntryPointAddress, pvStackAddress, TASK_STACKSIZE);

    // 자식 쓰레드 리스트 초기화
    InitializeList(&(pstTask->stChildThreadList));

    // FPU 레지스터 사용 여부를 사용하지 않음으로 초기화
    pstTask->bFPUUsed = FALSE;

    // 삽입 전 후로 인터럽트 제어
    // 우선순위에 맞는 준비 리스트에 삽입
    pstTask->bAPICID = bCurrentAPICID;
    pstTask->bAffinity = bAffinity;
    AddTaskToSchedulerWithLoadBalancing(pstTask);
    
    return pstTask;
}

/**
 * TCB를 설정하는 코드
*/
static void SetUpTask(TCB *pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress, void *pvStackAddress, QWORD qwStackSize)
{
    // TCB의 컨텍스트 초기화
    MemSet(pstTCB->stContext.vqRegister, 0, sizeof(pstTCB->stContext.vqRegister));

    // RSP, RBP 레지스터 저장
    pstTCB->stContext.vqRegister[TASK_RSPOFFSET] = (QWORD)pvStackAddress + qwStackSize - 8;
    pstTCB->stContext.vqRegister[TASK_RBPOFFSET] = (QWORD)pvStackAddress + qwStackSize - 8;

    *(QWORD *)( (QWORD) pvStackAddress + qwStackSize - 8) = (QWORD) ExitTask;

    // 위치에 맞게 값을 설정하며, 아래는 세그먼트 레지스터에 값을 설정하는 것
    // 커널에서 실행되므로 커널 코드, 데이터 세그먼트 디스크립터로 설정해준다
    pstTCB->stContext.vqRegister[TASK_CSOFFSET] = GDT_KERNELCODESEGMENT;
    pstTCB->stContext.vqRegister[TASK_DSOFFSET] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[TASK_ESOFFSET] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[TASK_FSOFFSET] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[TASK_GSOFFSET] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[TASK_SSOFFSET] = GDT_KERNELDATASEGMENT;

    // 태스크의 엔트리 포인트를 저장하여 태스크가 실행할 위치를 설정
    // 예를 들면, 함수의 주소를 건네주면 태스크가 실행되면 해당 함수를 실행함
    pstTCB->stContext.vqRegister[TASK_RIPOFFSET] = qwEntryPointAddress;

    // RFLAGS IF비트를 1로 설정하여 인터럽트 활성화
    pstTCB->stContext.vqRegister[TASK_RFLAGSOFFSET] |= 0x0200;

    // 스택 주소, 크기, 플래그 저장
    pstTCB->pvStackAddress = pvStackAddress;
    pstTCB->qwStackSize = qwStackSize;
    pstTCB->qwFlags = qwFlags;
}



//////////////////////////////////////////////////////
// 스케줄러
/**
 * 
*/
void InitializeScheduler(void)
{
    int i;
    int j;
    BYTE bCurrentAPICID;
    TCB *pstTask;

    // 현재 코어의 로컬 APIC ID 확인
    bCurrentAPICID = GetAPICID();

    // Bootstrap Processor만 태스크 풀과 스케줄러 자료구조를 모두 초기화
    if (bCurrentAPICID == 0)
    {
        // 태스크 풀 초기화
        InitializeTCBPool();

        // 준비 리스트와 우선 순위별 실행 횟수를 초기화하고 대기 리스트와 스핀락을 초기화
        for (j = 0; j < MAXPROCESSORCOUNT; j++)
        {
            // 준비 리스트 초기화
            for (i = 0; i < TASK_MAXREADYLISTCOUNT; i++)
            {
                InitializeList(&(gs_vstScheduler[j].vstReadyList[i]));
                gs_vstScheduler[j].viExecuteCount[i] = 0;
            }
            // 대기 리스트 초기화
            InitializeList(&(gs_vstScheduler[j].stWaitList));

            // 스핀락 초기화
            InitializeSpinLock(&(gs_vstScheduler[j].stSpinLock));
        }
    }

    // TCB를 할당 받아 부팅을 수행한 태스크를 커널 최초의 프로세스로 설정
    pstTask = AllocateTCB();
    gs_vstScheduler[bCurrentAPICID].pstRunningTask = pstTask;

    // BSP의 콘솔 쉘이나 AP의 유휴 태스크(Idle Task)는 모두 현재 코어에서만 실행하도록
    // 로컬 APIC ID와 프로세서 친화도를 현재 코어의 로컬 APIC ID로 설정
    pstTask->bAPICID = bCurrentAPICID;
    pstTask->bAffinity = bCurrentAPICID;

    // Bootstrap Processor는 콘솔 셸을 실행
    if (bCurrentAPICID == 0)
    {
        pstTask->qwFlags = TASK_FLAGS_HIGHEST | TASK_FLAGS_PROCESS | TASK_FLAGS_SYSTEM;
    }
    // Application Processor는 특별히 긴급한 태스크가 없으므로 유휴(Idle) 태스크를 실행
    else
    {
        pstTask->qwFlags = TASK_FLAGS_LOWEST | TASK_FLAGS_PROCESS | TASK_FLAGS_SYSTEM | TASK_FLAGS_IDLE;
    }

    pstTask->qwParentProcessID = pstTask->stLink.qwID;
    pstTask->pvMemoryAddress = (void *)0x100000;
    pstTask->qwMemorySize = 0x500000;
    pstTask->pvStackAddress = (void *)0x600000;
    pstTask->qwStackSize = 0x100000;

    // 프로세서 사용률을 계산하는데 사용하는 자료구조 초기화
    gs_vstScheduler[bCurrentAPICID].qwSpendProcessorTimeInIdleTask = 0;
    gs_vstScheduler[bCurrentAPICID].qwProcessorLoad = 0;

    // FPU를 사용한 태스크 ID를 유효하지 않은 값으로 초기화
    gs_vstScheduler[bCurrentAPICID].qwLastFPUUsedTaskID = TASK_INVALIDID;
}

/**
 * 인자로 받은 태스크를 현재 수행 중인 태스크로 설정
*/
void SetRunningTask(BYTE bAPICID, TCB *pstTask)
{
    LockForSpinLock(&(gs_vstScheduler[bAPICID].stSpinLock));

    gs_vstScheduler[bAPICID].pstRunningTask = pstTask;

    UnlockForSpinLock(&(gs_vstScheduler[bAPICID].stSpinLock));
}

/**
 * 현재 수행중인 태스크 반환
*/
TCB *GetRunningTask(BYTE bAPICID)
{
    TCB *pstRunningTask;

    LockForSpinLock(&(gs_vstScheduler[bAPICID].stSpinLock));

    pstRunningTask = gs_vstScheduler[bAPICID].pstRunningTask;

    UnlockForSpinLock(&(gs_vstScheduler[bAPICID].stSpinLock));

    return pstRunningTask;
}

/**
 * 스케줄 준비 리스트에서 다음 태스크를 반환
*/
static TCB *GetNextTaskToRun(BYTE bAPICID)
{
    TCB *pstTarget = NULL;
    int iTaskCount, i, j;

    // 
    for(j=0; j<2; j++)
    {
        // 우선순위별 준비 큐들을 돌면서 멀티레벨 큐 스케줄링 진행
        for(i=0; i<TASK_MAXREADYLISTCOUNT; i++)
        {
            // 준비 큐에 있는 태스크 개수 반환
            iTaskCount = GetListCount(&(gs_vstScheduler[bAPICID].vstReadyList[i]));

            // 해당 우선순위 큐에서 태스크를 실행한 횟수가 전체 태스크 수보다 큰지 비교
            // 즉, 준비 큐에 있는 태스크들을 모두 한번 씩 실행했었는지 검사
            if (gs_vstScheduler[bAPICID].viExecuteCount[i] < iTaskCount)
            {
                // 아직 전부 한번 씩 실행하지 않았다면 진행
                // 큐(리스트)에서 데이터를 꺼내오고 반환
                pstTarget = (TCB *)RemoveListFromHeader(&(gs_vstScheduler[bAPICID].vstReadyList[i]));
                // 실행 횟수 수정
                gs_vstScheduler[bAPICID].viExecuteCount[i]++;
                break;
            }
            // 전부 한번 씩 실행했다면 다음을 위해 실행 횟수를 0으로 초기화 
            else
            {
                gs_vstScheduler[bAPICID].viExecuteCount[i] = 0;
            }
        }

        // 만약 수행할 태스크를 찾았으면 종료
        if(pstTarget != NULL) break;
    }

    return pstTarget;
}

/**
 * 우선순위에 따라 해당 우선순위 준비 큐에 태스크 삽입
*/
static BOOL AddTaskToReadyList(BYTE bAPICID, TCB *pstTask)
{
    // 우선순위
    BYTE bPriority;

    // 우선순위 확인
    bPriority = GETPRIORITY(pstTask->qwFlags);
    if(bPriority >= TASK_MAXREADYLISTCOUNT)
    {
        return FALSE;
    }

    // 우선순위에 맞는 준비 큐(리스트)에 태스크 삽입
    AddListToTail(&(gs_vstScheduler[bAPICID].vstReadyList[bPriority]), pstTask);
    return TRUE;
}

/**
 * 준비 큐에서 태스크를 제거하고 반환하는 함수
 * 우선순위를 동적으로 변경해 주는 부분이 포함됨
*/
static TCB* RemoveTaskFromReadyList(BYTE bAPICID, QWORD qwTaskID)
{
    TCB *pstTarget;
    BYTE bPriority;

    // 유효하지 않은 태스크 ID이면 종료
    if(GETTCBOFFSET(qwTaskID) >= TASK_MAXCOUNT)
    {
        return NULL;
    }

    // 태스크의 오프셋을 이용해 TCB 풀에서 TCB를 검색
    pstTarget = &(gs_stTCBPoolManager.pstStartAddress[GETTCBOFFSET(qwTaskID)]);
    
    // TCB풀에서 찾은 태스크가 맞는지 검사
    if (pstTarget->stLink.qwID != qwTaskID)
    {
        return NULL;
    }
    
    // 우선순위 확인
    bPriority = GETPRIORITY(pstTarget->qwFlags);

    // 해당 우선순위 준비 리스트에서 해당 태스크 제거 후 반환
    pstTarget = RemoveList(&(gs_vstScheduler[bAPICID].vstReadyList[bPriority]), qwTaskID);

    return pstTarget;
}

/**
 *  태스크가 포함된 스케줄러의 ID를 반환하고, 해당 스케줄러의 스핀락을 잠금
 */
static BOOL FindSchedulerOfTaskAndLock(QWORD qwTaskID, BYTE *pbAPICID)
{
    TCB *pstTarget;
    BYTE bAPICID;

    while (1)
    {
        // 태스크 ID로 태스크 자료구조를 찾아서 어느 스케줄러에서 실행 중인지 확인
        pstTarget = &(gs_stTCBPoolManager.pstStartAddress[GETTCBOFFSET(qwTaskID)]);
        if ((pstTarget == NULL) || (pstTarget->stLink.qwID != qwTaskID))
        {
            return FALSE;
        }

        // 현재 태스크가 실행되는 코어의 ID를 확인
        bAPICID = pstTarget->bAPICID;

        // 임계 영역 시작
        LockForSpinLock(&(gs_vstScheduler[bAPICID].stSpinLock));

        // 스핀락을 획득한 이후 다시 확인하여 같은 코어에서 실행되는지 확인
        // 태스크가 수행되는 코어를 찾은 후 정확하게 스핀락을 걸기 위해 2중으로 검사
        pstTarget = &(gs_stTCBPoolManager.pstStartAddress[GETTCBOFFSET(qwTaskID)]);
        if (pstTarget->bAPICID == bAPICID)
        {
            break;
        }

        // 태스크 자료구조에 저장된 로컬 APIC ID의 값이 스핀락을 획득하기 전과 후가
        // 다르다면, 스핀락을 획득하는 동안 태스크가 다른 코어로 옮겨간 것임
        // 따라서 다시 스핀락을 해제하고 옮겨진 코어의 스핀락을 획득해야 함
        // 임계 영역 끝
        UnlockForSpinLock(&(gs_vstScheduler[bAPICID].stSpinLock));
    }

    *pbAPICID = bAPICID;
    return TRUE;
}

/**
 * 우선순위를 변경하는 함수
*/
BOOL ChangePriority(QWORD qwTaskID, BYTE bPriority)
{
    TCB *pstTarget;
    BYTE bAPICID;
    
    if (bPriority > TASK_MAXREADYLISTCOUNT)
    {
        return FALSE;
    }

    if(FindSchedulerOfTaskAndLock(qwTaskID, &bAPICID) == FALSE)
    {
        return FALSE;
    }

    // 현재 수행중인 태스크의 우선순위를 변경하는 경우
    // 우선순위만 변경해주면 됨
    pstTarget = gs_vstScheduler[bAPICID].pstRunningTask;

    // 현재 태스크가 변경하고자 하는 태스크이면 우선순위만 변경 
    if (pstTarget->stLink.qwID == qwTaskID)
    {
        SETPRIORITY(pstTarget->qwFlags, bPriority);
    }
    // 준비 리스트에 있는 태스크의 우선순위를 변경하는 경우
    else
    {
        // 먼저 해당 태스크를 해당 우선순위 큐에서 제거해야 함
        pstTarget = RemoveTaskFromReadyList(bAPICID, qwTaskID);
        
        // 해당 큐에 없으면 TCB풀에서 검색
        if (pstTarget == NULL)
        {
            // TCB 풀에서 검색
            pstTarget = GetTCBInTCBPool(GETTCBOFFSET(qwTaskID));
            
            // 태스크를 찾았으면 우선순위 변경
            if (pstTarget != NULL)
            {
                SETPRIORITY(pstTarget->qwFlags, bPriority);
            }
        }
        // 성공적으로 제거했으면 우선순위 변경 후 변경한 우선순위 큐에 삽입
        else
        {
            SETPRIORITY(pstTarget->qwFlags, bPriority);
            AddTaskToReadyList(bAPICID, pstTarget);
        }
    }

    UnlockForSpinLock(&(gs_vstScheduler[bAPICID].stSpinLock));
    return TRUE;
}

/**
 * 태스크 전환 함수
*/
BOOL Schedule(void)
{
    TCB *pstRunningTask, *pstNextTask;
    BOOL bPreviousInterrupt;
    BYTE bCurrentAPICID;

    // 인터럽트 비활성화
    bPreviousInterrupt = SetInterruptFlag(FALSE);

    bCurrentAPICID = GetAPICID();

    if(GetReadyTaskCount(bCurrentAPICID) < 1)
    {
        SetInterruptFlag(bPreviousInterrupt);
        return FALSE;
    }

    LockForSpinLock(&(gs_vstScheduler[bCurrentAPICID].stSpinLock));

    // 전환할 다음 태스크 검색
    pstNextTask = GetNextTaskToRun(bCurrentAPICID);
    if (pstNextTask == NULL)
    {
        UnlockForSpinLock(&(gs_vstScheduler[bCurrentAPICID].stSpinLock));
        SetInterruptFlag(bPreviousInterrupt);
        return FALSE;
    }

    // 현재 수행중인 태스크를 다음 태스크로 수정
    pstRunningTask = gs_vstScheduler[bCurrentAPICID].pstRunningTask;
    gs_vstScheduler[bCurrentAPICID].pstRunningTask = pstNextTask;

    // 유휴 태스크에서 전환된 것이라면 사용한 프로세서 시간을 증가시킴
    if ((pstRunningTask->qwFlags & TASK_FLAGS_IDLE) == TASK_FLAGS_IDLE)
    {
        gs_vstScheduler[bCurrentAPICID].qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME - gs_vstScheduler[bCurrentAPICID].iProcessorTime;
    }

    // 다음에 수행할 태스크가 FPU를 마지막으로 사용하지 않았다면
    if (gs_vstScheduler[bCurrentAPICID].qwLastFPUUsedTaskID != pstNextTask->stLink.qwID)
    {
        // 태스크가 전환되었으니 CR0의 TS 비트를 1로 설정
        SetTS();
    }
    else
    {
        // 사용한 경우 바로 FPU 컨텍스트에서 복원해서 사용하면 되므로 0으로 설정
        ClearTS();
    }
    
    // 프로세서 사용 시간 업데이트
    gs_vstScheduler[bCurrentAPICID].iProcessorTime = TASK_PROCESSORTIME;

    // 태스크가 종료 태스크인 경우
    if (pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK)
    {
        // 대기 큐에 현재 태스크를 삽입
        AddListToTail(&(gs_vstScheduler[bCurrentAPICID].stWaitList), pstRunningTask);
        UnlockForSpinLock(&(gs_vstScheduler[bCurrentAPICID].stSpinLock));
        // 컨텍스트는 저장할 필요가 없으므로 콘텍스트 전환
        SwitchContext(NULL, &(pstNextTask->stContext));
    }
    // 종료 태스크가 아닌 경우
    else
    {
        // 전환되었으니 조금 전까지 실행된 태스크는 준비 큐에 삽입되어야 함
        AddTaskToReadyList(bCurrentAPICID, pstRunningTask);
        UnlockForSpinLock(&(gs_vstScheduler[bCurrentAPICID].stSpinLock));
        // 삽입 후 컨텍스트 전환
        SwitchContext(&(pstRunningTask->stContext), &(pstNextTask->stContext));
    }
    
    SetInterruptFlag(bPreviousInterrupt);
    return FALSE;
}

/**
 * 인터럽트에 의해 태스크가 전환되는 경우
*/
BOOL ScheduleInInterrupt(void)
{
    TCB *pstRunningTask, *pstNextTask;
    char *pcContextAddress;
    BYTE bCurrentAPICID;
    QWORD qwISTStartAddress;

    // 현재 로컬 APIC ID 확인
    bCurrentAPICID = GetAPICID();

    // 임계 영역 시작
    LockForSpinLock(&(gs_vstScheduler[bCurrentAPICID].stSpinLock));

    // 전환할 태스크가 없으면 종료
    pstNextTask = GetNextTaskToRun(bCurrentAPICID);
    if (pstNextTask == NULL)
    {
        // 임계 영역 끝
        UnlockForSpinLock(&(gs_vstScheduler[bCurrentAPICID].stSpinLock));
        return FALSE;
    }

    //==========================================================================
    //  태스크 전환 처리
    //      인터럽트 핸들러에서 저장한 콘텍스트를 다른 콘텍스트로 덮어쓰는 방법으로 처리
    //==========================================================================
    // IST의 끝부분부터 코어 0 -> 코어 15 순으로 64Kbyte씩 쓰고 있으므로, 로컬 APIC ID를
    // 이용해서 IST 어드레스를 계산
    qwISTStartAddress = IST_STARTADDRESS + IST_SIZE -
                        (IST_SIZE / MAXPROCESSORCOUNT * bCurrentAPICID);
    pcContextAddress = (char *)qwISTStartAddress - sizeof(CONTEXT);

    pstRunningTask = gs_vstScheduler[bCurrentAPICID].pstRunningTask;
    gs_vstScheduler[bCurrentAPICID].pstRunningTask = pstNextTask;

    // 유휴 태스크에서 전환되었다면 사용한 Tick Count를 증가시킴
    if ((pstRunningTask->qwFlags & TASK_FLAGS_IDLE) == TASK_FLAGS_IDLE)
    {
        gs_vstScheduler[bCurrentAPICID].qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME;
    }

    // 태스크 종료 플래그가 설정된 경우, 콘텍스트를 저장하지 않고 대기 리스트에만 삽입
    if (pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK)
    {
        AddListToTail(&(gs_vstScheduler[bCurrentAPICID].stWaitList),
                       pstRunningTask);
    }
    // 태스크가 종료되지 않으면 IST에 있는 콘텍스트를 복사하고, 현재 태스크를 준비 리스트로
    // 옮김
    else
    {
        MemCpy(&(pstRunningTask->stContext), pcContextAddress, sizeof(CONTEXT));
    }

    // 다음에 수행할 태스크가 FPU를 쓴 태스크가 아니라면 TS 비트 설정
    if (gs_vstScheduler[bCurrentAPICID].qwLastFPUUsedTaskID !=
        pstNextTask->stLink.qwID)
    {
        SetTS();
    }
    else
    {
        ClearTS();
    }

    // 임계 영역 끝
    UnlockForSpinLock(&(gs_vstScheduler[bCurrentAPICID].stSpinLock));

    // 전환해서 실행할 태스크를 Running Task로 설정하고 콘텍스트를 IST에 복사해서
    // 자동으로 태스크 전환이 일어나도록 함
    MemCpy(pcContextAddress, &(pstNextTask->stContext), sizeof(CONTEXT));

    // 종료하는 태스크가 아니면 스케줄러에 태스크 추가
    if ((pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK) != TASK_FLAGS_ENDTASK)
    {
        // 스케줄러에 태스크를 추가, 부하 분산을 고려함
        AddTaskToSchedulerWithLoadBalancing(pstRunningTask);
    }

    // 프로세서 사용 시간을 업데이트
    gs_vstScheduler[bCurrentAPICID].iProcessorTime = TASK_PROCESSORTIME;

    return TRUE;
}

/**
 * 태스크의 CPU 사용 시간을 하나 줄임
*/
void DecreaseProcessorTime(BYTE bAPICID)
{
    gs_vstScheduler[bAPICID].iProcessorTime--;
}

/**
 * 태스크의 CPU 사용 시간이 다 지났는지 확인
*/
BOOL IsProcessorTimeExpired(BYTE bAPICID)
{
    if (gs_vstScheduler[bAPICID].iProcessorTime <= 0)
    {
        return TRUE;
    }
    return FALSE;
}

/**
 * 태스크를 종료하는 함수
*/
BOOL EndTask(QWORD qwTaskID)
{
    TCB *pstTarget;
    BYTE bPriority;
    BYTE bAPICID;

    // 태스크가 포함된 코어의 로컬 APIC ID를 찾은 후, 스핀락을 잠금
    if (FindSchedulerOfTaskAndLock(qwTaskID, &bAPICID) == FALSE)
    {
        return FALSE;
    }

    // 현재 실행중인 태스크이면 EndTask 비트를 설정하고 태스크를 전환
    pstTarget = gs_vstScheduler[bAPICID].pstRunningTask;
    if (pstTarget->stLink.qwID == qwTaskID)
    {
        pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
        SETPRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);

        // 임계 영역 끝
        UnlockForSpinLock(&(gs_vstScheduler[bAPICID].stSpinLock));

        // 현재 스케줄러에서 실행중인 태스크의 경우만 아래를 적용
        if (GetAPICID() == bAPICID)
        {
            Schedule();

            // 태스크가 전환 되었으므로 아래 코드는 절대 실행되지 않음
            while (1)
            {
                ;
            }
        }

        return TRUE;
    }

    // 실행 중인 태스크가 아니면 준비 큐에서 직접 찾아서 대기 리스트에 연결
    // 준비 리스트에서 태스크를 찾지 못하면 직접 태스크를 찾아서 태스크 종료 비트를
    // 설정
    pstTarget = RemoveTaskFromReadyList(bAPICID, qwTaskID);
    if (pstTarget == NULL)
    {
        // 태스크 ID로 직접 찾아서 설정
        pstTarget = GetTCBInTCBPool(GETTCBOFFSET(qwTaskID));
        if (pstTarget != NULL)
        {
            pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
            SETPRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);
        }

        // 임계 영역 끝
        UnlockForSpinLock(&(gs_vstScheduler[bAPICID].stSpinLock));
        return TRUE;
    }

    pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
    SETPRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);
    AddListToTail(&(gs_vstScheduler[bAPICID].stWaitList), pstTarget);

    // 임계 영역 끝
    UnlockForSpinLock(&(gs_vstScheduler[bAPICID].stSpinLock));
    return TRUE;
}

/**
 *  태스크가 자신을 종료함
 */
void ExitTask(void)
{
    EndTask(gs_vstScheduler[GetAPICID()].pstRunningTask->stLink.qwID);
}

/**
 *  준비 큐에 있는 모든 태스크의 수를 반환
 */
int GetReadyTaskCount(BYTE bAPICID)
{
    int iTotalCount = 0;
    int i;

    // 임계 영역 시작
    LockForSpinLock(&(gs_vstScheduler[bAPICID].stSpinLock));

    // 모든 준비 큐를 확인하여 태스크 개수를 구함
    for (i = 0; i < TASK_MAXREADYLISTCOUNT; i++)
    {
        iTotalCount += GetListCount(&(gs_vstScheduler[bAPICID].vstReadyList[i]));
    }

    // 임계 영역 끝
    UnlockForSpinLock(&(gs_vstScheduler[bAPICID].stSpinLock));
    return iTotalCount;
}

/**
 *  전체 태스크의 수를 반환
 */
int GetTaskCount(BYTE bAPICID)
{
    int iTotalCount;

    // 준비 큐의 태스크 수를 구한 후, 대기 큐의 태스크 수와 현재 수행 중인 태스크 수를 더함
    iTotalCount = GetReadyTaskCount(bAPICID);

    // 임계 영역 시작
    LockForSpinLock(&(gs_vstScheduler[bAPICID].stSpinLock));

    iTotalCount += GetListCount(&(gs_vstScheduler[bAPICID].stWaitList)) + 1;

    // 임계 영역 끝
    UnlockForSpinLock(&(gs_vstScheduler[bAPICID].stSpinLock));
    return iTotalCount;
}

/**
 *  TCB 풀에서 해당 오프셋의 TCB를 반환
 */
TCB *GetTCBInTCBPool(int iOffset)
{
    if ((iOffset < -1) && (iOffset > TASK_MAXCOUNT))
    {
        return NULL;
    }

    return &(gs_stTCBPoolManager.pstStartAddress[iOffset]);
}

/**
 *  태스크가 존재하는지 여부를 반환
 */
BOOL IsTaskExist(QWORD qwID)
{
    TCB *pstTCB;

    // ID로 TCB를 반환
    pstTCB = GetTCBInTCBPool(GETTCBOFFSET(qwID));
    // TCB가 없거나 ID가 일치하지 않으면 존재하지 않는 것임
    if ((pstTCB == NULL) || (pstTCB->stLink.qwID != qwID))
    {
        return FALSE;
    }
    return TRUE;
}

/**
 *  프로세서의 사용률을 반환
 */
QWORD GetProcessorLoad(BYTE bAPICID)
{
    return gs_vstScheduler[bAPICID].qwProcessorLoad;
}

/**
 *  스레드가 소속된 프로세스를 반환
 */
static TCB *GetProcessByThread(TCB *pstThread)
{
    TCB *pstProcess;

    // 만약 내가 프로세스이면 자신을 반환
    if (pstThread->qwFlags & TASK_FLAGS_PROCESS)
    {
        return pstThread;
    }

    // 내가 프로세스가 아니라면, 부모 프로세스로 설정된 태스크 ID를 통해
    // TCB 풀에서 태스크 자료구조 추출
    pstProcess = GetTCBInTCBPool(GETTCBOFFSET(pstThread->qwParentProcessID));

    // 만약 프로세스가 없거나, 태스크 ID가 일치하지 않는다면 NULL을 반환
    if ((pstProcess == NULL) || (pstProcess->stLink.qwID != pstThread->qwParentProcessID))
    {
        return NULL;
    }

    return pstProcess;
}

/**
 *  각 스케줄러의 태스크 수를 이용하여 적절한 스케줄러에 태스크 추가
 *      부하 분산 기능을 사용하지 않는 경우 현재 코어에 삽입
 *      부하 분산을 사용하지 않는 경우, 태스크가 현재 수행되는 코어에서 계속 수행하므로
 *      pstTask에는 적어도 APIC ID가 설정되어 있어야 함
 */
void AddTaskToSchedulerWithLoadBalancing(TCB *pstTask)
{
    BYTE bCurrentAPICID;
    BYTE bTargetAPICID;

    // 태스크가 동작하던 코어의 APIC를 확인
    bCurrentAPICID = pstTask->bAPICID;

    // 부하 분산 기능을 사용하고, 프로세서 친화도(Affinity)가 모든 코어(0xFF)로
    // 설정되었으면 부하 분산 수행
    if ((gs_vstScheduler[bCurrentAPICID].bUseLoadBalancing == TRUE) &&
        (pstTask->bAffinity == TASK_LOADBALANCINGID))
    {
        // 태스크를 추가할 스케줄러를 선택
        bTargetAPICID = FindSchedulerOfMinumumTaskCount(pstTask);
    }
    // 태스크 부하 분산 기능과 관계 없이 프로세서 친화도 필드에 다른 코어의 APIC ID가
    // 들어있으면 해당 스케줄러로 옮겨줌
    else if ((pstTask->bAffinity != bCurrentAPICID) &&
             (pstTask->bAffinity != TASK_LOADBALANCINGID))
    {
        bTargetAPICID = pstTask->bAffinity;
    }
    // 부하 분산 기능을 사용하지 않는 경우는 현재 스케줄러에 다시 삽입
    else
    {
        bTargetAPICID = bCurrentAPICID;
    }

    // 임계 영역 시작
    LockForSpinLock(&(gs_vstScheduler[bCurrentAPICID].stSpinLock));
    // 태스크를 추가할 스케줄러가 현재 스케줄러와 다르다면 태스크를 이동함.
    // FPU는 공유되지 않으므로 현재 태스크가 FPU를 마지막으로 썼다면 FPU 콘텍스트를
    // 메모리에 저장해야 함
    if ((bCurrentAPICID != bTargetAPICID) &&
        (pstTask->stLink.qwID ==
         gs_vstScheduler[bCurrentAPICID].qwLastFPUUsedTaskID))
    {
        // FPU를 저장하기 전에 TS 비트를 끄지 않으면, 예외 7(Device Not Available)이
        // 발생하므로 주의해야 함
        ClearTS();
        SaveFPUContext(pstTask->vqwFPUContext);
        gs_vstScheduler[bCurrentAPICID].qwLastFPUUsedTaskID = TASK_INVALIDID;
    }
    // 임계 영역 끝
    UnlockForSpinLock(&(gs_vstScheduler[bCurrentAPICID].stSpinLock));

    // 임계 영역 시작
    LockForSpinLock(&(gs_vstScheduler[bTargetAPICID].stSpinLock));

    // 태스크를 수행할 코어의 APIC ID를 설정하고, 해당 스케줄러에 태스크 삽입
    pstTask->bAPICID = bTargetAPICID;
    AddTaskToReadyList(bTargetAPICID, pstTask);

    // 임계 영역 끝
    UnlockForSpinLock(&(gs_vstScheduler[bTargetAPICID].stSpinLock));
}

/**
 *  태스크를 추가할 스케줄러의 ID를 반환
 *      파라미터로 전달된 태스크 자료구조에는 적어도 플래그와 프로세서 친화도(Affinity) 필드가
 *      채워져있어야 함
 */
static BYTE FindSchedulerOfMinumumTaskCount(const TCB *pstTask)
{
    BYTE bPriority;
    BYTE i;
    int iCurrentTaskCount;
    int iMinTaskCount;
    BYTE bMinCoreIndex;
    int iTempTaskCount;
    int iProcessorCount;

    // 코어의 개수를 확인
    iProcessorCount = GetProcessorCount();

    // 코어가 하나라면 현재 코어에서 계속 수행
    if (iProcessorCount == 1)
    {
        return pstTask->bAPICID;
    }

    // 우선 순위 추출
    bPriority = GETPRIORITY(pstTask->qwFlags);

    // 태스크가 포함된 스케줄러에서 태스크와 같은 우선 순위의 태스크 수를 확인
    iCurrentTaskCount = GetListCount(&(gs_vstScheduler[pstTask->bAPICID].vstReadyList[bPriority]));

    // 나머지 코어에서 같은 현재 태스크와 같은 레벨을 검사
    // 자신과 태스크의 수가 적어도 2 이상 차이 나는 것 중에서 가장 태스크 수가 작은
    // 스케줄러의 ID를 반환
    iMinTaskCount = TASK_MAXCOUNT;
    bMinCoreIndex = pstTask->bAPICID;
    for (i = 0; i < iProcessorCount; i++)
    {
        if (i == pstTask->bAPICID)
        {
            continue;
        }

        // 모든 스케줄러를 돌면서 확인
        iTempTaskCount = GetListCount(&(gs_vstScheduler[i].vstReadyList[bPriority]));

        // 현재 코어와 태스크 수가 2개 이상 차이가 나고 이전까지 태스크 수가 가장 작았던
        // 코어보다 더 작다면 정보를 갱신함
        if ((iTempTaskCount + 2 <= iCurrentTaskCount) &&
            (iTempTaskCount < iMinTaskCount))
        {
            bMinCoreIndex = i;
            iMinTaskCount = iTempTaskCount;
        }
    }

    return bMinCoreIndex;
}

/**
 *  파라미터로 전달된 코어에 태스크 부하 분산 기능 사용 여부를 설정
 */
void SetTaskLoadBalancing(BYTE bAPICID, BOOL bUseLoadBalancing)
{
    gs_vstScheduler[bAPICID].bUseLoadBalancing = bUseLoadBalancing;
}

/**
 *  프로세서 친화도를 변경
 */
BOOL ChangeProcessorAffinity(QWORD qwTaskID, BYTE bAffinity)
{
    TCB *pstTarget;
    BYTE bAPICID;

    // 태스크가 포함된 코어의 로컬 APIC ID를 찾은 후, 스핀락을 잠금
    if (FindSchedulerOfTaskAndLock(qwTaskID, &bAPICID) == FALSE)
    {
        return FALSE;
    }

    // 현재 실행중인 태스크이면 프로세서 친화도만 변경. 실제 태스크가 옮겨지는 시점은
    // 태스크 전환이 수행될 때임
    pstTarget = gs_vstScheduler[bAPICID].pstRunningTask;
    if (pstTarget->stLink.qwID == qwTaskID)
    {
        // 프로세서 친화도 변경
        pstTarget->bAffinity = bAffinity;

        // 임계 영역 끝
        UnlockForSpinLock(&(gs_vstScheduler[bAPICID].stSpinLock));
    }
    // 실행중인 태스크가 아니면 준비 리스트에서 찾아서 즉시 이동
    else
    {
        // 준비 리스트에서 태스크를 찾지 못하면 직접 태스크를 찾아서 친화도를 설정
        pstTarget = RemoveTaskFromReadyList(bAPICID, qwTaskID);
        if (pstTarget == NULL)
        {
            pstTarget = GetTCBInTCBPool(GETTCBOFFSET(qwTaskID));
            if (pstTarget != NULL)
            {
                // 프로세서 친화도 변경
                pstTarget->bAffinity = bAffinity;
            }
        }
        else
        {
            // 프로세서 친화도 변경
            pstTarget->bAffinity = bAffinity;
        }

        // 임계 영역 끝
        UnlockForSpinLock(&(gs_vstScheduler[bAPICID].stSpinLock));

        // 프로세서 부하 분산을 고려해서 스케줄러에 등록
        AddTaskToSchedulerWithLoadBalancing(pstTarget);
    }

    return TRUE;
}

//==============================================================================
//  유휴 태스크 관련
//==============================================================================
/**
 *  유휴 태스크
 *      대기 큐에 삭제 대기중인 태스크를 정리
 */
void IdleTask(void)
{
    TCB *pstTask, *pstChildThread, *pstProcess;
    QWORD qwLastMeasureTickCount, qwLastSpendTickInIdleTask;
    QWORD qwCurrentMeasureTickCount, qwCurrentSpendTickInIdleTask;
    QWORD qwTaskID, qwChildThreadID;
    int i, iCount;
    void *pstThreadLink;
    BYTE bCurrentAPICID;
    BYTE bProcessAPICID;

    // 현재 코어의 로컬 APIC ID를 확인
    bCurrentAPICID = GetAPICID();

    // 프로세서 사용량 계산을 위해 기준 정보를 저장
    qwLastSpendTickInIdleTask =
        gs_vstScheduler[bCurrentAPICID].qwSpendProcessorTimeInIdleTask;
    qwLastMeasureTickCount = GetTickCount();

    while (1)
    {
        // 현재 상태를 저장
        qwCurrentMeasureTickCount = GetTickCount();
        qwCurrentSpendTickInIdleTask =
            gs_vstScheduler[bCurrentAPICID].qwSpendProcessorTimeInIdleTask;

        // 프로세서 사용량을 계산
        // 100 - ( 유휴 태스크가 사용한 프로세서 시간 ) * 100 / ( 시스템 전체에서
        // 사용한 프로세서 시간 )
        if (qwCurrentMeasureTickCount - qwLastMeasureTickCount == 0)
        {
            gs_vstScheduler[bCurrentAPICID].qwProcessorLoad = 0;
        }
        else
        {
            gs_vstScheduler[bCurrentAPICID].qwProcessorLoad = 100 -
                                                              (qwCurrentSpendTickInIdleTask - qwLastSpendTickInIdleTask) *
                                                                  100 / (qwCurrentMeasureTickCount - qwLastMeasureTickCount);
        }

        // 현재 상태를 이전 상태에 보관
        qwLastMeasureTickCount = qwCurrentMeasureTickCount;
        qwLastSpendTickInIdleTask = qwCurrentSpendTickInIdleTask;

        // 프로세서의 부하에 따라 쉬게 함
        HaltProcessorByLoad(bCurrentAPICID);

        // 대기 큐에 대기중인 태스크가 있으면 태스크를 종료함
        if (GetListCount(&(gs_vstScheduler[bCurrentAPICID].stWaitList)) > 0)
        {
            while (1)
            {
                // 임계 영역 시작
                LockForSpinLock(&(gs_vstScheduler[bCurrentAPICID].stSpinLock));
                pstTask = RemoveListFromHeader(
                    &(gs_vstScheduler[bCurrentAPICID].stWaitList));
                // 임계 영역 끝
                UnlockForSpinLock(&(gs_vstScheduler[bCurrentAPICID].stSpinLock));

                if (pstTask == NULL)
                {
                    break;
                }

                if (pstTask->qwFlags & TASK_FLAGS_PROCESS)
                {
                    // 프로세스를 종료할 때 자식 스레드가 존재하면 스레드를 모두
                    // 종료하고, 다시 자식 스레드 리스트에 삽입
                    iCount = GetListCount(&(pstTask->stChildThreadList));
                    for (i = 0; i < iCount; i++)
                    {
                        // 임계 영역 시작
                        LockForSpinLock(
                            &(gs_vstScheduler[bCurrentAPICID].stSpinLock));
                        // 스레드 링크의 어드레스에서 꺼내 스레드를 종료시킴
                        pstThreadLink = (TCB *)RemoveListFromHeader(
                            &(pstTask->stChildThreadList));
                        if (pstThreadLink == NULL)
                        {
                            // 임계 영역 끝
                            UnlockForSpinLock(
                                &(gs_vstScheduler[bCurrentAPICID].stSpinLock));
                            break;
                        }

                        // 자식 스레드 리스트에 연결된 정보는 태스크 자료구조에 있는
                        // stThreadLink의 시작 어드레스이므로, 태스크 자료구조의 시작
                        // 어드레스를 구하려면 별도의 계산이 필요함
                        pstChildThread = GETTCBFROMTHREADLINK(pstThreadLink);

                        // 다시 자식 스레드 리스트에 삽입하여 해당 스레드가 종료될 때
                        // 자식 스레드가 프로세스를 찾아 스스로 리스트에서 제거하도록 함
                        AddListToTail(&(pstTask->stChildThreadList),
                                       &(pstChildThread->stThreadLink));
                        qwChildThreadID = pstChildThread->stLink.qwID;
                        // 임계 영역 끝
                        UnlockForSpinLock(
                            &(gs_vstScheduler[bCurrentAPICID].stSpinLock));

                        // 자식 스레드를 찾아서 종료
                        EndTask(qwChildThreadID);
                    }

                    // 아직 자식 스레드가 남아있다면 자식 스레드가 다 종료될 때까지
                    // 기다려야 하므로 다시 대기 리스트에 삽입
                    if (GetListCount(&(pstTask->stChildThreadList)) > 0)
                    {
                        // 임계 영역 시작
                        LockForSpinLock(
                            &(gs_vstScheduler[bCurrentAPICID].stSpinLock));
                        AddListToTail(
                            &(gs_vstScheduler[bCurrentAPICID].stWaitList),
                            pstTask);
                        // 임계 영역 끝
                        UnlockForSpinLock(
                            &(gs_vstScheduler[bCurrentAPICID].stSpinLock));
                        continue;
                    }
                    // 프로세스를 종료해야 하므로 할당 받은 메모리 영역을 삭제
                    else
                    {
                        // TODO: 추후에 코드 삽입
                    }
                }
                else if (pstTask->qwFlags & TASK_FLAGS_THREAD)
                {
                    // 스레드라면 프로세스의 자식 스레드 리스트에서 제거
                    pstProcess = GetProcessByThread(pstTask);
                    if (pstProcess != NULL)
                    {
                        // 프로세스 ID로 프로세스가 속한 스케줄러의 ID를 찾고 스핀락 잠금
                        if (FindSchedulerOfTaskAndLock(pstProcess->stLink.qwID,
                                                        &bProcessAPICID) == TRUE)
                        {
                            RemoveList(&(pstProcess->stChildThreadList),
                                        pstTask->stLink.qwID);
                            UnlockForSpinLock(&(gs_vstScheduler[bProcessAPICID].stSpinLock));
                        }
                    }
                }

                // 여기까지오면 태스크가 정상적으로 종료된 것이므로, 태스크 자료구조(TCB)를
                // 반환
                qwTaskID = pstTask->stLink.qwID;
                FreeTCB(qwTaskID);
                Printf("IDLE: Task ID[0x%q] is completely ended.\n",
                        qwTaskID);
            }
        }

        Schedule();
    }
}

/**
 *  측정된 프로세서 부하에 따라 프로세서를 쉬게 함
 */
void HaltProcessorByLoad(BYTE bAPICID)
{
    if (gs_vstScheduler[bAPICID].qwProcessorLoad < 40)
    {
        Hlt();
        Hlt();
        Hlt();
    }
    else if (gs_vstScheduler[bAPICID].qwProcessorLoad < 80)
    {
        Hlt();
        Hlt();
    }
    else if (gs_vstScheduler[bAPICID].qwProcessorLoad < 95)
    {
        Hlt();
    }
}

//==============================================================================
//  FPU 관련
//==============================================================================
/**
 *  마지막으로 FPU를 사용한 태스크 ID를 반환
 */
QWORD GetLastFPUUsedTaskID(BYTE bAPICID)
{
    return gs_vstScheduler[bAPICID].qwLastFPUUsedTaskID;
}

/**
 *  마지막으로 FPU를 사용한 태스크 ID를 설정
 */
void SetLastFPUUsedTaskID(BYTE bAPICID, QWORD qwTaskID)
{
    gs_vstScheduler[bAPICID].qwLastFPUUsedTaskID = qwTaskID;
}