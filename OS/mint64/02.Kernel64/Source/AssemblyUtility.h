#ifndef __ASSEMBLYUTILITY_H__
#define __ASSEMBLYUTILITY_H__

#include "Types.h"

BYTE InPortByte(WORD wPort);
void OutPortByte(WORD wPort, BYTE bData);
void LoadGDTR(QWORD qwGDTRAddress);
void LoadTR(WORD wTSSSegmentOffset);
void LoadIDTR(QWORD qwIDTRAddress);
void EnableInterrupt(void);
void DisableInterrupt(void);
QWORD ReadRFLAGS(void);

#endif /* __ASSEMBLMYUTILITY_H__ */