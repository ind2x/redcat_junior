#include "FileSystem.h"
#include "HardDisk.h"
#include "DynamicMemory.h"
#include "Utility.h"
#include "Task.h"
#include "Console.h"
#include "CacheManager.h"
#include "RAMDisk.h"

static FILESYSTEMMANAGER gs_stFileSystemManager;

static BYTE gs_vbTempBuffer[FILESYSTEM_SECTORSPERCLUSTER * 512];

fReadHDDInformation gs_pfReadHDDInformation = NULL;
fReadHDDSector gs_pfReadHDDSector = NULL;
fWriteHDDSector gs_pfWriteHDDSector = NULL;

BOOL InitializeFileSystem(void)
{
    BOOL bCacheEnable = FALSE;

    MemSet(&gs_stFileSystemManager, 0, sizeof(gs_stFileSystemManager));

    InitializeMutex(&(gs_stFileSystemManager.stMutex));

    if (InitializeHDD() == TRUE)
    {
        gs_pfReadHDDInformation = ReadHDDInformation;
        gs_pfReadHDDSector = ReadHDDSector;
        gs_pfWriteHDDSector = WriteHDDSector;

        bCacheEnable = TRUE;
    }
    else if(InitializeRDD(RDD_TOTALSECTORCOUNT) == TRUE)
    {
        gs_pfReadHDDInformation = ReadRDDInformation;
        gs_pfReadHDDSector = ReadRDDSector;
        gs_pfWriteHDDSector = WriteRDDSector;

        if(Format() == FALSE)
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    if (Mount() == FALSE)
    {
        return FALSE;
    }

    gs_stFileSystemManager.pstHandlePool = (FILE *) AllocateMemory(FILESYSTEM_HANDLE_MAXCOUNT * sizeof(FILE));

    if(gs_stFileSystemManager.pstHandlePool == NULL)
    {
        gs_stFileSystemManager.bMounted = FALSE;
        return FALSE;
    }

    MemSet(gs_stFileSystemManager.pstHandlePool, 0, FILESYSTEM_HANDLE_MAXCOUNT * sizeof(FILE));

    if(bCacheEnable == TRUE)
    {
        gs_stFileSystemManager.bCacheEnable = InitializeCacheManager();
    }

    return TRUE;
}

BOOL Mount(void)
{
    MBR *pstMBR;

    Lock(&(gs_stFileSystemManager.stMutex));

    if (gs_pfReadHDDSector(TRUE, TRUE, 0, 1, gs_vbTempBuffer) == FALSE)
    {
        Unlock(&(gs_stFileSystemManager.stMutex));
        return FALSE;
    }

    pstMBR = (MBR *)gs_vbTempBuffer;
    if (pstMBR->dwSignature != FILESYSTEM_SIGNATURE)    // 에러 발생..
    {
        Unlock(&(gs_stFileSystemManager.stMutex));
        return FALSE;
    }

    gs_stFileSystemManager.bMounted = TRUE;

    gs_stFileSystemManager.dwReservedSectorCount = pstMBR->dwReservedSectorCount;
    
    gs_stFileSystemManager.dwClusterLinkAreaStartAddress = pstMBR->dwReservedSectorCount + 1;
    
    gs_stFileSystemManager.dwClusterLinkAreaSize = pstMBR->dwClusterLinkSectorCount;
    
    gs_stFileSystemManager.dwDataAreaStartAddress = pstMBR->dwReservedSectorCount + pstMBR->dwClusterLinkSectorCount + 1;
    
    gs_stFileSystemManager.dwTotalClusterCount = pstMBR->dwTotalClusterCount;

    Unlock(&(gs_stFileSystemManager.stMutex));
    
    return TRUE;
}

BOOL Format(void)
{
    HDDINFORMATION *pstHDD;
    MBR *pstMBR;
    DWORD dwTotalSectorCount, dwRemainSectorCount;
    DWORD dwMaxClusterCount, dwClsuterCount;
    DWORD dwClusterLinkSectorCount;
    DWORD i;


    Lock(&(gs_stFileSystemManager.stMutex));

    pstHDD = (HDDINFORMATION *)gs_vbTempBuffer;
    if (gs_pfReadHDDInformation(TRUE, TRUE, pstHDD) == FALSE)
    {
        Unlock(&(gs_stFileSystemManager.stMutex));
        return FALSE;
    }
    dwTotalSectorCount = pstHDD->dwTotalSectors;

    dwMaxClusterCount = dwTotalSectorCount / FILESYSTEM_SECTORSPERCLUSTER;

    dwClusterLinkSectorCount = (dwMaxClusterCount + 127) / 128;

    dwRemainSectorCount = dwTotalSectorCount - dwClusterLinkSectorCount - 1;
    dwClsuterCount = dwRemainSectorCount / FILESYSTEM_SECTORSPERCLUSTER;

    dwClusterLinkSectorCount = (dwClsuterCount + 127) / 128;


    if (gs_pfReadHDDSector(TRUE, TRUE, 0, 1, gs_vbTempBuffer) == FALSE)
    {
        Unlock(&(gs_stFileSystemManager.stMutex));
        return FALSE;
    }

    pstMBR = (MBR *)gs_vbTempBuffer;
    
    MemSet(pstMBR->vstPartition, 0, sizeof(pstMBR->vstPartition));
    
    pstMBR->dwSignature = FILESYSTEM_SIGNATURE;
    pstMBR->dwReservedSectorCount = 0;
    pstMBR->dwClusterLinkSectorCount = dwClusterLinkSectorCount;
    pstMBR->dwTotalClusterCount = dwClsuterCount;

    if (gs_pfWriteHDDSector(TRUE, TRUE, 0, 1, gs_vbTempBuffer) == FALSE)
    {
        Unlock(&(gs_stFileSystemManager.stMutex));
        return FALSE;
    }

    MemSet(gs_vbTempBuffer, 0, 512);

    for (i = 0; i < (dwClusterLinkSectorCount + FILESYSTEM_SECTORSPERCLUSTER); i++)
    {

        if (i == 0)
        {
            ((DWORD *)(gs_vbTempBuffer))[0] = FILESYSTEM_LASTCLUSTER;
        }
        else
        {
            ((DWORD *)(gs_vbTempBuffer))[0] = FILESYSTEM_FREECLUSTER;
        }

        if (gs_pfWriteHDDSector(TRUE, TRUE, i + 1, 1, gs_vbTempBuffer) == FALSE)
        {
            Unlock(&(gs_stFileSystemManager.stMutex));
            return FALSE;
        }
    }

    if(gs_stFileSystemManager.bCacheEnable == TRUE)
    {
        DiscardAllCacheBuffer(CACHE_CLUSTERLINKTABLEAREA);
        DiscardAllCacheBuffer(CACHE_DATAAREA);
    }

    Unlock(&(gs_stFileSystemManager.stMutex));
    return TRUE;
}

BOOL GetHDDInformation(HDDINFORMATION *pstInformation)
{
    BOOL bResult;

    Lock(&(gs_stFileSystemManager.stMutex));

    bResult = gs_pfReadHDDInformation(TRUE, TRUE, pstInformation);

    Unlock(&(gs_stFileSystemManager.stMutex));

    return bResult;
}

static BOOL ReadClusterLinkTable(DWORD dwOffset, BYTE *pbBuffer)
{
    if(gs_stFileSystemManager.bCacheEnable == FALSE)
    {
        return InternalReadClusterLinkTableWithoutCache(dwOffset, pbBuffer);
    }
    else
    {
        return InternalReadClusterLinkTableWithCache(dwOffset, pbBuffer);
    }
}

static BOOL InternalReadClusterLinkTableWithoutCache(DWORD dwOffset,BYTE *pbBuffer)
{
    return gs_pfReadHDDSector(TRUE, TRUE, dwOffset + gs_stFileSystemManager.dwClusterLinkAreaStartAddress, 1, pbBuffer);
}

static BOOL InternalReadClusterLinkTableWithCache(DWORD dwOffset, BYTE *pbBuffer)
{
    CACHEBUFFER *pstCacheBuffer;

    pstCacheBuffer = FindCacheBuffer(CACHE_CLUSTERLINKTABLEAREA, dwOffset);

    if (pstCacheBuffer != NULL)
    {
        MemCpy(pbBuffer, pstCacheBuffer->pbBuffer, 512);
        return TRUE;
    }

    if (InternalReadClusterLinkTableWithoutCache(dwOffset, pbBuffer) == FALSE)
    {
        return FALSE;
    }

    pstCacheBuffer = AllocateCacheBufferWithFlush(CACHE_CLUSTERLINKTABLEAREA);
    
    if (pstCacheBuffer == NULL)
    {
        return FALSE;
    }

    MemCpy(pstCacheBuffer->pbBuffer, pbBuffer, 512);
    pstCacheBuffer->dwTag = dwOffset;

    pstCacheBuffer->bChanged = FALSE;
    return TRUE;
}

static CACHEBUFFER *AllocateCacheBufferWithFlush(int iCacheTableIndex)
{
    CACHEBUFFER *pstCacheBuffer;

    pstCacheBuffer = AllocateCacheBuffer(iCacheTableIndex);
    if (pstCacheBuffer == NULL)
    {
        pstCacheBuffer = GetVictimInCacheBuffer(iCacheTableIndex);
        if (pstCacheBuffer == NULL)
        {
            Printf("[!] Cache Allocate Fail~!!!!\n");
            return NULL;
        }

        if (pstCacheBuffer->bChanged == TRUE)
        {
            switch (iCacheTableIndex)
            {
            case CACHE_CLUSTERLINKTABLEAREA:
                if (InternalWriteClusterLinkTableWithoutCache(pstCacheBuffer->dwTag, pstCacheBuffer->pbBuffer) == FALSE)
                {
                    Printf("[!] Cache Buffer Write Fail~!!!!\n");
                    return NULL;
                }
                break;

            case CACHE_DATAAREA:
                if (InternalWriteClusterWithoutCache(pstCacheBuffer->dwTag, pstCacheBuffer->pbBuffer) == FALSE)
                {
                    Printf("[!] Cache Buffer Write Fail~!!!!\n");
                    return NULL;
                }
                break;

            default:
                Printf("[!] AllocateCacheBufferWithFlush Fail\n");
                return NULL;
                break;
            }
        }
    }
    return pstCacheBuffer;
}

static BOOL WriteClusterLinkTable(DWORD dwOffset, BYTE *pbBuffer)
{
    if (gs_stFileSystemManager.bCacheEnable == FALSE)
    {
        return InternalWriteClusterLinkTableWithoutCache(dwOffset, pbBuffer);
    }
    else
    {
        return InternalWriteClusterLinkTableWithCache(dwOffset, pbBuffer);
    }
}

static BOOL InternalWriteClusterLinkTableWithoutCache(DWORD dwOffset,BYTE *pbBuffer)
{
    return gs_pfWriteHDDSector(TRUE, TRUE, dwOffset + gs_stFileSystemManager.dwClusterLinkAreaStartAddress, 1, pbBuffer);
}

static BOOL InternalWriteClusterLinkTableWithCache(DWORD dwOffset, BYTE *pbBuffer)
{
    CACHEBUFFER *pstCacheBuffer;

    pstCacheBuffer = FindCacheBuffer(CACHE_CLUSTERLINKTABLEAREA, dwOffset);

    if (pstCacheBuffer != NULL)
    {
        MemCpy(pstCacheBuffer->pbBuffer, pbBuffer, 512);

        pstCacheBuffer->bChanged = TRUE;
        return TRUE;
    }

    pstCacheBuffer = AllocateCacheBufferWithFlush(CACHE_CLUSTERLINKTABLEAREA);
    
    if (pstCacheBuffer == NULL)
    {
        return FALSE;
    }

    MemCpy(pstCacheBuffer->pbBuffer, pbBuffer, 512);
    pstCacheBuffer->dwTag = dwOffset;

    pstCacheBuffer->bChanged = TRUE;

    return TRUE;
}

static BOOL ReadCluster(DWORD dwOffset, BYTE *pbBuffer)
{
    if (gs_stFileSystemManager.bCacheEnable == FALSE)
    {
        return InternalReadClusterWithoutCache(dwOffset, pbBuffer);
    }
    else
    {
        return InternalReadClusterWithCache(dwOffset, pbBuffer);
    }
}

static BOOL InternalReadClusterWithoutCache(DWORD dwOffset, BYTE *pbBuffer)
{
    return gs_pfReadHDDSector(TRUE, TRUE, (dwOffset * FILESYSTEM_SECTORSPERCLUSTER) + gs_stFileSystemManager.dwDataAreaStartAddress, FILESYSTEM_SECTORSPERCLUSTER, pbBuffer);
}

static BOOL InternalReadClusterWithCache(DWORD dwOffset, BYTE *pbBuffer)
{
    CACHEBUFFER *pstCacheBuffer;

    pstCacheBuffer = FindCacheBuffer(CACHE_DATAAREA, dwOffset);

    if (pstCacheBuffer != NULL)
    {
        MemCpy(pbBuffer, pstCacheBuffer->pbBuffer, FILESYSTEM_CLUSTERSIZE);
        return TRUE;
    }

    if (InternalReadClusterWithoutCache(dwOffset, pbBuffer) == FALSE)
    {
        return FALSE;
    }

    pstCacheBuffer = AllocateCacheBufferWithFlush(CACHE_DATAAREA);
    if (pstCacheBuffer == NULL)
    {
        return FALSE;
    }

    MemCpy(pstCacheBuffer->pbBuffer, pbBuffer, FILESYSTEM_CLUSTERSIZE);
    pstCacheBuffer->dwTag = dwOffset;

    pstCacheBuffer->bChanged = FALSE;
    return TRUE;
}

static BOOL WriteCluster(DWORD dwOffset, BYTE *pbBuffer)
{
    if (gs_stFileSystemManager.bCacheEnable == FALSE)
    {
        return InternalWriteClusterWithoutCache(dwOffset, pbBuffer);
    }
    else
    {
        return InternalWriteClusterWithCache(dwOffset, pbBuffer);
    }
}

static BOOL InternalWriteClusterWithoutCache(DWORD dwOffset, BYTE *pbBuffer)
{
    return gs_pfWriteHDDSector(TRUE, TRUE, (dwOffset * FILESYSTEM_SECTORSPERCLUSTER) + gs_stFileSystemManager.dwDataAreaStartAddress, FILESYSTEM_SECTORSPERCLUSTER, pbBuffer);
}

static BOOL InternalWriteClusterWithCache(DWORD dwOffset, BYTE *pbBuffer)
{
    CACHEBUFFER *pstCacheBuffer;

    pstCacheBuffer = FindCacheBuffer(CACHE_DATAAREA, dwOffset);

    if (pstCacheBuffer != NULL)
    {
        MemCpy(pstCacheBuffer->pbBuffer, pbBuffer, FILESYSTEM_CLUSTERSIZE);

        pstCacheBuffer->bChanged = TRUE;

        return TRUE;
    }

    pstCacheBuffer = AllocateCacheBufferWithFlush(CACHE_DATAAREA);
    if (pstCacheBuffer == NULL)
    {
        return FALSE;
    }

    MemCpy(pstCacheBuffer->pbBuffer, pbBuffer, FILESYSTEM_CLUSTERSIZE);
    pstCacheBuffer->dwTag = dwOffset;

    pstCacheBuffer->bChanged = TRUE;

    return TRUE;
}

static DWORD FindFreeCluster(void)
{
    DWORD dwLinkCountInSector;
    DWORD dwLastSectorOffset, dwCurrentSectorOffset;
    DWORD i, j;

    if (gs_stFileSystemManager.bMounted == FALSE)
    {
        return FILESYSTEM_LASTCLUSTER;
    }

    dwLastSectorOffset = gs_stFileSystemManager.dwLastAllocatedClusterLinkSectorOffset;

    for (i = 0; i < gs_stFileSystemManager.dwClusterLinkAreaSize; i++)
    {

        if ((dwLastSectorOffset + i) == (gs_stFileSystemManager.dwClusterLinkAreaSize - 1))
        {
            dwLinkCountInSector = gs_stFileSystemManager.dwTotalClusterCount % 128;
        }
        else
        {
            dwLinkCountInSector = 128;
        }

        dwCurrentSectorOffset = (dwLastSectorOffset + i) % gs_stFileSystemManager.dwClusterLinkAreaSize;
        
        if (ReadClusterLinkTable(dwCurrentSectorOffset, gs_vbTempBuffer) == FALSE)
        {
            return FILESYSTEM_LASTCLUSTER;
        }

        for (j = 0; j < dwLinkCountInSector; j++)
        {
            if (((DWORD *)gs_vbTempBuffer)[j] == FILESYSTEM_FREECLUSTER)
            {
                break;
            }
        }

        if (j != dwLinkCountInSector)
        {
            gs_stFileSystemManager.dwLastAllocatedClusterLinkSectorOffset = dwCurrentSectorOffset;

            return (dwCurrentSectorOffset * 128) + j;
        }
    }

    return FILESYSTEM_LASTCLUSTER;
}

static BOOL SetClusterLinkData(DWORD dwClusterIndex, DWORD dwData)
{
    DWORD dwSectorOffset;

    if (gs_stFileSystemManager.bMounted == FALSE)
    {
        return FALSE;
    }

    dwSectorOffset = dwClusterIndex / 128;

    if (ReadClusterLinkTable(dwSectorOffset, gs_vbTempBuffer) == FALSE)
    {
        return FALSE;
    }

    ((DWORD *)gs_vbTempBuffer)[dwClusterIndex % 128] = dwData;

    if (WriteClusterLinkTable(dwSectorOffset, gs_vbTempBuffer) == FALSE)
    {
        return FALSE;
    }

    return TRUE;
}

static BOOL GetClusterLinkData(DWORD dwClusterIndex, DWORD *pdwData)
{
    DWORD dwSectorOffset;

    if (gs_stFileSystemManager.bMounted == FALSE)
    {
        return FALSE;
    }

    dwSectorOffset = dwClusterIndex / 128;

    if (dwSectorOffset > gs_stFileSystemManager.dwClusterLinkAreaSize)
    {
        return FALSE;
    }

    if (ReadClusterLinkTable(dwSectorOffset, gs_vbTempBuffer) == FALSE)
    {
        return FALSE;
    }

    *pdwData = ((DWORD *)gs_vbTempBuffer)[dwClusterIndex % 128];
    
    return TRUE;
}

static int FindFreeDirectoryEntry(void)
{
    DIRECTORYENTRY *pstEntry;
    int i;

    if (gs_stFileSystemManager.bMounted == FALSE)
    {
        return -1;
    }

    if (ReadCluster(0, gs_vbTempBuffer) == FALSE)
    {
        return -1;
    }

    pstEntry = (DIRECTORYENTRY *)gs_vbTempBuffer;
    
    for (i = 0; i < FILESYSTEM_MAXDIRECTORYENTRYCOUNT; i++)
    {
        if (pstEntry[i].dwStartClusterIndex == 0)
        {
            return i;
        }
    }
    
    return -1;
}

static BOOL SetDirectoryEntryData(int iIndex, DIRECTORYENTRY *pstEntry)
{
    DIRECTORYENTRY *pstRootEntry;

    if ((gs_stFileSystemManager.bMounted == FALSE) || (iIndex < 0) || (iIndex >= FILESYSTEM_MAXDIRECTORYENTRYCOUNT))
    {
        return FALSE;
    }

    if (ReadCluster(0, gs_vbTempBuffer) == FALSE)
    {
        return FALSE;
    }

    pstRootEntry = (DIRECTORYENTRY *)gs_vbTempBuffer;
    MemCpy(pstRootEntry + iIndex, pstEntry, sizeof(DIRECTORYENTRY));

    if (WriteCluster(0, gs_vbTempBuffer) == FALSE)
    {
        return FALSE;
    }
    
    return TRUE;
}


static BOOL GetDirectoryEntryData(int iIndex, DIRECTORYENTRY *pstEntry)
{
    DIRECTORYENTRY *pstRootEntry;

    if ((gs_stFileSystemManager.bMounted == FALSE) || (iIndex < 0) || (iIndex >= FILESYSTEM_MAXDIRECTORYENTRYCOUNT))
    {
        return FALSE;
    }

    if (ReadCluster(0, gs_vbTempBuffer) == FALSE)
    {
        return FALSE;
    }

    pstRootEntry = (DIRECTORYENTRY *)gs_vbTempBuffer;
    
    MemCpy(pstEntry, pstRootEntry + iIndex, sizeof(DIRECTORYENTRY));
    
    return TRUE;
}


static int FindDirectoryEntry(const char *pcFileName, DIRECTORYENTRY *pstEntry)
{
    DIRECTORYENTRY *pstRootEntry;
    int i;
    int iLength;

    if (gs_stFileSystemManager.bMounted == FALSE)
    {
        return -1;
    }

    if (ReadCluster(0, gs_vbTempBuffer) == FALSE)
    {
        return -1;
    }

    iLength = Strlen(pcFileName);
    
    pstRootEntry = (DIRECTORYENTRY *)gs_vbTempBuffer;
    
    for (i = 0; i < FILESYSTEM_MAXDIRECTORYENTRYCOUNT; i++)
    {
        if (MemCmp(pstRootEntry[i].vcFileName, pcFileName, iLength) == 0)
        {
            MemCpy(pstEntry, pstRootEntry + i, sizeof(DIRECTORYENTRY));
            return i;
        }
    }
    
    return -1;
}

void GetFileSystemInformation(FILESYSTEMMANAGER *pstManager)
{
    MemCpy(pstManager, &gs_stFileSystemManager, sizeof(gs_stFileSystemManager));
}

static void *AllocateFileDirectoryHandle(void)
{
    int i;
    FILE *pstFile;

    pstFile = gs_stFileSystemManager.pstHandlePool;
    
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

static void FreeFileDirectoryHandle(FILE *pstFile)
{
    MemSet(pstFile, 0, sizeof(FILE));

    pstFile->bType = FILESYSTEM_TYPE_FREE;
}

static BOOL CreateFile(const char *pcFileName, DIRECTORYENTRY *pstEntry, int *piDirectoryEntryIndex)
{
    DWORD dwCluster;

    dwCluster = FindFreeCluster();
    
    if((dwCluster == FILESYSTEM_LASTCLUSTER) || (SetClusterLinkData(dwCluster, FILESYSTEM_LASTCLUSTER) == FALSE))
    {
        return FALSE;
    }

    *piDirectoryEntryIndex = FindFreeDirectoryEntry();

    if(*piDirectoryEntryIndex == -1)
    {
        SetClusterLinkData(dwCluster, FILESYSTEM_FREECLUSTER);
        return FALSE;
    }

    MemCpy(pstEntry->vcFileName, pcFileName, Strlen(pcFileName) + 1);
    pstEntry->dwStartClusterIndex = dwCluster;
    pstEntry->dwFileSize = 0;

    if(SetDirectoryEntryData(*piDirectoryEntryIndex, pstEntry) == FALSE)
    {
        SetClusterLinkData(dwCluster, FILESYSTEM_FREECLUSTER);
        return FALSE;
    }

    return TRUE;
}

static BOOL FreeClusterUntilEnd(DWORD dwClusterIndex)
{
    DWORD dwCurrentClusterIndex;
    DWORD dwNextClusterIndex;

    dwCurrentClusterIndex = dwClusterIndex;

    while(dwCurrentClusterIndex != FILESYSTEM_LASTCLUSTER)
    {
        if(GetClusterLinkData(dwCurrentClusterIndex, &dwNextClusterIndex) == FALSE)
        {
            return FALSE;
        }

        if(SetClusterLinkData(dwCurrentClusterIndex, FILESYSTEM_FREECLUSTER) == FALSE)
        {
            return FALSE;
        }
        
        dwCurrentClusterIndex = dwNextClusterIndex;
    }

    return TRUE;
}

FILE *OpenFile(const char *pcFileName, const char *pcMode)
{
    DIRECTORYENTRY stEntry;
    int iDirectoryEntryOffset;
    int iFileNameLength;
    DWORD dwSecondCluster;
    FILE *pstFile;

    iFileNameLength = Strlen(pcFileName);
    if( (iFileNameLength > (sizeof(stEntry.vcFileName) -1) ) || (iFileNameLength == 0))
    {
        return NULL;
    }

    Lock(&(gs_stFileSystemManager.stMutex));

    iDirectoryEntryOffset = FindDirectoryEntry(pcFileName, &stEntry);
    if(iDirectoryEntryOffset == -1)
    {
        if(pcMode[0] == 'r')
        {
            Unlock(&(gs_stFileSystemManager.stMutex));
            return NULL;
        }

        if(CreateFile(pcFileName, &stEntry, &iDirectoryEntryOffset) == FALSE)
        {
            Unlock(&(gs_stFileSystemManager.stMutex));
            return NULL;
        }
    }
    else if(pcMode[0] == 'w')
    {
        if(GetClusterLinkData(stEntry.dwStartClusterIndex, &dwSecondCluster) == FALSE)
        {
            Unlock(&(gs_stFileSystemManager.stMutex));
            return NULL;
        }

        if(SetClusterLinkData(stEntry.dwStartClusterIndex, FILESYSTEM_LASTCLUSTER) == FALSE)
        {
            Unlock(&(gs_stFileSystemManager.stMutex));
            return NULL;
        }

        if(FreeClusterUntilEnd(dwSecondCluster) == FALSE)
        {
            Unlock(&(gs_stFileSystemManager.stMutex));
            return NULL;
        }

        stEntry.dwFileSize = 0;
        if(SetDirectoryEntryData(iDirectoryEntryOffset, &stEntry) == FALSE)
        {
            Unlock(&(gs_stFileSystemManager.stMutex));
            return NULL;
        }
    }

    pstFile = AllocateFileDirectoryHandle();
    if(pstFile == NULL)
    {
        Unlock(&(gs_stFileSystemManager.stMutex));
        return NULL;
    }

    pstFile->bType = FILESYSTEM_TYPE_FILE;
    pstFile->stFileHandle.iDirectoryEntryOffset = iDirectoryEntryOffset;
    pstFile->stFileHandle.dwFileSize = stEntry.dwFileSize;
    pstFile->stFileHandle.dwStartClusterIndex = stEntry.dwStartClusterIndex;
    pstFile->stFileHandle.dwCurrentClusterIndex = stEntry.dwStartClusterIndex;
    pstFile->stFileHandle.dwPreviousClusterIndex = stEntry.dwStartClusterIndex;
    pstFile->stFileHandle.dwCurrentOffset = 0;

    if(pcMode[0] == 'a')
    {
        SeekFile(pstFile, 0, FILESYSTEM_SEEK_END);
    }

    Unlock(&(gs_stFileSystemManager.stMutex));
    
    return pstFile;
}

DWORD ReadFile(void *pvBuffer, DWORD dwSize, DWORD dwCount, FILE *pstFile)
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

    pstFileHandle = &(pstFile->stFileHandle);

    if((pstFileHandle->dwCurrentOffset == pstFileHandle->dwFileSize) || (pstFileHandle->dwCurrentClusterIndex == FILESYSTEM_LASTCLUSTER))
    {
        return 0;
    }

    dwTotalCount = MIN(dwSize * dwCount, pstFileHandle->dwFileSize - pstFileHandle->dwCurrentOffset);

    Lock(&(gs_stFileSystemManager.stMutex));

    dwReadCount = 0;

    while(dwReadCount != dwTotalCount)
    {
        if (ReadCluster(pstFileHandle->dwCurrentClusterIndex, gs_vbTempBuffer) == FALSE)
        {
            break;
        }

        dwOffsetInCluster = pstFileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE;

        dwCopySize = MIN(FILESYSTEM_CLUSTERSIZE - dwOffsetInCluster, dwTotalCount - dwReadCount);
        
        MemCpy((char *)pvBuffer + dwReadCount, gs_vbTempBuffer + dwOffsetInCluster, dwCopySize);

        dwReadCount += dwCopySize;
        pstFileHandle->dwCurrentOffset += dwCopySize;

        if ((pstFileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE) == 0)
        {
            if (GetClusterLinkData(pstFileHandle->dwCurrentClusterIndex, &dwNextClusterIndex) == FALSE)
            {
                break;
            }

            pstFileHandle->dwPreviousClusterIndex = pstFileHandle->dwCurrentClusterIndex;
            pstFileHandle->dwCurrentClusterIndex = dwNextClusterIndex;
        }
    }

    Unlock(&(gs_stFileSystemManager.stMutex));

    return (dwReadCount / dwSize);
}

static BOOL UpdateDirectoryEntry(FILEHANDLE *pstFileHandle)
{
    DIRECTORYENTRY stEntry;

    if ((pstFileHandle == NULL) || (GetDirectoryEntryData(pstFileHandle->iDirectoryEntryOffset, &stEntry) == FALSE))
    {
        return FALSE;
    }

    stEntry.dwFileSize = pstFileHandle->dwFileSize;
    stEntry.dwStartClusterIndex = pstFileHandle->dwStartClusterIndex;

    if (SetDirectoryEntryData(pstFileHandle->iDirectoryEntryOffset, &stEntry) == FALSE)
    {
        return FALSE;
    }
    
    return TRUE;
}

DWORD WriteFile(const void *pvBuffer, DWORD dwSize, DWORD dwCount, FILE *pstFile)
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

    Lock(&(gs_stFileSystemManager.stMutex));

    dwWriteCount = 0;
    while (dwWriteCount != dwTotalCount)
    {
        if (pstFileHandle->dwCurrentClusterIndex == FILESYSTEM_LASTCLUSTER)
        {
            dwAllocatedClusterIndex = FindFreeCluster();
            
            if (dwAllocatedClusterIndex == FILESYSTEM_LASTCLUSTER)
            {
                break;
            }

            if (SetClusterLinkData(dwAllocatedClusterIndex, FILESYSTEM_LASTCLUSTER) == FALSE)
            {
                break;
            }

            if (SetClusterLinkData(pstFileHandle->dwPreviousClusterIndex, dwAllocatedClusterIndex) == FALSE)
            {
                SetClusterLinkData(dwAllocatedClusterIndex, FILESYSTEM_FREECLUSTER);
                break;
            }

            pstFileHandle->dwCurrentClusterIndex = dwAllocatedClusterIndex;

            MemSet(gs_vbTempBuffer, 0, FILESYSTEM_LASTCLUSTER);
        }

        else if (((pstFileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE) != 0) || ((dwTotalCount - dwWriteCount) < FILESYSTEM_CLUSTERSIZE))
        {
            if (ReadCluster(pstFileHandle->dwCurrentClusterIndex,  gs_vbTempBuffer) == FALSE)
            {
                break;
            }
        }

        dwOffsetInCluster = pstFileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE;

        dwCopySize = MIN(FILESYSTEM_CLUSTERSIZE - dwOffsetInCluster, dwTotalCount - dwWriteCount);
        
        MemCpy(gs_vbTempBuffer + dwOffsetInCluster, (char *)pvBuffer + dwWriteCount, dwCopySize);

        if (WriteCluster(pstFileHandle->dwCurrentClusterIndex, gs_vbTempBuffer) == FALSE)
        {
            break;
        }

        dwWriteCount += dwCopySize;
        pstFileHandle->dwCurrentOffset += dwCopySize;

        if ((pstFileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE) == 0)
        {
            if (GetClusterLinkData(pstFileHandle->dwCurrentClusterIndex, &dwNextClusterIndex) == FALSE)
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
        
        UpdateDirectoryEntry(pstFileHandle);
    }

    Unlock(&(gs_stFileSystemManager.stMutex));

    return (dwWriteCount / dwSize);
}

BOOL WriteZero(FILE *pstFile, DWORD dwCount)
{
    BYTE *pbBuffer;
    DWORD dwRemainCount;
    DWORD dwWriteCount;

    if (pstFile == NULL)
    {
        return FALSE;
    }

    pbBuffer = (BYTE *)AllocateMemory(FILESYSTEM_CLUSTERSIZE);
    if (pbBuffer == NULL)
    {
        return FALSE;
    }

    MemSet(pbBuffer, 0, FILESYSTEM_CLUSTERSIZE);
    dwRemainCount = dwCount;

    while (dwRemainCount != 0)
    {
        dwWriteCount = MIN(dwRemainCount, FILESYSTEM_CLUSTERSIZE);
        
        if (WriteFile(pbBuffer, 1, dwWriteCount, pstFile) != dwWriteCount)
        {
            FreeMemory(pbBuffer);
            return FALSE;
        }
        
        dwRemainCount -= dwWriteCount;
    }
    
    FreeMemory(pbBuffer);
    
    return TRUE;
}

int SeekFile(FILE *pstFile, int iOffset, int iOrigin)
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

    Lock(&(gs_stFileSystemManager.stMutex));

    dwCurrentClusterIndex = dwStartClusterIndex;
    
    for (i = 0; i < dwMoveCount; i++)
    {
        dwPreviousClusterIndex = dwCurrentClusterIndex;

        if (GetClusterLinkData(dwPreviousClusterIndex, &dwCurrentClusterIndex) ==
            FALSE)
        {
            Unlock(&(gs_stFileSystemManager.stMutex));
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
        Unlock(&(gs_stFileSystemManager.stMutex));

        if (WriteZero(pstFile, dwRealOffset - pstFileHandle->dwFileSize) == FALSE)
        {
            return 0;
        }
    }

    pstFileHandle->dwCurrentOffset = dwRealOffset;

    Unlock(&(gs_stFileSystemManager.stMutex));

    return 0;
}

int CloseFile(FILE *pstFile)
{
    if ((pstFile == NULL) || (pstFile->bType != FILESYSTEM_TYPE_FILE))
    {
        return -1;
    }

    FreeFileDirectoryHandle(pstFile);
    
    return 0;
}

BOOL IsFileOpened(const DIRECTORYENTRY *pstEntry)
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

int RemoveFile(const char *pcFileName)
{
    DIRECTORYENTRY stEntry;
    int iDirectoryEntryOffset;
    int iFileNameLength;

    iFileNameLength = Strlen(pcFileName);
    
    if ((iFileNameLength > (sizeof(stEntry.vcFileName) - 1)) || (iFileNameLength == 0))
    {
        return NULL;
    }

    Lock(&(gs_stFileSystemManager.stMutex));

    iDirectoryEntryOffset = FindDirectoryEntry(pcFileName, &stEntry);
    
    if (iDirectoryEntryOffset == -1)
    {
        Unlock(&(gs_stFileSystemManager.stMutex));
        return -1;
    }

    if (IsFileOpened(&stEntry) == TRUE)
    {
        Unlock(&(gs_stFileSystemManager.stMutex));
        return -1;
    }

    if (FreeClusterUntilEnd(stEntry.dwStartClusterIndex) == FALSE)
    {
        Unlock(&(gs_stFileSystemManager.stMutex));
        return -1;
    }

    MemSet(&stEntry, 0, sizeof(stEntry));
    
    if (SetDirectoryEntryData(iDirectoryEntryOffset, &stEntry) == FALSE)
    {
        Unlock(&(gs_stFileSystemManager.stMutex));
        return -1;
    }

    Unlock(&(gs_stFileSystemManager.stMutex));
    return 0;
}

DIR *OpenDirectory(const char *pcDirectoryName)
{
    DIR *pstDirectory;
    DIRECTORYENTRY *pstDirectoryBuffer;

    Lock(&(gs_stFileSystemManager.stMutex));

    pstDirectory = AllocateFileDirectoryHandle();
    
    if (pstDirectory == NULL)
    {
        Unlock(&(gs_stFileSystemManager.stMutex));
        return NULL;
    }

    pstDirectoryBuffer = (DIRECTORYENTRY *)AllocateMemory(FILESYSTEM_CLUSTERSIZE);
    
    if (pstDirectoryBuffer == NULL)
    {
        FreeFileDirectoryHandle(pstDirectory);
        Unlock(&(gs_stFileSystemManager.stMutex));
        return NULL;
    }

    if (ReadCluster(0, (BYTE *)pstDirectoryBuffer) == FALSE)
    {
        FreeFileDirectoryHandle(pstDirectory);
        FreeMemory(pstDirectoryBuffer);

        Unlock(&(gs_stFileSystemManager.stMutex));
        return NULL;
    }

    pstDirectory->bType = FILESYSTEM_TYPE_DIRECTORY;
    pstDirectory->stDirectoryHandle.iCurrentOffset = 0;
    pstDirectory->stDirectoryHandle.pstDirectoryBuffer = pstDirectoryBuffer;

    Unlock(&(gs_stFileSystemManager.stMutex));
    
    return pstDirectory;
}

struct DirectoryEntryStruct *ReadDirectory(DIR *pstDirectory)
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

    Lock(&(gs_stFileSystemManager.stMutex));

    pstEntry = pstDirectoryHandle->pstDirectoryBuffer;
    
    while (pstDirectoryHandle->iCurrentOffset < FILESYSTEM_MAXDIRECTORYENTRYCOUNT)
    {
        if (pstEntry[pstDirectoryHandle->iCurrentOffset].dwStartClusterIndex != 0)
        {
            Unlock(&(gs_stFileSystemManager.stMutex));
            return &(pstEntry[pstDirectoryHandle->iCurrentOffset++]);
        }

        pstDirectoryHandle->iCurrentOffset++;
    }

    Unlock(&(gs_stFileSystemManager.stMutex));
    
    return NULL;
}

