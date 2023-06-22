#include "Task.h"
#include "Descriptor.h"
#include "Utility.h"
#include "Synchronization.h"
#include "AssemblyUtility.h"
#include "Console.h"

static SCHEDULER gs_stScheduler;
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
}

/**
 * TCB를 할당하고 반환하는 함수
*/
static TCB* AllocateTCB(void)
{
    TCB *pstEmptyTCB;   // 할당해줄 빈 TCB
    int i;

    // 최대로 할당할 수 있는 개수와 현재까지 할당해준 TCB 개수가 같은 경우
    if (gs_stTCBPoolManager.iUseCount == gs_stTCBPoolManager.iMaxCount)
    {
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

    // TCB가 할당되었던 공간을 빈 공간으로 설정
    // 즉, 상위 32비트 값이 0이 되는 것
    gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID = i;

    gs_stTCBPoolManager.iUseCount--;
}

/**
 * 태스크를 생성하는 함수
*/
TCB *CreateTask(QWORD qwFlags, void *pvMemoryAddress, QWORD qwMemorySize, QWORD qwEntryPointAddress)
{
    TCB *pstTask, *pstProcess;
    void *pvStackAddress;

    // 임계 영역이므로 잠금
    LockForSpinLock(&(gs_stScheduler.stSpinLock));
    // TCB 할당
    pstTask = AllocateTCB();
    
    // 할당에 실패하면 리턴
    if (pstTask == NULL)
    {
        UnlockForSpinLock(&(gs_stScheduler.stSpinLock));
        return NULL;
    }

    // 현재 프로세스 또는 쓰레드가 속한 프로세스 검색
    pstProcess = GetProcessByThread(GetRunningTask());

    // 프로세스가 없다면 아무런 작업도 하지 않음
    if(pstProcess == NULL)
    {
        FreeTCB(pstTask->stLink.qwID);
        UnlockForSpinLock(&(gs_stScheduler.stSpinLock));
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

    UnlockForSpinLock(&(gs_stScheduler.stSpinLock));

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
    LockForSpinLock(&(gs_stScheduler.stSpinLock));
    AddTaskToReadyList(pstTask);
    UnlockForSpinLock(&(gs_stScheduler.stSpinLock));

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
    TCB *pstTask;

    // TCB풀 초기화
    InitializeTCBPool();

    // 우선순위 리스트 5개를 전부 초기화
    for(i=0; i<TASK_MAXREADYLISTCOUNT; i++)
    {
        InitializeList(&(gs_stScheduler.vstReadyList[i]));
        gs_stScheduler.viExecuteCount[i] = 0;
    }

    // 대기 큐 초기화
    InitializeList(&(gs_stScheduler.stWaitList));

    // TCB 할당받아 부팅을 수행한 태스크를 커널 최초의 프로세스로 설정
    pstTask = AllocateTCB();
    gs_stScheduler.pstRunningTask = pstTask;
    
    // 태스크 속성 설정 (가장 높은 우선순위 부여)
    // 콘솔 셸의 우선순위는 가장 높음으로 설정해줘야 다른 태스크의 영향을 받지 않음
    pstTask->qwFlags = TASK_FLAGS_HIGHEST | TASK_FLAGS_PROCESS | TASK_FLAGS_SYSTEM;
    // 부모 프로세스는 자신이므로 자신으로 설정
    pstTask->qwParentProcessID = pstTask->stLink.qwID;
    // 64비트 커널은 1MB ~ 6MB에 존재하므로 시작 주소인 1MB로 설정
    pstTask->pvMemoryAddress = (void *)0x100000;
    // 6MB까지가 IA-32e의 커널영역이므로 크기는 5MB로 설정
    pstTask->qwMemorySize = 0x500000;
    // IA-32e 커널의 스택 영역은 6MB ~ 7MB
    pstTask->pvStackAddress = (void *)0x600000;
    pstTask->qwStackSize = 0x100000;

    gs_stScheduler.qwSpendProcessorTimeInIdleTask = 0;
    gs_stScheduler.qwProcessorLoad = 0;

    // FPU 사용 태스크가 아직 없으므로 유효하지 않은 태스크로 초기화
    gs_stScheduler.qwLastFPUUsedTaskID = TASK_INVALIDID;

    InitializeSpinLock(&(gs_stScheduler.stSpinLock));
}

/**
 * 인자로 받은 태스크를 현재 수행 중인 태스크로 설정
*/
void SetRunningTask(TCB *pstTask)
{
    LockForSpinLock(&(gs_stScheduler.stSpinLock));

    gs_stScheduler.pstRunningTask = pstTask;

    UnlockForSpinLock(&(gs_stScheduler.stSpinLock));
}

/**
 * 현재 수행중인 태스크 반환
*/
TCB *GetRunningTask(void)
{
    TCB *pstRunningTask;

    LockForSpinLock(&(gs_stScheduler.stSpinLock));

    pstRunningTask = gs_stScheduler.pstRunningTask;

    UnlockForSpinLock(&(gs_stScheduler.stSpinLock));

    return pstRunningTask;
}

/**
 * 스케줄 준비 리스트에서 다음 태스크를 반환
*/
static TCB *GetNextTaskToRun(void)
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
            iTaskCount = GetListCount(&(gs_stScheduler.vstReadyList[i]));

            // 해당 우선순위 큐에서 태스크를 실행한 횟수가 전체 태스크 수보다 큰지 비교
            // 즉, 준비 큐에 있는 태스크들을 모두 한번 씩 실행했었는지 검사
            if(gs_stScheduler.viExecuteCount[i] < iTaskCount)
            {
                // 아직 전부 한번 씩 실행하지 않았다면 진행
                // 큐(리스트)에서 데이터를 꺼내오고 반환
                pstTarget = (TCB*)RemoveListFromHeader(&(gs_stScheduler.vstReadyList[i]));
                // 실행 횟수 수정
                gs_stScheduler.viExecuteCount[i]++;
                break;
            }
            // 전부 한번 씩 실행했다면 다음을 위해 실행 횟수를 0으로 초기화 
            else
            {
                gs_stScheduler.viExecuteCount[i] = 0;
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
static BOOL AddTaskToReadyList(TCB *pstTask)
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
    AddListToTail(&(gs_stScheduler.vstReadyList[bPriority]), pstTask);
    return TRUE;
}

/**
 * 준비 큐에서 태스크를 제거하고 반환하는 함수
 * 우선순위를 동적으로 변경해 주는 부분이 포함됨
*/
static TCB* RemoveTaskFromReadyList(QWORD qwTaskID)
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
    pstTarget = RemoveList(&(gs_stScheduler.vstReadyList[bPriority]),qwTaskID);
    
    return pstTarget;
}

/**
 * 우선순위를 변경하는 함수
*/
BOOL ChangePriority(QWORD qwTaskID, BYTE bPriority)
{
    TCB *pstTarget;
    
    if (bPriority > TASK_MAXREADYLISTCOUNT)
    {
        return FALSE;
    }

    LockForSpinLock(&(gs_stScheduler.stSpinLock));

    // 현재 수행중인 태스크의 우선순위를 변경하는 경우
    // 우선순위만 변경해주면 됨
    pstTarget = gs_stScheduler.pstRunningTask;
    
    // 현재 태스크가 변경하고자 하는 태스크이면 우선순위만 변경 
    if (pstTarget->stLink.qwID == qwTaskID)
    {
        SETPRIORITY(pstTarget->qwFlags, bPriority);
    }
    // 준비 리스트에 있는 태스크의 우선순위를 변경하는 경우
    else
    {
        // 먼저 해당 태스크를 해당 우선순위 큐에서 제거해야 함
        pstTarget = RemoveTaskFromReadyList(qwTaskID);
        
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
            AddTaskToReadyList(pstTarget);
        }
    }

    UnlockForSpinLock(&(gs_stScheduler.stSpinLock));
    return TRUE;
}

