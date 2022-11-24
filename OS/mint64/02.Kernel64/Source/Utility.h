#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <stdarg.h>
#include "Types.h"

void MemSet(void *pvDestination, BYTE bData, int iSize);
int MemCpy(void *pvDestination, const void *pvSource, int iSize);
int MemCmp(const void *pvDestination, const void *pvSource, int iSize);
BOOL SetInterruptFlag(BOOL bEnableInterrupt);

int Strlen(const char *pcBuffer);
void CheckTotalRAMSize(void);
QWORD GetTotalRAMSize(void);
void ReverseString(char *pcBuffer);
long AToI(const char *pcBuffer, int iRadix);
QWORD HexStringToQword(const char *pcBuffer);
long DecimalStringToLong(const char *pcBuffer);
int IToA(long lValue, char *pcBuffer, int iRadix);
int HexToString(QWORD qwValue, char *pcBuffer);
int DecimalToString(long lValue, char *pcBuffer);
int SPrintf(char *pcBuffer, const char *pcFormatString, ...);
int VSPrintf(char *pcBuffer, const char *pcFormatString, va_list ap);

QWORD GetTickCount(void);

void Sleep(QWORD qwMillisecond);

extern volatile QWORD g_qwTickCount;

#endif /*__UTILITY_H__*/