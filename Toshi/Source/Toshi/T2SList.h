#pragma once
#include "Toshi/Typedefs.h"

namespace Toshi {

	template <class T, int Unknown = 0>
	class T2SList
	{
	public:
		class Node
		{
		public:
			friend class T2SList;

		public:
			Node()
			{
				m_pNext = this;
			}

			T* GetNext() const
			{
				return TSTATICCAST(T*, m_pNext);
			}

			TBOOL IsLinked() const
			{
				return m_pNext != this;
			}

			void InsertAfter(Node* a_pNode)
			{
				TASSERT(!IsLinked());
				m_pNext = a_pNode->m_pNext;
				a_pNode->m_pNext = this;
			}

		private:
			Node* m_pNext;
		};

	public:
		~T2SList()
		{
			Clear();
		}

		T* Begin()
		{
			return m_oRoot.GetNext();
		}

		T* End()
		{
			return TREINTERPRETCAST(T*, &m_oRoot);
		}

		TBOOL IsLinked() const
		{
			return m_oRoot.m_pNext != &m_oRoot;
		}

		T* Back()
		{
			T* pBack = End();

			for (auto it = Begin(); it != End(); it = it->GetNext())
			{
				pBack = it;
			}

			return pBack;
		}

		T* FindNodeBefore(T* a_pNode)
		{
			T* pResult = End();

			for (auto it = Begin(); it != a_pNode; it = it->GetNext())
			{
				TASSERT(it != End());
				pResult = it;
			}

			return pResult;
		}

		void PushBack(T* a_pNode)
		{
			a_pNode->InsertAfter(Back());
		}

		void PushFront(T* a_pNode)
		{
			a_pNode->InsertAfter(&m_oRoot);
		}

		T* PopBack()
		{
			T* pBack = Back();
			T* pNodeBefore = FindNodeBefore(pBack);

			pNodeBefore->m_pNext = TSTATICCAST(T*, &m_oRoot);
			pBack->m_pNext = pBack;

			return pBack;
		}

		T* PopFront()
		{
			T* pNode = Begin();
			m_oRoot.m_pNext = pNode->m_pNext;
			pNode->m_pNext = pNode;
			return pNode;
		}

		T* Erase(T* a_pFrom, T* a_pTo)
		{
			T* pNodeBefore = FindNodeBefore(a_pFrom);

			while (a_pFrom != a_pTo)
			{
				T* pNext = TSTATICCAST(T*, a_pFrom->m_pNext);
				a_pFrom->m_pNext = a_pFrom;
				a_pFrom = pNext;
			}

			pNodeBefore->m_pNext = a_pTo;
			return a_pTo;
		}

		void Clear()
		{
			T* pNode = Begin();

			while (pNode != End())
			{
				T* pNext = TSTATICCAST(T*, pNode->m_pNext);
				pNode->m_pNext = pNode;
				pNode = pNext;
			}

			m_oRoot.m_pNext = TSTATICCAST(T*, &m_oRoot);
		}

		TUINT Size()
		{
			TUINT uiSize = 0;

			for (auto it = Begin(); it != End(); it = it->GetNext())
			{
				uiSize++;
			}

			return uiSize;
		}

		T* Transfer(T* a_pNode, T2SList& a_rList)
		{
			T* pNodeBefore = FindNodeBefore(a_pNode);
			pNodeBefore->m_pNext = a_pNode->m_pNext;
			a_pNode->m_pNext = a_pNode;
			a_rList.PushBack(a_pNode);

			return pNodeBefore;
		}

		T* Remove(T*& a_rpRemovedNode, T* a_pNode)
		{
			Erase(a_pNode, a_pNode->GetNext());
			a_rpRemovedNode = a_pNode;
			return a_rpRemovedNode;
		}

	private:
		Node m_oRoot;
	};

}