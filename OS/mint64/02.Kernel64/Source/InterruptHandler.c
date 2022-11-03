#include "InterruptHandler.h"
#include "PIC.h"
#include "Keyboard.h"
#include "Console.h"


void CommonExceptionHandler(int iVectorNumber, QWORD qwErrorCode)
{
    char vcBuffer[3];

    vcBuffer[0] = '0' + iVectorNumber / 10;
    vcBuffer[1] = '0' + iVectorNumber % 10;

    PrintStringXY(0, 0, "====================================================");
    PrintStringXY(0, 1, "                 Exception Occur~!!!!               ");
    PrintStringXY(0, 2, "                    Vector:                         ");
    PrintStringXY(27, 2, vcBuffer);
    PrintStringXY(0, 3, "====================================================");

    while (1)
        ;
}


void CommonInterruptHandler(int iVectorNumber)
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iCommonInterruptCount = 0;

    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;

    vcBuffer[8] = '0' + g_iCommonInterruptCount;
    g_iCommonInterruptCount = (g_iCommonInterruptCount + 1) % 10;
    PrintStringXY(70, 0, vcBuffer);
    //=========================================================================

    // EOI 전송
    SendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);
}

/**
 *  키보드 인터럽트의 핸들러
 */
void KeyboardHandler(int iVectorNumber)
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iKeyboardInterruptCount = 0;
    BYTE bTemp;

   
    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;

    vcBuffer[8] = '0' + g_iKeyboardInterruptCount;
    g_iKeyboardInterruptCount = (g_iKeyboardInterruptCount + 1) % 10;
    PrintStringXY(0, 0, vcBuffer);

    if (IsOutputBufferFull() == TRUE)
    {
        bTemp = GetKeyboardScanCode();
        ConvertScanCodeAndPutQueue(bTemp);
    }

    // EOI 전송
    SendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);
}