#ifndef __LIST_H__
#define __LIST_H__

#include "Types.h"

#pragma pack(push, 1)

// 리스트에 있는 데이터를 나타내는 자료구조
typedef struct ListLinkStruct
{
    void *pvNext;   // 다음 데이터의 위치
    QWORD qwID;     // 리스트 속 데이터의 ID
} LISTLINK;

// 리스트를 관리하는 자료구조
typedef struct ListManagerStruct
{
    int iItemCount;     // 리스트 내 데이터 개수

    void *pvHeader;     // 리스트의 처음
    void *pvTail;       // 리스트의 끝
} LIST;

#pragma pack(pop)

void kInitializeList(LIST *pstList);
int kGetListCount(const LIST *pstList);
void kAddListToTail(LIST *pstList, void *pvItem);
void kAddListToHeader(LIST *pstList, void *pvItem);
void* kRemoveList(LIST *pstList, QWORD qwID);
void* kRemoveListFromHeader(LIST *pstList);
void* kRemoveListFromTail(LIST *pstList);
void* kFindList(const LIST *pstList, QWORD qwID);
void* kGetHeaderFromList(const LIST *pstList);
void* kGetTailFromList(const LIST *pstList);
void* kGetNextFromList(const LIST *pstList, void *pstCurrent);

#endif /*__LIST_H__*/