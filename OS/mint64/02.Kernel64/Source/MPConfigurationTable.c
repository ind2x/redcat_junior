#include "MPConfigurationTable.h"
#include "Console.h"
#include "Utility.h"

static MPCONFIGURATIONMANAGER gs_stMPConfigurationManager;

BOOL FindMPFloatingPointerAddress(QWORD *pstAddress)
{
    char *pcMPFloatingPointer;
    QWORD qwEBDAAddress;
    QWORD qwSystemBaseMemory;

    //Printf( "[*] Extended BIOS Data Area = [0x%X] \n", ( DWORD ) ( *( WORD* ) 0x040E ) * 16 );
    
    //Printf( "[*] System Base Address = [0x%X]\n", ( DWORD ) ( *( WORD* ) 0x0413 ) * 1024 );

    qwEBDAAddress = *(WORD *)(0x040E);
    qwEBDAAddress *= 16;

    for(pcMPFloatingPointer = (char *)qwEBDAAddress; (QWORD) pcMPFloatingPointer <= (qwEBDAAddress + 1024); pcMPFloatingPointer++)
    {
        if( MemCmp( pcMPFloatingPointer, "_MP_", 4 ) == 0 )
        {
            //Printf( "[*] MP Floating Pointer Is In EBDA, [0x%X] Address\n", ( QWORD ) pcMPFloatingPointer );
            
            *pstAddress = ( QWORD ) pcMPFloatingPointer;
            return TRUE;
        }
    }

    qwSystemBaseMemory = *( WORD* ) 0x0413;
    qwSystemBaseMemory *= 1024;

    for( pcMPFloatingPointer = ( char* )(qwSystemBaseMemory - 1024); (QWORD) pcMPFloatingPointer <= qwSystemBaseMemory; pcMPFloatingPointer++ )
    {
        if( MemCmp( pcMPFloatingPointer, "_MP_", 4 ) == 0 )
        {
            //Printf( "[*] MP Floating Pointer Is In System Base Memory, [0x%X] Address\n", ( QWORD ) pcMPFloatingPointer );
            
            *pstAddress = ( QWORD ) pcMPFloatingPointer;
            
            return TRUE;
        }
    }
    
    for( pcMPFloatingPointer = (char*)0x0F0000; (QWORD) pcMPFloatingPointer < 0x0FFFFF; pcMPFloatingPointer++ )
    {
        if( MemCmp( pcMPFloatingPointer, "_MP_", 4 ) == 0 )
        {
            //Printf( "[*] MP Floating Pointer Is In ROM, [0x%X] Address\n", pcMPFloatingPointer );
            
            *pstAddress = ( QWORD ) pcMPFloatingPointer;
            
            return TRUE;
        }
    }

    return FALSE;
}

BOOL AnalysisMPConfigurationTable(void)
{
    QWORD qwMPFloatingPointerAddress;
    MPFLOATINGPOINTER* pstMPFloatingPointer;
    MPCONFIGURATIONTABLEHEADER* pstMPConfigurationHeader;
    BYTE bEntryType;
    WORD i;
    QWORD qwEntryAddress;
    PROCESSORENTRY* pstProcessorEntry;
    BUSENTRY* pstBusEntry;

    MemSet( &gs_stMPConfigurationManager, 0, sizeof( MPCONFIGURATIONMANAGER ) );
    
    gs_stMPConfigurationManager.bISABusID = 0xFF;
    
    if( FindMPFloatingPointerAddress( &qwMPFloatingPointerAddress ) == FALSE )
    {
        return FALSE;
    }
    
    pstMPFloatingPointer = ( MPFLOATINGPOINTER* ) qwMPFloatingPointerAddress;
    
    gs_stMPConfigurationManager.pstMPFloatingPointer = pstMPFloatingPointer;
    
    pstMPConfigurationHeader = ( MPCONFIGURATIONTABLEHEADER* )((QWORD) pstMPFloatingPointer->dwMPConfigurationTableAddress & 0xFFFFFFFF );
    
    if( pstMPFloatingPointer->vbMPFeatureByte[ 1 ] & MP_FLOATINGPOINTER_FEATUREBYTE2_PICMODE )
    {
        gs_stMPConfigurationManager.bUsePICMode = TRUE;
    }    
    
    gs_stMPConfigurationManager.pstMPConfigurationTableHeader = pstMPConfigurationHeader;
    
    gs_stMPConfigurationManager.qwBaseEntryStartAddress = pstMPFloatingPointer->dwMPConfigurationTableAddress + sizeof( MPCONFIGURATIONTABLEHEADER );
    
    qwEntryAddress = gs_stMPConfigurationManager.qwBaseEntryStartAddress;
    
    for( i = 0 ; i < pstMPConfigurationHeader->wEntryCount ; i++ )
    {
        bEntryType = *( BYTE* ) qwEntryAddress;
        
        switch( bEntryType )
        {
        case MP_ENTRYTYPE_PROCESSOR:
            pstProcessorEntry = ( PROCESSORENTRY* ) qwEntryAddress;
            
            if( pstProcessorEntry->bCPUFlags & MP_PROCESSOR_CPUFLAGS_ENABLE )
            {
                gs_stMPConfigurationManager.iProcessorCount++;
            }
            
            qwEntryAddress += sizeof( PROCESSORENTRY );
            break;
            
        case MP_ENTRYTYPE_BUS:
            pstBusEntry = ( BUSENTRY* ) qwEntryAddress;
            
            if( MemCmp( pstBusEntry->vcBusTypeString, MP_BUS_TYPESTRING_ISA,Strlen( MP_BUS_TYPESTRING_ISA ) ) == 0 )
            {
                gs_stMPConfigurationManager.bISABusID = pstBusEntry->bBusID;
            }
            
            qwEntryAddress += sizeof( BUSENTRY );
            break;
            
        case MP_ENTRYTYPE_IOAPIC:
        case MP_ENTRYTYPE_IOINTERRUPTASSIGNMENT:
        case MP_ENTRYTYPE_LOCALINTERRUPTASSIGNMENT:
        default:
            qwEntryAddress += 8;
            break;
        }
    }
    return TRUE;
}

