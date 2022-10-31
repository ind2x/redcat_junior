#ifndef __MODESWITCH_H__
#define __MODESWITCH_H__

#include "Types.h"

void ReadCPUID(DWORD dwEAX, DWORD *pdwEAX, DWORD *pdwEBX, DWORD *pdwECX, DWORD *pdwEDX);

void SwitchAndExecute64bitKernel(void);

#endif /*__MODESWITCH_H__*/