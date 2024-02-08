#include "ToshiPCH.h"
#include "TDList.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi {

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

	void TGenericPriList::TNode::InsertAfter(TNode* node)
	{
		TASSERT(!IsLinked(), "TNode::InsertAfter - TNode shouldn't be linked");

		m_iPriority = -0x8000;
		m_Prev = node;
		m_Next = node->m_Next;
		node->m_Next = this;
		m_Next->m_Prev = this;
	}

	void TGenericPriList::TNode::InsertBefore(TNode* node)
	{
		TASSERT(!IsLinked(), "TNode::InsertBefore - TNode shouldn't be linked");

		m_iPriority = 0x7FFF;
		m_Next = node;
		m_Prev = node->m_Prev;
		node->m_Prev = this;
		m_Prev->m_Next = this;
	}

	void TGenericPriList::Insert(TNode* node)
	{
		int priority = node->m_iPriority;

		if (priority < 0)
		{
			TNode* curNode = GetRoot().m_Next;

			while (curNode != &GetRoot() && curNode->m_iPriority <= priority)
			{
				curNode = curNode->m_Next;
			}

			node->m_Next = curNode;
			node->m_Prev = curNode->m_Prev;
			curNode->m_Prev = node;
			node->m_Prev->m_Next = node;
		}
		else
		{
			TNode* curNode = GetRoot().m_Prev;
			while (curNode != &GetRoot() && priority < curNode->m_iPriority)
			{
				curNode = curNode->m_Prev;
			}
			node->m_Prev = curNode;
			node->m_Next = curNode->m_Next;
			curNode->m_Next = node;
			node->m_Next->m_Prev = node;
		}
	}

	void TGenericPriList::Insert(TNode* node, int iPriority)
	{
		node->SetPriority(iPriority);
		Insert(node);
	}

	void TGenericPriList::RemoveAll()
	{
		auto pNode = m_Next;

		while (pNode != &GetRoot())
		{
			pNode->Remove();
			pNode = m_Next;
		}
	}

}
