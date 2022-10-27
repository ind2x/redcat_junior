#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"

void PrintString(int iX, int iY, const char *pcString);

void main(void)
{
    char vcTemp[2];
    BYTE bFlags;
    BYTE bTemp;
    int i = 0;

    PrintString(0, 10, "[*] Switch To IA-32e Mode Success..!!");
    PrintString(0, 11, "[*] IA-32e C Language Kernel Start................[Pass]");
    PrintString(0, 12, "[*] GDT Initialize And Switch For IA-32e Mode.....[    ]");
    InitializeGDTTableAndTSS();
    LoadGDTR(GDTR_STARTADDRESS);
    PrintString(51, 12, "Pass");

    PrintString(0, 13, "[*] TSS Segment Load..............................[    ]");
    LoadTR(GDT_TSSSEGMENT);
    PrintString(51, 13, "Pass");

    PrintString(0, 14, "[*] IDT Initialize................................[    ]");
    InitializeIDTTables();
    LoadIDTR(IDTR_STARTADDRESS);
    PrintString(51, 14, "Pass");

    PrintString(0, 15, "[*] Keyboard Activate.............................[    ]");

    if(ActivateKeyboard() == TRUE)
    {
        PrintString(51, 15, "Pass");
        ChangeKeyboardLED(FALSE, FALSE, FALSE);
    }
    else
    {
        PrintString(51, 15, "Fail");
        while(1);
    }

    while(1)
    {
        if (IsOutputBufferFull() == TRUE)
        {
            bTemp = GetKeyboardScanCode();
            if(ConvertScanCodeToASCIICode(bTemp, &(vcTemp[0]), &bFlags) == TRUE)
            {
                if(bFlags & KEY_FLAGS_DOWN)
                {
                    PrintString(i++, 16, vcTemp);
                    if(vcTemp[0] == '0')
                    {
                        bTemp = bTemp / 0;
                    }
                }
            }
        }
    }
}

void PrintString(int iX, int iY, const char *pcString)
{
    CHARACTER *pstScreen = (CHARACTER *)0xB8000;
    int i;

    pstScreen += (iY * 80) + iX;
    for (int i = 0; pcString[i] != 0; i++)
    {
        pstScreen[i].bCharactor = pcString[i];
    }
}