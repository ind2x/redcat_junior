#include "HardDisk.h"
#include "AssemblyUtility.h"
#include "Utility.h"
#include "Console.h"

static HDDMANAGER gs_stHDDManager;

/**
 * 하드 디스크 디바이스 드라이버 초기화
*/
BOOL InitializeHDD(void)
{
    // 뮤텍스 초기화
    InitializeMutex(&(gs_stHDDManager.stMutex));

    // 인터럽트 플래그 초기화
    gs_stHDDManager.bPrimaryInterruptOccur = FALSE;
    gs_stHDDManager.bSecondaryInterruptOccur = FALSE;

    // 디지털 레지스터는 인터럽트를 활성화 하는 기능이 있음
    // 비트 1이 0이면 인터럽트 활성화 
    OutPortByte(HDD_PORT_PRIMARYBASE + HDD_PORT_INDEX_DIGITALOUTPUT, 0);
    OutPortByte(HDD_PORT_SECONDARYBASE + HDD_PORT_INDEX_DIGITALOUTPUT, 0);

    // 하드 디스크 정보 요청
    if (ReadHDDInformation(TRUE, TRUE, &(gs_stHDDManager.stHDDInformation)) == FALSE)
    {
        // 정보를 읽을 수 없다면 종료
        gs_stHDDManager.bHDDDetected = FALSE;
        gs_stHDDManager.bCanWrite = FALSE;
        return FALSE;
    }

    gs_stHDDManager.bHDDDetected = TRUE;
    // 하드 디스크 검색 후 QEMU에서만 쓸 수 있도록 설정
    if (MemCmp(gs_stHDDManager.stHDDInformation.vwModelNumber, "QEMU", 4) == 0)
    {
        gs_stHDDManager.bCanWrite = TRUE;
    }
    else
    {
        gs_stHDDManager.bCanWrite = FALSE;
    }
    return TRUE;
}

/**
 * 하드 디스크의 상태를 반환
 * 상태 레지스터를 읽음
*/
static BYTE ReadHDDStatus(BOOL bPrimary)
{
    // 첫 번째 PATA I/O 포트인 경우
    if (bPrimary == TRUE)
    {
        return InPortByte(HDD_PORT_PRIMARYBASE + HDD_PORT_INDEX_STATUS);
    }
    return InPortByte(HDD_PORT_SECONDARYBASE + HDD_PORT_INDEX_STATUS);
}

/**
 * 하드 디스크의 Busy가 해제될 때 까지 대기
 * 즉, 하드디스크가 실행중인 커맨드가 끝날 때 까지 대기
*/
static BOOL WaitForHDDNoBusy(BOOL bPrimary)
{
    QWORD qwStartTickCount;
    BYTE bStatus;

    qwStartTickCount = GetTickCount();

    while ((GetTickCount() - qwStartTickCount) <= HDD_WAITTIME)
    {
        bStatus = ReadHDDStatus(bPrimary);

        if ((bStatus & HDD_STATUS_BUSY) != HDD_STATUS_BUSY)
        {
            return TRUE;
        }
        Sleep(1);
    }
    return FALSE;
}

static BOOL WaitForHDDReady(BOOL bPrimary)
{
    QWORD qwStartTickCount;
    BYTE bStatus;

    qwStartTickCount = GetTickCount();

    while ((GetTickCount() - qwStartTickCount) <= HDD_WAITTIME)
    {
        bStatus = ReadHDDStatus(bPrimary);

        if ((bStatus & HDD_STATUS_READY) == HDD_STATUS_READY)
        {
            return TRUE;
        }
        
        Sleep(1);
    }
    return FALSE;
}

void SetHDDInterruptFlag(BOOL bPrimary, BOOL bFlag)
{
    if (bPrimary == TRUE)
    {
        gs_stHDDManager.bPrimaryInterruptOccur = bFlag;
    }
    else
    {
        gs_stHDDManager.bSecondaryInterruptOccur = bFlag;
    }
}

