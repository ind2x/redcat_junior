#ifndef __TYPES_H__
#define __TYPES_H__

#define BYTE    unsigned char   // 1 BYTE
#define WORD    unsigned short  // 2 BYTE
#define DWORD   unsigned int    // 4 BYTE
#define QWORD   unsigned long   // 8 BYTE
#define BOOL    unsigned char   // 1 BYTE

#define TRUE    1
#define FALSE   0
#define NULL    0

#pragma pack( push, 1 )

typedef struct CharactorStruct
{
    BYTE bCharactor;
    BYTE bAttribute;
} CHARACTER;

#pragma pack( pop )
#endif /*__TYPES_H__*/