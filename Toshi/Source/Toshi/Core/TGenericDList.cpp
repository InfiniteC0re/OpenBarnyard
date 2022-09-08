#include "ToshiPCH.h"
#include "TGenericDList.h"

Toshi::TGenericDList::TNode::TNode()
{
	Reset();
}

void Toshi::TGenericDList::TNode::InsertAfter(TNode* param_1)
{
	m_pPrev = param_1;
	m_pNext = param_1->m_pNext;
	param_1->m_pNext = this;
	m_pNext->m_pPrev = this;
}

void Toshi::TGenericDList::TNode::InsertBefore(TNode* param_1)
{
	m_pNext = param_1;
	m_pPrev = param_1->m_pPrev;
	param_1->m_pPrev = this;
	m_pPrev->m_pNext = this;
}

void Toshi::TGenericDList::TNode::Remove()
{
	m_pPrev->m_pNext = m_pNext;
	m_pNext->m_pPrev = m_pPrev;
	m_pNext = this;
	m_pPrev = this;
}

void Toshi::TGenericDList::TNode::Reset()
{
	m_pNext = this;
	m_pPrev = this;
}

Toshi::TGenericDList::TNode& Toshi::TGenericDList::TNode::operator=(const TNode& param_1)
{
	m_pNext = param_1.m_pNext;
	m_pPrev = param_1.m_pPrev;
	return *this;
}

Toshi::TGenericDList::TGenericDList()
{
}
