#ifndef __INTERRUPTHANDLER_H__
#define __INTERRUPTHANDLER_H__

#include "Types.h"
#include "MultiProcessor.h"

#define INTERRUPT_MAXVECTORCOUNT    16
#define INTERRUPT_LOADBALANCINGIVIDOR   10

typedef struct InterruptManagerStruct
{
    QWORD vvqwCoreInterruptCount[MAXPROCESSORCOUNT][INTERRUPT_MAXVECTORCOUNT];

    BOOL bUseLoadBalancing;

    BOOL bSymmetricIOMode;
} INTERRUPTMANAGER;

void SetSymmetricIOMode(BOOL bSymmetricIOMode);
void SetInterruptLoadBalancing(BOOL bUseLoadBalancing);
void IncreaseInterruptCount(int iIRQ);
void SendEOI(int iIRQ);
INTERRUPTMANAGER* GetInterruptManager(void);
void ProcessLoadBalancing(int iIRQ);

void CommonExceptionHandler(int iVectorNumber, QWORD qwErrorCode);
void CommonInterruptHandler(int iVectorNumber);
void KeyboardHandler(int iVectorNumber);
void TimerHandler(int iVectorNumber);

void DeviceNotAvailableHandler(int iVectorNumber);

void HDDHandler(int iVectorNumber);

#endif /*__INTERRUPTHANDLER_H__*/