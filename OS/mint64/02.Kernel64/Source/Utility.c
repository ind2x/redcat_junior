#include "Utility.h"
#include "AssemblyUtility.h"

#include <stdarg.h>

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

int Strlen(const char *pcBuffer)
{   
    int i;

    for(i=0; ; i++)
    {
        if(pcBuffer[i] == '\0')
        {
            break;
        }
    }

    return i;
}

static QWORD gs_qwTotalRAMMBSize = 0;

void CheckTotalRAMSize(void)
{
    DWORD *pdwCurrentAddress;
    DWORD dwPreviousValue;

    pdwCurrentAddress = (DWORD *)0x4000000;

    while(1)
    {
        dwPreviousValue = *pdwCurrentAddress;
        
        *pdwCurrentAddress = 0x12345678;
        if (*pdwCurrentAddress != 0x12345678)
        {
            break;
        }
        
        *pdwCurrentAddress = dwPreviousValue;
        pdwCurrentAddress += (0x400000 / 4);
    }

    gs_qwTotalRAMMBSize = (QWORD)pdwCurrentAddress / 0x100000;
}

QWORD GetTotalRAMSize(void)
{
    return gs_qwTotalRAMMBSize;
}

long AToI(const char *pcBuffer, int iRadix)
{
    long lReturn;

    switch(iRadix)
    {
        case 16:
            lReturn = HexStringToQword(pcBuffer);
            break;
        case 10:
        default:
            lReturn = DecimalStringToLong(pcBuffer);
            break;
    }
    return lReturn;
}

QWORD HexStringToQword(const char *pcBuffer)
{
    QWORD qwValue = 0;
    int i;

    for(i=0; pcBuffer[i] != '\0'; i++)
    {
        qwValue *= 16;
        if(('A' <= pcBuffer[i]) && (pcBuffer[i] <= 'Z'))
        {
            qwValue += (pcBuffer[i] - 'A') + 10;
        }
        else if(('a' <= pcBuffer[i]) && (pcBuffer[i] <= 'z'))
        {
            qwValue += (pcBuffer[i] - 'a') + 10;
        }
        else
        {
            qwValue += pcBuffer[i] - '0';
        }
    }

    return qwValue;
}

long DecimalStringToLong(const char *pcBuffer)
{
    long lValue = 0;
    int i;

    if (pcBuffer[0] == '-')
    {
        i = 1;
    }
    else
    {
        i = 0;
    }

    for (; pcBuffer[i] != '\0'; i++)
    {
        lValue *= 10;
        lValue += pcBuffer[i] - '0';
    }

    if (pcBuffer[0] == '-')
    {
        lValue = -lValue;
    }
    return lValue;
}

int IToA(long lValue, char *pcBuffer, int iRadix)
{
    int iReturn;

    switch(iRadix)
    {
        case 16:
            iReturn = HexToString(lValue, pcBuffer);
            break;
        case 10:
        default:
            iReturn = DecimalToString(lValue, pcBuffer);
            break;
    }

    return iReturn;
}

int HexToString(QWORD qwValue, char *pcBuffer)
{
    QWORD i, qwCurrentValue;

    if(qwValue == 0)
    {
        pcBuffer[0] = '0';
        pcBuffer[1] = '\0';
        return 1;
    }

    for(i=0; qwValue > 0; i++)
    {
        qwCurrentValue = qwValue % 16;
        if(qwCurrentValue >= 10)
        {
            pcBuffer[i] = 'A' + (qwCurrentValue - 10);
        }
        else
        {
            pcBuffer[i] = '0' + qwCurrentValue;
        }

        qwValue = qwValue / 16;
    }
    pcBuffer[i] = '\0';

    ReverseString(pcBuffer);
    return i;
}

int DecimalToString(long lValue, char *pcBuffer)
{
    long i;

    if (lValue == 0)
    {
        pcBuffer[0] = '0';
        pcBuffer[1] = '\0';
        return 1;
    }

    if (lValue < 0)
    {
        i = 1;
        pcBuffer[0] = '-';
        lValue = -lValue;
    }
    else
    {
        i = 0;
    }

    for (; lValue > 0; i++)
    {
        pcBuffer[i] = '0' + lValue % 10;
        lValue = lValue / 10;
    }
    pcBuffer[i] = '\0';

    if (pcBuffer[0] == '-')
    {
        ReverseString(&(pcBuffer[1]));
    }
    else
    {
        ReverseString(pcBuffer);
    }

    return i;
}

