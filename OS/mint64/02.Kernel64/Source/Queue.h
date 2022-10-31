#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "Types.h"

#pragma pack(push, 1)

typedef struct kQueueManagerStruct
{
    int iDataSize;
    int iMaxDataCount;

    void *pvQueueArray;
    int iPutIndex;
    int iGetIndex;

    BOOL bLastOperationPut;
} QUEUE;

#pragma pack(pop)

void InitializeQueue(QUEUE *pstQueue, void *pvQueueBuffer, int iMaxDataCount,
                      int iDataSize);
BOOL IsQueueFull(const QUEUE *pstQueue);
BOOL IsQueueEmpty(const QUEUE *pstQueue);
BOOL PutQueue(QUEUE *pstQueue, const void *pvData);
BOOL GetQueue(QUEUE *pstQueue, void *pvData);

#endif /*__QUEUE_H__*/