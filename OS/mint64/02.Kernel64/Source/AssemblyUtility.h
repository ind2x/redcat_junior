#ifndef __ASSEMBLYUTILITY_H__
#define __ASSEMBLYUTILITY_H__

#include "Types.h"
#include "Task.h"

BYTE InPortByte(WORD wPort);
void OutPortByte(WORD wPort, BYTE bData);
void LoadGDTR(QWORD qwGDTRAddress);
void LoadTR(WORD wTSSSegmentOffset);
void LoadIDTR(QWORD qwIDTRAddress);
void EnableInterrupt(void);
void DisableInterrupt(void);
QWORD ReadRFLAGS(void);
QWORD ReadTSC( void );
void SwitchContext(CONTEXT *pstCurrentContext, CONTEXT *pstNextContext);

#endif /* __ASSEMBLMYUTILITY_H__ */