void ReverseString(char *pcBuffer)
{
    int iLength, i;
    char cTemp;

    iLength = Strlen(pcBuffer);
    for(i=0; i<iLength/2; i++)
    {
        cTemp = pcBuffer[i];
        pcBuffer[i] = pcBuffer[iLength - 1 - i];
        pcBuffer[iLength - 1 - i] = cTemp;
    }
}

int SPrintf(char *pcBuffer, const char *pcFormatString, ...)
{
    va_list ap;
    int iReturn;

    va_start(ap, pcFormatString);
    iReturn = VSPrintf(pcBuffer, pcFormatString, ap);
    va_end(ap);

    return iReturn;
}

int VSPrintf(char *pcBuffer, const char *pcFormatString, va_list ap)
{
    QWORD i, j;
    int iBufferIndex = 0;
    int iFormatLength, iCopyLength;
    char *pcCopyString;
    QWORD qwValue;
    int iValue;

    iFormatLength = Strlen(pcFormatString);

    for (i = 0; i < iFormatLength; i++)
    {
        // %로 시작하면 데이터 타입 문자로 처리
        if (pcFormatString[i] == '%')
        {
            // % 다음의 문자로 이동
            i++;
            switch (pcFormatString[i])
            {
                // 문자열 출력
            case 's':
                // 가변 인자에 들어있는 파라미터를 문자열 타입으로 변환
                pcCopyString = (char *)(va_arg(ap, char *));
                iCopyLength = Strlen(pcCopyString);
                // 문자열의 길이만큼을 출력 버퍼로 복사하고 출력한 길이만큼
                // 버퍼의 인덱스를 이동
                MemCpy(pcBuffer + iBufferIndex, pcCopyString, iCopyLength);
                iBufferIndex += iCopyLength;
                break;

                // 문자 출력
            case 'c':
                // 가변 인자에 들어있는 파라미터를 문자 타입으로 변환하여
                // 출력 버퍼에 복사하고 버퍼의 인덱스를 1만큼 이동
                pcBuffer[iBufferIndex] = (char)(va_arg(ap, int));
                iBufferIndex++;
                break;

                // 정수 출력
            case 'd':
            case 'i':
                // 가변 인자에 들어있는 파라미터를 정수 타입으로 변환하여
                // 출력 버퍼에 복사하고 출력한 길이만큼 버퍼의 인덱스를 이동
                iValue = (int)(va_arg(ap, int));
                iBufferIndex += IToA(iValue, pcBuffer + iBufferIndex, 10);
                break;

                // 4바이트 Hex 출력
            case 'x':
            case 'X':
                // 가변 인자에 들어있는 파라미터를 DWORD 타입으로 변환하여
                // 출력 버퍼에 복사하고 출력한 길이만큼 버퍼의 인덱스를 이동
                qwValue = (DWORD)(va_arg(ap, DWORD)) & 0xFFFFFFFF;
                iBufferIndex += IToA(qwValue, pcBuffer + iBufferIndex, 16);
                break;

                // 8바이트 Hex 출력
            case 'q':
            case 'Q':
            case 'p':
                // 가변 인자에 들어있는 파라미터를 QWORD 타입으로 변환하여
                // 출력 버퍼에 복사하고 출력한 길이만큼 버퍼의 인덱스를 이동
                qwValue = (QWORD)(va_arg(ap, QWORD));
                iBufferIndex += IToA(qwValue, pcBuffer + iBufferIndex, 16);
                break;

                // 위에 해당하지 않으면 문자를 그대로 출력하고 버퍼의 인덱스를
                // 1만큼 이동
            default:
                pcBuffer[iBufferIndex] = pcFormatString[i];
                iBufferIndex++;
                break;
            }
        }
        // 일반 문자열 처리
        else
        {
            // 문자를 그대로 출력하고 버퍼의 인덱스를 1만큼 이동
            pcBuffer[iBufferIndex] = pcFormatString[i];
            iBufferIndex++;
        }
    }

    // NULL을 추가하여 완전한 문자열로 만들고 출력한 문자의 길이를 반환
    pcBuffer[iBufferIndex] = '\0';
    return iBufferIndex;
}