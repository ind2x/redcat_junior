#include "Types.h"

void kPrintString(int iX, int iY, const char* pcString);

void main(void)
{
    kPrintString(0,3,"C Language Kernel Started~!!!");
    while(1);
}

void kPrintString(int iX, int iY, const char *pcString)
{
    CHARACTER* pstScreen = (CHARACTER*) 0xB8000;
    int i;

    pstScreen += (iY * 80) + iX;
    for(int i=0; pcString[i] != 0; i++)
    {
        pstScreen[i].bCharactor = pcString[i];
    }
}