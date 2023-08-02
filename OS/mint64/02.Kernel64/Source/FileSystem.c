#include "FileSystem.h"
#include "HardDisk.h"
#include "DynamicMemory.h"
#include "Utility.h"
#include "Task.h"
#include "Console.h"
#include "CacheManager.h"
#include "RAMDisk.h"

// 파일 시스템 자료구조
static FILESYSTEMMANAGER gs_stFileSystemManager;

// 파일 시스템 임시 버퍼 4KB
static BYTE gs_vbTempBuffer[FILESYSTEM_SECTORSPERCLUSTER * 512];


fkReadHDDInformation gs_pfkReadHDDInformation = NULL;
fkReadHDDSector gs_pfkReadHDDSector = NULL;
fkWriteHDDSector gs_pfkWriteHDDSector = NULL;

BOOL kInitializeFileSystem(void)
{
    BOOL bCacheEnable = FALSE;

    kMemSet(&gs_stFileSystemManager, 0, sizeof(gs_stFileSystemManager));

    kInitializeMutex(&(gs_stFileSystemManager.stMutex));

    // 하드 디스크 초기화
    if (kInitializeHDD() == TRUE)
    {
        // 기능 함수 설정
        gs_pfkReadHDDInformation = kReadHDDInformation;
        gs_pfkReadHDDSector = kReadHDDSector;
        gs_pfkWriteHDDSector = kWriteHDDSector;

        // 캐시 활성화
        bCacheEnable = TRUE;
    }
    // 램 디스크인 경우
    else if(kInitializeRDD(RDD_TOTALSECTORCOUNT) == TRUE)
    {
        gs_pfkReadHDDInformation = kReadRDDInformation;
        gs_pfkReadHDDSector = kReadRDDSector;
        gs_pfkWriteHDDSector = kWriteRDDSector;

        if(kFormat() == FALSE)
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    // 파일 시스템 인식
    if (kMount() == FALSE)
    {
        return FALSE;
    }

    // 파일은 Pool 형태로 관리하므로 영역을 할당해줘야 한다고 함
    gs_stFileSystemManager.pstHandlePool = (FILE *) kAllocateMemory(FILESYSTEM_HANDLE_MAXCOUNT * sizeof(FILE));

    if(gs_stFileSystemManager.pstHandlePool == NULL)
    {
        gs_stFileSystemManager.bMounted = FALSE;
        return FALSE;
    }

    kMemSet(gs_stFileSystemManager.pstHandlePool, 0, FILESYSTEM_HANDLE_MAXCOUNT * sizeof(FILE));

    if(bCacheEnable == TRUE)
    {
        gs_stFileSystemManager.bCacheEnable = kInitializeCacheManager();
    }

    return TRUE;
}

/**
 * MINT 파일 시스템 인식 함수
*/
BOOL kMount(void)
{
    MBR *pstMBR;

    kLock(&(gs_stFileSystemManager.stMutex));
    
    // MBR을 읽음
    if (gs_pfkReadHDDSector(TRUE, TRUE, 0, 1, gs_vbTempBuffer) == FALSE)
    {
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return FALSE;
    }

    // MBR에서 MINT 파일 시스템 시그니쳐 확인
    pstMBR = (MBR *)gs_vbTempBuffer;
    if (pstMBR->dwSignature != FILESYSTEM_SIGNATURE)    // 에러 발생..
    {
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return FALSE;
    }
    
    // 시스템 인식 성공
    gs_stFileSystemManager.bMounted = TRUE;

    // 파일 시스템 정보 설정
    gs_stFileSystemManager.dwReservedSectorCount = pstMBR->dwReservedSectorCount;
    gs_stFileSystemManager.dwClusterLinkAreaStartAddress = pstMBR->dwReservedSectorCount + 1;
    gs_stFileSystemManager.dwClusterLinkAreaSize = pstMBR->dwClusterLinkSectorCount;
    gs_stFileSystemManager.dwDataAreaStartAddress = pstMBR->dwReservedSectorCount + pstMBR->dwClusterLinkSectorCount + 1;
    gs_stFileSystemManager.dwTotalClusterCount = pstMBR->dwTotalClusterCount;

    kUnlock(&(gs_stFileSystemManager.stMutex));
    
    return TRUE;
}

/**
 * 파일 시스탬 생성 함수
*/
BOOL kFormat(void)
{
    HDDINFORMATION *pstHDD;
    MBR *pstMBR;
    DWORD dwTotalSectorCount, dwRemainSectorCount;
    DWORD dwMaxClusterCount, dwClusterCount;
    DWORD dwClusterLinkSectorCount;
    DWORD i;

    kLock(&(gs_stFileSystemManager.stMutex));

    // 하드 디스크 정보를 읽음
    pstHDD = (HDDINFORMATION *)gs_vbTempBuffer;
    if (gs_pfkReadHDDInformation(TRUE, TRUE, pstHDD) == FALSE)
    {
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return FALSE;
    }

    // 하드 디스크 전체 섹터 수
    dwTotalSectorCount = pstHDD->dwTotalSectors;
    // 전체 섹터 수를 클러스터 단위로 나누어 최대 클러스터 개수 파악
    dwMaxClusterCount = dwTotalSectorCount / FILESYSTEM_SECTORSPERCLUSTER;
    // 클러스터 링크는 4바이트이므로 1섹터 당 128개의 링크 가능
    // 최대 클러스터 개수를 128로 나누어 올림하면 
    // 최대 클러스터 개수에서의 클러스터 링크 영역 섹터 수 파악 가능
    dwClusterLinkSectorCount = (dwMaxClusterCount + 127) / 128;
    // 사용 가능한 데이터 영역
    // 전체 - MBR + 클러스터 링크 = 데이터 영역
    dwRemainSectorCount = dwTotalSectorCount - dwClusterLinkSectorCount - 1;
    // 데이터 영역의 클러스터 개수
    dwClusterCount = dwRemainSectorCount / FILESYSTEM_SECTORSPERCLUSTER;
    // 실제 사용 가능한 클러스터 링크 테이블 섹터 수 근사치
    dwClusterLinkSectorCount = (dwClusterCount + 127) / 128;


    // 메타 데이터 영역을 파악했으니 이제 초기화 해야함
    // MBR 영역 읽기
    if (gs_pfkReadHDDSector(TRUE, TRUE, 0, 1, gs_vbTempBuffer) == FALSE)
    {
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return FALSE;
    }
    pstMBR = (MBR *)gs_vbTempBuffer;
    
    // MBR 영역 초기화
    // MINT는 파티션은 사용하지 않으므로 0으로 초기화
    kMemSet(pstMBR->vstPartition, 0, sizeof(pstMBR->vstPartition));
    
    // 시그니쳐는 MINT 파일 시스템 시그니쳐로
    pstMBR->dwSignature = FILESYSTEM_SIGNATURE;
    // 예약된 영역도 사용하지 않으므로 0
    pstMBR->dwReservedSectorCount = 0;
    // 클러스터 링크 개수와 데이터 영역 클러스터 개수 설정
    pstMBR->dwClusterLinkSectorCount = dwClusterLinkSectorCount;
    pstMBR->dwTotalClusterCount = dwClusterCount;

    // MBR 영역에 1섹터 씀
    if (gs_pfkWriteHDDSector(TRUE, TRUE, 0, 1, gs_vbTempBuffer) == FALSE)
    {
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return FALSE;
    }

    // 임시 버퍼 초기화 후 MBR 이후부터 루트 디렉터리까지 모두 0으로 초기화
    kMemSet(gs_vbTempBuffer, 0, 512);
    for (i = 0; i < (dwClusterLinkSectorCount + FILESYSTEM_SECTORSPERCLUSTER); i++)
    {
        // 루트 디렉터리는 클러스터 0이므로 할당되었음으로 표시
        if (i == 0)
        {
            ((DWORD *)(gs_vbTempBuffer))[0] = FILESYSTEM_LASTCLUSTER;
        }
        // 나머지는 FREE로 설정
        else
        {
            ((DWORD *)(gs_vbTempBuffer))[0] = FILESYSTEM_FREECLUSTER;
        }

        // 1섹터씩 씀
        if (gs_pfkWriteHDDSector(TRUE, TRUE, i + 1, 1, gs_vbTempBuffer) == FALSE)
        {
            kUnlock(&(gs_stFileSystemManager.stMutex));
            return FALSE;
        }
    }

    if(gs_stFileSystemManager.bCacheEnable == TRUE)
    {
        kDiscardAllCacheBuffer(CACHE_CLUSTERLINKTABLEAREA);
        kDiscardAllCacheBuffer(CACHE_DATAAREA);
    }

    kUnlock(&(gs_stFileSystemManager.stMutex));
    return TRUE;
}

/**
 * HDD 정보를 읽어옴
*/
BOOL kGetHDDInformation(HDDINFORMATION *pstInformation)
{
    BOOL bResult;

    kLock(&(gs_stFileSystemManager.stMutex));

    bResult = gs_pfkReadHDDInformation(TRUE, TRUE, pstInformation);

    kUnlock(&(gs_stFileSystemManager.stMutex));

    return bResult;
}

/**
 * 클러스터 링크 테이블 영역 내의 오프셋에서 1섹터 읽음
 * 
*/
static BOOL kkReadClusterLinkTable(DWORD dwOffset, BYTE *pbBuffer)
{
    if(gs_stFileSystemManager.bCacheEnable == FALSE)
    {
        return kInternalkkReadClusterLinkTableWithoutCache(dwOffset, pbBuffer);
    }
    else
    {
        return kInternalkkReadClusterLinkTableWithCache(dwOffset, pbBuffer);
    }
}

static BOOL kInternalkkReadClusterLinkTableWithoutCache(DWORD dwOffset,BYTE *pbBuffer)
{
    return gs_pfkReadHDDSector(TRUE, TRUE, dwOffset + gs_stFileSystemManager.dwClusterLinkAreaStartAddress, 1, pbBuffer);
}

static BOOL kInternalkkReadClusterLinkTableWithCache(DWORD dwOffset, BYTE *pbBuffer)
{
    CACHEBUFFER *pstCacheBuffer;

    pstCacheBuffer = kFindCacheBuffer(CACHE_CLUSTERLINKTABLEAREA, dwOffset);

    if (pstCacheBuffer != NULL)
    {
        kMemCpy(pbBuffer, pstCacheBuffer->pbBuffer, 512);
        return TRUE;
    }

    if (kInternalkkReadClusterLinkTableWithoutCache(dwOffset, pbBuffer) == FALSE)
    {
        return FALSE;
    }

    pstCacheBuffer = kAllocateCacheBufferWithFlush(CACHE_CLUSTERLINKTABLEAREA);
    
    if (pstCacheBuffer == NULL)
    {
        return FALSE;
    }

    kMemCpy(pstCacheBuffer->pbBuffer, pbBuffer, 512);
    pstCacheBuffer->dwTag = dwOffset;

    pstCacheBuffer->bChanged = FALSE;
    return TRUE;
}

static CACHEBUFFER *kAllocateCacheBufferWithFlush(int iCacheTableIndex)
{
    CACHEBUFFER *pstCacheBuffer;

    pstCacheBuffer = kAllocateCacheBuffer(iCacheTableIndex);
    if (pstCacheBuffer == NULL)
    {
        pstCacheBuffer = kGetVictimInCacheBuffer(iCacheTableIndex);
        if (pstCacheBuffer == NULL)
        {
            kPrintf("[!] Cache Allocate Fail~!!!!\n");
            return NULL;
        }

        if (pstCacheBuffer->bChanged == TRUE)
        {
            switch (iCacheTableIndex)
            {
            case CACHE_CLUSTERLINKTABLEAREA:
                if (kInternalkkWriteClusterLinkTableWithoutCache(pstCacheBuffer->dwTag, pstCacheBuffer->pbBuffer) == FALSE)
                {
                    kPrintf("[!] Cache Buffer Write Fail~!!!!\n");
                    return NULL;
                }
                break;

            case CACHE_DATAAREA:
                if (kInternalkWriteClusterWithoutCache(pstCacheBuffer->dwTag, pstCacheBuffer->pbBuffer) == FALSE)
                {
                    kPrintf("[!] Cache Buffer Write Fail~!!!!\n");
                    return NULL;
                }
                break;

            default:
                kPrintf("[!] kAllocateCacheBufferWithFlush Fail\n");
                return NULL;
                break;
            }
        }
    }
    return pstCacheBuffer;
}

/**
 * 클러스터 링크 테이블 영역의 오프셋에서 1섹터 씀
*/
static BOOL kkWriteClusterLinkTable(DWORD dwOffset, BYTE *pbBuffer)
{
    if (gs_stFileSystemManager.bCacheEnable == FALSE)
    {
        return kInternalkkWriteClusterLinkTableWithoutCache(dwOffset, pbBuffer);
    }
    else
    {
        return kInternalkkWriteClusterLinkTableWithCache(dwOffset, pbBuffer);
    }
}

static BOOL kInternalkkWriteClusterLinkTableWithoutCache(DWORD dwOffset,BYTE *pbBuffer)
{
    return gs_pfkWriteHDDSector(TRUE, TRUE, dwOffset + gs_stFileSystemManager.dwClusterLinkAreaStartAddress, 1, pbBuffer);
}

static BOOL kInternalkkWriteClusterLinkTableWithCache(DWORD dwOffset, BYTE *pbBuffer)
{
    CACHEBUFFER *pstCacheBuffer;

    pstCacheBuffer = kFindCacheBuffer(CACHE_CLUSTERLINKTABLEAREA, dwOffset);

    if (pstCacheBuffer != NULL)
    {
        kMemCpy(pstCacheBuffer->pbBuffer, pbBuffer, 512);

        pstCacheBuffer->bChanged = TRUE;
        return TRUE;
    }

    pstCacheBuffer = kAllocateCacheBufferWithFlush(CACHE_CLUSTERLINKTABLEAREA);
    
    if (pstCacheBuffer == NULL)
    {
        return FALSE;
    }

    kMemCpy(pstCacheBuffer->pbBuffer, pbBuffer, 512);
    pstCacheBuffer->dwTag = dwOffset;

    pstCacheBuffer->bChanged = TRUE;

    return TRUE;
}

/**
 * 데이터 영역의 클러스터를 읽는 함수
 * 클러스터 단위로 읽으며 오프셋을 이용해서 계산
*/
static BOOL kReadCluster(DWORD dwOffset, BYTE *pbBuffer)
{
    if (gs_stFileSystemManager.bCacheEnable == FALSE)
    {
        return kInternalkReadClusterWithoutCache(dwOffset, pbBuffer);
    }
    else
    {
        return kInternalkReadClusterWithCache(dwOffset, pbBuffer);
    }
}

static BOOL kInternalkReadClusterWithoutCache(DWORD dwOffset, BYTE *pbBuffer)
{
    return gs_pfkReadHDDSector(TRUE, TRUE, (dwOffset * FILESYSTEM_SECTORSPERCLUSTER) + gs_stFileSystemManager.dwDataAreaStartAddress, FILESYSTEM_SECTORSPERCLUSTER, pbBuffer);
}

static BOOL kInternalkReadClusterWithCache(DWORD dwOffset, BYTE *pbBuffer)
{
    CACHEBUFFER *pstCacheBuffer;

    pstCacheBuffer = kFindCacheBuffer(CACHE_DATAAREA, dwOffset);

    if (pstCacheBuffer != NULL)
    {
        kMemCpy(pbBuffer, pstCacheBuffer->pbBuffer, FILESYSTEM_CLUSTERSIZE);
        return TRUE;
    }

    if (kInternalkReadClusterWithoutCache(dwOffset, pbBuffer) == FALSE)
    {
        return FALSE;
    }

    pstCacheBuffer = kAllocateCacheBufferWithFlush(CACHE_DATAAREA);
    if (pstCacheBuffer == NULL)
    {
        return FALSE;
    }

    kMemCpy(pstCacheBuffer->pbBuffer, pbBuffer, FILESYSTEM_CLUSTERSIZE);
    pstCacheBuffer->dwTag = dwOffset;

    pstCacheBuffer->bChanged = FALSE;
    return TRUE;
}

/**
 * 데이터 영역의 클러스터에 값을 씀
*/
static BOOL kWriteCluster(DWORD dwOffset, BYTE *pbBuffer)
{
    if (gs_stFileSystemManager.bCacheEnable == FALSE)
    {
        return kInternalkWriteClusterWithoutCache(dwOffset, pbBuffer);
    }
    else
    {
        return kInternalkWriteClusterWithCache(dwOffset, pbBuffer);
    }
}

static BOOL kInternalkWriteClusterWithoutCache(DWORD dwOffset, BYTE *pbBuffer)
{
    return gs_pfkWriteHDDSector(TRUE, TRUE, (dwOffset * FILESYSTEM_SECTORSPERCLUSTER) + gs_stFileSystemManager.dwDataAreaStartAddress, FILESYSTEM_SECTORSPERCLUSTER, pbBuffer);
}

static BOOL kInternalkWriteClusterWithCache(DWORD dwOffset, BYTE *pbBuffer)
{
    CACHEBUFFER *pstCacheBuffer;

    pstCacheBuffer = kFindCacheBuffer(CACHE_DATAAREA, dwOffset);

    if (pstCacheBuffer != NULL)
    {
        kMemCpy(pstCacheBuffer->pbBuffer, pbBuffer, FILESYSTEM_CLUSTERSIZE);

        pstCacheBuffer->bChanged = TRUE;

        return TRUE;
    }

    pstCacheBuffer = kAllocateCacheBufferWithFlush(CACHE_DATAAREA);
    if (pstCacheBuffer == NULL)
    {
        return FALSE;
    }

    kMemCpy(pstCacheBuffer->pbBuffer, pbBuffer, FILESYSTEM_CLUSTERSIZE);
    pstCacheBuffer->dwTag = dwOffset;

    pstCacheBuffer->bChanged = TRUE;

    return TRUE;
}

/**
 * 링크 테이블 영역을 돌면서 빈 클러스터를 검색
 * 읽은 섹터 내에서 값이 0x00으로 저장된 부분을 찾으면 됨
 * 클러스터 링크 테이블에는 1섹터 당 128개의 링크가 있음
*/
static DWORD kFindFreeCluster(void)
{
    DWORD dwLinkCountInSector;
    DWORD dwLastSectorOffset, dwCurrentSectorOffset;
    DWORD i, j;

    // 인식이 안됬다면 어차피 못하므로 종료
    if (gs_stFileSystemManager.bMounted == FALSE)
    {
        // 빈 클러스터가 없다는 의미임
        return FILESYSTEM_LASTCLUSTER;
    }

    // 마지막으로 할당한 클러스터의 오프셋을 검색하여 그 이후부터 검색
    dwLastSectorOffset = gs_stFileSystemManager.dwLastAllocatedClusterLinkSectorOffset;

    // 마지막 오프셋 이후부터 검색
    for (i = 0; i < gs_stFileSystemManager.dwClusterLinkAreaSize; i++)
    {
        // 클러스터 링크 테이블의 마지막 섹터라면 남은 클러스터의 수만큼 루프를 돌아야 함
        if ((dwLastSectorOffset + i) == (gs_stFileSystemManager.dwClusterLinkAreaSize - 1))
        {
            dwLinkCountInSector = gs_stFileSystemManager.dwTotalClusterCount % 128;
        }
        else
        {
            dwLinkCountInSector = 128;
        }

        // 이번에 읽어야 할 클러스터 링크 테이블 내의 섹터 오프셋을 구해서 읽음
        dwCurrentSectorOffset = (dwLastSectorOffset + i) % gs_stFileSystemManager.dwClusterLinkAreaSize;
        
        if (kkReadClusterLinkTable(dwCurrentSectorOffset, gs_vbTempBuffer) == FALSE)
        {
            return FILESYSTEM_LASTCLUSTER;
        }

        // 섹터 내에서 루프를 돌면서 빈 클러스터 검색
        // 링크 테이블 1섹터에는 128개의 링크 존재
        for (j = 0; j < dwLinkCountInSector; j++)
        {
            if (((DWORD *)gs_vbTempBuffer)[j] == FILESYSTEM_FREECLUSTER)
            {
                break;
            }
        }
        
        // 찾았다면 마지막 할당된 클러스터 링크 오프셋을 설정
        if (j != dwLinkCountInSector)
        {
            gs_stFileSystemManager.dwLastAllocatedClusterLinkSectorOffset = dwCurrentSectorOffset;

            // 현재 클러스터 링크 오프셋을 감안해서 클러스터 인덱스 계산
            return (dwCurrentSectorOffset * 128) + j;
        }
    }

    return FILESYSTEM_LASTCLUSTER;
}

/**
 * 클러스터 링크 테이블에 값을 설정
 
 * 클러스터 인덱스로 클러스터 링크 테이블 내의 섹터 오프셋을 계산하는 방법은
 * 한 섹터에는 128개의 링크가 있으므로 클러스터 인덱스 / 128
 
 * 오프셋을 구한 후 링크 정보를 넣어 줄 데이터 공간의 오프셋을 구해야 함
 * 방법은 클러스터 인덱스 % 128
*/
static BOOL kSetClusterLinkData(DWORD dwClusterIndex, DWORD dwData)
{
    DWORD dwSectorOffset;

    // 파일 시스템 인식 확인
    if (gs_stFileSystemManager.bMounted == FALSE)
    {
        return FALSE;
    }

    // 클러스터 링크 섹터를 구함
    dwSectorOffset = dwClusterIndex / 128;
    
    // 해당 섹터를 읽음
    if (kkReadClusterLinkTable(dwSectorOffset, gs_vbTempBuffer) == FALSE)
    {
        return FALSE;
    }

    // 링크 데이터의 오프셋에 링크 정보 설정
    ((DWORD *)gs_vbTempBuffer)[dwClusterIndex % 128] = dwData;

    // 해당 섹터에 다시 값을 설정
    if (kkWriteClusterLinkTable(dwSectorOffset, gs_vbTempBuffer) == FALSE)
    {
        return FALSE;
    }

    return TRUE;
}

/**
 * 클러스터 링크 테이블의 값을 읽음
*/
static BOOL kGetClusterLinkData(DWORD dwClusterIndex, DWORD *pdwData)
{
    DWORD dwSectorOffset;

    if (gs_stFileSystemManager.bMounted == FALSE)
    {
        return FALSE;
    }

    // 섹터 오프셋을 구함
    dwSectorOffset = dwClusterIndex / 128;
    // 클러스터 링크 영역을 벗어났으면 에러
    if (dwSectorOffset > gs_stFileSystemManager.dwClusterLinkAreaSize)
    {
        return FALSE;
    }
    // 해당 섹터를 읽음
    if (kkReadClusterLinkTable(dwSectorOffset, gs_vbTempBuffer) == FALSE)
    {
        return FALSE;
    }
    // 읽어서 링크 정보가 들어있는 링크를 가져옴
    *pdwData = ((DWORD *)gs_vbTempBuffer)[dwClusterIndex % 128];
    
    return TRUE;
}

/**
 * 루트 디렉터리에서 빈 디렉터리 엔트리 검색
 * 즉, 생성할 파일을 담을 공간을 검색
*/
static int kFindFreeDirectoryEntry(void)
{
    DIRECTORYENTRY *pstEntry;
    int i;

    if (gs_stFileSystemManager.bMounted == FALSE)
    {
        return -1;
    }

    // 클러스터 0은 루트 디렉터리임
    // 루트 디렉터리를 읽음
    if (kReadCluster(0, gs_vbTempBuffer) == FALSE)
    {
        return -1;
    }

    // 루트 디렉터리에서 빈 디렉터리 엔트리 검색
    pstEntry = (DIRECTORYENTRY *)gs_vbTempBuffer;
    
    for (i = 0; i < FILESYSTEM_MAXDIRECTORYENTRYCOUNT; i++)
    {
        // 시작 클러스터가 0인 경우 빈 디렉터리 엔트리
        if (pstEntry[i].dwStartClusterIndex == 0)
        {
            return i;
        }
    }
    
    return -1;
}

/**
 * 루트 디렉터리의 해당 인덱스에 엔트리 설정
*/
static BOOL kSetDirectoryEntryData(int iIndex, DIRECTORYENTRY *pstEntry)
{
    DIRECTORYENTRY *pstRootEntry;

    // 파일 시스템이 인식이 안된 경우
    // 인덱스가 음수인 경우 또는 최대 엔트리 값을 벗어난 경우
    if ((gs_stFileSystemManager.bMounted == FALSE) || (iIndex < 0) || (iIndex >= FILESYSTEM_MAXDIRECTORYENTRYCOUNT))
    {
        return FALSE;
    }

    // 루트 디렉터리를 읽음
    if (kReadCluster(0, gs_vbTempBuffer) == FALSE)
    {
        return FALSE;
    }

    // 루트 디렉터리에 있는 해당 데이터를 갱신
    pstRootEntry = (DIRECTORYENTRY *)gs_vbTempBuffer;
    kMemCpy(pstRootEntry + iIndex, pstEntry, sizeof(DIRECTORYENTRY));

    // 루트 디렉터리에 씀
    if (kWriteCluster(0, gs_vbTempBuffer) == FALSE)
    {
        return FALSE;
    }
    
    return TRUE;
}

/**
 * 루트 디렉터리의 해당 인덱스에 위치하는 디렉터리 엔트리 반환
*/
static BOOL kGetDirectoryEntryData(int iIndex, DIRECTORYENTRY *pstEntry)
{
    DIRECTORYENTRY *pstRootEntry;

    if ((gs_stFileSystemManager.bMounted == FALSE) || (iIndex < 0) || (iIndex >= FILESYSTEM_MAXDIRECTORYENTRYCOUNT))
    {
        return FALSE;
    }

    // 루트 디렉터리를 읽음
    if (kReadCluster(0, gs_vbTempBuffer) == FALSE)
    {
        return FALSE;
    }

    pstRootEntry = (DIRECTORYENTRY *)gs_vbTempBuffer;
    // 해당 엔트리를 가져옴
    kMemCpy(pstEntry, pstRootEntry + iIndex, sizeof(DIRECTORYENTRY));
    
    return TRUE;
}

/**
 * 루트 디렉터리에서 파일 이름이 일치하는 엔트리 검색
*/
static int kFindDirectoryEntry(const char *pcFileName, DIRECTORYENTRY *pstEntry)
{
    DIRECTORYENTRY *pstRootEntry;
    int i;
    int iLength;

    if (gs_stFileSystemManager.bMounted == FALSE)
    {
        return -1;
    }
    // 루트 읽음
    if (kReadCluster(0, gs_vbTempBuffer) == FALSE)
    {
        return -1;
    }
    // 파일 길이 확인
    iLength = kStrLen(pcFileName);
    
    pstRootEntry = (DIRECTORYENTRY *)gs_vbTempBuffer;
    // 전체 디렉터리 엔트리에서 파일 명 검색
    for (i = 0; i < FILESYSTEM_MAXDIRECTORYENTRYCOUNT; i++)
    {
        // 파일 이름과 길이가 같은지 확인
        if (kMemCmp(pstRootEntry[i].vcFileName, pcFileName, iLength) == 0)
        {
            // 같으면 가져옴
            kMemCpy(pstEntry, pstRootEntry + i, sizeof(DIRECTORYENTRY));
            return i;
        }
    }
    
    return -1;
}

void kGetFileSystemInformation(FILESYSTEMMANAGER *pstManager)
{
    kMemCpy(pstManager, &gs_stFileSystemManager, sizeof(gs_stFileSystemManager));
}

/**
 * 비어있는 핸들 할당
*/
static void *kAllocateFileDirectoryHandle(void)
{
    int i;
    FILE *pstFile;

    pstFile = gs_stFileSystemManager.pstHandlePool;
    
    // 핸들 풀을 모두 검색해서 비어 있는 핸들 반환
    for(i=0; i< FILESYSTEM_HANDLE_MAXCOUNT; i++)
    {
        if(pstFile->bType == FILESYSTEM_TYPE_FREE)
        {
            pstFile->bType = FILESYSTEM_TYPE_FILE;
            return pstFile;
        }

        pstFile++;
    }

    return NULL;
}

/**
 * 사용한 핸들을 반환
*/
static void kFreeFileDirectoryHandle(FILE *pstFile)
{
    // 전체 영역을 초기화 후 비어 있는 타입으로 설정
    kMemSet(pstFile, 0, sizeof(FILE));
    pstFile->bType = FILESYSTEM_TYPE_FREE;
}


//////////////////////////////////////////////////////////////////////////
// 파일, 디렉터리 관련 함수

/**
 * 파일 생성 함수
*/
static BOOL kCreateFile(const char *pcFileName, DIRECTORYENTRY *pstEntry, int *piDirectoryEntryIndex)
{
    DWORD dwCluster;

    // 빈 클러스터 검색
    dwCluster = kFindFreeCluster();
    if((dwCluster == FILESYSTEM_LASTCLUSTER) || (kSetClusterLinkData(dwCluster, FILESYSTEM_LASTCLUSTER) == FALSE))
    {
        return FALSE;
    }

    // 빈 디렉터리 엔트리 검색
    *piDirectoryEntryIndex = kFindFreeDirectoryEntry();
    if(*piDirectoryEntryIndex == -1)
    {
        kSetClusterLinkData(dwCluster, FILESYSTEM_FREECLUSTER);
        return FALSE;
    }

    // 디렉터리 엔트리 설정
    // 파일 명, 파일 크기, 시작 클러스터 위치
    kMemCpy(pstEntry->vcFileName, pcFileName, kStrLen(pcFileName) + 1);
    pstEntry->dwStartClusterIndex = dwCluster;
    pstEntry->dwFileSize = 0;

    // 디렉터리 엔트리 등록
    if(kSetDirectoryEntryData(*piDirectoryEntryIndex, pstEntry) == FALSE)
    {
        kSetClusterLinkData(dwCluster, FILESYSTEM_FREECLUSTER);
        return FALSE;
    }

    return TRUE;
}

/**
 * 파일의 시작 클러스터부터 마지막 클러스터까지 모두 해제
*/
static BOOL kFreeClusterUntilEnd(DWORD dwClusterIndex)
{
    DWORD dwCurrentClusterIndex;
    DWORD dwNextClusterIndex;

    // 클러스터 인덱스 초기화
    dwCurrentClusterIndex = dwClusterIndex;
    // 마지막 클러스터까지 해제
    while(dwCurrentClusterIndex != FILESYSTEM_LASTCLUSTER)
    {
        // 다음 클러스터 위치 검색
        if(kGetClusterLinkData(dwCurrentClusterIndex, &dwNextClusterIndex) == FALSE)
        {
            return FALSE;
        }
        
        // 현재 클러스터를 해제
        if(kSetClusterLinkData(dwCurrentClusterIndex, FILESYSTEM_FREECLUSTER) == FALSE)
        {
            return FALSE;
        }
        
        // 다음 클러스터로 설정
        dwCurrentClusterIndex = dwNextClusterIndex;
    }

    return TRUE;
}

/**
 * POSIX fopen 함수 구현
 * 파일 명과 모드를 인자로 받음
*/
FILE *kOpenFile(const char *pcFileName, const char *pcMode)
{
    DIRECTORYENTRY stEntry;
    int iDirectoryEntryOffset;
    int iFileNameLength;
    DWORD dwSecondCluster;
    FILE *pstFile;

    // 파일 길이 확인
    iFileNameLength = kStrLen(pcFileName);
    if( (iFileNameLength > (sizeof(stEntry.vcFileName) -1) ) || (iFileNameLength == 0))
    {
        return NULL;
    }

    kLock(&(gs_stFileSystemManager.stMutex));

    // 해당 파일이 있는지 확인
    iDirectoryEntryOffset = kFindDirectoryEntry(pcFileName, &stEntry);
    // 해당 파일이 없는 경우
    if(iDirectoryEntryOffset == -1)
    {
        // 모드가 r이면 파일을 생성할 수 없으므로 종료
        if(pcMode[0] == 'r')
        {
            kUnlock(&(gs_stFileSystemManager.stMutex));
            return NULL;
        }

        // 나머지 경우 파일을 생성
        if(kCreateFile(pcFileName, &stEntry, &iDirectoryEntryOffset) == FALSE)
        {
            kUnlock(&(gs_stFileSystemManager.stMutex));
            return NULL;
        }
    }
    // 파일이 있고 모드가 w인 경우 덮어써야 함 
    else if(pcMode[0] == 'w')
    {
        // 시작 클러스터 위치를 넘겨서 두 번째 클러스터 위치 정보를 가져옴
        if(kGetClusterLinkData(stEntry.dwStartClusterIndex, &dwSecondCluster) == FALSE)
        {
            kUnlock(&(gs_stFileSystemManager.stMutex));
            return NULL;
        }

        // 시작 클러스터를 마지막 클러스터로 설정
        if(kSetClusterLinkData(stEntry.dwStartClusterIndex, FILESYSTEM_LASTCLUSTER) == FALSE)
        {
            kUnlock(&(gs_stFileSystemManager.stMutex));
            return NULL;
        }

        // 다음 클러스터부터 마지막 클러스터까지 모두 해제
        if(kFreeClusterUntilEnd(dwSecondCluster) == FALSE)
        {
            kUnlock(&(gs_stFileSystemManager.stMutex));
            return NULL;
        }

        // 파일 크기를 0으로 설정
        stEntry.dwFileSize = 0;
        // 해당 파일의 디렉터리 엔트리에 값 재설정
        if(kSetDirectoryEntryData(iDirectoryEntryOffset, &stEntry) == FALSE)
        {
            kUnlock(&(gs_stFileSystemManager.stMutex));
            return NULL;
        }
    }

    // 파일에 핸들 할당
    pstFile = kAllocateFileDirectoryHandle();
    if(pstFile == NULL)
    {
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return NULL;
    }

    // 파일 핸들 설정
    pstFile->bType = FILESYSTEM_TYPE_FILE;
    pstFile->stFileHandle.iDirectoryEntryOffset = iDirectoryEntryOffset;
    pstFile->stFileHandle.dwFileSize = stEntry.dwFileSize;
    pstFile->stFileHandle.dwStartClusterIndex = stEntry.dwStartClusterIndex;
    pstFile->stFileHandle.dwCurrentClusterIndex = stEntry.dwStartClusterIndex;
    pstFile->stFileHandle.dwPreviousClusterIndex = stEntry.dwStartClusterIndex;
    pstFile->stFileHandle.dwCurrentOffset = 0;

    // 모드가 a인 경우 덧붙여서 써야 함
    if(pcMode[0] == 'a')
    {
        // 파일 끝으로 이동
        kSeekFile(pstFile, 0, FILESYSTEM_SEEK_END);
    }

    kUnlock(&(gs_stFileSystemManager.stMutex));
    
    return pstFile;
}

/**
 * POSIX fread 함수 구현
 * 파일을 읽는 함수
*/
DWORD kReadFile(void *pvBuffer, DWORD dwSize, DWORD dwCount, FILE *pstFile)
{
    DWORD dwTotalCount;
    DWORD dwReadCount;
    DWORD dwOffsetInCluster;
    DWORD dwCopySize;
    FILEHANDLE *pstFileHandle;
    DWORD dwNextClusterIndex;

    if((pstFile == NULL) || (pstFile->bType != FILESYSTEM_TYPE_FILE))
    {
        return 0;
    }

    // 파일 핸들 가져옴
    pstFileHandle = &(pstFile->stFileHandle);

    // 파일의 끝이거나 마지막 클러스터이면 종료
    if((pstFileHandle->dwCurrentOffset == pstFileHandle->dwFileSize) || (pstFileHandle->dwCurrentClusterIndex == FILESYSTEM_LASTCLUSTER))
    {
        return 0;
    }

    // 파일 끝과 비교해서 실제로 읽을 수 있는 값 계산
    dwTotalCount = MIN(dwSize * dwCount, pstFileHandle->dwFileSize - pstFileHandle->dwCurrentOffset);

    kLock(&(gs_stFileSystemManager.stMutex));

    // 계산된 값만큼 다 읽을 때 까지 반복
    dwReadCount = 0;
    while(dwReadCount != dwTotalCount)
    {
        // 파일의 현재 클러스터를 읽어서 버퍼에 복사
        if(kReadCluster(pstFileHandle->dwCurrentClusterIndex, gs_vbTempBuffer) == FALSE)
        {
            break;
        }

        // 클러스터 내에서 파일 포인터가 존재하는 오프셋 계산
        dwOffsetInCluster = pstFileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE;

        // 여러 클러스터에 걸쳐 있다면 현재 클러스터에서 남은 만큼 읽고 다음 클러스터로 이동
        dwCopySize = MIN(FILESYSTEM_CLUSTERSIZE - dwOffsetInCluster, dwTotalCount - dwReadCount);
        
        kMemCpy((char *)pvBuffer + dwReadCount, gs_vbTempBuffer + dwOffsetInCluster, dwCopySize);

        // 읽은 바이트 수와 파일 포인터 위치를 갱신
        dwReadCount += dwCopySize;
        pstFileHandle->dwCurrentOffset += dwCopySize;

        // 현재 클러스터를 다 읽었다면 다음 클러스터로 이동
        if ((pstFileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE) == 0)
        {
            // 현재 클러스터의 링크를 찾아 다음 클러스터의 인덱스를 얻음
            if (kGetClusterLinkData(pstFileHandle->dwCurrentClusterIndex, &dwNextClusterIndex) == FALSE)
            {
                break;
            }

            pstFileHandle->dwPreviousClusterIndex = pstFileHandle->dwCurrentClusterIndex;
            pstFileHandle->dwCurrentClusterIndex = dwNextClusterIndex;
        }
    }

    kUnlock(&(gs_stFileSystemManager.stMutex));

    return (dwReadCount / dwSize);
}

static BOOL kUpdateDirectoryEntry(FILEHANDLE *pstFileHandle)
{
    DIRECTORYENTRY stEntry;

    if ((pstFileHandle == NULL) || (kGetDirectoryEntryData(pstFileHandle->iDirectoryEntryOffset, &stEntry) == FALSE))
    {
        return FALSE;
    }

    stEntry.dwFileSize = pstFileHandle->dwFileSize;
    stEntry.dwStartClusterIndex = pstFileHandle->dwStartClusterIndex;

    if (kSetDirectoryEntryData(pstFileHandle->iDirectoryEntryOffset, &stEntry) == FALSE)
    {
        return FALSE;
    }
    
    return TRUE;
}

DWORD kWriteFile(const void *pvBuffer, DWORD dwSize, DWORD dwCount, FILE *pstFile)
{
    DWORD dwWriteCount;
    DWORD dwTotalCount;
    DWORD dwOffsetInCluster;
    DWORD dwCopySize;
    DWORD dwAllocatedClusterIndex;
    DWORD dwNextClusterIndex;
    FILEHANDLE *pstFileHandle;

    if ((pstFile == NULL) || (pstFile->bType != FILESYSTEM_TYPE_FILE))
    {
        return 0;
    }
    pstFileHandle = &(pstFile->stFileHandle);

    dwTotalCount = dwSize * dwCount;

    kLock(&(gs_stFileSystemManager.stMutex));

    dwWriteCount = 0;
    while (dwWriteCount != dwTotalCount)
    {
        if (pstFileHandle->dwCurrentClusterIndex == FILESYSTEM_LASTCLUSTER)
        {
            dwAllocatedClusterIndex = kFindFreeCluster();
            
            if (dwAllocatedClusterIndex == FILESYSTEM_LASTCLUSTER)
            {
                break;
            }

            if (kSetClusterLinkData(dwAllocatedClusterIndex, FILESYSTEM_LASTCLUSTER) == FALSE)
            {
                break;
            }

            if (kSetClusterLinkData(pstFileHandle->dwPreviousClusterIndex, dwAllocatedClusterIndex) == FALSE)
            {
                kSetClusterLinkData(dwAllocatedClusterIndex, FILESYSTEM_FREECLUSTER);
                break;
            }

            pstFileHandle->dwCurrentClusterIndex = dwAllocatedClusterIndex;

            kMemSet(gs_vbTempBuffer, 0, FILESYSTEM_LASTCLUSTER);
        }

        else if (((pstFileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE) != 0) || ((dwTotalCount - dwWriteCount) < FILESYSTEM_CLUSTERSIZE))
        {
            if (kReadCluster(pstFileHandle->dwCurrentClusterIndex,  gs_vbTempBuffer) == FALSE)
            {
                break;
            }
        }

        dwOffsetInCluster = pstFileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE;

        dwCopySize = MIN(FILESYSTEM_CLUSTERSIZE - dwOffsetInCluster, dwTotalCount - dwWriteCount);
        
        kMemCpy(gs_vbTempBuffer + dwOffsetInCluster, (char *)pvBuffer + dwWriteCount, dwCopySize);

        if (kWriteCluster(pstFileHandle->dwCurrentClusterIndex, gs_vbTempBuffer) == FALSE)
        {
            break;
        }

        dwWriteCount += dwCopySize;
        pstFileHandle->dwCurrentOffset += dwCopySize;

        if ((pstFileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE) == 0)
        {
            if (kGetClusterLinkData(pstFileHandle->dwCurrentClusterIndex, &dwNextClusterIndex) == FALSE)
            {
                break;
            }

            pstFileHandle->dwPreviousClusterIndex = pstFileHandle->dwCurrentClusterIndex;
            pstFileHandle->dwCurrentClusterIndex = dwNextClusterIndex;
        }
    }

    if (pstFileHandle->dwFileSize < pstFileHandle->dwCurrentOffset)
    {
        pstFileHandle->dwFileSize = pstFileHandle->dwCurrentOffset;
        
        kUpdateDirectoryEntry(pstFileHandle);
    }

    kUnlock(&(gs_stFileSystemManager.stMutex));

    return (dwWriteCount / dwSize);
}

BOOL kWriteZero(FILE *pstFile, DWORD dwCount)
{
    BYTE *pbBuffer;
    DWORD dwRemainCount;
    DWORD dwWriteCount;

    if (pstFile == NULL)
    {
        return FALSE;
    }

    pbBuffer = (BYTE *)kAllocateMemory(FILESYSTEM_CLUSTERSIZE);
    if (pbBuffer == NULL)
    {
        return FALSE;
    }

    kMemSet(pbBuffer, 0, FILESYSTEM_CLUSTERSIZE);
    dwRemainCount = dwCount;

    while (dwRemainCount != 0)
    {
        dwWriteCount = MIN(dwRemainCount, FILESYSTEM_CLUSTERSIZE);
        
        if (kWriteFile(pbBuffer, 1, dwWriteCount, pstFile) != dwWriteCount)
        {
            kFreeMemory(pbBuffer);
            return FALSE;
        }
        
        dwRemainCount -= dwWriteCount;
    }
    
    kFreeMemory(pbBuffer);
    
    return TRUE;
}

int kSeekFile(FILE *pstFile, int iOffset, int iOrigin)
{
    DWORD dwRealOffset;
    DWORD dwClusterOffsetToMove;
    DWORD dwCurrentClusterOffset;
    DWORD dwLastClusterOffset;
    DWORD dwMoveCount;
    DWORD i;
    DWORD dwStartClusterIndex;
    DWORD dwPreviousClusterIndex;
    DWORD dwCurrentClusterIndex;
    FILEHANDLE *pstFileHandle;

    if ((pstFile == NULL) || (pstFile->bType != FILESYSTEM_TYPE_FILE))
    {
        return 0;
    }
    pstFileHandle = &(pstFile->stFileHandle);

    switch (iOrigin)
    {
    case FILESYSTEM_SEEK_SET:
        if (iOffset <= 0)
        {
            dwRealOffset = 0;
        }
        else
        {
            dwRealOffset = iOffset;
        }
        break;

    case FILESYSTEM_SEEK_CUR:
        if ((iOffset < 0) &&
            (pstFileHandle->dwCurrentOffset <= (DWORD)-iOffset))
        {
            dwRealOffset = 0;
        }
        else
        {
            dwRealOffset = pstFileHandle->dwCurrentOffset + iOffset;
        }
        break;

    case FILESYSTEM_SEEK_END:
        if ((iOffset < 0) &&
            (pstFileHandle->dwFileSize <= (DWORD)-iOffset))
        {
            dwRealOffset = 0;
        }
        else
        {
            dwRealOffset = pstFileHandle->dwFileSize + iOffset;
        }
        break;
    }

    dwLastClusterOffset = pstFileHandle->dwFileSize / FILESYSTEM_CLUSTERSIZE;
    
    dwClusterOffsetToMove = dwRealOffset / FILESYSTEM_CLUSTERSIZE;
    
    dwCurrentClusterOffset = pstFileHandle->dwCurrentOffset / FILESYSTEM_CLUSTERSIZE;

    if (dwLastClusterOffset < dwClusterOffsetToMove)
    {
        dwMoveCount = dwLastClusterOffset - dwCurrentClusterOffset;
        dwStartClusterIndex = pstFileHandle->dwCurrentClusterIndex;
    }
    else if (dwCurrentClusterOffset <= dwClusterOffsetToMove)
    {
        dwMoveCount = dwClusterOffsetToMove - dwCurrentClusterOffset;
        dwStartClusterIndex = pstFileHandle->dwCurrentClusterIndex;
    }
    else
    {
        dwMoveCount = dwClusterOffsetToMove;
        dwStartClusterIndex = pstFileHandle->dwStartClusterIndex;
    }

    kLock(&(gs_stFileSystemManager.stMutex));

    dwCurrentClusterIndex = dwStartClusterIndex;
    
    for (i = 0; i < dwMoveCount; i++)
    {
        dwPreviousClusterIndex = dwCurrentClusterIndex;

        if (kGetClusterLinkData(dwPreviousClusterIndex, &dwCurrentClusterIndex) ==
            FALSE)
        {
            kUnlock(&(gs_stFileSystemManager.stMutex));
            return -1;
        }
    }

    if (dwMoveCount > 0)
    {
        pstFileHandle->dwPreviousClusterIndex = dwPreviousClusterIndex;
        pstFileHandle->dwCurrentClusterIndex = dwCurrentClusterIndex;
    }
    else if (dwStartClusterIndex == pstFileHandle->dwStartClusterIndex)
    {
        pstFileHandle->dwPreviousClusterIndex = pstFileHandle->dwStartClusterIndex;
        pstFileHandle->dwCurrentClusterIndex = pstFileHandle->dwStartClusterIndex;
    }

    if (dwLastClusterOffset < dwClusterOffsetToMove)
    {
        pstFileHandle->dwCurrentOffset = pstFileHandle->dwFileSize;
        kUnlock(&(gs_stFileSystemManager.stMutex));

        if (kWriteZero(pstFile, dwRealOffset - pstFileHandle->dwFileSize) == FALSE)
        {
            return 0;
        }
    }

    pstFileHandle->dwCurrentOffset = dwRealOffset;

    kUnlock(&(gs_stFileSystemManager.stMutex));

    return 0;
}

int kCloseFile(FILE *pstFile)
{
    if ((pstFile == NULL) || (pstFile->bType != FILESYSTEM_TYPE_FILE))
    {
        return -1;
    }

    kFreeFileDirectoryHandle(pstFile);
    
    return 0;
}

BOOL kIsFileOpened(const DIRECTORYENTRY *pstEntry)
{
    int i;
    FILE *pstFile;

    pstFile = gs_stFileSystemManager.pstHandlePool;
    
    for (i = 0; i < FILESYSTEM_HANDLE_MAXCOUNT; i++)
    {
        if ((pstFile[i].bType == FILESYSTEM_TYPE_FILE) && (pstFile[i].stFileHandle.dwStartClusterIndex == pstEntry->dwStartClusterIndex))
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

int kRemoveFile(const char *pcFileName)
{
    DIRECTORYENTRY stEntry;
    int iDirectoryEntryOffset;
    int iFileNameLength;

    iFileNameLength = kStrLen(pcFileName);
    
    if ((iFileNameLength > (sizeof(stEntry.vcFileName) - 1)) || (iFileNameLength == 0))
    {
        return NULL;
    }

    kLock(&(gs_stFileSystemManager.stMutex));

    iDirectoryEntryOffset = kFindDirectoryEntry(pcFileName, &stEntry);
    
    if (iDirectoryEntryOffset == -1)
    {
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return -1;
    }

    if (kIsFileOpened(&stEntry) == TRUE)
    {
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return -1;
    }

    if (kFreeClusterUntilEnd(stEntry.dwStartClusterIndex) == FALSE)
    {
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return -1;
    }

    kMemSet(&stEntry, 0, sizeof(stEntry));
    
    if (kSetDirectoryEntryData(iDirectoryEntryOffset, &stEntry) == FALSE)
    {
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return -1;
    }

    kUnlock(&(gs_stFileSystemManager.stMutex));
    return 0;
}

DIR *kOpenDirectory(const char *pcDirectoryName)
{
    DIR *pstDirectory;
    DIRECTORYENTRY *pstDirectoryBuffer;

    kLock(&(gs_stFileSystemManager.stMutex));

    pstDirectory = kAllocateFileDirectoryHandle();
    
    if (pstDirectory == NULL)
    {
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return NULL;
    }

    pstDirectoryBuffer = (DIRECTORYENTRY *)kAllocateMemory(FILESYSTEM_CLUSTERSIZE);
    
    if (pstDirectoryBuffer == NULL)
    {
        kFreeFileDirectoryHandle(pstDirectory);
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return NULL;
    }

    if (kReadCluster(0, (BYTE *)pstDirectoryBuffer) == FALSE)
    {
        kFreeFileDirectoryHandle(pstDirectory);
        kFreeMemory(pstDirectoryBuffer);

        kUnlock(&(gs_stFileSystemManager.stMutex));
        return NULL;
    }

    pstDirectory->bType = FILESYSTEM_TYPE_DIRECTORY;
    pstDirectory->stDirectoryHandle.iCurrentOffset = 0;
    pstDirectory->stDirectoryHandle.pstDirectoryBuffer = pstDirectoryBuffer;

    kUnlock(&(gs_stFileSystemManager.stMutex));
    
    return pstDirectory;
}

struct DirectoryEntryStruct *kReadDirectory(DIR *pstDirectory)
{
    DIRECTORYHANDLE *pstDirectoryHandle;
    DIRECTORYENTRY *pstEntry;

    if ((pstDirectory == NULL) || (pstDirectory->bType != FILESYSTEM_TYPE_DIRECTORY))
    {
        return NULL;
    }
    
    pstDirectoryHandle = &(pstDirectory->stDirectoryHandle);

    if ((pstDirectoryHandle->iCurrentOffset < 0) || (pstDirectoryHandle->iCurrentOffset >= FILESYSTEM_MAXDIRECTORYENTRYCOUNT))
    {
        return NULL;
    }

    kLock(&(gs_stFileSystemManager.stMutex));

    pstEntry = pstDirectoryHandle->pstDirectoryBuffer;
    
    while (pstDirectoryHandle->iCurrentOffset < FILESYSTEM_MAXDIRECTORYENTRYCOUNT)
    {
        if (pstEntry[pstDirectoryHandle->iCurrentOffset].dwStartClusterIndex != 0)
        {
            kUnlock(&(gs_stFileSystemManager.stMutex));
            return &(pstEntry[pstDirectoryHandle->iCurrentOffset++]);
        }

        pstDirectoryHandle->iCurrentOffset++;
    }

    kUnlock(&(gs_stFileSystemManager.stMutex));
    
    return NULL;
}

void kRewindDirectory(DIR *pstDirectory)
{
    DIRECTORYHANDLE *pstDirectoryHandle;

    if ((pstDirectory == NULL) || (pstDirectory->bType != FILESYSTEM_TYPE_DIRECTORY))
    {
        return;
    }
    
    pstDirectoryHandle = &(pstDirectory->stDirectoryHandle);

    kLock(&(gs_stFileSystemManager.stMutex));

    pstDirectoryHandle->iCurrentOffset = 0;

    kUnlock(&(gs_stFileSystemManager.stMutex));
}


int kCloseDirectory(DIR *pstDirectory)
{
    DIRECTORYHANDLE *pstDirectoryHandle;

    if ((pstDirectory == NULL) || (pstDirectory->bType != FILESYSTEM_TYPE_DIRECTORY))
    {
        return -1;
    }
    
    pstDirectoryHandle = &(pstDirectory->stDirectoryHandle);

    kLock(&(gs_stFileSystemManager.stMutex));

    kFreeMemory(pstDirectoryHandle->pstDirectoryBuffer);
    kFreeFileDirectoryHandle(pstDirectory);

    kUnlock(&(gs_stFileSystemManager.stMutex));

    return 0;
}

BOOL kFlushFileSystemCache(void)
{
    CACHEBUFFER *pstCacheBuffer;
    int iCacheCount;
    int i;

    if (gs_stFileSystemManager.bCacheEnable == FALSE)
    {
        return TRUE;
    }

    kLock(&(gs_stFileSystemManager.stMutex));

    kGetCacheBufferAndCount(CACHE_CLUSTERLINKTABLEAREA, &pstCacheBuffer, &iCacheCount);
    
    for (i = 0; i < iCacheCount; i++)
    {
        if (pstCacheBuffer[i].bChanged == TRUE)
        {
            if (kInternalkkWriteClusterLinkTableWithoutCache(pstCacheBuffer[i].dwTag, pstCacheBuffer[i].pbBuffer) == FALSE)
            {
                return FALSE;
            }
            
            pstCacheBuffer[i].bChanged = FALSE;
        }
    }

    kGetCacheBufferAndCount(CACHE_DATAAREA, &pstCacheBuffer, &iCacheCount);
    
    for (i = 0; i < iCacheCount; i++)
    {
        if (pstCacheBuffer[i].bChanged == TRUE)
        {
            if (kInternalkWriteClusterWithoutCache(pstCacheBuffer[i].dwTag, pstCacheBuffer[i].pbBuffer) == FALSE)
            {
                return FALSE;
            }
            
            pstCacheBuffer[i].bChanged = FALSE;
        }
    }

    kUnlock(&(gs_stFileSystemManager.stMutex));
    
    return TRUE;
}