MPCONFIGURATIONMANAGER *GetMPConfigurationManager(void)
{
    return &gs_stMPConfigurationManager;
}

void PrintMPConfigurationTable( void )
{
    MPCONFIGURATIONMANAGER* pstMPConfigurationManager;
    QWORD qwMPFloatingPointerAddress;
    MPFLOATINGPOINTER* pstMPFloatingPointer;
    MPCONFIGURATIONTABLEHEADER* pstMPTableHeader;
    PROCESSORENTRY* pstProcessorEntry;
    BUSENTRY* pstBusEntry;
    IOAPICENTRY* pstIOAPICEntry;
    IOINTERRUPTASSIGNMENTENTRY* pstIOAssignmentEntry;
    LOCALINTERRUPTASSIGNMENTENTRY* pstLocalAssignmentEntry;
    QWORD qwBaseEntryAddress;
    char vcStringBuffer[ 20 ];
    WORD i;
    BYTE bEntryType;
    
    char* vpcInterruptType[ 4 ] = { "INT", "NMI", "SMI", "ExtINT" };
    char* vpcInterruptFlagsPO[ 4 ] = { "Conform", "Active High", 
            "Reserved", "Active Low" };
    char* vpcInterruptFlagsEL[ 4 ] = { "Conform", "Edge-Trigger", "Reserved", 
            "Level-Trigger"};

    
    Printf( "================ MP Configuration Table Summary ================\n" );
    pstMPConfigurationManager = GetMPConfigurationManager();
    if( ( pstMPConfigurationManager->qwBaseEntryStartAddress == 0 ) &&
        ( AnalysisMPConfigurationTable() == FALSE ) )
    {
        Printf( "MP Configuration Table Analysis Fail\n" );
        return ;
    }
    Printf( "MP Configuration Table Analysis Success\n" );

    Printf( "MP Floating Pointer Address : 0x%Q\n", 
            pstMPConfigurationManager->pstMPFloatingPointer );
    Printf( "PIC Mode Support : %d\n", pstMPConfigurationManager->bUsePICMode );
    Printf( "MP Configuration Table Header Address : 0x%Q\n",
            pstMPConfigurationManager->pstMPConfigurationTableHeader );
    Printf( "Base MP Configuration Table Entry Start Address : 0x%Q\n",
            pstMPConfigurationManager->qwBaseEntryStartAddress );
    Printf( "Processor Count : %d\n", pstMPConfigurationManager->iProcessorCount );
    Printf( "ISA Bus ID : %d\n", pstMPConfigurationManager->bISABusID );

    Printf( "Press any key to continue... ('q' is exit) : " );
    if( GetCh() == 'q' )
    {
        Printf( "\n" );
        return ;
    }
    Printf( "\n" );            
    
   
    Printf( "=================== MP Floating Pointer ===================\n" );
    pstMPFloatingPointer = pstMPConfigurationManager->pstMPFloatingPointer;
    MemCpy( vcStringBuffer, pstMPFloatingPointer->vcSignature, 4 );
    vcStringBuffer[ 4 ] = '\0';
    Printf( "Signature : %s\n", vcStringBuffer );
    Printf( "MP Configuration Table Address : 0x%Q\n", 
            pstMPFloatingPointer->dwMPConfigurationTableAddress );
    Printf( "Length : %d * 16 Byte\n", pstMPFloatingPointer->bLength );
    Printf( "Version : %d\n", pstMPFloatingPointer->bRevision );
    Printf( "CheckSum : 0x%X\n", pstMPFloatingPointer->bCheckSum );
    Printf( "Feature Byte 1 : 0x%X ", pstMPFloatingPointer->vbMPFeatureByte[ 0 ] );
    // MP 설정 테이블 사용 여부 출력
    if( pstMPFloatingPointer->vbMPFeatureByte[ 0 ] == 0 )
    {
        Printf( "(Use MP Configuration Table)\n" );
    }
    else
    {
        Printf( "(Use Default Configuration)\n" );
    }    
    // PIC 모드 지원 여부 출력
    Printf( "Feature Byte 2 : 0x%X ", pstMPFloatingPointer->vbMPFeatureByte[ 1 ] );
    if( pstMPFloatingPointer->vbMPFeatureByte[ 2 ] & MP_FLOATINGPOINTER_FEATUREBYTE2_PICMODE )
    {
        Printf( "(PIC Mode Support)\n" );
    }
    else
    {
        Printf( "(Virtual Wire Mode Support)\n" );
    }
    
   
    Printf( "\n=============== MP Configuration Table Header ===============\n" );
    pstMPTableHeader = pstMPConfigurationManager->pstMPConfigurationTableHeader;
    MemCpy( vcStringBuffer, pstMPTableHeader->vcSignature, 4 );
    vcStringBuffer[ 4 ] = '\0';
    
    Printf( "Signature : %s\n", vcStringBuffer );
    Printf( "Length : %d Byte\n", pstMPTableHeader->wBaseTableLength );
    Printf( "Version : %d\n", pstMPTableHeader->bRevision );
    Printf( "CheckSum : 0x%X\n", pstMPTableHeader->bCheckSum );
    MemCpy( vcStringBuffer, pstMPTableHeader->vcOEMIDString, 8 );
    vcStringBuffer[ 8 ] = '\0';
    Printf( "OEM ID String : %s\n", vcStringBuffer );
    MemCpy( vcStringBuffer, pstMPTableHeader->vcProductIDString, 12 );
    vcStringBuffer[ 12 ] = '\0';
    Printf( "Product ID String : %s\n", vcStringBuffer );
    Printf( "OEM Table Pointer : 0x%X\n", 
             pstMPTableHeader->dwOEMTablePointerAddress );
    Printf( "OEM Table Size : %d Byte\n", pstMPTableHeader->wOEMTableSize );
    Printf( "Entry Count : %d\n", pstMPTableHeader->wEntryCount );
    Printf( "Memory Mapped I/O Address Of Local APIC : 0x%X\n",
            pstMPTableHeader->dwMemoryMapIOAddressOfLocalAPIC );
    Printf( "Extended Table Length : %d Byte\n", 
            pstMPTableHeader->wExtendedTableLength );
    Printf( "Extended Table Checksum : 0x%X\n", 
            pstMPTableHeader->bExtendedTableChecksum );
    
    Printf( "Press any key to continue... ('q' is exit) : " );
    if( GetCh() == 'q' )
    {
        Printf( "\n" );
        return ;
    }
    Printf( "\n" );
    
   
    Printf( "\n============= Base MP Configuration Table Entry =============\n" );
    qwBaseEntryAddress = pstMPFloatingPointer->dwMPConfigurationTableAddress + sizeof( MPCONFIGURATIONTABLEHEADER );
    
    for( i = 0 ; i < pstMPTableHeader->wEntryCount ; i++ )
    {
        bEntryType = *( BYTE* ) qwBaseEntryAddress;
        
        switch( bEntryType )
        {
        case MP_ENTRYTYPE_PROCESSOR:
            pstProcessorEntry = ( PROCESSORENTRY* ) qwBaseEntryAddress;
            
            Printf( "Entry Type : Processor\n" );
            
            Printf( "Local APIC ID : %d\n", pstProcessorEntry->bLocalAPICID );
            
            Printf( "Local APIC Version : 0x%X\n", pstProcessorEntry->bLocalAPICVersion );
            
            Printf( "CPU Flags : 0x%X ", pstProcessorEntry->bCPUFlags );
            
            if( pstProcessorEntry->bCPUFlags & MP_PROCESSOR_CPUFLAGS_ENABLE )
            {
                Printf( "(Enable, " );
            }
            else
            {
                Printf( "(Disable, " );
            }
            
            if( pstProcessorEntry->bCPUFlags & MP_PROCESSOR_CPUFLAGS_BSP )
            {
                Printf( "BSP)\n" );
            }
            else
            {
                Printf( "AP)\n" );
            }            
            
            Printf( "CPU Signature : 0x%X\n", pstProcessorEntry->vbCPUSignature );
            
            Printf( "Feature Flags : 0x%X\n\n", pstProcessorEntry->dwFeatureFlags );

            qwBaseEntryAddress += sizeof( PROCESSORENTRY );
            break;

        case MP_ENTRYTYPE_BUS:
            pstBusEntry = ( BUSENTRY* ) qwBaseEntryAddress;
            
            Printf( "Entry Type : Bus\n" );
            
            Printf( "Bus ID : %d\n", pstBusEntry->bBusID );
            
            MemCpy( vcStringBuffer, pstBusEntry->vcBusTypeString, 6 );
            vcStringBuffer[ 6 ] = '\0';
            
            Printf( "Bus Type String : %s\n\n", vcStringBuffer );
            
            qwBaseEntryAddress += sizeof( BUSENTRY );
            break;
            
        case MP_ENTRYTYPE_IOAPIC:
            pstIOAPICEntry = ( IOAPICENTRY* ) qwBaseEntryAddress;
            
            Printf( "Entry Type : I/O APIC\n" );
            
            Printf( "I/O APIC ID : %d\n", pstIOAPICEntry->bIOAPICID );
            
            Printf( "I/O APIC Version : 0x%X\n", pstIOAPICEntry->bIOAPICVersion );
            
            Printf( "I/O APIC Flags : 0x%X ", pstIOAPICEntry->bIOAPICFlags );
            
            if( pstIOAPICEntry->bIOAPICFlags == 1 )
            {
                Printf( "(Enable)\n" );
            }
            else
            {
                Printf( "(Disable)\n" );
            }
            
            Printf( "Memory Mapped I/O Address : 0x%X\n\n", pstIOAPICEntry->dwMemoryMapAddress );

            qwBaseEntryAddress += sizeof( IOAPICENTRY );
            break;
            
        case MP_ENTRYTYPE_IOINTERRUPTASSIGNMENT:
            pstIOAssignmentEntry = ( IOINTERRUPTASSIGNMENTENTRY* ) qwBaseEntryAddress;
            
            Printf( "Entry Type : I/O Interrupt Assignment\n" );
            
            Printf( "Interrupt Type : 0x%X ", pstIOAssignmentEntry->bInterruptType );
            
            Printf( "(%s)\n", vpcInterruptType[ pstIOAssignmentEntry->bInterruptType ] );
            
            Printf( "I/O Interrupt Flags : 0x%X ", pstIOAssignmentEntry->wInterruptFlags );
            
            Printf( "(%s, %s)\n", vpcInterruptFlagsPO[ pstIOAssignmentEntry->wInterruptFlags & 0x03 ], 
                    vpcInterruptFlagsEL[ ( pstIOAssignmentEntry->wInterruptFlags >> 2 ) & 0x03 ] );
            
            Printf( "Source BUS ID : %d\n", pstIOAssignmentEntry->bSourceBUSID );
            
            Printf( "Source BUS IRQ : %d\n", pstIOAssignmentEntry->bSourceBUSIRQ );
            
            Printf( "Destination I/O APIC ID : %d\n", 
                     pstIOAssignmentEntry->bDestinationIOAPICID );
           
            Printf( "Destination I/O APIC INTIN : %d\n\n", 
                     pstIOAssignmentEntry->bDestinationIOAPICINTIN );

            qwBaseEntryAddress += sizeof( IOINTERRUPTASSIGNMENTENTRY );
            break;
            
        case MP_ENTRYTYPE_LOCALINTERRUPTASSIGNMENT:
            pstLocalAssignmentEntry = ( LOCALINTERRUPTASSIGNMENTENTRY* )qwBaseEntryAddress;
            
            Printf( "Entry Type : Local Interrupt Assignment\n" );
            
            Printf( "Interrupt Type : 0x%X ", pstLocalAssignmentEntry->bInterruptType );
            
            Printf( "(%s)\n", vpcInterruptType[ pstLocalAssignmentEntry->bInterruptType ] );
            
            Printf( "I/O Interrupt Flags : 0x%X ", pstLocalAssignmentEntry->wInterruptFlags );

            Printf( "(%s, %s)\n", vpcInterruptFlagsPO[ pstLocalAssignmentEntry->wInterruptFlags & 0x03 ], vpcInterruptFlagsEL[ ( pstLocalAssignmentEntry->wInterruptFlags >> 2 ) & 0x03 ] );

            Printf( "Source BUS ID : %d\n", pstLocalAssignmentEntry->bSourceBUSID );
            
            Printf( "Source BUS IRQ : %d\n", pstLocalAssignmentEntry->bSourceBUSIRQ );
            
            Printf( "Destination Local APIC ID : %d\n", 
                     pstLocalAssignmentEntry->bDestinationLocalAPICID );
            
            Printf( "Destination Local APIC LINTIN : %d\n\n", 
                     pstLocalAssignmentEntry->bDestinationLocalAPICLINTIN );
            

            qwBaseEntryAddress += sizeof( LOCALINTERRUPTASSIGNMENTENTRY );
            break;
            
        default :
            Printf( "Unknown Entry Type. %d\n", bEntryType );
            break;
        }

        if( ( i != 0 ) && ( ( ( i + 1 ) % 3 ) == 0 ) )
        {
            Printf( "Press any key to continue... ('q' is exit) : " );
            if( GetCh() == 'q' )
            {
                Printf( "\n" );
                return ;
            }
            Printf( "\n" );            
        }        
    }
}

