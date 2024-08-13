#include "ToshiPCH.h"
#include "TDList.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TGenericDList::TNode::TNode()
{
	Reset();
}

TGenericDList::TNode::TNode( TUninitialised )
{
}

TGenericDList::TNode::~TNode()
{
	Remove();
}

void TGenericDList::TNode::InsertAfter( TNode* a_pNode )
{
	TASSERT( !IsLinked(), "TNode::InsertAfter - TNode shouldn't be linked" );

	m_pPrev			 = a_pNode;
	m_pNext			 = a_pNode->m_pNext;
	a_pNode->m_pNext = this;
	m_pNext->m_pPrev = this;
}

void TGenericDList::TNode::InsertBefore( TNode* a_pNode )
{
	TASSERT( !IsLinked(), "TNode::InsertBefore - TNode shouldn't be linked" );

	m_pNext			 = a_pNode;
	m_pPrev			 = a_pNode->m_pPrev;
	a_pNode->m_pPrev = this;
	m_pPrev->m_pNext = this;
}

void TGenericDList::TNode::Remove()
{
	m_pNext->m_pPrev = m_pPrev;
	m_pPrev->m_pNext = m_pNext;
	Reset();
}

void TGenericDList::TNode::Reset()
{
	m_pPrev = this;
	m_pNext = this;
}

void TGenericDList::InsertSegmentAfter( TNode* a_pNode1, TNode* a_pNode2, TNode* a_pNode3 )
{
	a_pNode1->m_pNext		   = a_pNode3;
	a_pNode2->m_pPrev		   = a_pNode3->m_pPrev;
	a_pNode1->m_pNext->m_pPrev = a_pNode1;
	a_pNode2->m_pPrev->m_pNext = a_pNode2;
}

void TGenericDList::InsertSegmentBefore( TNode* a_pNode1, TNode* a_pNode2, TNode* a_pNode3 )
{
	a_pNode2->m_pPrev		   = a_pNode3;
	a_pNode1->m_pNext		   = a_pNode3->m_pNext;
	a_pNode2->m_pPrev->m_pNext = a_pNode2;
	a_pNode1->m_pNext->m_pPrev = a_pNode1;
}

TGenericDList::TGenericDList()
{
}

TGenericDList::~TGenericDList()
{
	RemoveAll();
}

void TGenericDList::InsertSegmentAtHead( TNode* a_pNode1, TNode* a_pNode2 )
{
	a_pNode1->m_pNext		   = &m_oRoot;
	a_pNode2->m_pPrev		   = m_oRoot.m_pPrev;
	a_pNode1->m_pNext->m_pPrev = a_pNode1;
	a_pNode2->m_pPrev->m_pNext = a_pNode2;
}

void TGenericDList::InsertSegmentAtTail( TNode* a_pNode1, TNode* a_pNode2 )
{
	a_pNode2->m_pPrev		   = &m_oRoot;
	a_pNode1->m_pNext		   = m_oRoot.m_pNext;
	a_pNode2->m_pPrev->m_pNext = a_pNode2;
	a_pNode1->m_pNext->m_pPrev = a_pNode1;
}

void TGenericDList::RemoveAll()
{
	while ( !IsEmpty() )
	{
		RemoveHead();
	}
}

TOSHI_NAMESPACE_END
