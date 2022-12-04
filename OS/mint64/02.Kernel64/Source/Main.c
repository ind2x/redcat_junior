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

void Main(void)
{
    int iCursorX, iCursorY;

    InitializeConsole(0, 10);

    Printf("[*] Switch To IA-32e Mode Success.................\n");
    Printf("[*] IA-32e C Language Kernel Start................[Pass]\n");
    Printf("[*] Initialize Console............................[Pass]\n");
    
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

    Printf("[*] Serial Port Initialize........................[PASS]");
    iCursorY++;
    InitializeSerialPort();

    CreateTask(TASK_FLAGS_LOWEST | TASK_FLAGS_THREAD | TASK_FLAGS_SYSTEM | TASK_FLAGS_IDLE, 0, 0, (QWORD)IdleTask);
    StartConsoleShell();
}