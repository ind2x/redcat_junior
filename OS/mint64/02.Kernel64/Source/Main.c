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
#include "MPConfigurationTable.h"
#include "Mouse.h"
#include "WindowManagerTask.h"
#include "SystemCall.h"

void MainForApplicationProcessor(void);
BOOL ChangeToMultiCoreMode(void);

void Main(void)
{
    int iCursorX, iCursorY;
    BYTE bButton;
    int iX, iY;

    if(*((BYTE*) BOOTSTRAPPROCESSOR_FLAGADDRESS) == 0)
    {
        MainForApplicationProcessor();
    }
    
    *((BYTE*) BOOTSTRAPPROCESSOR_FLAGADDRESS) = 0;

    // 콘솔 초기화, (0,10) 좌표로 커서 업데이트
    kInitializeConsole(0, 10);

    // 11번째 줄(0,10) 부터 출력
    kPrintf("==============>> Switch To IA-32e Mode Success <<==============\n");
    kPrintf("[*] IA-32e C Language Kernel Start................[Pass]\n");
    kPrintf("[*] Initialize Console............................[Pass]\n");
    
    // 현재 커서의 위치 저장
    kGetCursor(&iCursorX, &iCursorY);
    kPrintf("[*] GDT Initialize And Switch For IA-32e Mode.....[    ]");
    kInitializeGDTTableAndTSS();
    kLoadGDTR(GDTR_STARTADDRESS);
    kSetCursor(51, iCursorY++);
    kPrintf("Pass\n");

    kPrintf("[*] TSS Segment Load..............................[    ]");
    kLoadTR(GDT_TSSSEGMENT);
    kSetCursor(51, iCursorY++);
    kPrintf("Pass\n");

    kPrintf("[*] IDT Initialize................................[    ]");
    kInitializeIDTTables();
    kLoadIDTR(IDTR_STARTADDRESS);
    kSetCursor(51, iCursorY++);
    kPrintf("Pass\n");

    kPrintf("[*] Total RAM Size Check..........................[    ]");
    kCheckTotalRAMSize();
    kSetCursor(51, iCursorY++);
    kPrintf("Pass], Size = %d MB\n", kGetTotalRAMSize());

    kPrintf("[*] TCB Pool And Scheduler Initialize.............[Pass]\n");
    iCursorY++;
    kInitializeScheduler();

    kPrintf("[*] Dynamic Memory Initialize.....................[Pass]\n");
    iCursorY++;
    kInitializeDynamicMemory();

    // 1ms마다 주기적으로 인터럽트 설정
    kInitializePIT(MSTOCOUNT(1), 1);

    kPrintf("[*] Keyboard Activate And Queue Initialize........[    ]");
    if (kInitializeKeyboard() == TRUE)
    {
        kSetCursor(51, iCursorY++);
        kPrintf("Pass\n");
        kChangeKeyboardLED(FALSE, FALSE, FALSE);
    }
    else
    {
        kSetCursor(51, iCursorY++);
        kPrintf("Fail\n");
        while (1);
    }

    kPrintf("[*] Mouse Activate And Queue Initialize...........[    ]");
    // 마우스를 활성화
    if (kInitializeMouse() == TRUE)
    {
        kEnableMouseInterrupt();
        kSetCursor(51, iCursorY++);
        kPrintf("Pass\n");
    }
    else
    {
        kSetCursor(51, iCursorY++);
        kPrintf("Fail\n");
        while (1);
    }

    kPrintf("[*] PIC Controller And Interrupt Initialize.......[    ]");
    
    kInitializePIC();
    kMaskPICInterrupt(0);
    kEnableInterrupt();
    kSetCursor(51, iCursorY++);
    kPrintf("Pass\n");

    kPrintf("[*] File System Initialize........................[    ]");
    if(kInitializeFileSystem() == TRUE)
    {
        kSetCursor(51, iCursorY++);
        kPrintf("Pass\n");
    }
    else
    {
        kSetCursor(51, iCursorY++);
        kPrintf("Fail\n");
    }

    kPrintf("[*] Serial Port Initialize........................[Pass]\n");
    iCursorY++;
    kInitializeSerialPort();

    // 멀티코어 프로세서 모드로 전환
    // Application Processor 활성화, I/O 모드 활성화, 인터럽트와 태스크 부하 분산
    // 기능 활성화
    kPrintf("[*] Change To MultiCore Processor Mode............[    ]");
    if (ChangeToMultiCoreMode() == TRUE)
    {
        kSetCursor(51, iCursorY++);
        kPrintf("Pass\n");
    }
    else
    {
        kSetCursor(51, iCursorY++);
        kPrintf("Fail\n");
    }

    // 시스템 콜에 관련된 MSR을 초기화
    kPrintf("[*] System Call MSR Initialize...................[Pass]\n");
    iCursorY++;
    kInitializeSystemCall();

    kCreateTask(TASK_FLAGS_LOWEST | TASK_FLAGS_THREAD | TASK_FLAGS_SYSTEM | TASK_FLAGS_IDLE, 0, 0, (QWORD)kIdleTask, kGetAPICID());

    // 그래픽 모드가 아니면 콘솔 셸 실행
    if (*(BYTE *)VBE_STARTGRAPHICMODEFLAGADDRESS == 0)
    {
        kStartConsoleShell();
    }
    // 그래픽 모드이면 그래픽 모드 테스트 함수 실행
    else
    {
        kStartWindowManager();
    }
}

