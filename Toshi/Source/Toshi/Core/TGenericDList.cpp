#include "pch.h"
#include "TGenericDList.h"

Toshi::TGenericDList::TNode::TNode()
{
	Reset();
}

void Toshi::TGenericDList::TNode::InsertAfter(TNode* param_1)
{
	m_pPrevNode = param_1;
	m_pNextNode = param_1->m_pNextNode;
	param_1->m_pNextNode = this;
	m_pNextNode->m_pPrevNode = this;
}

void Toshi::TGenericDList::TNode::InsertBefore(TNode* param_1)
{
	m_pNextNode = param_1;
	m_pPrevNode = param_1->m_pPrevNode;
	param_1->m_pPrevNode = this;
	m_pPrevNode->m_pNextNode = this;
}

void Toshi::TGenericDList::TNode::Remove()
{
	m_pPrevNode->m_pNextNode = m_pNextNode;
	m_pNextNode->m_pPrevNode = m_pPrevNode;
	m_pNextNode = this;
	m_pPrevNode = this;
}

void Toshi::TGenericDList::TNode::Reset()
{
	m_pNextNode = this;
	m_pPrevNode = this;
}

Toshi::TGenericDList::TNode& Toshi::TGenericDList::TNode::operator=(const TNode& param_1)
{
	m_pNextNode = param_1.m_pNextNode;
	m_pPrevNode = param_1.m_pPrevNode;
	return *this;
}

Toshi::TGenericDList::TGenericDList()
{
}
