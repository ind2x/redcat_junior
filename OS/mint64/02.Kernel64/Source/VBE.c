#include "VBE.h"

// 모드 정보 블록 자료구조
static VBEMODEINFOBLOCK *gs_pstVBEModeBlockInfo = (VBEMODEINFOBLOCK *)VBE_MODEINFOBLOCKADDRESS;

/**
 *  VBE 모드 정보 블록을 반환
 */
inline VBEMODEINFOBLOCK *GetVBEModeInfoBlock(void)
{
    return gs_pstVBEModeBlockInfo;
}