static BOOL WaitForHDDInterrupt(BOOL bPrimary)
{
    QWORD qwTickCount;

    qwTickCount = GetTickCount();

    while (GetTickCount() - qwTickCount <= HDD_WAITTIME)
    {
        if ((bPrimary == TRUE) && (gs_stHDDManager.bPrimaryInterruptOccur == TRUE))
        {
            return TRUE;
        }
        else if ((bPrimary == FALSE) && (gs_stHDDManager.bSecondaryInterruptOccur == TRUE))
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

/**
 * 하드 디스크의 정보를 반환
*/
BOOL ReadHDDInformation(BOOL bPrimary, BOOL bMaster, HDDINFORMATION *pstHDDInformation)
{
    WORD wPortBase;
    QWORD qwLastTickCount;
    BYTE bStatus;
    BYTE bDriveFlag;
    int i;
    WORD wTemp;
    BOOL bWaitResult;

    // 어떤 PATA I/O 포트인지 확인
    if (bPrimary == TRUE)
    {
        wPortBase = HDD_PORT_PRIMARYBASE;
    }
    else
    {
        wPortBase = HDD_PORT_SECONDARYBASE;
    }

    // 뮤텍스 설정
    Lock(&(gs_stHDDManager.stMutex));

    // 아직 수행 중인 커맨드가 있다면 대기
    if (WaitForHDDNoBusy(bPrimary) == FALSE)
    {
        Unlock(&(gs_stHDDManager.stMutex));
        return FALSE;
    }

    if (bMaster == TRUE)
    {
        bDriveFlag = HDD_DRIVEANDHEAD_LBA;
    }
    else
    {
        bDriveFlag = HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;
    }
    
    OutPortByte(wPortBase + HDD_PORT_INDEX_DRIVEANDHEAD, bDriveFlag);

    if (WaitForHDDReady(bPrimary) == FALSE)
    {
        Unlock(&(gs_stHDDManager.stMutex));
        return FALSE;
    }

    SetHDDInterruptFlag(bPrimary, FALSE);

    OutPortByte(wPortBase + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_IDENTIFY);

    bWaitResult = WaitForHDDInterrupt(bPrimary);
    
    bStatus = ReadHDDStatus(bPrimary);
    
    if ((bWaitResult == FALSE) || ((bStatus & HDD_STATUS_ERROR) == HDD_STATUS_ERROR))
    {
        Unlock(&(gs_stHDDManager.stMutex));
        return FALSE;
    }

    for (i = 0; i < 512 / 2; i++)
    {
        ((WORD *)pstHDDInformation)[i] = InPortWord(wPortBase + HDD_PORT_INDEX_DATA);
    }

    SwapByteInWord(pstHDDInformation->vwModelNumber, sizeof(pstHDDInformation->vwModelNumber) / 2);
    
    SwapByteInWord(pstHDDInformation->vwSerialNumber, sizeof(pstHDDInformation->vwSerialNumber) / 2);

    Unlock(&(gs_stHDDManager.stMutex));
    return TRUE;
}


static void SwapByteInWord(WORD *pwData, int iWordCount)
{
    int i;
    WORD wTemp;

    for (i = 0; i < iWordCount; i++)
    {
        wTemp = pwData[i];
        pwData[i] = (wTemp >> 8) | (wTemp << 8);
    }
}


int ReadHDDSector(BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, char *pcBuffer)
{
    WORD wPortBase;
    int i, j;
    BYTE bDriveFlag;
    BYTE bStatus;
    long lReadCount = 0;
    BOOL bWaitResult;

    if ((gs_stHDDManager.bHDDDetected == FALSE) || (iSectorCount <= 0) || (256 < iSectorCount) || ((dwLBA + iSectorCount) >= gs_stHDDManager.stHDDInformation.dwTotalSectors))
    {
        return 0;
    }

    if (bPrimary == TRUE)
    {
        wPortBase = HDD_PORT_PRIMARYBASE;
    }
    else
    {
        wPortBase = HDD_PORT_SECONDARYBASE;
    }

    Lock(&(gs_stHDDManager.stMutex));

    if (WaitForHDDNoBusy(bPrimary) == FALSE)
    {
        Unlock(&(gs_stHDDManager.stMutex));
        return FALSE;
    }

    OutPortByte(wPortBase + HDD_PORT_INDEX_SECTORCOUNT, iSectorCount);
    OutPortByte(wPortBase + HDD_PORT_INDEX_SECTORNUMBER, dwLBA);
    OutPortByte(wPortBase + HDD_PORT_INDEX_CYLINDERLSB, dwLBA >> 8);
    OutPortByte(wPortBase + HDD_PORT_INDEX_CYLINDERMSB, dwLBA >> 16);

    if (bMaster == TRUE)
    {
        bDriveFlag = HDD_DRIVEANDHEAD_LBA;
    }
    else
    {
        bDriveFlag = HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;
    }

    OutPortByte(wPortBase + HDD_PORT_INDEX_DRIVEANDHEAD, bDriveFlag | ((dwLBA >> 24) & 0x0F));

    if (WaitForHDDReady(bPrimary) == FALSE)
    {
        Unlock(&(gs_stHDDManager.stMutex));
        return FALSE;
    }

    SetHDDInterruptFlag(bPrimary, FALSE);

    OutPortByte(wPortBase + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_READ);

    for (i = 0; i < iSectorCount; i++)
    {
        bStatus = ReadHDDStatus(bPrimary);
        
        if ((bStatus & HDD_STATUS_ERROR) == HDD_STATUS_ERROR)
        {
            Printf("Error Occur....\n");
            Unlock(&(gs_stHDDManager.stMutex));
            
            return i;
        }

        if ((bStatus & HDD_STATUS_DATAREQUEST) != HDD_STATUS_DATAREQUEST)
        {
            bWaitResult = WaitForHDDInterrupt(bPrimary);
            SetHDDInterruptFlag(bPrimary, FALSE);

            if (bWaitResult == FALSE)
            {
                Printf("Interrupt Not Occur....\n");
                Unlock(&(gs_stHDDManager.stMutex));
                return FALSE;
            }
        }

        for (j = 0; j < 512 / 2; j++)
        {
            ((WORD *)pcBuffer)[lReadCount++] = InPortWord(wPortBase + HDD_PORT_INDEX_DATA);
        }
    }

    Unlock(&(gs_stHDDManager.stMutex));
    return i;
}

int WriteHDDSector(BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, char *pcBuffer)
{
    WORD wPortBase;
    WORD wTemp;
    int i, j;
    BYTE bDriveFlag;
    BYTE bStatus;
    long lReadCount = 0;
    BOOL bWaitResult;

    if ((gs_stHDDManager.bCanWrite == FALSE) || (iSectorCount <= 0) || (256 < iSectorCount) || ((dwLBA + iSectorCount) >= gs_stHDDManager.stHDDInformation.dwTotalSectors))
    {
        return 0;
    }

    if (bPrimary == TRUE)
    {
        wPortBase = HDD_PORT_PRIMARYBASE;
    }
    else
    {
        wPortBase = HDD_PORT_SECONDARYBASE;
    }

    if (WaitForHDDNoBusy(bPrimary) == FALSE)
    {
        return FALSE;
    }

    Lock(&(gs_stHDDManager.stMutex));

    OutPortByte(wPortBase + HDD_PORT_INDEX_SECTORCOUNT, iSectorCount);
    OutPortByte(wPortBase + HDD_PORT_INDEX_SECTORNUMBER, dwLBA);
    OutPortByte(wPortBase + HDD_PORT_INDEX_CYLINDERLSB, dwLBA >> 8);
    OutPortByte(wPortBase + HDD_PORT_INDEX_CYLINDERMSB, dwLBA >> 16);

    if (bMaster == TRUE)
    {
        bDriveFlag = HDD_DRIVEANDHEAD_LBA;
    }
    else
    {
        bDriveFlag = HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;
    }

    OutPortByte(wPortBase + HDD_PORT_INDEX_DRIVEANDHEAD, bDriveFlag | ((dwLBA >> 24) & 0x0F));

    if (WaitForHDDReady(bPrimary) == FALSE)
    {
        Unlock(&(gs_stHDDManager.stMutex));
        return FALSE;
    }

    OutPortByte(wPortBase + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_WRITE);

    while (1)
    {
        bStatus = ReadHDDStatus(bPrimary);
        
        if ((bStatus & HDD_STATUS_ERROR) == HDD_STATUS_ERROR)
        {
            Unlock(&(gs_stHDDManager.stMutex));
            return 0;
        }

        if ((bStatus & HDD_STATUS_DATAREQUEST) == HDD_STATUS_DATAREQUEST)
        {
            break;
        }

        Sleep(1);
    }

    for (i = 0; i < iSectorCount; i++)
    {
        SetHDDInterruptFlag(bPrimary, FALSE);
        
        for (j = 0; j < 512 / 2; j++)
        {
            OutPortWord(wPortBase + HDD_PORT_INDEX_DATA, ((WORD *)pcBuffer)[lReadCount++]);
        }

        bStatus = ReadHDDStatus(bPrimary);
        if ((bStatus & HDD_STATUS_ERROR) == HDD_STATUS_ERROR)
        {
            Unlock(&(gs_stHDDManager.stMutex));
            return i;
        }

        if ((bStatus & HDD_STATUS_DATAREQUEST) != HDD_STATUS_DATAREQUEST)
        {
            bWaitResult = WaitForHDDInterrupt(bPrimary);
            
            SetHDDInterruptFlag(bPrimary, FALSE);
            
            if (bWaitResult == FALSE)
            {
                Unlock(&(gs_stHDDManager.stMutex));
                return FALSE;
            }
        }
    }

    Unlock(&(gs_stHDDManager.stMutex));
    return i;
}