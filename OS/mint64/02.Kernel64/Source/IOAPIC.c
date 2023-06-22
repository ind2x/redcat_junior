#include "IOAPIC.h"
#include "MPConfigurationTable.h"
#include "PIC.h"
#include "Console.h"
#include "Utility.h"


static IOAPICMANAGER gs_stIOAPICManager;

QWORD GetIOAPICBaseAddressOfISA( void )
{
    MPCONFIGURATIONMANAGER* pstMPManager;
    IOAPICENTRY* pstIOAPICEntry;
    
    if( gs_stIOAPICManager.qwIOAPICBaseAddressOfISA == NULL )
    {
        pstIOAPICEntry = FindIOAPICEntryForISA();
        if( pstIOAPICEntry != NULL )
        {
            gs_stIOAPICManager.qwIOAPICBaseAddressOfISA = pstIOAPICEntry->dwMemoryMapAddress & 0xFFFFFFFF;
        }
    }

    return gs_stIOAPICManager.qwIOAPICBaseAddressOfISA;
}


void SetIOAPICRedirectionEntry( IOREDIRECTIONTABLE* pstEntry, BYTE bAPICID, BYTE bInterruptMask, BYTE bFlagsAndDeliveryMode, BYTE bVector )
{
    MemSet( pstEntry, 0, sizeof( IOREDIRECTIONTABLE ) );
    
    pstEntry->bDestination = bAPICID;
    pstEntry->bFlagsAndDeliveryMode = bFlagsAndDeliveryMode;
    pstEntry->bInterruptMask = bInterruptMask;
    pstEntry->bVector = bVector;
}


void ReadIOAPICRedirectionTable( int iINTIN, IOREDIRECTIONTABLE* pstEntry )
{
    QWORD* pqwData;
    QWORD qwIOAPICBaseAddress;
    
    qwIOAPICBaseAddress = GetIOAPICBaseAddressOfISA();
    
    pqwData = ( QWORD* ) pstEntry;
    

    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOREGISTERSELECTOR ) =  IOAPIC_REGISTERINDEX_HIGHIOREDIRECTIONTABLE + iINTIN * 2;

    *pqwData = *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOWINDOW );
    *pqwData = *pqwData << 32;
    
 
    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOREGISTERSELECTOR ) = IOAPIC_REGISTERINDEX_LOWIOREDIRECTIONTABLE + iINTIN * 2 ;

    *pqwData |= *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOWINDOW );
}

void WriteIOAPICRedirectionTable( int iINTIN, IOREDIRECTIONTABLE* pstEntry )
{
    QWORD* pqwData;
    QWORD qwIOAPICBaseAddress;
    
    qwIOAPICBaseAddress = GetIOAPICBaseAddressOfISA();

    pqwData = ( QWORD* ) pstEntry;
    

    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOREGISTERSELECTOR ) = IOAPIC_REGISTERINDEX_HIGHIOREDIRECTIONTABLE + iINTIN * 2;

    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOWINDOW ) = *pqwData >> 32;
    

    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOREGISTERSELECTOR ) = IOAPIC_REGISTERINDEX_LOWIOREDIRECTIONTABLE + iINTIN * 2 ;

    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOWINDOW ) = *pqwData;
}


void MaskAllInterruptInIOAPIC( void )
{
    IOREDIRECTIONTABLE stEntry;
    int i;
    
    for( i = 0 ; i < IOAPIC_MAXIOREDIRECTIONTABLECOUNT ; i++ )
    {

        ReadIOAPICRedirectionTable( i, &stEntry );
        stEntry.bInterruptMask = IOAPIC_INTERRUPT_MASK;
        
        WriteIOAPICRedirectionTable( i, &stEntry );
    }
}

