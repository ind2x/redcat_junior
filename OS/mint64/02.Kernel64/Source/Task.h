#ifndef __TASK_H__
#define __TASK_H__

#include "Types.h"
#include "List.h"
#include "Synchronization.h"

#define TASK_REGISTERCOUNT (5+19)
#define TASK_REGISTERSIZE   8

// 컨텍스트 자료구조 레지스터 오프셋(저장되는 순서 또는 위치)
#define TASK_GSOFFSET 0
#define TASK_FSOFFSET 1
#define TASK_ESOFFSET 2
#define TASK_DSOFFSET 3
#define TASK_R15OFFSET 4
#define TASK_R14OFFSET 5
#define TASK_R13OFFSET 6
#define TASK_R12OFFSET 7
#define TASK_R11OFFSET 8
#define TASK_R10OFFSET 9
#define TASK_R9OFFSET 10
#define TASK_R8OFFSET 11
#define TASK_RSIOFFSET 12
#define TASK_RDIOFFSET 13
#define TASK_RDXOFFSET 14
#define TASK_RCXOFFSET 15
#define TASK_RBXOFFSET 16
#define TASK_RAXOFFSET 17
#define TASK_RBPOFFSET 18
#define TASK_RIPOFFSET 19
#define TASK_CSOFFSET 20
#define TASK_RFLAGSOFFSET 21
#define TASK_RSPOFFSET 22
#define TASK_SSOFFSET 23

// TCBPOOL 주소와 생성 가능한 TCB 최대 개수
#define TASK_TCBPOOLADDRESS 0x800000 // 8MB
#define TASK_MAXCOUNT 1024

// 스택풀 주소와 최대 개수
#define TASK_STACKPOOLADDRESS (TASK_TCBPOOLADDRESS + sizeof(TCB) * TASK_MAXCOUNT) // TCB풀 다음에 위치
#define TASK_STACKSIZE 8192

// 유효하지 않는 태스크
#define TASK_INVALIDID 0xFFFFFFFFFFFFFFFF

// 태스크가 CPU를 사용할 수 있는 시간 = 5ms
#define TASK_PROCESSORTIME 5

// 준비 리스트의 수 5개 -> 가장 높음, 높음, 중간, 낮음, 가장 낮음
#define TASK_MAXREADYLISTCOUNT 5

// 태스크의 우선순위
#define TASK_FLAGS_HIGHEST 0
#define TASK_FLAGS_HIGH 1
#define TASK_FLAGS_MEDIUM 2
#define TASK_FLAGS_LOW 3
#define TASK_FLAGS_LOWEST 4

// 대기 큐에 있음을 알려주는 값
#define TASK_FLAGS_WAIT 0xFF

// 태스크의 플래그
// 종료 태스크
#define TASK_FLAGS_ENDTASK 0x8000000000000000
// 유휴 태스크
#define TASK_FLAGS_IDLE 0x0800000000000000

// 프로세스와 쓰레드와 관련된 플래그
// 시스템에서 자체적으로 생성한 프로세스나 쓰레드
#define TASK_FLAGS_SYSTEM 0x4000000000000000
// 프로세스임을 나타내는 플래그
#define TASK_FLAGS_PROCESS 0x2000000000000000
// 쓰레드임을 나타내는 플래그
#define TASK_FLAGS_THREAD 0x1000000000000000


// 우선순위를 가져옴
#define GETPRIORITY(x) ((x)&0xFF)
// 우선순위를 설정함
#define SETPRIORITY(x, priority) ((x) = ((x)&0xFFFFFFFFFFFFFF00) | (priority))
// TCB의 오프셋(위치)를 가져옴
#define GETTCBOFFSET(x) ((x)&0xFFFFFFFF)

// 쓰레드의 TCB를 가져옴
#define GETTCBFROMTHREADLINK(x) (TCB *)((QWORD)(x)-offsetof(TCB, stThreadLink))

#define TASK_LOADBALANCINGID 0xFF

//========================================================
// 자료구조
//========================================================

#pragma pack(push, 1)

// 컨텍스트 자료구조
typedef struct ContextStruct
{
    // 프로세서의 레지스터가 저장될 공간
    QWORD vqRegister[TASK_REGISTERCOUNT];
} CONTEXT;

