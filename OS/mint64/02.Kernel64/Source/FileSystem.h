#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include "Types.h"
#include "Synchronization.h"
#include "HardDisk.h"
#include "CacheManager.h"

#define FILESYSTEM_SIGNATURE 0x7E38CF10
#define FILESYSTEM_SECTORSPERCLUSTER 8
#define FILESYSTEM_LASTCLUSTER 0xFFFFFFFF
#define FILESYSTEM_FREECLUSTER 0x00

#define FILESYSTEM_MAXDIRECTORYENTRYCOUNT ((FILESYSTEM_SECTORSPERCLUSTER * 512) / sizeof(DIRECTORYENTRY))

#define FILESYSTEM_CLUSTERSIZE (FILESYSTEM_SECTORSPERCLUSTER * 512)

#define FILESYSTEM_HANDLE_MAXCOUNT (TASK_MAXCOUNT * 3)

#define FILESYSTEM_MAXFILENAMELENGTH 24

#define FILESYSTEM_TYPE_FREE 0
#define FILESYSTEM_TYPE_FILE 1
#define FILESYSTEM_TYPE_DIRECTORY 2

#define FILESYSTEM_SEEK_SET 0
#define FILESYSTEM_SEEK_CUR 1
#define FILESYSTEM_SEEK_END 2

typedef BOOL (*fReadHDDInformation)(BOOL bPrimary, BOOL bMaster,
                                    HDDINFORMATION *pstHDDInformation);
typedef int (*fReadHDDSector)(BOOL bPrimary, BOOL bMaster, DWORD dwLBA,
                              int iSectorCount, char *pcBuffer);
typedef int (*fWriteHDDSector)(BOOL bPrimary, BOOL bMaster, DWORD dwLBA,
                               int iSectorCount, char *pcBuffer);

#define fopen OpenFile
#define fread ReadFile
#define fwrite WriteFile
#define fseek SeekFile
#define fclose CloseFile
#define remove RemoveFile
#define opendir OpenDirectory
#define readdir ReadDirectory
#define rewinddir RewindDirectory
#define closedir CloseDirectory

#define SEEK_SET FILESYSTEM_SEEK_SET
#define SEEK_CUR FILESYSTEM_SEEK_CUR
#define SEEK_END FILESYSTEM_SEEK_END

#define size_t DWORD
#define dirent DirectoryEntryStruct
#define d_name vcFileName

#pragma pack(push, 1)

typedef struct PartitionStruct
{
    BYTE bBootableFlag;
    
    BYTE vbStartingCHSAddress[3];

    BYTE bPartitionType;

    BYTE bcEndingCHSAddress[3];

    DWORD dwStartingLBAAddress;

    DWORD dwSizeInSector;
} PARTITION;

typedef struct MBRStruct
{
    BYTE vbBootCode[430];

    DWORD dwSignature;

    DWORD dwReservedSectorCount;

    DWORD dwClusterLinkSectorCount;

    DWORD dwTotalClusterCount;

    PARTITION vstPartition[4];

    BYTE vbBootLoaderSignature[2];
} MBR;

typedef struct DirectoryEntryStruct
{
    char vcFileName[FILESYSTEM_MAXFILENAMELENGTH];

    DWORD dwFileSize;

    DWORD dwStartClusterIndex;
} DIRECTORYENTRY;

#pragma pack(pop)

typedef struct FileHandleStruct
{
    int iDirectoryEntryOffset;

    DWORD dwFileSize;

    DWORD dwStartClusterIndex;

    DWORD dwCurrentClusterIndex;

    DWORD dwPreviousClusterIndex;

    DWORD dwCurrentOffset;
} FILEHANDLE;

typedef struct DirectoryHandleStruct
{
    DIRECTORYENTRY *pstDirectoryBuffer;

    int iCurrentOffset;
} DIRECTORYHANDLE;

typedef struct FileDirectoryHandleStruct
{
    BYTE bType;

    union
    {
        FILEHANDLE stFileHandle;

        DIRECTORYHANDLE stDirectoryHandle;
    };
    
} FILE, DIR;

