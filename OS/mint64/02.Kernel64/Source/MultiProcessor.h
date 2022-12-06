#ifndef __MULTIPROCESSOR_H__
#define __MULTIPROCESSOR_H__

#include "Types.h"

#define BOOTSTRAPPROCESSOR_FLAGADDRESS      0x7C09
#define MAXPROCESSORCOUNT                   16


BOOL StartUpApplicationProcessor( void );
BYTE GetAPICID( void );
static BOOL WakeUpApplicationProcessor( void );

#endif /*__MULTIPROCESSOR_H__*/