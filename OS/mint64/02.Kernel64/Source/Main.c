#include "string.h"
#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"
#include "PIC.h"
#include "Console.h"
#include "ConsoleShell.h"
#include "AssemblyUtility.h"
#include "Utility.h"
#include "PIT.h"
#include "Task.h"
#include "DynamicMemory.h"
#include "HardDisk.h"
#include "FileSystem.h"
#include "SerialPort.h"
#include "MultiProcessor.h"
#include "LocalAPIC.h"
#include "IOAPIC.h"
#include "InterruptHandler.h"
#include "VBE.h"
#include "2DGraphics.h"

// x를 절대값으로 변환하는 매크로
#define ABS(x) (((x) >= 0) ? (x) : -(x))

void MainForApplicationProcessor(void);
void GetRandomXY(int *piX, int *piY);
COLOR GetRandomColor(void);
void DrawWindowFrame(int iX, int iY, int iWidth, int iHeight, const char *pcTitle);
void StartGraphicModeTest();

void Main(void)
{
    int iCursorX, iCursorY;

    if(*((BYTE*) BOOTSTRAPPROCESSOR_FLAGADDRESS) == 0)
    {
        MainForApplicationProcessor();
    }
    
    *((BYTE*) BOOTSTRAPPROCESSOR_FLAGADDRESS) = 0;

    // 콘솔 초기화, (0,10) 좌표로 커서 업데이트
    InitializeConsole(0, 10);

    // 11번째 줄(0,10) 부터 출력
    Printf("==============>> Switch To IA-32e Mode Success <<==============\n");
    Printf("[*] IA-32e C Language Kernel Start................[Pass]\n");
    Printf("[*] Initialize Console............................[Pass]\n");
    
    // 현재 커서의 위치 저장
    GetCursor(&iCursorX, &iCursorY);
    Printf("[*] GDT Initialize And Switch For IA-32e Mode.....[    ]");
    InitializeGDTTableAndTSS();
    LoadGDTR(GDTR_STARTADDRESS);
    SetCursor(51, iCursorY++);
    Printf("Pass\n");

    Printf("[*] TSS Segment Load..............................[    ]");
    LoadTR(GDT_TSSSEGMENT);
    SetCursor(51, iCursorY++);
    Printf("Pass\n");

    Printf("[*] IDT Initialize................................[    ]");
    InitializeIDTTables();
    LoadIDTR(IDTR_STARTADDRESS);
    SetCursor(51, iCursorY++);
    Printf("Pass\n");

    Printf("[*] Total RAM Size Check..........................[    ]");
    CheckTotalRAMSize();
    SetCursor(51, iCursorY++);
    Printf("Pass], Size = %d MB\n", GetTotalRAMSize());

    Printf("[*] TCB Pool And Scheduler Initialize.............[Pass]\n");
    iCursorY++;
    InitializeScheduler();

    Printf("[*] Dynamic Memory Initialize.....................[Pass]\n");
    iCursorY++;
    InitializeDynamicMemory();

    // 1ms마다 주기적으로 인터럽트 설정
    InitializePIT(MSTOCOUNT(1), 1);

    Printf("[*] Keyboard Activate And Queue Initialize........[    ]");
    if (InitializeKeyboard() == TRUE)
    {
        SetCursor(51, iCursorY++);
        Printf("Pass\n");
        ChangeKeyboardLED(FALSE, FALSE, FALSE);
    }
    else
    {
        SetCursor(51, iCursorY++);
        Printf("Fail\n");
        while (1);
    }

    Printf("[*] PIC Controller And Interrupt Initialize.......[    ]");
    
    InitializePIC();
    MaskPICInterrupt(0);
    EnableInterrupt();
    SetCursor(51, iCursorY++);
    Printf("Pass\n");

    Printf("[*] File System Initialize........................[    ]");
    if(InitializeFileSystem() == TRUE)
    {
        SetCursor(51, iCursorY++);
        Printf("Pass\n");
    }
    else
    {
        SetCursor(51, iCursorY++);
        Printf("Fail\n");
    }

    Printf("[*] Serial Port Initialize........................[Pass]");
    iCursorY++;
    InitializeSerialPort();

    CreateTask(TASK_FLAGS_LOWEST | TASK_FLAGS_THREAD | TASK_FLAGS_SYSTEM | TASK_FLAGS_IDLE, 0, 0, (QWORD)IdleTask, GetAPICID());

    // 그래픽 모드가 아니면 콘솔 셸 실행
    if (*(BYTE *)VBE_STARTGRAPHICMODEFLAGADDRESS == 0)
    {
        StartConsoleShell();
    }
    // 그래픽 모드이면 그래픽 모드 테스트 함수 실행
    else
    {
        StartGraphicModeTest();
    }
}

