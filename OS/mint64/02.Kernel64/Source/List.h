#ifndef __LIST_H__
#define __LIST_H__

#include "Types.h"

#pragma pack(push, 1)

typedef struct ListLinkStruct
{
    void *pvNext;
    QWORD qwID;
} LISTLINK;

typedef struct ListManagerStruct
{
    int iItemCount;

    void *pvHeader;
    void *pvTail;
} LIST;

#pragma pack(pop)

void InitializeList(LIST *pstList);
int GetListCount(const LIST *pstList);
void AddListToTail(LIST *pstList, void *pvItem);
void AddListToHeader(LIST *pstList, void *pvItem);
void* RemoveList(LIST *pstList, QWORD qwID);
void* RemoveListFromHeader(LIST *pstList);
void* RemoveListFromTail(LIST *pstList);
void* FindList(const LIST *pstList, QWORD qwID);
void* GetHeaderFromList(const LIST *pstList);
void* GetTailFromList(const LIST *pstList);
void* GetNextFromList(const LIST *pstList, void *pstCurrent);

#endif /*__LIST_H__*/