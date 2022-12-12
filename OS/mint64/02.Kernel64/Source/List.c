#include "List.h"

/**
 * 리스트 초기화 함수
 * 데이터가 없으므로 개수 = 0, 처음과 끝은 NULL로 설정
*/
void InitializeList(LIST *pstList)
{
    pstList->iItemCount = 0;
    pstList->pvHeader = NULL;
    pstList->pvTail = NULL;
}

/**
 * 리스트의 데이터 개수를 반환하는 함수
*/
int GetListCount(const LIST *pstList)
{
    return pstList->iItemCount;
}

/**
 * 리스트의 끝에 데이터를 추가하는 함수ㅏ
*/
void AddListToTail(LIST *pstList, void *pvItem)
{
    LISTLINK *pstLink;  // 리스트의 데이터 변수
    
    // 넣어줄 데이터 값을 리스트 데이터 변수에 저장
    pstLink = (LISTLINK *) pvItem;  
    // 데이터가 끝에 추가되므로 NULL로 설정
    pstLink->pvNext = NULL;

    // 현재 리스트에 데이터가 없는 경우
    if(pstList->pvHeader == NULL)
    {
        // 헤드와 테일을 해당 데이터로 설정
        pstList->pvHeader = pvItem;
        pstList->pvTail = pvItem;
        pstList->iItemCount = 1;

        return ;
    }

    // 현재 가장 마지막 데이터의 pvNext를 추가한 데이터로 설정
    pstLink = (LISTLINK *) pstList->pvTail;
    pstLink->pvNext = pvItem;

    // 리스트의 가장 마지막 데이터를 추가한 데이터로 설정 후 개수 수정
    pstList->pvTail = pvItem;
    pstList->iItemCount++;
}

/**
 * 리스트 헤더에 데이터를 추가하는 함수
*/
void AddListToHeader(LIST *pstList, void *pvItem)
{
    LISTLINK *pstLink;

    // 추가할 데이터의 다음 데이터를 가리키는 값을 현재 헤더의 주소로 설정
    pstLink = (LISTLINK *)pvItem;
    pstLink->pvNext = pstList->pvHeader;

    // 리스트에 데이터가 없는 경우
    if (pstList->pvHeader == NULL)
    {
        pstList->pvHeader = pvItem;
        pstList->pvTail = pvItem;
        pstList->iItemCount = 1;

        return;
    }

    // 리스트의 헤더를 추가한 데이터로 설정
    pstList->pvHeader = pvItem;
    pstList->iItemCount++;
}

/**
 * 리스트에서 데이터를 제거한 후 데이터의 포인터를 반환
 * 데이터의 ID를 이용해서 찾은 후 제거 및 반환
*/
void *RemoveList(LIST *pstList, QWORD qwID)
{
    LISTLINK *pstLink;          
    LISTLINK *pstPreviousLink;

    pstPreviousLink = (LISTLINK*) pstList->pvHeader;
    
    // 삭제할 데이터의 qwID를 현재 헤더부터 끝까지 검색
    for(pstLink = pstPreviousLink; pstLink != NULL; pstLink = pstLink->pvNext)
    {
        // 삭제할 데이터를 찾은 경우
        if(pstLink->qwID == qwID)
        {
            // 리스트에 데이터가 한 개인 경우
            if((pstLink == pstList->pvHeader) && (pstLink == pstList->pvTail))
            {
                // 데이터를 삭제 후 리스트에 데이터가 없으므로 모두 NULL로 설정
                pstList->pvHeader = NULL;
                pstList->pvTail = NULL;
            }
            // 리스트의 헤더를 삭제하는 경우
            else if(pstLink == pstList->pvHeader)
            {
                // 헤더를 다음 데이터로 변경해줘야 함
                pstList->pvHeader = pstLink->pvNext;
            }
            // 리스트의 꼬리를 삭제하는 경우
            else if(pstLink == pstList->pvTail)
            {
                // 꼬리 이전의 데이터를 꼬리로 설정
                pstList->pvTail = pstPreviousLink;
            }
            // 중간에 있는 데이터를 삭제하는 경우
            else
            {
                // 삭제한 데이터의 이전 데이터의 pvNext를 이후의 데이터로 설정 
                pstPreviousLink->pvNext = pstLink->pvNext;
            }

            pstList->iItemCount--;
            return pstLink;
        }
        // 현재 검사한 데이터를 이전 데이터로 설정
        pstPreviousLink = pstLink;
    }
    
    return NULL;
}

/**
 * 리스트의 헤더 데이터를 제거한 뒤 반환
*/
void *RemoveListFromHeader(LIST *pstList)
{
    LISTLINK *pstLink;

    if(pstList->iItemCount == 0)
    {
        return NULL;
    }

    // 리스트의 헤더 데이터를 가져와서 RemoveList에 ID를 인자로 넘겨줌
    pstLink = (LISTLINK *) pstList->pvHeader;
    return RemoveList(pstList, pstLink->qwID);
}

/**
 * 리스트의 마지막 데이터를 제거한 뒤 반환
*/
void *RemoveListFromTail(LIST *pstList)
{
    LISTLINK *pstLink;

    if(pstList->iItemCount == 0) 
    {
        return NULL;
    }

    // 마지막 데이터를 가져와서 RemoveList에 ID를 인자로 넘겨줌
    pstLink = (LISTLINK *) pstList->pvTail;
    return RemoveList(pstList, pstLink->qwID);
}

/**
 * 리스트 내에서 데이터를 찾아 반환
*/
void *FindList(const LIST *pstList, QWORD qwID)
{
    LISTLINK *pstLink;

    // ID를 이용해서 헤더부터 테일까지 검색
    for(pstLink = (LISTLINK *)pstList->pvHeader; pstLink != NULL; pstLink = pstLink->pvNext)
    {
        if(pstLink->qwID == qwID) 
        {
            return pstLink;
        }
    }

    return NULL;
}

/**
 * 리스트의 헤더 데이터를 반환
*/
void *GetHeaderFromList(const LIST *pstList)
{
    return pstList->pvHeader;
}

/**
 * 리스트의 마지막 데이터를 반환
*/
void *GetTailFromList(const LIST *pstList)
{
    return pstList->pvTail;
}

/**
 * 리스트에서 현재 데이터의 다음 데이터를 반환
*/
void *GetNextFromList(const LIST *pstList, void *pstCurrent)
{
    LISTLINK *pstLink;

    pstLink = (LISTLINK *)pstCurrent;

    return pstLink->pvNext;
}
