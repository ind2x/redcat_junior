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