/**
 * 태스크 전환 함수
*/
void Schedule(void)
{
    TCB *pstRunningTask, *pstNextTask;
    BOOL bPreviousInterrupt;

    // 전환할 태스크가 있는지 검색
    if(GetReadyTaskCount() < 1)
    {
        return ;
    }

    // 인터럽트 비활성화
    bPreviousInterrupt = SetInterruptFlag(FALSE);
    LockForSpinLock(&(gs_stScheduler.stSpinLock));

    // 전환할 다음 태스크 검색
    pstNextTask = GetNextTaskToRun();
    if (pstNextTask == NULL)
    {
        UnlockForSpinLock(&(gs_stScheduler.stSpinLock));
        SetInterruptFlag(bPreviousInterrupt);
        return;
    }

    // 현재 수행중인 태스크를 다음 태스크로 수정
    pstRunningTask = gs_stScheduler.pstRunningTask;
    gs_stScheduler.pstRunningTask = pstNextTask;

    // 유휴 태스크에서 전환된 것이라면 사용한 프로세서 시간을 증가시킴
    if ((pstRunningTask->qwFlags & TASK_FLAGS_IDLE) == TASK_FLAGS_IDLE)
    {
        gs_stScheduler.qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME - gs_stScheduler.iProcessorTime;
    }

    // 다음에 수행할 태스크가 FPU를 마지막으로 사용하지 않았다면
    if (gs_stScheduler.qwLastFPUUsedTaskID != pstNextTask->stLink.qwID)
    {
        // 태스크가 전환되었으니 CR0의 TS 비트를 1로 설정
        SetTS();
    }
    else
    {
        // 사용한 경우 바로 FPU 컨텍스트에서 복원해서 사용하면 되므로 0으로 설정
        ClearTS();
    }

    // 태스크가 종료 태스크인 경우
    if (pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK)
    {
        // 대기 큐에 현재 태스크를 삽입
        AddListToTail(&(gs_stScheduler.stWaitList), pstRunningTask);
        UnlockForSpinLock(&(gs_stScheduler.stSpinLock));
        // 컨텍스트는 저장할 필요가 없으므로 콘텍스트 전환
        SwitchContext(NULL, &(pstNextTask->stContext));
    }
    // 종료 태스크가 아닌 경우
    else
    {
        // 전환되었으니 조금 전까지 실행된 태스크는 준비 큐에 삽입되어야 함
        AddTaskToReadyList(pstRunningTask);
        UnlockForSpinLock(&(gs_stScheduler.stSpinLock));
        // 삽입 후 컨텍스트 전환
        SwitchContext(&(pstRunningTask->stContext), &(pstNextTask->stContext));
    }
    
    // 프로세서 사용 시간 업데이트
    gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;

    SetInterruptFlag(bPreviousInterrupt);
}

