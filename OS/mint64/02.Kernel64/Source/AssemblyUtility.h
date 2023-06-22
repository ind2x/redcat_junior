#ifndef __ASSEMBLYUTILITY_H__
#define __ASSEMBLYUTILITY_H__

#include "Types.h"
#include "Task.h"

BYTE InPortByte(WORD wPort);
void OutPortByte(WORD wPort, BYTE bData);

WORD InPortWord(WORD wPort);
void OutPortWord(WORD wPort, WORD wData);

void LoadGDTR(QWORD qwGDTRAddress);
void LoadTR(WORD wTSSSegmentOffset);
void LoadIDTR(QWORD qwIDTRAddress);
void EnableInterrupt(void);
void DisableInterrupt(void);
QWORD ReadRFLAGS(void);
QWORD ReadTSC( void );
void SwitchContext(CONTEXT *pstCurrentContext, CONTEXT *pstNextContext);
void Hlt(void);

BOOL TestAndSet(volatile BYTE *pbDestination, BYTE bCompare, BYTE bSource);

void InitializeFPU(void);
void SaveFPUContext(void *pvFPUContext);
void LoadFPUContext(void *pvFPUContext);
void SetTS(void);
void ClearTS(void);

void EnableGlobalLocalAPIC(void);

void Pause(void);

#endif /* __ASSEMBLMYUTILITY_H__ */