void RewindDirectory(DIR *pstDirectory)
{
    DIRECTORYHANDLE *pstDirectoryHandle;

    if ((pstDirectory == NULL) || (pstDirectory->bType != FILESYSTEM_TYPE_DIRECTORY))
    {
        return;
    }
    
    pstDirectoryHandle = &(pstDirectory->stDirectoryHandle);

    Lock(&(gs_stFileSystemManager.stMutex));

    pstDirectoryHandle->iCurrentOffset = 0;

    Unlock(&(gs_stFileSystemManager.stMutex));
}


int CloseDirectory(DIR *pstDirectory)
{
    DIRECTORYHANDLE *pstDirectoryHandle;

    if ((pstDirectory == NULL) || (pstDirectory->bType != FILESYSTEM_TYPE_DIRECTORY))
    {
        return -1;
    }
    
    pstDirectoryHandle = &(pstDirectory->stDirectoryHandle);

    Lock(&(gs_stFileSystemManager.stMutex));

    FreeMemory(pstDirectoryHandle->pstDirectoryBuffer);
    FreeFileDirectoryHandle(pstDirectory);

    Unlock(&(gs_stFileSystemManager.stMutex));

    return 0;
}

BOOL FlushFileSystemCache(void)
{
    CACHEBUFFER *pstCacheBuffer;
    int iCacheCount;
    int i;

    if (gs_stFileSystemManager.bCacheEnable == FALSE)
    {
        return TRUE;
    }

    Lock(&(gs_stFileSystemManager.stMutex));

    GetCacheBufferAndCount(CACHE_CLUSTERLINKTABLEAREA, &pstCacheBuffer, &iCacheCount);
    
    for (i = 0; i < iCacheCount; i++)
    {
        if (pstCacheBuffer[i].bChanged == TRUE)
        {
            if (InternalWriteClusterLinkTableWithoutCache(pstCacheBuffer[i].dwTag, pstCacheBuffer[i].pbBuffer) == FALSE)
            {
                return FALSE;
            }
            
            pstCacheBuffer[i].bChanged = FALSE;
        }
    }

    GetCacheBufferAndCount(CACHE_DATAAREA, &pstCacheBuffer, &iCacheCount);
    
    for (i = 0; i < iCacheCount; i++)
    {
        if (pstCacheBuffer[i].bChanged == TRUE)
        {
            if (InternalWriteClusterWithoutCache(pstCacheBuffer[i].dwTag, pstCacheBuffer[i].pbBuffer) == FALSE)
            {
                return FALSE;
            }
            
            pstCacheBuffer[i].bChanged = FALSE;
        }
    }

    Unlock(&(gs_stFileSystemManager.stMutex));
    
    return TRUE;
}