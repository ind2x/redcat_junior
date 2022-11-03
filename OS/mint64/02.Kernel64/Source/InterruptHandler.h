#ifndef __INTERRUPTHANDLER_H__
#define __INTERRUPTHANDLER_H__

#include "Types.h"

void CommonExceptionHandler(int iVectorNumber, QWORD qwErrorCode);
void CommonInterruptHandler(int iVectorNumber);
void KeyboardHandler(int iVectorNumber);

#endif /*__INTERRUPTHANDLER_H__*/