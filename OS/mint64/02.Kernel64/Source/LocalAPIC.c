#include "LocalAPIC.h"
#include "MPConfigurationTable.h"

QWORD GetLocalAPICBaseAddress( void )
{
    MPCONFIGURATIONTABLEHEADER* pstMPHeader;

    pstMPHeader = GetMPConfigurationManager()->pstMPConfigurationTableHeader;
    return pstMPHeader->dwMemoryMapIOAddressOfLocalAPIC;
}

void EnableSoftwareLocalAPIC( void )
{
    QWORD qwLocalAPICBaseAddress;
    
    qwLocalAPICBaseAddress = GetLocalAPICBaseAddress();
    
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_SVR ) |= 0x100;
}

void SendEOIToLocalAPIC( void )
{
    QWORD qwLocalAPICBaseAddress;
    
    qwLocalAPICBaseAddress = GetLocalAPICBaseAddress();
    
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_EOI ) = 0;
}

void SetTaskPriority( BYTE bPriority )
{
    QWORD qwLocalAPICBaseAddress;
    
    qwLocalAPICBaseAddress = GetLocalAPICBaseAddress();
    
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_TASKPRIORITY ) = bPriority;
}


void InitializeLocalVectorTable( void )
{
    QWORD qwLocalAPICBaseAddress;
    DWORD dwTempValue;
    
    qwLocalAPICBaseAddress = GetLocalAPICBaseAddress();

    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_TIMER ) |= APIC_INTERRUPT_MASK;
    
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_LINT0 ) |= APIC_INTERRUPT_MASK;

    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_LINT1 ) = APIC_TRIGGERMODE_EDGE | APIC_POLARITY_ACTIVEHIGH | APIC_DELIVERYMODE_NMI;

    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_ERROR ) |= APIC_INTERRUPT_MASK;

    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_PERFORMANCEMONITORINGCOUNTER ) |= APIC_INTERRUPT_MASK;

    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_THERMALSENSOR ) |= APIC_INTERRUPT_MASK;
}