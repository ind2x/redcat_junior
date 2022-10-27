#include "Utility.h"

void MemSet(void *pvDestination, BYTE bData, int iSize)
{
    for(int i=0; i<iSize; i++) 
    {
        ((char*) pvDestination)[i] = bData;
    }
}

int MemCpy(void *pvDestination, const void *pvSource, int iSize)
{
    for(int i=0; i<iSize; i++)
    {
        ((char*) pvDestination)[i] = ((char*) pvSource)[i];

        return iSize;
    }
}

int MemCmp(const void *pvDestination, const void *pvSource, int iSize)
{
    char cTemp;
    for(int i=0; i<iSize; i++)
    {
        cTemp = ((char*) pvDestination)[i] - ((char*) pvSource)[i];
        if(cTemp != 0) {
            return (int) cTemp;
        }
    }

    return 0;
}