// TCB 자료구조
// 그니까 태스크는 프로세스와 쓰레드와 같은 말임
// 물론 프로세스와 쓰레드는 다른 말
typedef struct TaskControlBlockStruct
{
    // 다음 TCB의 위치를 나타내기 위한 변수
    LISTLINK stLink;

    // 태스크의 속성을 나타내는 플래그 필드
    // 하위 8비트는 우선순위 값으로 사용함
    QWORD qwFlags;

    // 프로세스와 쓰레드를 동시에 지원하는데 필요한 필드
    // 프로세서는 메모리 영역의 시작주소와 크기, 실행중인 쓰레드 정보가 필요함 
    void *pvMemoryAddress;  // 영역
    QWORD qwMemorySize;     // 크기
    
    // 쓰레드 링크는 쓰레드로 생성되었을 때,
    // 프로세서의 자식 쓰레드 리스트에 연결하는 용도로 사용
    LISTLINK stThreadLink;  // 실행 중인 쓰레드 정보
    // 부모 프로세스 ID
    QWORD qwParentProcessID;
    
    // FPU 콘텍스트는 16의 배수로 정렬되어야 함.
    // 시작 어드레스가 16바이트로 정렬되기 위해선 이 위치에 있어야 함
    // 따라서 앞으로 추가할 데이터는 현재 라인 아래에 추가해야 함
    QWORD vqwFPUContext[512 / 8];
    
    // 프로세스만 사용, 프로세스나 자식 쓰레드가 쓰레드를 생성 시 리스트에 추가
    LIST stChildThreadList;

    // 프로세서(태스크)의 상태를 저장하는 컨텍스트
    CONTEXT stContext;

    // 프로세스(태스크)는 개별적인 스택을 가짐 (스택에 데이터가 저장되므로 곂치면 X)
    // 스택의 주소와 크기
    void *pvStackAddress;
    QWORD qwStackSize;
    
    // FPU 레지스터 사용 여부
    BOOL bFPUUsed;

    BYTE bAffinity;

    BYTE bAPICID;

    // FPU 컨텍스트로 인한 TCB 전체를 16바이트 배수로 맞추기 위한 패딩
    char vcPadding[9];
} TCB;

// TCB풀 관리용 자료구조
typedef struct TCBPoolManagerStruct
{
    SPINLOCK stSpinLock;

    // TCB 풀의 시작 주소
    TCB *pstStartAddress;
    // TCB 최대 개수
    int iMaxCount;
    // 사용한 TCB 개수
    int iUseCount;

    // TCB가 할당된 개수
    int iAllocatedCount;
} TCBPOOLMANAGER;

// 스케줄러 자료구조
typedef struct SchedulerStruct
{
    SPINLOCK stSpinLock;
    
    // 현재 실행중인 태스크
    TCB *pstRunningTask;

    // 태스크의 CPU 사용 가능 시간 
    int iProcessorTime;

    // 우선순위 5개에 따른 5개의 준비 리스트
    LIST vstReadyList[TASK_MAXREADYLISTCOUNT];
    
    // 태스크 종료 시 사용할 대기 큐 
    LIST stWaitList;

    // 각 우선순위 별로 태스크를 실행한 횟수를 저장
    int viExecuteCount[TASK_MAXREADYLISTCOUNT];

    // 프로세서의 부하를 계산하기 위한 자료구조
    QWORD qwProcessorLoad;
    // 유휴 태스크에서 사용한 프로세서 시간
    QWORD qwSpendProcessorTimeInIdleTask;

    // 마지막으로 FPU를 사용한 태스크 ID
    QWORD qwLastFPUUsedTaskID;

    BOOL bUseLoadBalancing;

} SCHEDULER;

#pragma pack(pop)

// TCB, TCB풀 함수
static void InitializeTCBPool(void);
static TCB *AllocateTCB(void);
static void FreeTCB(QWORD qwID);
TCB *CreateTask(QWORD qwFlags, void *pvMemoryAddress, QWORD qwMemorySize, QWORD qwEntryPointAddress, BYTE bAffinity);
static void SetUpTask(TCB *pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress, void *pvStackAddress, QWORD qwStackSize);

// 스케줄러 함수
void InitializeScheduler(void);
void SetRunningTask(BYTE bAPICID, TCB *pstTask);
TCB *GetRunningTask(BYTE bAPICID);
static TCB *GetNextTaskToRun(BYTE bAPICID);
static BOOL AddTaskToReadyList(BYTE bAPICID, TCB *pstTask);
BOOL Schedule(void);
BOOL ScheduleInInterrupt(void);
void DecreaseProcessorTime(BYTE bAPICID);
BOOL IsProcessorTimeExpired(BYTE bAPICID);

static TCB *RemoveTaskFromReadyList(BYTE bAPICID, QWORD qwTaskID);
static BOOL FindSchedulerOfTaskAndLock(QWORD qwTaskID, BYTE* pbAPICID);
BOOL ChangePriority(QWORD qwID, BYTE bPriority);
BOOL EndTask(QWORD qwTaskID);
void ExitTask(void);
int GetReadyTaskCount(BYTE bAPICID);
int GetTaskCount(BYTE bAPICID);
TCB* GetTCBInTCBPool(int iOffset);
BOOL IsTaskExist(QWORD qwID);
QWORD GetProcessorLoad(BYTE bAPICID);

static TCB *GetProcessByThread(TCB *pstThread);
void AddTaskToSchedulerWithLoadBalancing(TCB* pstTask);
static BYTE FindSchedulerOfMinumumTaskCount(const TCB* pstTask);
void SetTaskLoadBalancing(BYTE bAPICID, BOOL bUseLoadBalancing);
BOOL ChangeProcessorAffinity(QWORD qwTaskID, BYTE bAffinity);

void IdleTask(void);
void HaltProcessorByLoad(BYTE bAPICID);

QWORD GetLastFPUUsedTaskID(BYTE bAPICID);
void SetLastFPUUsedTaskID(BYTE bAPICID, QWORD qwTaskID);

#endif /*__TASK_H__*/