/**
 * 인터럽트에 의해 태스크가 전환되는 경우
*/
BOOL ScheduleInInterrupt(void)
{
    TCB *pstRunningTask, *pstNextTask;
    char *pcContextAddress; // IST 주소

    // 인터럽트 제어
    LockForSpinLock(&(gs_stScheduler.stSpinLock));

    // 전환할 다음 태스크를 찾음
    pstNextTask = GetNextTaskToRun();
    if (pstNextTask == NULL)
    {
        UnlockForSpinLock(&(gs_stScheduler.stSpinLock));
        return FALSE;
    }

    // IST의 주소를 계산
    pcContextAddress = (char *)IST_STARTADDRESS + IST_SIZE - sizeof(CONTEXT);

    // 현재 수행중인 태스크를 전환할 태스크로 수정
    pstRunningTask = gs_stScheduler.pstRunningTask;
    gs_stScheduler.pstRunningTask = pstNextTask;

    // 유휴 태스크에서 전환된 경우 프로세스 사용 시간 증가
    if ((pstRunningTask->qwFlags & TASK_FLAGS_IDLE) == TASK_FLAGS_IDLE)
    {
        gs_stScheduler.qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME;
    }

    // 조금 전까지 수행된 태스크가 종료 태스크로 설정된 경우
    if (pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK)
    {
        // 대기 큐에 삽입
        AddListToTail(&(gs_stScheduler.stWaitList), pstRunningTask);
    }
    // 아닌 경우
    else
    {
        // IST에 있던 자신의 컨텍스트를 저장
        MemCpy(&(pstRunningTask->stContext), pcContextAddress, sizeof(CONTEXT));
        // 우선순위에 맞는 준비 리스트에 삽입
        AddTaskToReadyList(pstRunningTask);
    }

    UnlockForSpinLock(&(gs_stScheduler.stSpinLock));

    // 다음에 수행할 태스크가 FPU를 마지막으로 사용하지 않았다면
    if (gs_stScheduler.qwLastFPUUsedTaskID != pstNextTask->stLink.qwID)
    {
        // 태스크가 전환되었으니 CR0의 TS 비트를 1로 설정
        SetTS();
    }
    else
    {
        // 사용한 경우 바로 FPU 컨텍스트에서 복원해서 사용하면 되므로 0으로 설정
        ClearTS();
    }

    // IST에 전환할 태스크의 컨텍스트를 저장
    MemCpy(pcContextAddress, &(pstNextTask->stContext), sizeof(CONTEXT));

    // 프로세서 사용 시간 업데이트
    gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
    
    return TRUE;
}

