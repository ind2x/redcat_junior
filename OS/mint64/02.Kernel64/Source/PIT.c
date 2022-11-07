#include "PIT.h"
#include "AssemblyUtility.h"

void InitializePIT(WORD wCount, BOOL bPeriodic)
{
    OutPortByte(PIT_PORT_CONTROL, PIT_COUNTER0_ONCE);
    
    if(bPeriodic == TRUE)
    {
        OutPortByte(PIT_PORT_CONTROL, PIT_COUNTER0_PERIODIC);
    }

    OutPortByte( PIT_PORT_COUNTER0, wCount );
    OutPortByte( PIT_PORT_COUNTER0, wCount >> 8 );
}

WORD ReadCounter0(void)
{
    BYTE bHighByte, bLowByte;
    WORD wTemp = 0;

    OutPortByte(PIT_PORT_CONTROL, PIT_COUNTER0_LATCH);

    bLowByte = InPortByte(PIT_PORT_COUNTER0);
    bHighByte = InPortByte(PIT_PORT_COUNTER0);

    wTemp = bHighByte;
    wTemp = (wTemp << 8) | bLowByte;
    return wTemp;
}

void WaitUsingDirectPIT(WORD wCount)
{
    WORD wLastCounter0;
    WORD wCurrentCounter0;
    
    InitializePIT( 0, TRUE );
    
    wLastCounter0 = ReadCounter0();
    while( 1 )
    {
        wCurrentCounter0 = ReadCounter0();
        if( ( ( wLastCounter0 - wCurrentCounter0 ) & 0xFFFF ) >= wCount )
        {
            break;
        }
    }
}
