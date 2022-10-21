#include "Types.h"
#include "Keyboard.h"

void PrintString(int iX, int iY, const char *pcString);

void main(void)
{
    char vcTemp[2];
    BYTE bFlags;
    BYTE bTemp;
    int i = 0;

    PrintString(0, 10, "[*] Switch To IA-32e Mode Success..!!");
    PrintString(0, 11, "[*] IA-32e C Language Kernel Start................[Pass]");
    PrintString(0, 12, "[*] Keyboard Activate.............................[    ]");

    if(ActivateKeyboard() == TRUE)
    {
        PrintString(51, 12, "Pass");
        ChangeKeyboardLED(FALSE, FALSE, FALSE);
    }
    else
    {
        PrintString(51, 12, "Fail");
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
                    PrintString(i++, 13, vcTemp);
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