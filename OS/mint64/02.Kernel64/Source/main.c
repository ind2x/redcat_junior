#include "Types.h"

void PrintString(int iX, int iY, const char *pcString);

void main(void)
{
    PrintString(0, 10, "[*] Switch To IA-32e Mode Success..!!");
    PrintString(0, 11, "[*] IA-32e C Language Kernel Start................[Pass]");
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