#include "Descriptor.h"
#include "Utility.h"

void InitializeGDTTableAndTSS(void)
{
    GDTR *pstGDTR = (GDTR *) GDTR_STARTADDRESS; // 0x142000
    GDTENTRY8 *pstEntry = (GDTENTRY8 *) (GDTR_STARTADDRESS + sizeof(GDTR));
    TSSSEGMENT *pstTSS = (TSSSEGMENT*) ((QWORD) pstEntry + GDT_TABLESIZE);
    int i;

    pstGDTR->wLimit = GDT_TABLESIZE - 1;
    pstGDTR->qwBaseAddress = (QWORD) pstEntry;
    
    SetGDTEntry8(&(pstEntry[0]), 0, 0, 0, 0, 0);    // NULL Descriptor
    SetGDTEntry8(&(pstEntry[1]), 0, 0xFFFFF, GDT_FLAGS_UPPER_CODE, GDT_FLAGS_LOWER_KERNELCODE, GDT_TYPE_CODE);     // 64bit CODE
    SetGDTEntry8(&(pstEntry[2]), 0, 0xFFFFF, GDT_FLAGS_UPPER_DATA, GDT_FLAGS_LOWER_KERNELDATA, GDT_TYPE_DATA);      // 64bit DATA
    SetGDTEntry16((GDTENTRY16 *)&(pstEntry[3]), (QWORD)pstTSS, sizeof(TSSSEGMENT) - 1, GDT_FLAGS_UPPER_TSS, GDT_FLAGS_LOWER_TSS, GDT_TYPE_TSS);

    InitializeTSSSegment(pstTSS);
}

void SetGDTEntry8(GDTENTRY8 *pstEntry, DWORD dwBaseAddress, DWORD dwLimit, BYTE bUpperFlags, BYTE bLowerFlags, BYTE bType)
{
    pstEntry->wLowerLimit = dwLimit & 0xFFFF;
    pstEntry->wLowerBaseAddress = dwBaseAddress & 0xFFFF;
    pstEntry->bUpperBaseAddress1 = (dwBaseAddress >> 16) & 0xFF;
    pstEntry->bTypeAndLowerFlag = bLowerFlags | bType;
    pstEntry->bUpperLimitAndUpperFlag = ((dwLimit >> 16) & 0xFF) | bUpperFlags;
    pstEntry->bUpperBaseAddress2 = (dwBaseAddress >> 24) & 0xFF;
}

void SetGDTEntry16(GDTENTRY16 *pstEntry, QWORD qwBaseAddress, DWORD dwLimit, BYTE bUpperFlags, BYTE bLowerFlags, BYTE bType)
{
    pstEntry->wLowerLimit = dwLimit & 0xFFFF;
    pstEntry->wLowerBaseAddress = qwBaseAddress & 0xFFFF;
    pstEntry->bMiddleBaseAddress1 = (qwBaseAddress >> 16) & 0xFF;
    pstEntry->bTypeAndLowerFlag = bLowerFlags | bType;
    pstEntry->bUpperLimitAndUpperFlag = ((dwLimit >> 16) & 0xFF) |
                                        bUpperFlags;
    pstEntry->bMiddleBaseAddress2 = (qwBaseAddress >> 24) & 0xFF;
    pstEntry->dwUpperBaseAddress = qwBaseAddress >> 32;
    pstEntry->dwReserved = 0;
}

void InitializeTSSSegment(TSSSEGMENT *pstTSS)
{
    MemSet(pstTSS, 0, sizeof(TSSSEGMENT));
    pstTSS->qwIST[0] = IST_STARTADDRESS + IST_SIZE;
    pstTSS->wIOMapBaseAddress = 0xFFFF;
}

void InitializeIDTTables(void)
{
    IDTR *pstIDTR;
    IDTENTRY *pstEntry;
    int i;

    // IDTR의 시작 어드레스
    pstIDTR = (IDTR *)IDTR_STARTADDRESS;
    // IDT 테이블의 정보 생성
    pstEntry = (IDTENTRY *)(IDTR_STARTADDRESS + sizeof(IDTR));
    pstIDTR->qwBaseAddress = (QWORD)pstEntry;
    pstIDTR->wLimit = IDT_TABLESIZE - 1;

    // 0~99까지 벡터를 모두 DummyHandler로 연결
    for (i = 0; i < IDT_MAXENTRYCOUNT; i++)
    {
        SetIDTEntry(&(pstEntry[i]), DummyHandler, 0x08, IDT_FLAGS_IST1,
                     IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    }
}

void SetIDTEntry(IDTENTRY *pstEntry, void *pvHandler, WORD wSelector, BYTE bIST, BYTE bFlags, BYTE bType)
{
    pstEntry->wLowerBaseAddress = (QWORD)pvHandler & 0xFFFF;
    pstEntry->wSegmentSelector = wSelector;
    pstEntry->bIST = bIST & 0x3;
    pstEntry->bTypeAndFlags = bType | bFlags;
    pstEntry->wMiddleBaseAddress = ((QWORD)pvHandler >> 16) & 0xFFFF;
    pstEntry->dwUpperBaseAddress = (QWORD)pvHandler >> 32;
    pstEntry->dwReserved = 0;
}

void DummyHandler(void)
{
    PrintString(0, 0, "====================================================");
    PrintString(0, 1, "          Dummy Interrupt Handler Execute~!!!       ");
    PrintString(0, 2, "           Interrupt or Exception Occur~!!!!        ");
    PrintString(0, 3, "====================================================");

    while (1);
}