void MainForApplicationProcessor(void)
{
    QWORD qwTickCount;

    LoadGDTR( GDTR_STARTADDRESS );

    LoadTR( GDT_TSSSEGMENT + ( GetAPICID() * sizeof( GDTENTRY16 ) ) );

    LoadIDTR( IDTR_STARTADDRESS );

    InitializeScheduler();

    EnableSoftwareLocalAPIC();

    SetTaskPriority(0);

    InitializeLocalVectorTable();

    EnableInterrupt();

    Printf("[*] Application Processor[APIC ID: %d] Is Activated\n", GetAPICID());

    IdleTask();
}

/**
 *  임의의 X, Y 좌표를 반환
 */
void GetRandomXY(int *piX, int *piY)
{
    int iRandom;

    // X좌표를 계산
    iRandom = Random();
    *piX = ABS(iRandom) % 1000;

    // Y좌표를 계산
    iRandom = Random();
    *piY = ABS(iRandom) % 700;
}

/**
 *  임의의 색을 반환
 */
COLOR GetRandomColor(void)
{
    int iR, iG, iB;
    int iRandom;

    iRandom = Random();
    iR = ABS(iRandom) % 256;

    iRandom = Random();
    iG = ABS(iRandom) % 256;

    iRandom = Random();
    iB = ABS(iRandom) % 256;

    return RGB(iR, iG, iB);
}

/**
 *  윈도우 프레임을 그림
 */