typedef struct FileSystemManagerStruct
{
    BOOL bMounted;

    DWORD dwReservedSectorCount;
    DWORD dwClusterLinkAreaStartAddress;
    DWORD dwClusterLinkAreaSize;
    DWORD dwDataAreaStartAddress;

    DWORD dwTotalClusterCount;

    DWORD dwLastAllocatedClusterLinkSectorOffset;

    MUTEX stMutex;

    FILE *pstHandlePool;

    BOOL bCacheEnable;
} FILESYSTEMMANAGER;


BOOL InitializeFileSystem(void);
BOOL Format(void);
BOOL Mount(void);
BOOL GetHDDInformation(HDDINFORMATION *pstInformation);

static BOOL ReadClusterLinkTable(DWORD dwOffset, BYTE *pbBuffer);
static BOOL WriteClusterLinkTable(DWORD dwOffset, BYTE *pbBuffer);
static BOOL ReadCluster(DWORD dwOffset, BYTE *pbBuffer);
static BOOL WriteCluster(DWORD dwOffset, BYTE *pbBuffer);
static DWORD FindFreeCluster(void);
static BOOL SetClusterLinkData(DWORD dwClusterIndex, DWORD dwData);
static BOOL GetClusterLinkData(DWORD dwClusterIndex, DWORD *pdwData);

static int FindFreeDirectoryEntry(void);
static BOOL SetDirectoryEntryData(int iIndex, DIRECTORYENTRY *pstEntry);
static BOOL GetDirectoryEntryData(int iIndex, DIRECTORYENTRY *pstEntry);
static int FindDirectoryEntry(const char *pcFileName, DIRECTORYENTRY *pstEntry);
void GetFileSystemInformation(FILESYSTEMMANAGER *pstManager);

static BOOL InternalReadClusterLinkTableWithoutCache(DWORD dwOffset, BYTE *pbBuffer);
static BOOL InternalReadClusterLinkTableWithCache(DWORD dwOffset, BYTE *pbBuffer);
static BOOL InternalWriteClusterLinkTableWithoutCache(DWORD dwOffset, BYTE *pbBuffer);
static BOOL InternalWriteClusterLinkTableWithCache(DWORD dwOffset, BYTE *pbBuffer);
static BOOL InternalReadClusterWithoutCache(DWORD dwOffset, BYTE *pbBuffer);
static BOOL InternalReadClusterWithCache(DWORD dwOffset, BYTE *pbBuffer);
static BOOL InternalWriteClusterWithoutCache(DWORD dwOffset, BYTE *pbBuffer);
static BOOL InternalWriteClusterWithCache(DWORD dwOffset, BYTE *pbBuffer);

static CACHEBUFFER *AllocateCacheBufferWithFlush(int iCacheTableIndex);
BOOL FlushFileSystemCache(void);

FILE *OpenFile(const char *pcFileName, const char *pcMode);
DWORD ReadFile(void *pvBuffer, DWORD dwSize, DWORD dwCount, FILE *pstFile);
DWORD WriteFile(const void *pvBuffer, DWORD dwSize, DWORD dwCount, FILE *pstFile);
int SeekFile(FILE *pstFile, int iOffset, int iOrigin);
int CloseFile(FILE *pstFile);
int RemoveFile(const char *pcFileName);
DIR *OpenDirectory(const char *pcDirectoryName);
struct DirectoryEntryStruct *ReadDirectory(DIR *pstDirectory);
void RewindDirectory(DIR *pstDirectory);
int CloseDirectory(DIR *pstDirectory);
BOOL WriteZero(FILE *pstFile, DWORD dwCount);
BOOL IsFileOpened(const DIRECTORYENTRY *pstEntry);

static void *AllocateFileDirectoryHandle(void);
static void FreeFileDirectoryHandle(FILE *pstFile);
static BOOL CreateFile(const char *pcFileName, DIRECTORYENTRY *pstEntry, int *piDirectoryEntryIndex);
static BOOL FreeClusterUntilEnd(DWORD dwClusterIndex);
static BOOL UpdateDirectoryEntry(FILEHANDLE *pstFileHandle);

#endif /*__FILESYSTEM_H__*/