IOAPICENTRY* FindIOAPICEntryForISA( void )
{
    MPCONFIGURATIONMANAGER* pstMPManager;
    MPCONFIGURATIONTABLEHEADER* pstMPHeader;
    IOINTERRUPTASSIGNMENTENTRY* pstIOAssignmentEntry;
    IOAPICENTRY* pstIOAPICEntry;
    QWORD qwEntryAddress;
    BYTE bEntryType;
    BOOL bFind = FALSE;
    int i;
    
    pstMPHeader = gs_stMPConfigurationManager.pstMPConfigurationTableHeader;
    qwEntryAddress = gs_stMPConfigurationManager.qwBaseEntryStartAddress;

    for( i = 0 ; ( i < pstMPHeader->wEntryCount ) && ( bFind == FALSE ) ; i++ )
    {
        bEntryType = *( BYTE* ) qwEntryAddress;
        
        switch( bEntryType )
        {
        case MP_ENTRYTYPE_PROCESSOR:
            qwEntryAddress += sizeof( PROCESSORENTRY );
            break;
            
        case MP_ENTRYTYPE_BUS:
        case MP_ENTRYTYPE_IOAPIC:
        case MP_ENTRYTYPE_LOCALINTERRUPTASSIGNMENT:
            qwEntryAddress += 8;
            break;
            
        case MP_ENTRYTYPE_IOINTERRUPTASSIGNMENT:
            pstIOAssignmentEntry = ( IOINTERRUPTASSIGNMENTENTRY* ) qwEntryAddress;
            
            if( pstIOAssignmentEntry->bSourceBUSID == 
                gs_stMPConfigurationManager.bISABusID )
            {
                bFind = TRUE;
            }                    
            qwEntryAddress += sizeof( IOINTERRUPTASSIGNMENTENTRY );
            break;
        }
    }

    if( bFind == FALSE )
    {
        return NULL;
    }
    
    qwEntryAddress = gs_stMPConfigurationManager.qwBaseEntryStartAddress;
    
    for( i = 0 ; i < pstMPHeader->wEntryCount ; i++ )
    {
        bEntryType = *( BYTE* ) qwEntryAddress;
        
        switch( bEntryType )
        {
        case MP_ENTRYTYPE_PROCESSOR:
            qwEntryAddress += sizeof( PROCESSORENTRY );
            break;
            
        case MP_ENTRYTYPE_BUS:
        case MP_ENTRYTYPE_IOINTERRUPTASSIGNMENT:
        case MP_ENTRYTYPE_LOCALINTERRUPTASSIGNMENT:
            qwEntryAddress += 8;
            break;
            
        case MP_ENTRYTYPE_IOAPIC:
            pstIOAPICEntry = ( IOAPICENTRY* ) qwEntryAddress;
            
            if( pstIOAPICEntry->bIOAPICID == pstIOAssignmentEntry->bDestinationIOAPICID )
            {
                return pstIOAPICEntry;
            }
            
            qwEntryAddress += sizeof( IOINTERRUPTASSIGNMENTENTRY );
            break;
        }
    }
    
    return NULL;
}


int GetProcessorCount( void )
{
    if( gs_stMPConfigurationManager.iProcessorCount == 0 )
    {
        return 1;
    }
    
    return gs_stMPConfigurationManager.iProcessorCount;
}

