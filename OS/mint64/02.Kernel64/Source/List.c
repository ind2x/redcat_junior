#include "List.h"

void InitializeList(LIST *pstList)
{
    pstList->iItemCount = 0;
    pstList->pvHeader = NULL;
    pstList->pvTail = NULL;
}

int GetListCount(const LIST *pstList)
{
    return pstList->iItemCount;
}

void AddListToTail(LIST *pstList, void *pvItem)
{
    LISTLINK *pstLink;

    pstLink = (LISTLINK *) pvItem;
    pstLink->pvNext = NULL;

    if(pstList->pvHeader == NULL)
    {
        pstList->pvHeader = pvItem;
        pstList->pvTail = pvItem;
        pstList->iItemCount = 1;

        return ;
    }

    pstLink = (LISTLINK *) pstList->pvTail;
    pstLink->pvNext = pvItem;

    pstList->pvTail = pvItem;
    pstList->iItemCount++;
}

void AddListToHeader(LIST *pstList, void *pvItem)
{
    LISTLINK *pstLink;

    pstLink = (LISTLINK *)pvItem;
    pstLink->pvNext = pstList->pvHeader;

    if (pstList->pvHeader == NULL)
    {
        pstList->pvHeader = pvItem;
        pstList->pvTail = pvItem;
        pstList->iItemCount = 1;

        return;
    }

    pstList->pvHeader = pvItem;
    pstList->iItemCount++;
}

void *RemoveList(LIST *pstList, QWORD qwID)
{
    LISTLINK *pstLink;
    LISTLINK *pstPreviousLink;

    pstPreviousLink = (LISTLINK*) pstList->pvHeader;
    
    for(pstLink = pstPreviousLink; pstLink != NULL; pstLink = pstLink->pvNext)
    {
        if(pstLink->qwID == qwID)
        {
            if((pstLink == pstList->pvHeader) && (pstLink == pstList->pvTail))
            {
                pstList->pvHeader = NULL;
                pstList->pvTail = NULL;
            }
            else if(pstLink == pstList->pvHeader)
            {
                pstList->pvHeader = pstLink->pvNext;
            }
            else if(pstLink == pstList->pvTail)
            {
                pstList->pvTail = pstPreviousLink;
            }
            else
            {
                pstPreviousLink->pvNext = pstLink->pvNext;
            }

            pstList->iItemCount--;
            return pstLink;
        }
        
        pstPreviousLink = pstLink;
    }
    
    return NULL;
}

void *RemoveListFromHeader(LIST *pstList)
{
    LISTLINK *pstLink;

    if(pstList->iItemCount == 0)
    {
        return NULL;
    }

    pstLink = (LISTLINK *) pstList->pvHeader;
    return RemoveList(pstList, pstLink->qwID);
}

void *RemoveListFromTail(LIST *pstList)
{
    LISTLINK *pstLink;

    if(pstList->iItemCount == 0) 
    {
        return NULL;
    }

    pstLink = (LISTLINK *) pstList->pvTail;
    return RemoveList(pstList, pstLink->qwID);
}

void *FindList(const LIST *pstList, QWORD qwID)
{
    LISTLINK *pstLink;

    for(pstLink = (LISTLINK *)pstList->pvHeader; pstLink != NULL; pstLink = pstLink->pvNext)
    {
        if(pstLink->qwID == qwID) 
        {
            return pstLink;
        }
    }

    return NULL;
}

void *GetHeaderFromList(const LIST *pstList)
{
    return pstList->pvHeader;
}

void *GetTailFromList(const LIST *pstList)
{
    return pstList->pvTail;
}

void *GetNextFromList(const LIST *pstList, void *pstCurrent)
{
    LISTLINK *pstLink;

    pstLink = (LISTLINK *)pstCurrent;

    return pstLink->pvNext;
}