void MainForApplicationProcessor(void)
{
    QWORD qwTickCount;

    kLoadGDTR( GDTR_STARTADDRESS );

    kLoadTR( GDT_TSSSEGMENT + ( kGetAPICID() * sizeof( GDTENTRY16 ) ) );

    kLoadIDTR( IDTR_STARTADDRESS );

    kInitializeScheduler();

    kEnableSoftwareLocalAPIC();

    kSetTaskPriority(0);

    kInitializeLocalVectorTable();

    kEnableInterrupt();

    // 시스템 콜에 관련된 MSR을 초기화
    kInitializeSystemCall();

    // kPrintf("[*] Application Processor[APIC ID: %d] Is Activated\n", kGetAPICID());

    kIdleTask();
}

/**
 *  멀티코어 프로세서 또는 멀티 프로세서 모드로 전환하는 함수
 */
BOOL ChangeToMultiCoreMode(void)
{
    MPCONFIGURATIONMANAGER *pstMPManager;
    BOOL bInterruptFlag;
    int i;

    // Application Processor 활성화
    if (kStartUpApplicationProcessor() == FALSE)
    {
        return FALSE;
    }

    //--------------------------------------------------------------------------
    // 대칭 I/O 모드로 전환
    //--------------------------------------------------------------------------
    // MP 설정 매니저를 찾아서 PIC 모드인가 확인
    pstMPManager = kGetMPConfigurationManager();
    if (pstMPManager->bUsePICMode == TRUE)
    {
        // PIC 모드이면 I/O 포트 어드레스 0x22에 0x70을 먼저 전송하고
        // I/O 포트 어드레스 0x23에 0x01을 전송하는 방법으로 IMCR 레지스터에 접근하여
        // PIC 모드 비활성화
        kOutPortByte(0x22, 0x70);
        kOutPortByte(0x23, 0x01);
    }

    // PIC 컨트롤러의 인터럽트를 모두 마스크하여 인터럽트가 발생할 수 없도록 함
    kMaskPICInterrupt(0xFFFF);

    // 프로세서 전체의 로컬 APIC를 활성화
    kEnableGlobalLocalAPIC();

    // 현재 코어의 로컬 APIC를 활성화
    kEnableSoftwareLocalAPIC();

    // 인터럽트를 불가로 설정
    bInterruptFlag = kSetInterruptFlag(FALSE);

    // 모든 인터럽트를 수신할 수 있도록 태스크 우선 순위 레지스터를 0으로 설정
    kSetTaskPriority(0);

    // 로컬 APIC의 로컬 벡터 테이블을 초기화
    kInitializeLocalVectorTable();

    // 대칭 I/O 모드로 변경되었음을 설정
    kSetSymmetricIOMode(TRUE);

    // I/O APIC 초기화
    kInitializeIORedirectionTable();

    // 이전 인터럽트 플래그를 복원
    kSetInterruptFlag(bInterruptFlag);

    // 인터럽트 부하 분산 기능 활성화
    kSetInterruptLoadBalancing(TRUE);

    // 태스크 부하 분산 기능 활성화
    for (i = 0; i < MAXPROCESSORCOUNT; i++)
    {
        kSetTaskLoadBalancing(i, TRUE);
    }

    return TRUE;
}