void InitializeIORedirectionTable( void )
{
    MPCONFIGURATIONMANAGER* pstMPManager;
    MPCONFIGURATIONTABLEHEADER* pstMPHeader;
    IOINTERRUPTASSIGNMENTENTRY* pstIOAssignmentEntry;
    IOREDIRECTIONTABLE stIORedirectionEntry;
    QWORD qwEntryAddress;
    BYTE bEntryType;
    BYTE bDestination;
    int i;


    MemSet( &gs_stIOAPICManager, 0, sizeof( gs_stIOAPICManager ) );
    
    GetIOAPICBaseAddressOfISA();
    
    for( i = 0 ; i < IOAPIC_MAXIRQTOINTINMAPCOUNT ; i++ )
    {
        gs_stIOAPICManager.vbIRQToINTINMap[ i ] = 0xFF;
    }
    
    MaskAllInterruptInIOAPIC();
    
    pstMPManager = GetMPConfigurationManager();
    pstMPHeader = pstMPManager->pstMPConfigurationTableHeader;
    qwEntryAddress = pstMPManager->qwBaseEntryStartAddress;
    
    for( i = 0 ; i < pstMPHeader->wEntryCount ; i++ )
    {
        bEntryType = *( BYTE* ) qwEntryAddress;
        
        switch( bEntryType )
        {
        case MP_ENTRYTYPE_IOINTERRUPTASSIGNMENT:
            pstIOAssignmentEntry = ( IOINTERRUPTASSIGNMENTENTRY* ) qwEntryAddress;

            if( ( pstIOAssignmentEntry->bSourceBUSID == pstMPManager->bISABusID ) &&
                ( pstIOAssignmentEntry->bInterruptType == MP_INTERRUPTTYPE_INT ) )                        
            {

                if( pstIOAssignmentEntry->bSourceBUSIRQ == 0 )
                {
                    bDestination = 0xFF;
                }
                else
                {
                    bDestination = 0x00;
                }
                
                SetIOAPICRedirectionEntry( &stIORedirectionEntry, bDestination, 0x00, IOAPIC_TRIGGERMODE_EDGE | IOAPIC_POLARITY_ACTIVEHIGH | IOAPIC_DESTINATIONMODE_PHYSICALMODE | IOAPIC_DELIVERYMODE_FIXED, PIC_IRQSTARTVECTOR + pstIOAssignmentEntry->bSourceBUSIRQ );

                WriteIOAPICRedirectionTable( pstIOAssignmentEntry->bDestinationIOAPICINTIN, &stIORedirectionEntry );
                
                gs_stIOAPICManager.vbIRQToINTINMap[ pstIOAssignmentEntry->bSourceBUSIRQ ] = pstIOAssignmentEntry->bDestinationIOAPICINTIN;                
            }                    
            
            qwEntryAddress += sizeof( IOINTERRUPTASSIGNMENTENTRY );
            break;
        
        case MP_ENTRYTYPE_PROCESSOR:
            qwEntryAddress += sizeof( PROCESSORENTRY );
            break;
            
        case MP_ENTRYTYPE_BUS:
        case MP_ENTRYTYPE_IOAPIC:
        case MP_ENTRYTYPE_LOCALINTERRUPTASSIGNMENT:
            qwEntryAddress += 8;
            break;
        }
    }  
}

void PrintIRQToINTINMap( void )
{
    int i;
    
    Printf( "=========== IRQ To I/O APIC INT IN Mapping Table ===========\n" );
    
    for( i = 0 ; i < IOAPIC_MAXIRQTOINTINMAPCOUNT ; i++ )
    {
        Printf( "[*] IRQ[%d] -> INTIN [%d]\n", i, gs_stIOAPICManager.vbIRQToINTINMap[ i ] );
    }
}

void RoutingIRQToAPICID(int iIRQ, BYTE bAPICID)
{
    int i;
    IOREDIRECTIONTABLE stEntry;

    // 범위 검사
    if (iIRQ > IOAPIC_MAXIRQTOINTINMAPCOUNT)
    {
        return;
    }

    // 설정된 I/O 리다이렉션 테이블을 읽어서 목적지(Destination) 필드만 수정
    ReadIOAPICRedirectionTable(gs_stIOAPICManager.vbIRQToINTINMap[iIRQ], &stEntry);
    stEntry.bDestination = bAPICID;
    WriteIOAPICRedirectionTable(gs_stIOAPICManager.vbIRQToINTINMap[iIRQ], &stEntry);
}