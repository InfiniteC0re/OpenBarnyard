#pragma once
#include "Toshi2/T2Iterator.h"

namespace Toshi {

	template <class T>
	class TQList
	{
	public:
		class TNode
		{
		public:
			friend TQList;
			T2_DEFINE_ITERATOR_FRIEND();
			T2_DEFINE_ITERATOR(T, TNode);

		public:
			TNode()
			{
				m_pNext = TSTATICCAST(T*, this);
				m_pPrev = TSTATICCAST(T*, this);
			}

			~TNode()
			{
				m_pPrev->m_pNext = m_pNext;
				m_pNext->m_pPrev = m_pPrev;
				m_pNext = TSTATICCAST(T*, this);
				m_pPrev = TSTATICCAST(T*, this);
			}

			TBOOL IsLinked() const
			{
				return this != m_pNext;
			}

			T* Next() const
			{
				return m_pNext;
			}

			T* Prev() const
			{
				return m_pPrev;
			}

		private:
			void InsertAfter(TNode* a_pNode)
			{
				TASSERT(TFALSE == IsLinked());
				m_pPrev = TSTATICCAST(T*, a_pNode);
				m_pNext = a_pNode->m_pNext;
				a_pNode->m_pNext = TSTATICCAST(T*, this);
				m_pNext->m_pPrev = TSTATICCAST(T*, this);
			}

			void InsertBefore(TNode* a_pNode)
			{
				TASSERT(TFALSE == IsLinked());
				m_pNext = TSTATICCAST(T*, a_pNode);
				m_pPrev = a_pNode->m_pPrev;
				a_pNode->m_pPrev = TSTATICCAST(T*, this);
				m_pPrev->m_pNext = TSTATICCAST(T*, this);
			}

			void Remove()
			{
				m_pPrev->m_pNext = m_pNext;
				m_pNext->m_pPrev = m_pPrev;
				m_pNext = TSTATICCAST(T*, this);
				m_pPrev = TSTATICCAST(T*, this);
			}

		private:
			T* m_pNext;
			T* m_pPrev;
		};

	public:
		TQList() = default;

		~TQList() { TASSERT(IsEmpty()); }

		void PushFront(T* a_pNode)
		{
			a_pNode->InsertAfter(&m_oRoot);
		}

		void Push(T* a_pNode)
		{
			a_pNode->InsertBefore(&m_oRoot);
		}

		T* Pop()
		{
			auto pFirstElement = m_oRoot.m_pNext;
			pFirstElement->Remove();
			return pFirstElement;
		}

		TNode::Iterator_t Head() const
		{
			return m_oRoot.m_pNext;
		}

		TNode::Iterator_t Begin() const
		{
			return m_oRoot.m_pNext;
		}

		const TNode* End() const
		{
			return &m_oRoot;
		}

		void Clear()
		{
			while (!IsEmpty())
			{
				Remove(Begin());
			}
		}

		void Remove(T* a_pNode)
		{
			a_pNode->Remove();
		}

		TBOOL IsEmpty() const
		{
			return !m_oRoot.IsLinked();
		}

	private:
		TNode m_oRoot;
	};

}