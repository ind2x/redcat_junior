#ifndef __PIT_H__
#define __PIT_H__

#include "Types.h"

#define PIT_FREQUENCY 1193182
#define MSTOCOUNT(x) (PIT_FREQUENCY * (x) / 1000) // milisecond
#define USTOCOUNT(x) (PIT_FREQUENCY * (x) / 1000000) // microsecond

// I/O 포트
#define PIT_PORT_CONTROL 0x43  // 컨트롤 레지스터
#define PIT_PORT_COUNTER0 0x40 // 카운터0 레지스터, 카운터0 만 사용
#define PIT_PORT_COUNTER1 0x41
#define PIT_PORT_COUNTER2 0x42

// 컨트롤 레지스터 필드 구성 값
// Select Counter 값 설정 필드
#define PIT_CONTROL_COUNTER0 0x00  // 카운터0 만 사용
#define PIT_CONTROL_COUNTER1 0x40
#define PIT_CONTROL_COUNTER2 0x80
// 2바이트 전송, 하위에서 상위 I/O 포트로 연속해서 값을 읽거나 씀
#define PIT_CONTROL_LSBMSBRW 0x30
// 카운터의 현재 값을 읽음
#define PIT_CONTROL_LATCH 0x00 
// interrupt during counting
// 현재 시간에서 일정 시간이 지났음을 확인하고자 하면 모드 0
#define PIT_CONTROL_MODE0 0x00 
// clock rate generator
// 주기적으로, 반복적으로 시간을 확인하고자 하면 모드 2
#define PIT_CONTROL_MODE2 0x04

// Binary or BCD
#define PIT_CONTROL_BINARYCOUNTER 0x00
#define PIT_CONTROL_BCDCOUNTER 0x01

// 모드 0으로 설정하여 원할 때 한번만 확인하도록 설정한 값
#define PIT_COUNTER0_ONCE (PIT_CONTROL_COUNTER0 | PIT_CONTROL_LSBMSBRW | PIT_CONTROL_MODE0 | PIT_CONTROL_BINARYCOUNTER)
// 모드 2로 설정하여 주기적으로 확인하도록 설정한 값
#define PIT_COUNTER0_PERIODIC (PIT_CONTROL_COUNTER0 | PIT_CONTROL_LSBMSBRW | PIT_CONTROL_MODE2 | PIT_CONTROL_BINARYCOUNTER)

// 카운터0 을 직접 읽기 위해서 사전에 보내야 하는 명령어
#define PIT_COUNTER0_LATCH (PIT_CONTROL_COUNTER0 | PIT_CONTROL_LATCH)

void kInitializePIT(WORD wCount, BOOL bPeriodic);
WORD kReadCounter0(void);
void kWaitUsingDirectPIT(WORD wCount);

#endif /*__PIT_H__*/