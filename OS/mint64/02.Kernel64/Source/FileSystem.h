#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include "Types.h"
#include "Synchronization.h"
#include "HardDisk.h"
#include "CacheManager.h"

// MINT 파일 시스템 시그니쳐
#define FILESYSTEM_SIGNATURE 0x7E38CF10
// 클러스터 당 섹터 수 -> 4KB == 8 섹터
#define FILESYSTEM_SECTORSPERCLUSTER 8
// 마지막 클러스터 임을 알리는 시그니쳐
#define FILESYSTEM_LASTCLUSTER 0xFFFFFFFF
// 할당되지 않은 클러스터
#define FILESYSTEM_FREECLUSTER 0x00

// 디렉터리 엔트리 최대 개수 128개
#define FILESYSTEM_MAXDIRECTORYENTRYCOUNT ((FILESYSTEM_SECTORSPERCLUSTER * 512) / sizeof(DIRECTORYENTRY))

// 파일 시스템 클러스터 크기는 4096 == 4KB, 8섹터
#define FILESYSTEM_CLUSTERSIZE (FILESYSTEM_SECTORSPERCLUSTER * 512)

#define FILESYSTEM_HANDLE_MAXCOUNT (TASK_MAXCOUNT * 3)

// 파일 이름의 최대 길이 24
#define FILESYSTEM_MAXFILENAMELENGTH 24

// 클러스터의 종류
#define FILESYSTEM_TYPE_FREE 0
#define FILESYSTEM_TYPE_FILE 1
#define FILESYSTEM_TYPE_DIRECTORY 2

#define FILESYSTEM_SEEK_SET 0
#define FILESYSTEM_SEEK_CUR 1
#define FILESYSTEM_SEEK_END 2

// 하드 디스크 제어에 관련된 함수 포인터
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

// 파티션 필드 정의
// MINT64 OS는 파티션을 사용하지 않으므로 파티션 영역은 0으로 초기화
typedef struct PartitionStruct
{
    // 부팅 가능 플래그
    BYTE bBootableFlag;
    // 파티션 시작 주소(CHS 방식), 크기는 3
    BYTE vbStartingCHSAddress[3];
    // 파티션 종류
    BYTE bPartitionType;
    // 마지막 CHS 어드레스, 크기는 3
    BYTE bcEndingCHSAddress[3];
    // 파티션 시작 주소 (LBA 방식)
    DWORD dwStartingLBAAddress;
    // 파티션에 포함된 총 섹터 수
    DWORD dwSizeInSector;   
} PARTITION;

// MBR 자료구조
// MBR에는 부트로더 코드, 파일 시스템 정보, 파티션 정보
// 부트코드 + 파일시스템 정보 = 446Byte
// 파티션 16B * 4개 = 64Byte
typedef struct MBRStruct
{
    // 부트로더 코드
    BYTE vbBootCode[430];

    // 파일 시스템 시그니쳐
    DWORD dwSignature;
    // 예약된 영역 섹터 수 = 0 (미사용)
    DWORD dwReservedSectorCount;
    // 클러스터 링크 테이블 영역 섹터 수
    DWORD dwClusterLinkSectorCount;
    // 클러스터 전체 개수 (데이터 영역 클러스터 개수)
    DWORD dwTotalClusterCount;

    // 파티션 테이블 4개, 0으로 초기화
    PARTITION vstPartition[4];

    // 부트 로더 시그니쳐 0x55, 0xAA
    BYTE vbBootLoaderSignature[2];
} MBR;

/**
 * 파일 정보를 저장하는 자료구조, 32바이트 크기
 * MINT64 OS에서는 최대 128개의 엔트리(파일) 생성 가능
 * 디렉터리는 루트 디렉터리만 존재
 * 파일의 이름, 파일의 시작 클러스터 위치, 파일의 크기를 저장
*/
typedef struct DirectoryEntryStruct
{
    // 파일 이름, 최대 24자
    char vcFileName[FILESYSTEM_MAXFILENAMELENGTH];
    // 파일 크기
    DWORD dwFileSize;
    // 시작 클러스터 위치
    // 0x00이면 루트 디렉터리이므로 빈 디렉터리 엔트리임을 알려줌
    DWORD dwStartClusterIndex;
} DIRECTORYENTRY;

#pragma pack(pop)

// FILE, DIR을 위한 자료구조 정의
// 파일 핸들 자료구조
typedef struct FileHandleStruct
{
    // 파일이 존재하는 디렉터리 엔트리 위치
    int iDirectoryEntryOffset;
    // 파일 크기
    DWORD dwFileSize;
    // 파일 시작 클러스터 위치
    DWORD dwStartClusterIndex;
    // 현재 I/O가 수행중인 클러스터 위치
    DWORD dwCurrentClusterIndex;
    // 현재 클러스터 이전 클러스터
    DWORD dwPreviousClusterIndex;
    // 파일 포인터의 현재 위치
    DWORD dwCurrentOffset;
} FILEHANDLE;

// 디렉터리 핸들 자료구조
typedef struct DirectoryHandleStruct
{
    // 루트 디렉터리를 저장해둔 버퍼
    DIRECTORYENTRY *pstDirectoryBuffer;
    // 디렉터리 포인터 현재 위치
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

/**
 * MINT 파일 시스템 관리 자료구조
*/
typedef struct FileSystemManagerStruct
{
    // 파일 시스템 인식 여부
    BOOL bMounted;

    // 각 영역의 섹터 수와 시작 LBA 주소
    // 예약된 영역 섹터 수
    DWORD dwReservedSectorCount;
    // 클러스터 링크 영역 시작 주소
    DWORD dwClusterLinkAreaStartAddress;
    // 클러스터 링크 영역 크기
    DWORD dwClusterLinkAreaSize;
    // 데이터 영역 시작 주소
    DWORD dwDataAreaStartAddress;
    // 데이터 영역의 클러스터 총 개수
    DWORD dwTotalClusterCount;

    // 마지막으로 클러스터를 할당한 클러스터 링크 테이블의 섹터 오프셋
    // 빈 클러스터 검색 시 비효율적이게 처음부터 검색하지 않도록 하기 위해서 추가
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