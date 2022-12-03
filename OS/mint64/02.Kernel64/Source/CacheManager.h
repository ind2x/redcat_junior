#ifndef __CACHEMANAGER_H__
#define __CACHEMANAGER_H__

#include "Types.h"

#define CACHE_MAXCLUSTERLINKTABLEAREACOUNT 16
#define CACHE_MAXDATAAREACOUNT 32
#define CACHE_INVALIDTAG 0xFFFFFFFF
#define CACHE_MAXCACHETABLEINDEX 2
#define CACHE_CLUSTERLINKTABLEAREA 0
#define CACHE_DATAAREA 1

typedef struct CacheBufferStruct
{
    DWORD dwTag;
    DWORD dwAccessTime;
    BOOL bChanged;
    BYTE *pbBuffer;
} CACHEBUFFER;

typedef struct CacheManagerStruct
{
    DWORD vdwAccessTime[CACHE_MAXCACHETABLEINDEX];

    BYTE *vpbBuffer[CACHE_MAXCACHETABLEINDEX];

    CACHEBUFFER vvstCacheBuffer[CACHE_MAXCACHETABLEINDEX][CACHE_MAXDATAAREACOUNT];

    DWORD vdwMaxCount[CACHE_MAXCACHETABLEINDEX];
} CACHEMANAGER;

BOOL InitializeCacheManager(void);
CACHEBUFFER *AllocateCacheBuffer(int iCacheTableIndex);
CACHEBUFFER *FindCacheBuffer(int iCacheTableIndex, DWORD dwTag);
CACHEBUFFER *GetVictimInCacheBuffer(int iCacheTableIndex);
void DiscardAllCacheBuffer(int iCacheTableIndex);
BOOL GetCacheBufferAndCount(int iCacheTableIndex, CACHEBUFFER **ppstCacheBuffer, int *piMaxCount);

static void CutDownAccessTime(int iCacheTableIndex);

#endif /*__CACHEMANAGER_H__*/