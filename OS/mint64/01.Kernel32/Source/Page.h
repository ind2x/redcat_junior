#ifndef __PAGE_H__
#define __PAGE_H__

#include "Types.h"

/**
 * 플래그 필드의 위치를 저장한 매크로 값
*/
// 하위 32비트용 속성 필드
#define PAGE_FLAGS_P        0x00000001    // Present
#define PAGE_FLAGS_RW       0x00000002    // Read/Write
#define PAGE_FLAGS_US       0x00000004    // User/Supervisor(플래그 설정 시 유저 레벨)
#define PAGE_FLAGS_PWT      0x00000008    // Page Level Write-through
#define PAGE_FLAGS_PCD      0x00000010    // Page Level Cache Disable
#define PAGE_FLAGS_A        0x00000020    // Accessed
#define PAGE_FLAGS_D        0x00000040    // Dirty
#define PAGE_FLAGS_PS       0x00000080    // Page Size
#define PAGE_FLAGS_G        0x00000100    // Global
#define PAGE_FLAGS_PAT      0x00001000    // Page Attribute Table Index
// 상위 32비트용 속성 필드
#define PAGE_FLAGS_EXB      0x80000000    // Execute Disable 비트
// 아래는 편의를 위해 정의한 플래그
#define PAGE_FLAGS_DEFAULT (PAGE_FLAGS_P | PAGE_FLAGS_RW)

// 기타 페이징 관련
#define PAGE_TABLESIZE      0x1000
#define PAGE_MAXENTRYCOUNT  512
#define PAGE_DEFAULTSIZE    0x200000    // MINT 64 OS에서는 페이지를 2MB 단위로 설정
                                        // 보통은 4KB인 듯함 (다른 책에서 봄)

#pragma pack(push, 1)

/**
 * 상위와 하위를 구분한 이유는 
 * 보호모드에서는 최대 32비트까지만 표현 가능하기 때문임
*/
typedef struct PageTableEntryStruct
{
    DWORD dwAttributeAndLowerBaseAddress;   // 하위 32비트 플래그들 의미
    DWORD dwUpperBaseAddressAndEXB;         // 상위 32비트 플래그들 (EXB 등)을 의미
} PML4TENTRY, PDPTENTRY, PDENTRY, PTENTRY;

#pragma pack( pop )

void InitializePageTables(void);
void SetPageEntryData(PTENTRY *pstEntry, DWORD dwUpperBaseAddress, DWORD dwLowerBaseAddress, DWORD dwLowerFlags, DWORD dwUpperFlags);

#endif /*__PAGE_H__*/