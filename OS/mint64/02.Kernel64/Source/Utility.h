#ifndef __UTILITY_H__
#define __UTILITY_H__

#include "Types.h"

void MemSet(void *pvDestination, BYTE bData, int iSize);
int MemCpy(void *pvDestination, const void *pvSource, int iSize);
int MemCmp(const void *pvDestination, const void *pvSource, int iSize);

#endif /*__UTILITY_H__*/