#include "PIT.h"
#include "AssemblyUtility.h"

/**
 * PIT 컨트롤러 초기화 함수
 * 타이머 초기 값과 주기를 인자로 받음
*/
void InitializePIT(WORD wCount, BOOL bPeriodic)
{
    // 카운트를 멈추고 모드 0으로 재설정
    OutPortByte(PIT_PORT_CONTROL, PIT_COUNTER0_ONCE);
    
    // 주기적으로 검사한다면 모드 2로 재설정
    if(bPeriodic == TRUE)
    {
        OutPortByte(PIT_PORT_CONTROL, PIT_COUNTER0_PERIODIC);
    }

    // 타이머 초기 값 설정, 하위 -> 상위
    OutPortByte( PIT_PORT_COUNTER0, wCount );
    OutPortByte( PIT_PORT_COUNTER0, wCount >> 8 );
}

/**
 * 카운터0을 직접 읽는 함수
*/
WORD ReadCounter0(void)
{
    BYTE bHighByte, bLowByte;
    WORD wTemp = 0;

    // LATCH 커맨드 전송
    OutPortByte(PIT_PORT_CONTROL, PIT_COUNTER0_LATCH);

    // 하위 -> 상위로 읽음
    bLowByte = InPortByte(PIT_PORT_COUNTER0);
    bHighByte = InPortByte(PIT_PORT_COUNTER0);

    wTemp = bHighByte;
    wTemp = (wTemp << 8) | bLowByte;
    return wTemp;
}

/**
 * 카운터 0을 직접 설정하여 일정 시간 대기
*/
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