void DrawWindowFrame(int iX, int iY, int iWidth, int iHeight, const char *pcTitle)
{
    char *pcTestString1 = "This is MINT64 OS's window prototype~!!!";
    char *pcTestString2 = "Coming soon~!!!";

    // 윈도우 프레임의 가장자리를 그림, 2 픽셀 두께
    DrawRect(iX, iY, iX + iWidth, iY + iHeight, RGB(109, 218, 22), FALSE);
    DrawRect(iX + 1, iY + 1, iX + iWidth - 1, iY + iHeight - 1, RGB(109, 218, 22),
              FALSE);

    // 제목 표시줄을 채움
    DrawRect(iX, iY + 3, iX + iWidth - 1, iY + 21, RGB(79, 204, 11), TRUE);

    // 윈도우 제목을 표시
    DrawText(iX + 6, iY + 3, RGB(255, 255, 255), RGB(79, 204, 11),
              pcTitle, StrLen(pcTitle));

    // 제목 표시줄을 입체로 보이게 위쪽의 선을 그림, 2 픽셀 두께
    DrawLine(iX + 1, iY + 1, iX + iWidth - 1, iY + 1, RGB(183, 249, 171));
    DrawLine(iX + 1, iY + 2, iX + iWidth - 1, iY + 2, RGB(150, 210, 140));

    DrawLine(iX + 1, iY + 2, iX + 1, iY + 20, RGB(183, 249, 171));
    DrawLine(iX + 2, iY + 2, iX + 2, iY + 20, RGB(150, 210, 140));

    // 제목 표시줄의 아래쪽에 선을 그림
    DrawLine(iX + 2, iY + 19, iX + iWidth - 2, iY + 19, RGB(46, 59, 30));
    DrawLine(iX + 2, iY + 20, iX + iWidth - 2, iY + 20, RGB(46, 59, 30));

    // 닫기 버튼을 그림, 오른쪽 상단에 표시
    DrawRect(iX + iWidth - 2 - 18, iY + 1, iX + iWidth - 2, iY + 19,
              RGB(255, 255, 255), TRUE);

    // 닫기 버튼을 입체로 보이게 선을 그림, 2 픽셀 두께로 그림
    DrawRect(iX + iWidth - 2, iY + 1, iX + iWidth - 2, iY + 19 - 1,
              RGB(86, 86, 86), TRUE);
    DrawRect(iX + iWidth - 2 - 1, iY + 1, iX + iWidth - 2 - 1, iY + 19 - 1,
              RGB(86, 86, 86), TRUE);
    DrawRect(iX + iWidth - 2 - 18 + 1, iY + 19, iX + iWidth - 2, iY + 19,
              RGB(86, 86, 86), TRUE);
    DrawRect(iX + iWidth - 2 - 18 + 1, iY + 19 - 1, iX + iWidth - 2, iY + 19 - 1,
              RGB(86, 86, 86), TRUE);

    DrawLine(iX + iWidth - 2 - 18, iY + 1, iX + iWidth - 2 - 1, iY + 1,
              RGB(229, 229, 229));
    DrawLine(iX + iWidth - 2 - 18, iY + 1 + 1, iX + iWidth - 2 - 2, iY + 1 + 1,
              RGB(229, 229, 229));
    DrawLine(iX + iWidth - 2 - 18, iY + 1, iX + iWidth - 2 - 18, iY + 19,
              RGB(229, 229, 229));
    DrawLine(iX + iWidth - 2 - 18 + 1, iY + 1, iX + iWidth - 2 - 18 + 1, iY + 19 - 1,
              RGB(229, 229, 229));

    // 대각선 X를 그림, 3 픽셀로 그림
    DrawLine(iX + iWidth - 2 - 18 + 4, iY + 1 + 4, iX + iWidth - 2 - 4, iY + 19 - 4,
              RGB(71, 199, 21));
    DrawLine(iX + iWidth - 2 - 18 + 5, iY + 1 + 4, iX + iWidth - 2 - 4, iY + 19 - 5,
              RGB(71, 199, 21));
    DrawLine(iX + iWidth - 2 - 18 + 4, iY + 1 + 5, iX + iWidth - 2 - 5, iY + 19 - 4,
              RGB(71, 199, 21));

    DrawLine(iX + iWidth - 2 - 18 + 4, iY + 19 - 4, iX + iWidth - 2 - 4, iY + 1 + 4,
              RGB(71, 199, 21));
    DrawLine(iX + iWidth - 2 - 18 + 5, iY + 19 - 4, iX + iWidth - 2 - 4, iY + 1 + 5,
              RGB(71, 199, 21));
    DrawLine(iX + iWidth - 2 - 18 + 4, iY + 19 - 5, iX + iWidth - 2 - 5, iY + 1 + 4,
              RGB(71, 199, 21));

    // 내부를 그림
    DrawRect(iX + 2, iY + 21, iX + iWidth - 2, iY + iHeight - 2,
              RGB(255, 255, 255), TRUE);

    // 테스트 문자 출력
    DrawText(iX + 10, iY + 30, RGB(0, 0, 0), RGB(255, 255, 255), pcTestString1,
              StrLen(pcTestString1));
    DrawText(iX + 10, iY + 50, RGB(0, 0, 0), RGB(255, 255, 255), pcTestString2,
              StrLen(pcTestString2));
}

/**
 *  그래픽 모드를 테스트하는 함수
 */
