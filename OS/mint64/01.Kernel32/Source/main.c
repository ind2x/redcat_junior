#include "Types.h"

void kPrintString(int iX, int iY, const char* pcString);
BOOL kInitializeKernel64Area(void);
BOOL kISMemoryEnough(void);

void main(void)
{
    DWORD i;

    kPrintString(0, 3, "[*] C Language Kernel Start...................... [Pass]");
    kPrintString(0, 4, "[*] Minimum Memory Size Check.................... [    ]");

    if(kISMemoryEnough() == FALSE)
    {
        kPrintString(51, 4, "Fail");
        kPrintString(0, 5, "[ERROR] Not Enough Memory.. MINT64 OS requires over 64MB Memory...");
        while(1);
    }
    else
    {
        kPrintString(51, 4, "Pass");
    }

    kPrintString(0, 5, "[*] IA-32e Kernel Area Initialize.................[    ]");
    if (kInitializeKernel64Area() == FALSE)
    {
        kPrintString(51, 5, "Fail");
        kPrintString(0, 6, "[ERROR] Kernel Area Initialization Failed...");
        while (1);
    }
    kPrintString(51, 5, "Pass");
    while(1);
        
}

void kPrintString(int iX, int iY, const char *pcString)
{
    CHARACTER *pstScreen = (CHARACTER*) 0xB8000;
    int i;

    pstScreen += (iY * 80) + iX;
    for(int i=0; pcString[i] != 0; i++)
    {
        pstScreen[i].bCharactor = pcString[i];
    }
}

BOOL kInitializeKernel64Area(void)
{
    DWORD *pdwCurrentAddress = (DWORD*) 0x100000; // 1MB

    while((DWORD)pdwCurrentAddress < 0x600000)
    {
        *pdwCurrentAddress = 0x00;
        if (*pdwCurrentAddress != 0) {
            return FALSE;
        }

        pdwCurrentAddress++;
    }
    return TRUE;
}

BOOL kISMemoryEnough(void)
{
    DWORD *pdwCurrentAddress = (DWORD*) 0x100000;

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