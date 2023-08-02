#ifndef __CACHEMANAGER_H__
#define __CACHEMANAGER_H__

#include "Types.h"

// 클러스터 링크 테이블 영역의 최대 캐시 버퍼의 개수
#define CACHE_MAXCLUSTERLINKTABLEAREACOUNT 16
// 데이터 영역의 최대 캐시 버퍼의 개수
#define CACHE_MAXDATAAREACOUNT 32
// 비어있는 캐시 버퍼를 나타냄
#define CACHE_INVALIDTAG 0xFFFFFFFF
// 캐시 테이블의 최대 개수 (클러스티 링크 영역와 데이터 영역 2개를 뜻함)
#define CACHE_MAXCACHETABLEINDEX 2
#define CACHE_CLUSTERLINKTABLEAREA 0
#define CACHE_DATAAREA 1

// 캐시 버퍼 자료구조
typedef struct CacheBufferStruct
{
    // 클러스터 링크 영역인지 데이터 영역인지 구분
    DWORD dwTag;
    // 마지막으로 접근한 시간
    DWORD dwAccessTime;
    // 데이터 변경 여부
    BOOL bChanged;
    // 캐시 데이터를 저장할 공간
    BYTE *pbBuffer;
} CACHEBUFFER;

// 파일 시스템 캐시 자료구조
typedef struct CacheManagerStruct
{
    // 클러스터 링크 영역돠 데이터 영역의 접근 시간 필드
    DWORD vdwAccessTime[CACHE_MAXCACHETABLEINDEX];

    // 클러스터 링크, 데이터 영역의 데이터 버퍼
    BYTE *vpbBuffer[CACHE_MAXCACHETABLEINDEX];

    // 클러스터 링크, 데이터 영역의 캐시 버퍼 (둘 중 큰 값만큼 생성해야 함)
    CACHEBUFFER vvstCacheBuffer[CACHE_MAXCACHETABLEINDEX][CACHE_MAXDATAAREACOUNT];

    // 캐시 버퍼 최대값
    DWORD vdwMaxCount[CACHE_MAXCACHETABLEINDEX];
} CACHEMANAGER;

BOOL kInitializeCacheManager(void);
CACHEBUFFER *kAllocateCacheBuffer(int iCacheTableIndex);
CACHEBUFFER *kFindCacheBuffer(int iCacheTableIndex, DWORD dwTag);
CACHEBUFFER *kGetVictimInCacheBuffer(int iCacheTableIndex);
void kDiscardAllCacheBuffer(int iCacheTableIndex);
BOOL kGetCacheBufferAndCount(int iCacheTableIndex, CACHEBUFFER **ppstCacheBuffer, int *piMaxCount);

static void kCutDownAccessTime(int iCacheTableIndex);

#endif /*__CACHEMANAGER_H__*/