#include "Utility.h"
#include "AssemblyUtility.h"
#include <stdarg.h>

volatile QWORD g_qwTickCount = 0;

void MemSet(void *pvDestination, BYTE bData, int iSize)
{
    int i;
    QWORD qwData;
    int iRemainByteStartOffset;
    
    // 8 바이트 데이터를 채움
    qwData = 0;
    for( i = 0 ; i < 8 ; i++ )
    {
        qwData = ( qwData << 8 ) | bData;
    }
    
    // 8 바이트씩 먼저 채움
    for( i = 0 ; i < ( iSize / 8 ) ; i++ )
    {
        ( ( QWORD* ) pvDestination )[ i ] = qwData;
    }
    
    // 8 바이트씩 채우고 남은 부분을 마무리
    iRemainByteStartOffset = i * 8;
    for( i = 0 ; i < ( iSize % 8 ) ; i++ )
    {
        ( ( char* ) pvDestination )[ iRemainByteStartOffset++ ] = bData;
    }
}

int MemCpy(void *pvDestination, const void *pvSource, int iSize)
{
    int i;
    int iRemainByteStartOffset;
    
    // 8 바이트씩 먼저 복사
    for( i = 0 ; i < ( iSize / 8 ) ; i++ )
    {
        ( ( QWORD* ) pvDestination )[ i ] = ( ( QWORD* ) pvSource )[ i ];
    }
    
    // 8 바이트씩 채우고 남은 부분을 마무리
    iRemainByteStartOffset = i * 8;
    for( i = 0 ; i < ( iSize % 8 ) ; i++ )
    {
        ( ( char* ) pvDestination )[ iRemainByteStartOffset ] = 
            ( ( char* ) pvSource )[ iRemainByteStartOffset ];
        iRemainByteStartOffset++;
    }
    return iSize;
}

int MemCmp(const void *pvDestination, const void *pvSource, int iSize)
{
    int i, j;
    int iRemainByteStartOffset;
    QWORD qwValue;
    char cValue;
    
    // 8 바이트씩 먼저 비교
    for( i = 0 ; i < ( iSize / 8 ) ; i++ )
    {
        qwValue = ( ( QWORD* ) pvDestination )[ i ] - ( ( QWORD* ) pvSource )[ i ];

        // 틀린 위치를 정확하게 찾아서 그 값을 반환
        if( qwValue != 0 )
        {
            for( i = 0 ; i < 8 ; i++ )
            {
                if( ( ( qwValue >> ( i * 8 ) ) & 0xFF ) != 0 )
                {
                    return ( qwValue >> ( i * 8 ) ) & 0xFF;
                }
            }
        }
    }
    
    // 8 바이트씩 채우고 남은 부분을 마무리
    iRemainByteStartOffset = i * 8;
    for( i = 0 ; i < ( iSize % 8 ) ; i++ )
    {
        cValue = ( ( char* ) pvDestination )[ iRemainByteStartOffset ] -
            ( ( char* ) pvSource )[ iRemainByteStartOffset ];
        if( cValue != 0 )
        {
            return cValue;
        }
        iRemainByteStartOffset++;
    }    
    return 0;
}

/**
 * 이전 인터럽트 상태 반환
*/
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

/**
 * atoi --> 문자열을 숫자로 변환
*/
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

/**
 * 16진수 문자열을 정수로 바꾸는 함수
 * FF -> 255
*/
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

/**
 * 10진수 문자열을 정수로 바꾸는 함수 (문자열 -> 숫자)
 * 10 -> 10
*/
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

/**
 * itoa -> 숫자를 문자열로
*/
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

/**
 * 16진수 숫자를 문자열 형태로 변환
*/
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

/**
 * 10진수 숫자를 문자열 형태로 변환
*/
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

    for (; lValue > 0; i++) // 1 -> 10 -> 100 ... 자리 순으로 연산 후 삽입
    {
        pcBuffer[i] = '0' + lValue % 10;
        lValue = lValue / 10;
    }
    pcBuffer[i] = '\0';

    if (pcBuffer[0] == '-') // 음수면 - 부호 빼고 뒤집어서 저장
    {
        ReverseString(&(pcBuffer[1]));
    }
    else
    {
        ReverseString(pcBuffer); // 버퍼의 문자열을 뒤집어서 순서대로 저장함
    }

    return i;
}

/**
 * 문자열의 순서를 뒤집는 함수
*/
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
    QWORD i, j, k;
    int iBufferIndex = 0;
    int iFormatLength, iCopyLength;
    char *pcCopyString;
    QWORD qwValue;
    int iValue;
    double dValue;

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
            case 'f':
                dValue = (double) (va_arg(ap, double));
                
                dValue += 0.005;

                pcBuffer[iBufferIndex] = '0' + (QWORD) (dValue * 100) % 10;
                pcBuffer[iBufferIndex + 1] = '0' + (QWORD) (dValue * 10) % 10;
                pcBuffer[iBufferIndex + 2] = '.';
                
                for(k=0; ; k++)
                {
                    if(((QWORD) dValue == 0) && (k != 0))
                    {
                        break;
                    }

                    pcBuffer[ iBufferIndex + 3 + k ] = '0' + ( ( QWORD ) dValue % 10 );
                    dValue = dValue / 10;
                }
                
                pcBuffer[ iBufferIndex + 3 + k ] = '\0';
                ReverseString( pcBuffer + iBufferIndex );
                iBufferIndex += 3 + k;
                break;
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

QWORD GetTickCount(void)
{
    return g_qwTickCount;
}

void Sleep(QWORD qwMillisecond)
{
    QWORD qwLastTickCount;

    qwLastTickCount = g_qwTickCount;

    while ((g_qwTickCount - qwLastTickCount) <= qwMillisecond)
    {
        Schedule();
    }
}