/**
 * 태스크의 CPU 사용 시간을 하나 줄임
*/
void DecreaseProcessorTime(void)
{
    if (gs_stScheduler.iProcessorTime > 0)
    {
        gs_stScheduler.iProcessorTime--;
    }
}

/**
 * 태스크의 CPU 사용 시간이 다 지났는지 확인
*/
BOOL IsProcessorTimeExpired(void)
{
    if (gs_stScheduler.iProcessorTime <= 0)
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

    LockForSpinLock(&(gs_stScheduler.stSpinLock));

    // 현재 실행중인 태스크를 종료하는 경우 (자신을 종료하는 경우)
    pstTarget = gs_stScheduler.pstRunningTask;
    if (pstTarget->stLink.qwID == qwTaskID)
    {
        // 속성을 종료 태스크로 설정
        pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
        // 우선순위도 대기 큐에 있도록 설정
        SETPRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);

        UnlockForSpinLock(&(gs_stScheduler.stSpinLock));

        // 다른 태스크로 전환
        Schedule();

        // 태스크가 전환되었으므로 아래 코드는 절대 실행되지 않음
        while (1);
    }
    // 준비 큐에 있는 태스크를 종료하는 경우
    else
    {
        // 리스트에서 해당 태스크 제거 후 반환
        pstTarget = RemoveTaskFromReadyList(qwTaskID);
        
        // 못찾으면 TCB 풀에서 검색 후 종료 태스크로 설정
        if (pstTarget == NULL)
        {
            pstTarget = GetTCBInTCBPool(GETTCBOFFSET(qwTaskID));
            if (pstTarget != NULL)
            {
                pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
                SETPRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);
            }

            UnlockForSpinLock(&(gs_stScheduler.stSpinLock));
            return TRUE;
        }

        // 종료 태스크로 설정 및 대기 큐로 삽입
        pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
        SETPRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);
        AddListToTail(&(gs_stScheduler.stWaitList), pstTarget);
    }

    UnlockForSpinLock(&(gs_stScheduler.stSpinLock));
    return TRUE;
}

/**
 * 태스크 자신을 종료하는 함수
*/
void ExitTask(void)
{
    EndTask(gs_stScheduler.pstRunningTask->stLink.qwID);
}

/**
 * 준비 큐에 있는 모든 태스크의 개수 계산
*/
int GetReadyTaskCount(void)
{
    int iTotalCount = 0;
    int i;

    LockForSpinLock(&(gs_stScheduler.stSpinLock));

    for (i = 0; i < TASK_MAXREADYLISTCOUNT; i++)
    {
        iTotalCount += GetListCount(&(gs_stScheduler.vstReadyList[i]));
    }

    UnlockForSpinLock(&(gs_stScheduler.stSpinLock));
    return iTotalCount;
}

/**
 * 전체 태스크의 수를 반환
 * 준비 큐 + 대기 큐
*/
int GetTaskCount(void)
{
    int iTotalCount;

    iTotalCount = GetReadyTaskCount();

    LockForSpinLock(&(gs_stScheduler.stSpinLock));

    iTotalCount += GetListCount(&(gs_stScheduler.stWaitList)) + 1;

    UnlockForSpinLock(&(gs_stScheduler.stSpinLock));
    return iTotalCount;
}

/**
 * TCB풀에서 TCB를 검색하여 반환
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
 * 태스크가 TCB에 존재하는지 검사
*/
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

/**
 * 프로세서의 부하를 반환
*/
QWORD GetProcessorLoad(void)
{
    return gs_stScheduler.qwProcessorLoad;
}

/**
 * 쓰레드의 부모 프로세스를 반환
*/
static TCB *GetProcessByThread(TCB *pstThread)
{
    TCB *pstProcess;

    // 내가 프로세스이면 자신을 반환
    if (pstThread->qwFlags & TASK_FLAGS_PROCESS)
    {
        return pstThread;
    }

    // 아니라면 나의 부모 프로세스를 TCB풀에서 검색
    pstProcess = GetTCBInTCBPool(GETTCBOFFSET(pstThread->qwParentProcessID));

    // 부모 프로세스가 없거나 내 부모 프로세스 ID와 찾은 프로세스의 ID가 일치하지 않으면 종료
    if ((pstProcess == NULL) || (pstProcess->stLink.qwID != pstThread->qwParentProcessID))
    {
        return NULL;
    }

    // 부모 프로세스가 맞다면 반환
    return pstProcess;
}

