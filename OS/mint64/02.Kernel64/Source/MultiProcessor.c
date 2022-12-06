#include "MultiProcessor.h"
#include "MPConfigurationTable.h"
#include "AssemblyUtility.h"
#include "LocalAPIC.h"
#include "PIT.h"
#include "Utility.h"

volatile int g_iWakeUpApplicationProcessorCount = 0;
volatile QWORD g_qwAPICIDAddress = 0;


BOOL StartUpApplicationProcessor( void )
{
    if( AnalysisMPConfigurationTable() == FALSE )
    {
        return FALSE;
    }
    
    EnableGlobalLocalAPIC();
    
    EnableSoftwareLocalAPIC();    
    
    if( WakeUpApplicationProcessor() == FALSE )
    {
        return FALSE;
    }
    
    return TRUE;
}


static BOOL WakeUpApplicationProcessor( void )
{
    MPCONFIGURATIONMANAGER* pstMPManager;
    MPCONFIGURATIONTABLEHEADER* pstMPHeader;
    QWORD qwLocalAPICBaseAddress;
    BOOL bInterruptFlag;
    int i;

    bInterruptFlag = SetInterruptFlag( FALSE );

    pstMPManager = GetMPConfigurationManager(); 
    pstMPHeader = pstMPManager->pstMPConfigurationTableHeader;
    qwLocalAPICBaseAddress = pstMPHeader->dwMemoryMapIOAddressOfLocalAPIC;


    g_qwAPICIDAddress = qwLocalAPICBaseAddress + APIC_REGISTER_APICID;

    *( DWORD* )( qwLocalAPICBaseAddress + APIC_REGISTER_ICR_LOWER ) = 
        APIC_DESTINATIONSHORTHAND_ALLEXCLUDINGSELF | APIC_TRIGGERMODE_EDGE |
        APIC_LEVEL_ASSERT | APIC_DESTINATIONMODE_PHYSICAL | APIC_DELIVERYMODE_INIT;
    
    WaitUsingDirectPIT( MSTOCOUNT( 10 ) );

    if( *( DWORD* )( qwLocalAPICBaseAddress + APIC_REGISTER_ICR_LOWER ) &
            APIC_DELIVERYSTATUS_PENDING )
    {
        InitializePIT( MSTOCOUNT( 1 ), TRUE );
        
        SetInterruptFlag( bInterruptFlag );
        return FALSE;
    }
    

    for( i = 0 ; i < 2 ; i++ )
    {
        *( DWORD* )( qwLocalAPICBaseAddress + APIC_REGISTER_ICR_LOWER ) = 
            APIC_DESTINATIONSHORTHAND_ALLEXCLUDINGSELF | APIC_TRIGGERMODE_EDGE |
            APIC_LEVEL_ASSERT | APIC_DESTINATIONMODE_PHYSICAL | 
            APIC_DELIVERYMODE_STARTUP | 0x10;
        
        WaitUsingDirectPIT( USTOCOUNT( 200 ) );
            
        if( *( DWORD* )( qwLocalAPICBaseAddress + APIC_REGISTER_ICR_LOWER ) &APIC_DELIVERYSTATUS_PENDING )
        {
            InitializePIT( MSTOCOUNT( 1 ), TRUE );
            
            SetInterruptFlag( bInterruptFlag );
            return FALSE;
        }
    }
    
    InitializePIT( MSTOCOUNT( 1 ), TRUE );
    
    SetInterruptFlag( bInterruptFlag );
    
    while( g_iWakeUpApplicationProcessorCount < ( pstMPManager->iProcessorCount - 1 ) )
    {
        Sleep( 50 );
    }    
    return TRUE;
}

BYTE GetAPICID( void )
{
    MPCONFIGURATIONTABLEHEADER* pstMPHeader;
    QWORD qwLocalAPICBaseAddress;

    if( g_qwAPICIDAddress == 0 )
    {

        pstMPHeader = GetMPConfigurationManager()->pstMPConfigurationTableHeader;
        if( pstMPHeader == NULL )
        {
            return 0;
        }
        

        qwLocalAPICBaseAddress = pstMPHeader->dwMemoryMapIOAddressOfLocalAPIC;
        
        g_qwAPICIDAddress = qwLocalAPICBaseAddress + APIC_REGISTER_APICID;
    }
    
    return *( ( DWORD* ) g_qwAPICIDAddress ) >> 24;
}