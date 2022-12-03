#include "CacheManager.h"
#include "FileSystem.h"
#include "DynamicMemory.h"
#include "Utility.h"
#include "Console.h"

static CACHEMANAGER gs_stCacheManager;

BOOL InitializeCacheManager(void)
{
    int i;

    MemSet(&gs_stCacheManager, 0, sizeof(gs_stCacheManager));

    gs_stCacheManager.vdwAccessTime[CACHE_CLUSTERLINKTABLEAREA] = 0;
    gs_stCacheManager.vdwAccessTime[CACHE_DATAAREA] = 0;

    gs_stCacheManager.vdwMaxCount[CACHE_CLUSTERLINKTABLEAREA] = CACHE_MAXCLUSTERLINKTABLEAREACOUNT;
    gs_stCacheManager.vdwMaxCount[CACHE_DATAAREA] = CACHE_MAXDATAAREACOUNT;

    gs_stCacheManager.vpbBuffer[CACHE_CLUSTERLINKTABLEAREA] = (BYTE *)AllocateMemory(CACHE_MAXCLUSTERLINKTABLEAREACOUNT * 512);

    if(gs_stCacheManager.vpbBuffer[CACHE_CLUSTERLINKTABLEAREA] == NULL)
    {
        return FALSE;
    }

    for(i=0; i<CACHE_MAXCLUSTERLINKTABLEAREACOUNT; i++)
    {
        gs_stCacheManager.vvstCacheBuffer[CACHE_CLUSTERLINKTABLEAREA][i].pbBuffer = gs_stCacheManager.vpbBuffer[CACHE_CLUSTERLINKTABLEAREA] + (i*512);

        gs_stCacheManager.vvstCacheBuffer[CACHE_CLUSTERLINKTABLEAREA][i].dwTag = CACHE_INVALIDTAG;
    }

    gs_stCacheManager.vpbBuffer[CACHE_DATAAREA] = (BYTE *)AllocateMemory(CACHE_MAXDATAAREACOUNT * FILESYSTEM_CLUSTERSIZE);
    
    if (gs_stCacheManager.vpbBuffer[CACHE_DATAAREA] == NULL)
    {
        FreeMemory(gs_stCacheManager.vpbBuffer[CACHE_CLUSTERLINKTABLEAREA]);
        return FALSE;
    }

    for (i = 0; i < CACHE_MAXDATAAREACOUNT; i++)
    {
        gs_stCacheManager.vvstCacheBuffer[CACHE_DATAAREA][i].pbBuffer = gs_stCacheManager.vpbBuffer[CACHE_DATAAREA] + (i * FILESYSTEM_CLUSTERSIZE);

        gs_stCacheManager.vvstCacheBuffer[CACHE_DATAAREA][i].dwTag = CACHE_INVALIDTAG;
    }

    return TRUE;
}

CACHEBUFFER *AllocateCacheBuffer(int iCacheTableIndex)
{
    CACHEBUFFER *pstCacheBuffer;
    int i;

    if(iCacheTableIndex > CACHE_MAXCACHETABLEINDEX)
    {
        return FALSE;
    }

    CutDownAccessTime(iCacheTableIndex);

    pstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[iCacheTableIndex];
    
    for(i=0; i<gs_stCacheManager.vdwMaxCount[iCacheTableIndex]; i++)
    {
        if(pstCacheBuffer[i].dwTag == CACHE_INVALIDTAG)
        {
            pstCacheBuffer[i].dwTag = CACHE_INVALIDTAG - 1;

            pstCacheBuffer[i].dwAccessTime = gs_stCacheManager.vdwAccessTime[iCacheTableIndex]++;

            return &(pstCacheBuffer[i]);
        }
    }

    return NULL;
}

CACHEBUFFER *FindCacheBuffer(int iCacheTableIndex, DWORD dwTag)
{
    CACHEBUFFER *pstCacheBuffer;
    int i;

    if(iCacheTableIndex > CACHE_MAXCACHETABLEINDEX)
    {
        return FALSE;
    }

    CutDownAccessTime(iCacheTableIndex);

    pstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[iCacheTableIndex];

    for(i=0; i<gs_stCacheManager.vdwMaxCount[iCacheTableIndex]; i++)
    {
        if(pstCacheBuffer[i].dwTag == dwTag)
        {
            pstCacheBuffer[i].dwAccessTime = gs_stCacheManager.vdwAccessTime[iCacheTableIndex]++;

            return &(pstCacheBuffer[i]);
        }
    }

    return NULL;
}