/**
 * 유휴 태스크
 * 대기 큐에 있는 태스크를 종료시키거나
 * 프로세서의 사용률을 계산하거나
 * 프로세서를 쉬게 할 수 있음 -> hlt
 * 프로세스 종료 시 자식 쓰레드가 모두 종료된 후 종료될 수 있도록 해야 함
*/
void IdleTask(void)
{
    TCB *pstTask, *pstChildThread, *pstProcess;
    QWORD qwLastMeasureTickCount, qwLastSpendTickInIdleTask;
    QWORD qwCurrentMeasureTickCount, qwCurrentSpendTickInIdleTask;
    
    QWORD qwTaskID;

    int i, iCount;
    void *pstThreadLink;

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

        // 대기 큐에 대기중인 태스크가 있으면 태스크를 종료시킴
        if (GetListCount(&(gs_stScheduler.stWaitList)) >= 0)
        {
            while (1)
            {
                // 임계 영역이므로 설정
                LockForSpinLock(&(gs_stScheduler.stSpinLock));

                // 대기 큐에서 태스크를 가져와서 제거
                pstTask = RemoveListFromHeader(&(gs_stScheduler.stWaitList));
                if (pstTask == NULL)
                {
                    UnlockForSpinLock(&(gs_stScheduler.stSpinLock));
                    break;
                }

                // 해당 태스크가 프로세서라면 자식 쓰레드를 모두 종료시킨 후 종료
                if(pstTask->qwFlags & TASK_FLAGS_PROCESS)
                {
                    // 자식 쓰레드의 개수를 파악하여 모두 종료시켜야 함
                    iCount = GetListCount(&(pstTask->stChildThreadList));
                    
                    // 모든 자식 쓰레드를 종료
                    for(i=0; i<iCount; i++)
                    {
                        // 자식 쓰레드 리스트에서 자식 쓰레드를 꺼냄
                        pstThreadLink = (TCB*)RemoveListFromHeader(&(pstTask->stChildThreadList));
                        
                        if(pstThreadLink == NULL)
                        {
                            break;
                        }

                        // 자식 쓰레드의 TCB 위치를 계산
                        pstChildThread = GETTCBFROMTHREADLINK(pstThreadLink);

                        // 자식 쓰레드를 프로세스의 자식 쓰레드 리스트에 넣어서 스스로 리스트에서 종료되도록 함
                        AddListToTail(&(pstTask->stChildThreadList), &(pstChildThread->stThreadLink));

                        // 자식 쓰레드를 종료(TCB를 종료한다는 것)
                        EndTask(pstChildThread->stLink.qwID);
                    }

                    // 아직 자식 쓰레드가 남아있다면 모두 종료될 때 까지 기다려야하므로 대기 큐에 넣어서 종료시킴
                    if (GetListCount(&(pstTask->stChildThreadList)) > 0)
                    {
                        AddListToTail(&(gs_stScheduler.stWaitList), pstTask);

                        UnlockForSpinLock(&(gs_stScheduler.stSpinLock));
                        continue;
                    }
                    else
                    {
                        // 추후에 코드 추가
                    }
                }
                // 프로세스가 아닌 쓰레드라면
                else if (pstTask->qwFlags & TASK_FLAGS_THREAD)
                {
                    // 부모  프로세스를 찾아서 프로세스의 자식 리스트에서 제거
                    pstProcess = GetProcessByThread(pstTask);
                    if (pstProcess != NULL)
                    {
                        RemoveList(&(pstProcess->stChildThreadList), pstTask->stLink.qwID);
                    }
                }

                // 리스트에서 제거 후 TCB에서도 제거
                qwTaskID = pstTask->stLink.qwID;
                FreeTCB(pstTask->stLink.qwID);

                UnlockForSpinLock(&(gs_stScheduler.stSpinLock));

                Printf("[*] IDLE: Task ID[0x%q] is completely ended.\n", qwTaskID);
                
            }
        }

        Schedule();
    }
}

/**
 * 프로세서의 부하에 따라 프로세서를 대기 상태로 변경
*/
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

/**
 * 마지막으로 FPU 레지스터를 사용한 태스크를 반환
*/
QWORD GetLastFPUUsedTaskID(void)
{
    return gs_stScheduler.qwLastFPUUsedTaskID;
}

/**
 * 마지막으로 FPU 레지스터를 사용한 태스크로 설정
*/
void SetLastFPUUsedTaskID(QWORD qwTaskID)
{
    gs_stScheduler.qwLastFPUUsedTaskID = qwTaskID;
}