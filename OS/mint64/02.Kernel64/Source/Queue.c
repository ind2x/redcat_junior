#include "Queue.h"
#include "Utility.h"

void InitializeQueue(QUEUE *pstQueue, void *pvQueueBuffer, int iMaxDataCount,
                     int iDataSize)
{

    pstQueue->iMaxDataCount = iMaxDataCount;
    pstQueue->iDataSize = iDataSize;
    pstQueue->pvQueueArray = pvQueueBuffer;

    pstQueue->iPutIndex = 0;
    pstQueue->iGetIndex = 0;
    pstQueue->bLastOperationPut = FALSE;
}

BOOL IsQueueFull(const QUEUE *pstQueue)
{
    if ((pstQueue->iGetIndex == pstQueue->iPutIndex) &&
        (pstQueue->bLastOperationPut == TRUE))
    {
        return TRUE;
    }
    return FALSE;
}

BOOL IsQueueEmpty(const QUEUE *pstQueue)
{
    if ((pstQueue->iGetIndex == pstQueue->iPutIndex) &&
        (pstQueue->bLastOperationPut == FALSE))
    {
        return TRUE;
    }
    return FALSE;
}

BOOL PutQueue(QUEUE *pstQueue, const void *pvData)
{
    if (IsQueueFull(pstQueue) == TRUE)
    {
        return FALSE;
    }

    MemCpy((char *)pstQueue->pvQueueArray + (pstQueue->iDataSize * pstQueue->iPutIndex), pvData, pstQueue->iDataSize);

    pstQueue->iPutIndex = (pstQueue->iPutIndex + 1) % pstQueue->iMaxDataCount;
    pstQueue->bLastOperationPut = TRUE;
    return TRUE;
}

BOOL GetQueue(QUEUE *pstQueue, void *pvData)
{
    if (IsQueueEmpty(pstQueue) == TRUE)
    {
        return FALSE;
    }

    MemCpy(pvData, (char *)pstQueue->pvQueueArray + (pstQueue->iDataSize * pstQueue->iGetIndex), pstQueue->iDataSize);

    pstQueue->iGetIndex = (pstQueue->iGetIndex + 1) % pstQueue->iMaxDataCount;
    pstQueue->bLastOperationPut = FALSE;
    return TRUE;
}