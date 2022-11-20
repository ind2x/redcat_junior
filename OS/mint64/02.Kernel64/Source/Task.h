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

    LIST stReadyList;
} SCHEDULER;

#pragma pack(pop)

void InitializeTCBPool(void);
TCB *AllocateTCB(void);
void FreeTCB(QWORD qwID);
TCB* CreateTask(QWORD qwFlags, QWORD qwEntryPointAddress);
void SetUpTask(TCB *pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress, void *pvStackAddress, QWORD qwStackSize);

void InitializeScheduler(void);
void SetRunningTask(TCB *pstTask);
TCB *GetRunningTask(void);
TCB *GetNextTaskToRun(void);
void AddTaskToReadyList(TCB *pstTask);
void Schedule(void);
BOOL ScheduleInInterrupt(void);
void DecreaseProcessorTime(void);
BOOL IsProcessorTimeExpired(void);

#endif /*__TASK_H__*/