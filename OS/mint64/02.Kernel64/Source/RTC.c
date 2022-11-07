#include "RTC.h"
#include "AssemblyUtility.h"

void ReadRTCTime( BYTE* pbHour, BYTE* pbMinute, BYTE* pbSecond )
{
    BYTE bData;
    
    OutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_HOUR);

    bData = InPortByte( RTC_CMOSDATA );
    *pbHour = RTC_BCDTOBINARY( bData );

    OutPortByte( RTC_CMOSADDRESS, RTC_ADDRESS_MINUTE );

    bData = InPortByte( RTC_CMOSDATA );
    *pbMinute = RTC_BCDTOBINARY( bData );
    
    OutPortByte( RTC_CMOSADDRESS, RTC_ADDRESS_SECOND );
    
    bData = InPortByte( RTC_CMOSDATA );
    *pbSecond = RTC_BCDTOBINARY( bData );
}

void ReadRTCDate( WORD* pwYear, BYTE* pbMonth, BYTE* pbDayOfMonth, BYTE* pbDayOfWeek )
{
    BYTE bData;
    
    OutPortByte( RTC_CMOSADDRESS, RTC_ADDRESS_YEAR );
    bData = InPortByte( RTC_CMOSDATA );
    *pwYear = RTC_BCDTOBINARY( bData ) + 2000;
    
    OutPortByte( RTC_CMOSADDRESS, RTC_ADDRESS_MONTH );
    bData = InPortByte( RTC_CMOSDATA );
    *pbMonth = RTC_BCDTOBINARY( bData );
    
    OutPortByte( RTC_CMOSADDRESS, RTC_ADDRESS_DAYOFMONTH );
    bData = InPortByte( RTC_CMOSDATA );
    *pbDayOfMonth = RTC_BCDTOBINARY( bData );
    
    OutPortByte( RTC_CMOSADDRESS, RTC_ADDRESS_DAYOFWEEK );
    bData = InPortByte( RTC_CMOSDATA );
    *pbDayOfWeek = RTC_BCDTOBINARY( bData );
}

char* ConvertDayOfWeekToString( BYTE bDayOfWeek )
{
    static char* vpcDayOfWeekString[ 8 ] = { "Error", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
    
    if( bDayOfWeek >= 8 )
    {
        return vpcDayOfWeekString[ 0 ];
    }
    
    return vpcDayOfWeekString[ bDayOfWeek ];
}