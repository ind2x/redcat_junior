#ifndef __RAMDISK_H__
#define __RAMDISK_H__

#include "Types.h"
#include "Synchronization.h"
#include "HardDisk.h"

#define RDD_TOTALSECTORCOUNT (8 * 1024 * 1024 / 512)

#pragma pack(push, 1)

typedef struct RDDManagerStruct
{
    BYTE *pbBuffer;

    DWORD dwTotalSectorCount;

    MUTEX stMutex;
} RDDMANAGER;

#pragma pack(pop)

BOOL InitializeRDD(DWORD dwTotalSectorCount);
BOOL ReadRDDInformation(BOOL bPrimary, BOOL bMaster,HDDINFORMATION *pstHDDInformation);
int ReadRDDSector(BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, char *pcBuffer);
int WriteRDDSector(BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, char *pcBuffer);

#endif /*__RAMDISK_H__*/