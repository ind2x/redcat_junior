#ifndef __TASK_H__
#define __TASK_H__

#include "Types.h"
#include "List.h"

#define TASK_REGISTERCOUNT (5+19)
#define TASK_REGISTERSIZE   8

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

#define TASK_TCBPOOLADDRESS 0x800000
#define TASK_MAXCOUNT 1024

#define TASK_STACKPOOLADDRESS (TASK_TCBPOOLADDRESS + sizeof(TCB) * TASK_MAXCOUNT)
#define TASK_STACKSIZE 8192

#define TASK_INVALIDID 0xFFFFFFFFFFFFFFFF

#define TASK_PROCESSORTIME 5

#define TASK_MAXREADYLISTCOUNT 5

#define TASK_FLAGS_HIGHEST 0
#define TASK_FLAGS_HIGH 1
#define TASK_FLAGS_MEDIUM 2
#define TASK_FLAGS_LOW 3
#define TASK_FLAGS_LOWEST 4
#define TASK_FLAGS_WAIT 0xFF

#define TASK_FLAGS_ENDTASK 0x8000000000000000
#define TASK_FLAGS_IDLE 0x0800000000000000

#define GETPRIORITY(x) ((x)&0xFF)
#define SETPRIORITY(x, priority) ((x) = ((x)&0xFFFFFFFFFFFFFF00) | \
                                        (priority))
#define GETTCBOFFSET(x) ((x)&0xFFFFFFFF)

#pragma pack(push, 1)

typedef struct ContextStruct
{
    QWORD vqRegister[TASK_REGISTERCOUNT];
} CONTEXT;

typedef struct TaskControlBlockStruct
{
    LISTLINK stLink;

    CONTEXT stContext;

    QWORD qwFlags;

    void *pvStackAddress;
    QWORD qwStackSize;
} TCB;

typedef struct TCBPoolManagerStruct
{
    TCB *pstStartAddress;
    int iMaxCount;
    int iUseCount;

    int iAllocatedCount;
} TCBPOOLMANAGER;

typedef struct SchedulerStruct
{
    TCB *pstRunningTask;

    int iProcessorTime;

    LIST vstReadyList[TASK_MAXREADYLISTCOUNT];
    
    LIST stWaitList;

    int viExecuteCount[TASK_MAXREADYLISTCOUNT];

    QWORD qwProcessorLoad;

    QWORD qwSpendProcessorTimeInIdleTask;
} SCHEDULER;

#pragma pack(pop)

static void InitializeTCBPool(void);
static TCB *AllocateTCB(void);
static void FreeTCB(QWORD qwID);
TCB* CreateTask(QWORD qwFlags, QWORD qwEntryPointAddress);
static void SetUpTask(TCB *pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress, void *pvStackAddress, QWORD qwStackSize);

void InitializeScheduler(void);
void SetRunningTask(TCB *pstTask);
TCB *GetRunningTask(void);
static TCB *GetNextTaskToRun(void);
static BOOL AddTaskToReadyList(TCB *pstTask);
void Schedule(void);
BOOL ScheduleInInterrupt(void);
void DecreaseProcessorTime(void);
BOOL IsProcessorTimeExpired(void);

static TCB* RemoveTaskFromReadyList(QWORD qwTaskID);
BOOL ChangePriority(QWORD qwID, BYTE bPriority);
BOOL EndTask(QWORD qwTaskID);
void ExitTask(void);
int GetReadyTaskCount(void);
int GetTaskCount(void);
TCB* GetTCBInTCBPool(int iOffset);
BOOL IsTaskExist(QWORD qwID);
QWORD GetProcessorLoad(void);

void IdleTask(void);
void HaltProcessorByLoad(void);

#endif /*__TASK_H__*/