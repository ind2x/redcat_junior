#include "RTC.h"
#include "AssemblyUtility.h"

/**
 * CMOS 메모리에서 RTC가 저장한 현재 시간을 읽음
*/
void kReadRTCTime( BYTE* pbHour, BYTE* pbMinute, BYTE* pbSecond )
{
    BYTE bData;
    
    // CMOS 메모리 어드레스 포트에 HOUR 레지스터 주소 설정
    kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_HOUR);
    // CMOS 메모리 데이터 포트에서 값을 읽어옴
    bData = kInPortByte( RTC_CMOSDATA );
    *pbHour = RTC_BCDTOBINARY( bData );

    // 분
    kOutPortByte( RTC_CMOSADDRESS, RTC_ADDRESS_MINUTE );
    bData = kInPortByte( RTC_CMOSDATA );
    *pbMinute = RTC_BCDTOBINARY( bData );
    
    // 초
    kOutPortByte( RTC_CMOSADDRESS, RTC_ADDRESS_SECOND );
    bData = kInPortByte( RTC_CMOSDATA );
    *pbSecond = RTC_BCDTOBINARY( bData );
}

/**
 * 현재 날짜를 읽어오는 함수
*/
void kReadRTCDate( WORD* pwYear, BYTE* pbMonth, BYTE* pbDayOfMonth, BYTE* pbDayOfWeek )
{
    BYTE bData;

    // 년도
    kOutPortByte( RTC_CMOSADDRESS, RTC_ADDRESS_YEAR );
    bData = kInPortByte( RTC_CMOSDATA );
    *pwYear = RTC_BCDTOBINARY( bData ) + 2000;
    
    // 월
    kOutPortByte( RTC_CMOSADDRESS, RTC_ADDRESS_MONTH );
    bData = kInPortByte( RTC_CMOSDATA );
    *pbMonth = RTC_BCDTOBINARY( bData );
    
    // 일
    kOutPortByte( RTC_CMOSADDRESS, RTC_ADDRESS_DAYOFMONTH );
    bData = kInPortByte( RTC_CMOSDATA );
    *pbDayOfMonth = RTC_BCDTOBINARY( bData );
    
    // 요일
    kOutPortByte( RTC_CMOSADDRESS, RTC_ADDRESS_DAYOFWEEK );
    bData = kInPortByte( RTC_CMOSDATA );
    *pbDayOfWeek = RTC_BCDTOBINARY( bData );
}

/**
 * 정수로 저장된 요일을 문자열로 변환해주는 함수
*/
char* kConvertDayOfWeekToString( BYTE bDayOfWeek )
{
    static char* vpcDayOfWeekString[ 8 ] = { "Error", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
    
    if( bDayOfWeek >= 8 )
    {
        return vpcDayOfWeekString[ 0 ];
    }
    
    return vpcDayOfWeekString[ bDayOfWeek ];
}