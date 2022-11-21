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

void main(void)
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

    CreateTask(TASK_FLAGS_LOWEST | TASK_FLAGS_IDLE, (QWORD) IdleTask);
    StartConsoleShell();
}