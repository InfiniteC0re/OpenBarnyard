#include "ToshiPCH.h"
#include "TGenericDList.h"

namespace Toshi
{
	void TGenericDList::TNode::InsertAfter(TNode* node)
	{
		TASSERT(!IsLinked(), "TNode::InsertAfter - TNode shouldn't be linked");

		m_Prev = node;
		m_Next = node->m_Next;
		node->m_Next = this;
		m_Next->m_Prev = this;
	}

	void TGenericDList::TNode::InsertBefore(TNode* node)
	{
		TASSERT(!IsLinked(), "TNode::InsertBefore - TNode shouldn't be linked");

		m_Next = node;
		m_Prev = node->m_Prev;
		node->m_Prev = this;
		m_Prev->m_Next = this;
	}

	void TGenericDList::TNode::Remove()
	{
		m_Prev->m_Next = m_Next;
		m_Next->m_Prev = m_Prev;
		Reset();
	}

	TGenericDList::TNode& TGenericDList::TNode::operator=(const TNode& node)
	{
		m_Next = node.m_Next;
		m_Prev = node.m_Prev;
		return *this;
	}

	TGenericDList::TGenericDList()
	{
	}
}