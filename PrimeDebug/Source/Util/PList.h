#pragma once
#include "PrimeDebug.h"

// An intrusive circular list (Prime's crList): the list is a node of its own, an empty one points to itself
struct PListNode
{
	PListNode* pNext;
	PListNode* pPrev;

	void Init()
	{
		pNext = this;
		pPrev = this;
	}

	TBOOL IsLinked() const { return pNext != TNULL || pPrev != TNULL; }

	void PushBack( PListNode* a_pNode )
	{
		a_pNode->pNext        = this;
		a_pNode->pPrev        = pPrev;
		pPrev                 = a_pNode;
		a_pNode->pPrev->pNext = a_pNode;
	}

	void Remove()
	{
		pPrev->pNext = pNext;
		pNext->pPrev = pPrev;
		pNext        = this;
		pPrev        = this;
	}

	PListNode* GetFirst() { return pNext != this ? pNext : TNULL; }
	PListNode* GetNext( PListNode* a_pNode ) { return a_pNode->pNext != this ? a_pNode->pNext : TNULL; }
};
