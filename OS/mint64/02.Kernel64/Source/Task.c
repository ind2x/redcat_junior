#include "Task.h"
#include "Descriptor.h"
#include "Utility.h"

void SetUpTask(TCB *pstTCB, QWORD qwID, QWORD qwFlags, QWORD qwEntryPointAddress, void *pvStackAddress, QWORD qwStackSize)
{
    MemSet(pstTCB->stContext.vqRegister, 0, sizeof(pstTCB->stContext.vqRegister));

    pstTCB->stContext.vqRegister[TASK_RSPOFFSET] = (QWORD)pvStackAddress + qwStackSize;
    pstTCB->stContext.vqRegister[TASK_RBPOFFSET] = (QWORD)pvStackAddress + qwStackSize;

    pstTCB->stContext.vqRegister[TASK_CSOFFSET] = GDT_KERNELCODESEGMENT;
    pstTCB->stContext.vqRegister[TASK_DSOFFSET] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[TASK_ESOFFSET] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[TASK_FSOFFSET] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[TASK_GSOFFSET] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[TASK_SSOFFSET] = GDT_KERNELDATASEGMENT;

    pstTCB->stContext.vqRegister[TASK_RIPOFFSET] = qwEntryPointAddress;

    pstTCB->stContext.vqRegister[TASK_RFLAGSOFFSET] |= 0x0200;

    // ID 및 스택, 그리고 플래그 저장
    pstTCB->qwID = qwID;
    pstTCB->pvStackAddress = pvStackAddress;
    pstTCB->qwStackSize = qwStackSize;
    pstTCB->qwFlags = qwFlags;
}
