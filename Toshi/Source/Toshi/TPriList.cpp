#include "ToshiPCH.h"
#include "TDList.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

void TGenericPriList::TNode::InsertAfter( TNode* a_pNode )
{
	TASSERT( !IsLinked(), "TNode::InsertAfter - TNode shouldn't be linked" );

	m_iPriority = -0x8000;
	m_pPrev = a_pNode;
	m_pNext = a_pNode->m_pNext;
	a_pNode->m_pNext = this;
	m_pNext->m_pPrev = this;
}

void TGenericPriList::TNode::InsertBefore( TNode* a_pNode )
{
	TASSERT( !IsLinked(), "TNode::InsertBefore - TNode shouldn't be linked" );

	m_iPriority = 0x7FFF;
	m_pNext = a_pNode;
	m_pPrev = a_pNode->m_pPrev;
	a_pNode->m_pPrev = this;
	m_pPrev->m_pNext = this;
}

void TGenericPriList::Insert( TNode* a_pNode )
{
	TINT iPriority = a_pNode->m_iPriority;

	if ( iPriority < 0 )
	{
		TNode* pCurNode = m_pNext;

		while ( pCurNode != End() && pCurNode->m_iPriority <= iPriority )
		{
			pCurNode = pCurNode->m_pNext;
		}

		a_pNode->m_pNext = pCurNode;
		a_pNode->m_pPrev = pCurNode->m_pPrev;
		pCurNode->m_pPrev = a_pNode;
		a_pNode->m_pPrev->m_pNext = a_pNode;
	}
	else
	{
		TNode* pCurNode = m_pPrev;
		while ( pCurNode != End() && iPriority < pCurNode->m_iPriority )
		{
			pCurNode = pCurNode->m_pPrev;
		}
		a_pNode->m_pPrev = pCurNode;
		a_pNode->m_pNext = pCurNode->m_pNext;
		pCurNode->m_pNext = a_pNode;
		a_pNode->m_pNext->m_pPrev = a_pNode;
	}
}

void TGenericPriList::Insert( TNode* a_pNode, TINT iPriority )
{
	a_pNode->SetPriority( iPriority );
	Insert( a_pNode );
}

void TGenericPriList::RemoveAll()
{
	TNode* pNode = m_pNext;

	while ( pNode != End() )
	{
		pNode->Remove();
		pNode = m_pNext;
	}
}

TGenericPriList::TNode::TNode( TUninitialised )
{

}


TGenericPriList::TNode::TNode()
{
	Reset();
}

TGenericPriList::TNode::~TNode()
{
	Remove();
}

TGenericPriList::TNode& TGenericPriList::TNode::operator=( const TNode& a_pNode )
{
	m_pNext = a_pNode.m_pNext;
	m_pPrev = a_pNode.m_pPrev;
	SetPriority( a_pNode.GetPriority() );
	return *this;
}

void TGenericPriList::TNode::Reset()
{
	m_pNext = this;
	m_pPrev = this;
	SetPriority( 0 );
}

void TGenericPriList::TNode::Remove()
{
	m_pPrev->m_pNext = m_pNext;
	m_pNext->m_pPrev = m_pPrev;
	m_pNext = this;
	m_pPrev = this;
}

TGenericPriList::TGenericPriList()
{
	m_pNext = End();
	m_pPrev = End();
}

TGenericPriList::~TGenericPriList()
{
	RemoveAll();
}

TOSHI_NAMESPACE_END