static void CutDownAccessTime(int iCacheTableIndex)
{
    CACHEBUFFER stTemp;
    CACHEBUFFER *pstCacheBuffer;
    BOOL bSorted;
    int i, j;

    if(iCacheTableIndex > CACHE_MAXCACHETABLEINDEX)
    {
        return ;
    }

    if(gs_stCacheManager.vdwAccessTime[iCacheTableIndex] < 0xfffffffe)
    {
        return ;
    }

    pstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[iCacheTableIndex];

    for(j=0; j<gs_stCacheManager.vdwMaxCount[iCacheTableIndex]-1; j++)
    {
        bSorted = TRUE;

        for(i=0; i<gs_stCacheManager.vdwMaxCount[iCacheTableIndex]-1-j; i++)
        {
            if(pstCacheBuffer[i].dwAccessTime > pstCacheBuffer[i+1].dwAccessTime)
            {
                bSorted = FALSE;

                MemCpy(&stTemp, &(pstCacheBuffer[i]), sizeof(CACHEBUFFER));
                MemCpy(&(pstCacheBuffer[i]), &(pstCacheBuffer[i + 1]), sizeof(CACHEBUFFER));
                MemCpy(&(pstCacheBuffer[i + 1]), &stTemp, sizeof(CACHEBUFFER));
            }
        }

        if(bSorted == TRUE)
        {
            break;
        }
    }

    for(i=0; i<gs_stCacheManager.vdwMaxCount[iCacheTableIndex]; i++)
    {
        pstCacheBuffer[i].dwAccessTime = i;
    }

    gs_stCacheManager.vdwAccessTime[iCacheTableIndex] = i;
}

CACHEBUFFER *GetVictimInCacheBuffer(int iCacheTableIndex)
{
    DWORD dwOldTime;
    CACHEBUFFER *pstCacheBuffer;
    int iOldIndex;
    int i;

    if(iCacheTableIndex > CACHE_MAXCACHETABLEINDEX)
    {
        return FALSE;
    }

    iOldIndex = -1;
    dwOldTime = 0xFFFFFFFF;

    pstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[iCacheTableIndex];

    for(i=0; i<gs_stCacheManager.vdwMaxCount[iCacheTableIndex]; i++)
    {
        if(pstCacheBuffer[i].dwTag == CACHE_INVALIDTAG)
        {
            iOldIndex = i;
            break;
        }

        if(pstCacheBuffer[i].dwAccessTime < dwOldTime)
        {
            dwOldTime = pstCacheBuffer[i].dwAccessTime;
            iOldIndex = i;
        }
    }

    if(iOldIndex == -1)
    {
        Printf("[!] Cache Buffer Find Error.......\n");
        return NULL;
    }

    pstCacheBuffer[iOldIndex].dwAccessTime = gs_stCacheManager.vdwAccessTime[iCacheTableIndex]++;

    return &(pstCacheBuffer[iOldIndex]);
}

void DiscardAllCacheBuffer(int iCacheTableIndex)
{
    CACHEBUFFER *pstCacheBuffer;
    int i;

    pstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[iCacheTableIndex];
    for (i = 0; i < gs_stCacheManager.vdwMaxCount[iCacheTableIndex]; i++)
    {
        pstCacheBuffer[i].dwTag = CACHE_INVALIDTAG;
    }

    gs_stCacheManager.vdwAccessTime[iCacheTableIndex] = 0;
}

BOOL GetCacheBufferAndCount(int iCacheTableIndex, CACHEBUFFER **ppstCacheBuffer, int *piMaxCount)
{
    if (iCacheTableIndex > CACHE_MAXCACHETABLEINDEX)
    {
        return FALSE;
    }

    *ppstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[iCacheTableIndex];
    *piMaxCount = gs_stCacheManager.vdwMaxCount[iCacheTableIndex];
    
    return TRUE;
}