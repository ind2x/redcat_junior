#ifndef __DYNAMICMEMORY_H__
#define __DYNAMICMEMORY_H__

#include "Types.h"
#include "Task.h"
#include "Synchronization.h"

// 동적메모리 관리 영역은 17M ~ 
#define DYNAMICMEMORY_START_ADDRESS ((TASK_STACKPOOLADDRESS + (TASK_STACKSIZE * TASK_MAXCOUNT) + 0xfffff) & 0xfffffffffff00000)

#define DYNAMICMEMORY_MIN_SIZE (1*1024)

#define DYNAMICMEMORY_EXIST 0x01
#define DYNAMICMEMORY_EMPTY 0x00

typedef struct BitmapStruct
{
    BYTE *pbBitmap;
    QWORD qwExistBitCount;
} BITMAP;

typedef struct DynamicMemoryManagerStruct
{
    SPINLOCK stSpinLock;
    
    int iMaxLevelCount;
    int iBlockCountOfSmallestBlock;
    QWORD qwUsedSize;

    QWORD qwStartAddress;
    QWORD qwEndAddress;

    BYTE *pbAllocatedBlockListIndex;
    
    BITMAP *pstBitmapOfLevel;
} DYNAMICMEMORY;




void InitializeDynamicMemory(void);
void *AllocateMemory(QWORD qwSize);
BOOL FreeMemory(void *pvAddress);
void GetDynamicMemoryInformation(QWORD *pqwDynamicMemoryStartAddress, QWORD *pqwDynamicMemoryTotalSize, QWORD *pqwMetaDataSize, QWORD *pqwUsedMemorySize);

DYNAMICMEMORY *GetDynamicMemoryManager(void);

static QWORD CalculateDynamicMemorySize(void);
static int CalculateMetaBlockCount(QWORD qwDynamicRAMSize);
static int AllocationBuddyBlock(QWORD qwAlignedSize);
static QWORD GetBuddyBlockSize(QWORD qwSize);
static int GetBlockListIndexOfMatchSize(QWORD qwAlignedSize);
static int FindFreeBlockInBitmap(int iBlockListIndex);
static void SetFlagInBitmap(int iBlockListIndex, int iOffset, BYTE bFlag);
static BOOL FreeBuddyBlock(int iBlockListIndex, int iBlockOffset);
static BYTE GetFlagInBitmap(int iBlockListIndex, int iOffset);

#endif /*__DYNAMICMEMORY_H__*/