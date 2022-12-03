#include "RAMDisk.h"
#include "Utility.h"
#include "DynamicMemory.h"
#include "Synchronization.h"

static RDDMANAGER gs_stRDDManager;

BOOL InitializeRDD(DWORD dwTotalSectorCount)
{
    MemSet(&gs_stRDDManager, 0, sizeof(gs_stRDDManager));

    gs_stRDDManager.pbBuffer = (BYTE *)AllocateMemory(dwTotalSectorCount * 512);

    if(gs_stRDDManager.pbBuffer == NULL)
    {
        return FALSE;
    }

    gs_stRDDManager.dwTotalSectorCount = dwTotalSectorCount;

    InitializeMutex(&(gs_stRDDManager.stMutex));

    return TRUE;
}

BOOL ReadRDDInformation(BOOL bPrimary, BOOL bMaster, HDDINFORMATION *pstHDDInformation)
{
    MemSet(pstHDDInformation, 0, sizeof(HDDINFORMATION));

    pstHDDInformation->dwTotalSectors = gs_stRDDManager.dwTotalSectorCount;

    MemCpy(pstHDDInformation->vwSerialNumber, "0000-0000", 9);
    MemCpy(pstHDDInformation->vwModelNumber, "MINT RAM Disk v1.0", 18);

    return TRUE;
}

int ReadRDDSector(BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, char *pcBuffer)
{
    int iRealReadCount;

    iRealReadCount = MIN(gs_stRDDManager.dwTotalSectorCount - (dwLBA + iSectorCount), iSectorCount);

    MemCpy(pcBuffer, gs_stRDDManager.pbBuffer + (dwLBA * 512), iRealReadCount * 512);

    return iRealReadCount;
}

int WriteRDDSector(BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, char *pcBuffer)
{
    int iRealWriteCount;

    iRealWriteCount = MIN(gs_stRDDManager.dwTotalSectorCount - (dwLBA + iSectorCount),iSectorCount);

    MemCpy(gs_stRDDManager.pbBuffer + (dwLBA * 512), pcBuffer, iRealWriteCount * 512);

    return iRealWriteCount;
}
