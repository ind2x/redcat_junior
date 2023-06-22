#ifndef __IOAPIC_H__
#define __IOAPIC_H__

#include "Types.h"

#define IOAPIC_REGISTER_IOREGISTERSELECTOR 0x00
#define IOAPIC_REGISTER_IOWINDOW    0x10

#define IOAPIC_REGISTERINDEX_IOAPICID 0x00
#define IOAPIC_REGISTERINDEX_IOAPICVERSION 0x01
#define IOAPIC_REGISTERINDEX_IOAPICARBID 0x02
#define IOAPIC_REGISTERINDEX_LOWIOREDIRECTIONTABLE 0x10
#define IOAPIC_REGISTERINDEX_HIGHIOREDIRECTIONTABLE     0x11

#define IOAPIC_MAXIOREDIRECTIONTABLECOUNT               24

#define IOAPIC_INTERRUPT_MASK                           0x01

#define IOAPIC_TRIGGERMODE_LEVEL                        0x80
#define IOAPIC_TRIGGERMODE_EDGE                         0x00

#define IOAPIC_REMOTEIRR_EOI                            0x40
#define IOAPIC_REMOTEIRR_ACCEPT                         0x00

#define IOAPIC_POLARITY_ACTIVELOW                       0x20
#define IOAPIC_POLARITY_ACTIVEHIGH                      0x00

#define IOAPIC_DELIFVERYSTATUS_SENDPENDING              0x10
#define IOAPIC_DELIFVERYSTATUS_IDLE                     0x00

#define IOAPIC_DESTINATIONMODE_LOGICALMODE              0x08
#define IOAPIC_DESTINATIONMODE_PHYSICALMODE             0x00

#define IOAPIC_DELIVERYMODE_FIXED                       0x00
#define IOAPIC_DELIVERYMODE_LOWESTPRIORITY              0x01
#define IOAPIC_DELIVERYMODE_SMI                         0x02
#define IOAPIC_DELIVERYMODE_NMI                         0x04
#define IOAPIC_DELIVERYMODE_INIT                        0x05
#define IOAPIC_DELIVERYMODE_EXTINT                      0x07

#define IOAPIC_MAXIRQTOINTINMAPCOUNT                    16

#pragma pack(push, 1)

typedef struct IORedirectionTableStruct
{
    BYTE bVector;

    BYTE bFlagsAndDeliveryMode;

    BYTE bInterruptMask;

    BYTE vbReserved[4];

    BYTE bDestination;
} IOREDIRECTIONTABLE;

#pragma pack(pop)

typedef struct IOAPICManagerStruct
{
    QWORD qwIOAPICBaseAddressOfISA;

    BYTE vbIRQToINTINMap[IOAPIC_MAXIRQTOINTINMAPCOUNT];
} IOAPICMANAGER;

QWORD GetIOAPICBaseAddressOfISA(void);
void SetIOAPICRedirectionEntry(IOREDIRECTIONTABLE *pstEntry, BYTE bAPICID, BYTE bInterruptMask, BYTE bFlagsAndDeliveryMode, BYTE bVector);
void ReadIOAPICRedirectionTable(int iINTIN, IOREDIRECTIONTABLE *pstEntry);
void WriteIOAPICRedirectionTable(int iINTIN, IOREDIRECTIONTABLE *pstEntry);
void MaskAllInterruptInIOAPIC(void);
void InitializeIORedirectionTable(void);
void PrintIRQToINTINMap(void);

void RoutingIRQToAPICID(int iIRQ, BYTE bAPICID);

#endif /*__IOAPIC_H__*/