void StartGraphicModeTest()
{
    VBEMODEINFOBLOCK *pstVBEMode;
    int iX1, iY1, iX2, iY2;
    COLOR stColor1, stColor2;
    int i;
    // memset reference error
    // char *vpcString[] = {"Pixel", "Line", "Rectangle", "Circle", "MINT64 OS~!!!"};

    //==========================================================================
    // 점, 선, 사각형, 원, 그리고 문자를 간단히 출력
    //==========================================================================
    // (0, 0)에 Pixel이란 문자열을 검은색 바탕에 흰색으로 출력
    DrawText(0, 0, RGB(255, 255, 255), RGB(0, 0, 0), "Pixel",
              StrLen("Pixel"));
    // 픽셀을 (1, 20), (2, 20)에 흰색으로 출력
    DrawPixel(1, 20, RGB(255, 255, 255));
    DrawPixel(2, 20, RGB(255, 255, 255));

    // (0, 25)에 Line이란 문자열을 검은색 바탕에 빨간색으로 출력
    DrawText(0, 25, RGB(255, 0, 0), RGB(0, 0, 0), "Line",
              StrLen("Line"));
    // (20, 50)을 중심으로 (1000, 50) (1000, 100), (1000, 150), (1000, 200),
    // (1000, 250)까지 빨간색으로 출력
    DrawLine(20, 50, 1000, 50, RGB(255, 0, 0));
    DrawLine(20, 50, 1000, 100, RGB(255, 0, 0));
    DrawLine(20, 50, 1000, 150, RGB(255, 0, 0));
    DrawLine(20, 50, 1000, 200, RGB(255, 0, 0));
    DrawLine(20, 50, 1000, 250, RGB(255, 0, 0));

    // (0, 180)에 Rectangle이란 문자열을 검은색 바탕에 녹색으로 출력
    DrawText(0, 180, RGB(0, 255, 0), RGB(0, 0, 0), "Rectangle",
              StrLen("Rectangle"));
    // (20, 200)에서 시작하여 길이가 각각 50, 100, 150, 200인 사각형을 녹색으로 출력
    DrawRect(20, 200, 70, 250, RGB(0, 255, 0), FALSE);
    DrawRect(120, 200, 220, 300, RGB(0, 255, 0), TRUE);
    DrawRect(270, 200, 420, 350, RGB(0, 255, 0), FALSE);
    DrawRect(470, 200, 670, 400, RGB(0, 255, 0), TRUE);

    // (0, 550)에 Circle이란 문자열을 검은색 바탕에 파란색으로 출력
    DrawText(0, 550, RGB(0, 0, 255), RGB(0, 0, 0), "Circle",
              StrLen("Circle"));
    // (45, 600)에서 시작하여 반지름이 25, 50, 75, 100인 원을 파란색으로 출력
    DrawCircle(45, 600, 25, RGB(0, 0, 255), FALSE);
    DrawCircle(170, 600, 50, RGB(0, 0, 255), TRUE);
    DrawCircle(345, 600, 75, RGB(0, 0, 255), FALSE);
    DrawCircle(570, 600, 100, RGB(0, 0, 255), TRUE);

    // 키 입력 대기
    GetCh();

    //==========================================================================
    // 점, 선, 사각형, 원, 그리고 문자를 무작위로 출력
    //==========================================================================
    // q 키가 입력될 때까지 아래를 반복
    do
    {
        // 점 그리기
        for (i = 0; i < 100; i++)
        {
            // 임의의 X좌표와 색을 반환
            GetRandomXY(&iX1, &iY1);
            stColor1 = GetRandomColor();

            // 점 그리기
            DrawPixel(iX1, iY1, stColor1);
        }

        // 선 그리기
        for (i = 0; i < 100; i++)
        {
            // 임의의 X좌표와 색을 반환
            GetRandomXY(&iX1, &iY1);
            GetRandomXY(&iX2, &iY2);
            stColor1 = GetRandomColor();

            // 선 그리기
            DrawLine(iX1, iY1, iX2, iY2, stColor1);
        }

        // 사각형 그리기
        for (i = 0; i < 20; i++)
        {
            // 임의의 X좌표와 색을 반환
            GetRandomXY(&iX1, &iY1);
            GetRandomXY(&iX2, &iY2);
            stColor1 = GetRandomColor();

            // 사각형 그리기
            DrawRect(iX1, iY1, iX2, iY2, stColor1, Random() % 2);
        }

        // 원 그리기
        for (i = 0; i < 100; i++)
        {
            // 임의의 X좌표와 색을 반환
            GetRandomXY(&iX1, &iY1);
            stColor1 = GetRandomColor();

            // 원 그리기
            DrawCircle(iX1, iY1, ABS(Random() % 50 + 1), stColor1, Random() % 2);
        }

        // 텍스트 표시
        for (i = 0; i < 100; i++)
        {
            // 임의의 X좌표와 색을 반환
            GetRandomXY(&iX1, &iY1);
            stColor1 = GetRandomColor();
            stColor2 = GetRandomColor();

            // 텍스트 출력
            DrawText(iX1, iY1, stColor1, stColor2, "MINT64 OS~!!!",
                      StrLen("MINT64 OS~!!!"));
        }
    } while (GetCh() != 'q');

    //==========================================================================
    // 윈도우 프로토타입을 출력
    //==========================================================================
    // q 키를 눌러서 빠져 나왔다면 윈도우 프로토타입을 표시함
    while (1)
    {
        // 배경을 출력
        DrawRect(0, 0, 1024, 768, RGB(232, 255, 232), TRUE);

        // 윈도우 프레임을 3개 그림
        for (i = 0; i < 3; i++)
        {
            GetRandomXY(&iX1, &iY1);
            DrawWindowFrame(iX1, iY1, 400, 200, "MINT64 OS Test Window");
        }

        GetCh();
    }
}