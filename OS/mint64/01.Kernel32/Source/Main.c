#include "Types.h"
#include "Page.h"
#include "ModeSwitch.h"

void PrintString(int iX, int iY, const char* pcString);
BOOL InitializeKernel64Area(void);
BOOL ISMemoryEnough(void);
void CopyKernel64ImageTo2MB(void);

#define BOOTSTRAPPROCESSOR_FLAGADDRESS  0x7C09

void Main(void)
{
    DWORD i;
    DWORD dwEAX, dwEBX, dwECX, dwEDX;
    char vcVendorString[13];

    if( *( ( BYTE* ) BOOTSTRAPPROCESSOR_FLAGADDRESS ) == 0 )
    {
        SwitchAndExecute64bitKernel();
        while( 1 ) ;
    }

    PrintString(0, 3, "[*] Protected Mode C Language Kernel Start........[Pass]");
    PrintString(0, 4, "[*] Minimum Memory Size Check.....................[    ]");

    if(ISMemoryEnough() == FALSE)
    {
        PrintString(51, 4, "Fail");
        PrintString(0, 5, "[!] Not Enough Memory.. MINT64 OS requires over 64MB Memory...");
        while(1);
    }
    else
    {
        PrintString(51, 4, "Pass");
    }

    PrintString(0, 5, "[*] IA-32e Kernel Area Initialize.................[    ]");
    if (InitializeKernel64Area() == FALSE)
    {
        PrintString(51, 5, "Fail");
        PrintString(0, 6, "[!] Kernel Area Initialization Failed...");
        while (1);
    }
    PrintString(51, 5, "Pass");

    PrintString(0, 6, "[*] IA-32e Page Tables Initialize.................[    ]");
    InitializePageTables();
    PrintString(51, 6, "Pass");
    
    ReadCPUID(0x00, &dwEAX, &dwEBX, &dwECX, &dwEDX);
    *(DWORD *) vcVendorString = dwEBX;
    *((DWORD *)vcVendorString + 1) = dwEDX;
    *((DWORD *)vcVendorString + 2) = dwECX;
    PrintString(0, 7, "[*] Processor Vendor String.......................[            ]");
    PrintString(51, 7, vcVendorString);

    ReadCPUID(0x80000001, &dwEAX, &dwEBX, &dwECX, &dwEDX);
    PrintString(0, 8, "[*] 64bit Mode Support Check......................[    ]");
    if(dwEDX & (1 << 29)) 
    { 
        PrintString(51, 8, "Pass"); 
    }
    else 
    {
        PrintString(51, 8, "Fail");
        PrintString(0, 9, "[!] This processor does not support 64bit mode...");
        while(1);
    }

    PrintString(0, 9, "[*] Copy IA-32e Kernel To 2MB Address.............[    ]");
    CopyKernel64ImageTo2MB();
    PrintString(51, 9, "Pass");

    PrintString(0, 10, "[*] Switch To IA-32e Mode......");

    SwitchAndExecute64bitKernel();

    while (1);
}

void PrintString(int iX, int iY, const char *pcString)
{
    CHARACTER *pstScreen = (CHARACTER*) 0xB8000;
    int i;

    pstScreen += (iY * 80) + iX;
    for(int i=0; pcString[i] != 0; i++)
    {
        pstScreen[i].bCharactor = pcString[i];
    }
}

BOOL InitializeKernel64Area(void)
{
    DWORD *pdwCurrentAddress = (DWORD *)0x100000; // 1MB

    while ((DWORD)pdwCurrentAddress < 0x600000)
    {
        *pdwCurrentAddress = 0x00;
        if (*pdwCurrentAddress != 0) {
            return FALSE;
        }

        pdwCurrentAddress++;
    }
    return TRUE;
}

BOOL ISMemoryEnough(void)
{
    DWORD *pdwCurrentAddress = (DWORD *)0x100000;

    while ((DWORD)pdwCurrentAddress < 0x4000000) // 64MB
    {
        *pdwCurrentAddress = 0x12345678;
        if (*pdwCurrentAddress != 0x12345678)
        {
            return FALSE;
        }

        pdwCurrentAddress += (0x100000 / 4);
    }
    return TRUE;
}

void CopyKernel64ImageTo2MB(void)
{
    WORD wKernel32SectorCount, wTotalKernelSectorCount;
    DWORD *pdwSourceAddress, *pdwDestinationAddress;
    int i;

    wTotalKernelSectorCount = *((WORD *)0x7C05);
    wKernel32SectorCount = *((WORD *)0x7C07);

    pdwSourceAddress = (DWORD *)(0x10000 + (wKernel32SectorCount * 512));
    pdwDestinationAddress = (DWORD *)0x200000;

    for (i = 0; i < 512 * (wTotalKernelSectorCount - wKernel32SectorCount) / 4; i++)
    {
        *pdwDestinationAddress = *pdwSourceAddress;
        pdwDestinationAddress++;
        pdwSourceAddress++;
    }
}