#include "Utility.h"
#include "AssemblyUtility.h"

void MemSet(void *pvDestination, BYTE bData, int iSize)
{
    int i;

    for (i = 0; i < iSize; i++)
    {
        ((char *)pvDestination)[i] = bData;
    }
}

int MemCpy(void *pvDestination, const void *pvSource, int iSize)
{
    int i;

    for (i = 0; i < iSize; i++)
    {
        ((char *)pvDestination)[i] = ((char *)pvSource)[i];
    }

    return iSize;
}


int MemCmp(const void *pvDestination, const void *pvSource, int iSize)
{
    int i;
    char cTemp;

    for (i = 0; i < iSize; i++)
    {
        cTemp = ((char *)pvDestination)[i] - ((char *)pvSource)[i];
        if (cTemp != 0)
        {
            return (int)cTemp;
        }
    }
    return 0;
}

BOOL SetInterruptFlag(BOOL bEnableInterrupt)
{
    QWORD qwRFLAGS;

    qwRFLAGS = ReadRFLAGS();
    if (bEnableInterrupt == TRUE)
    {
        EnableInterrupt();
    }
    else
    {
        DisableInterrupt();
    }

    // 이전 RFLAGS 레지스터의 IF 비트(비트 9)를 확인하여 이전의 인터럽트 상태를 반환
    if (qwRFLAGS & 0x0200)
    {
        return TRUE;
    }
    return FALSE;
}
