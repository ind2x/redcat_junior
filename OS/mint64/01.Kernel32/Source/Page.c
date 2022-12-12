#include "Page.h"

/**
 *	IA-32e 모드 커널을 위한 페이지 테이블 생성
 */
void InitializePageTables(void)
{
    PML4TENTRY *pstPML4TEntry;  // PML4
    PDPTENTRY *pstPDPTEntry;    // 페이지 디렉터리 포인터 테이블
    PDENTRY *pstPDEntry;        // 페이지 디렉터리
    DWORD dwMappingAddress;
    int i;

    /** 페이지의 공통된 속성은 다음과 같음
     * PCD   ==> 캐시 정책 비활성화 여부, 사용할 것이므로 0으로 설정
     * PWT   ==> Write-Through 캐시 정책 여부, Write-Back 사용할 것이므로 0으로 설정
     * U/S   ==> 유저/커널 레벨 여부, 커널로 설정(0)
     * R/W   ==> 읽기 쓰기 권한 여부, 1로 설정
     * EXB   ==> 페이지 내 코드 실행 비활성화 여부, 0으로 설정
     *  A    ==> 코드 실행 중 특정 페이지 접근 여부, 0로 설정
     *  P    ==> 페이지 유효 여부, 1로 설정
     * Avail ==> 필요 없으므로 0으로 설정
     */
    

    // PML4 엔트리 설정
    pstPML4TEntry = (PML4TENTRY *)0x100000; // 1MB ~ 1MB + 4KB
    SetPageEntryData(&(pstPML4TEntry[0]), 0x00, 0x101000, PAGE_FLAGS_DEFAULT, 0);
    
    for (i = 1; i < PAGE_MAXENTRYCOUNT; i++)   // 나머지 엔트리는 0으로 초기화
    {
        SetPageEntryData(&(pstPML4TEntry[i]), 0, 0, 0, 0);
    }


    // 페이지 디렉터리 포인터 테이블 엔트리 설정
    pstPDPTEntry = (PDPTENTRY *)0x101000;   // 1MB + 4KB ~ 1MB + 4KB + 4KB
    
    for (i = 0; i < 64; i++)    // 0 ~ 63, 총 64개의 엔트리가 필요함
    {
        SetPageEntryData(&(pstPDPTEntry[i]), 0, 0x102000 + (i * PAGE_TABLESIZE), PAGE_FLAGS_DEFAULT, 0);
    }
    
    for (i = 64; i < PAGE_MAXENTRYCOUNT; i++)   // 나머지 엔트리는 0으로 초기화
    {
        SetPageEntryData(&(pstPDPTEntry[i]), 0, 0, 0, 0);
    }


    /** 페이지 디렉터리 엔트리용 속성
     * PAT, G, D 속성은 딱히 중요하진 않은 내용으로 0으로 설정해주면 됨
     */
    pstPDEntry = (PDENTRY *)0x102000;       // 1MB + 8KB ~ 1MB + 8KB + 256KB
    dwMappingAddress = 0;
    
    for (i = 0; i < PAGE_MAXENTRYCOUNT * 64; i++)   // 테이블이 64개 필요함
    {
        SetPageEntryData(&(pstPDEntry[i]), (i * (PAGE_DEFAULTSIZE >> 20)) >> 12, dwMappingAddress, PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS, 0);
        
        dwMappingAddress += PAGE_DEFAULTSIZE;
    }
}

/**
 *	페이지 엔트리에 기준 주소와 속성 플래그를 설정
 */
void SetPageEntryData(PTENTRY *pstEntry, DWORD dwUpperBaseAddress, DWORD dwLowerBaseAddress, DWORD dwLowerFlags, DWORD dwUpperFlags)
{
    pstEntry->dwAttributeAndLowerBaseAddress = dwLowerBaseAddress | dwLowerFlags;
    
    pstEntry->dwUpperBaseAddressAndEXB = (dwUpperBaseAddress & 0xFF) | dwUpperFlags;
}