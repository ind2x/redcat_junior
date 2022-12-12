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

void MainForApplicationProcessor(void);

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

    CreateTask(TASK_FLAGS_LOWEST | TASK_FLAGS_THREAD | TASK_FLAGS_SYSTEM | TASK_FLAGS_IDLE, 0, 0, (QWORD)IdleTask);
    StartConsoleShell();
}

void MainForApplicationProcessor(void)
{
    QWORD qwTickCount;

    LoadGDTR( GDTR_STARTADDRESS );

    LoadTR( GDT_TSSSEGMENT + ( GetAPICID() * sizeof( GDTENTRY16 ) ) );

    LoadIDTR( IDTR_STARTADDRESS );

    EnableSoftwareLocalAPIC();

    SetTaskPriority(0);

    InitializeLocalVectorTable();

    EnableInterrupt();

    Printf("[*] Application Processor[APIC ID: %d] Is Activated\n", GetAPICID());

    qwTickCount = GetTickCount();
    
    while( 1 )
    {
        if( GetTickCount() - qwTickCount > 1000 )
        {
            qwTickCount = GetTickCount();
        }
    }
}