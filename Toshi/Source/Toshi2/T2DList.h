#pragma once
#include <type_traits>

namespace Toshi
{
	class T2GenericDList
	{
	public:
		class Node
		{
		protected:
			friend T2GenericDList;
			template<class T> friend class T2DList;

		protected:
			Node()
			{
				m_pNext = this;
				m_pPrev = this;
			}

			bool IsLinked() const
			{
				return m_pNext != this;
			}

			void InsertBefore(Node* pInsertBefore)
			{
				TASSERT(!IsLinked());
				m_pNext = pInsertBefore;
				m_pPrev = pInsertBefore->m_pPrev;
				pInsertBefore->m_pPrev = this;
				m_pPrev->m_pNext = this;
			}

			void InsertAfter(Node* pInsertAfter)
			{
				TASSERT(!IsLinked());
				m_pPrev = pInsertAfter;
				m_pNext = pInsertAfter->m_pNext;
				pInsertAfter->m_pNext = this;
				m_pNext->m_pPrev = this;
			}

			void Remove()
			{
				TASSERT(IsLinked());
				m_pPrev->m_pNext = this->m_pNext;
				m_pNext->m_pPrev = this->m_pPrev;
				m_pNext = this;
				m_pPrev = this;
			}

			Node* Next() const
			{
				return m_pNext;
			}

			Node* Prev() const
			{
				return m_pPrev;
			}

		private:
			Node* m_pNext;
			Node* m_pPrev;
		};

	protected:
		T2GenericDList()
		{
			
		}

		bool IsLinked() const
		{
			return m_oRoot.IsLinked();
		}

		void ClearBefore(Node* pNode)
		{
			if (pNode != &m_oRoot)
			{
				Node* pCurrentNode = m_oRoot.m_pNext;

				while (pCurrentNode != pNode)
				{
					IsLinked();
					pCurrentNode->Remove();
					pCurrentNode = m_oRoot.m_pNext;
				}
			}
		}

		void Clear()
		{
			Node* pNode = m_oRoot.m_pNext;

			while (pNode != &m_oRoot)
			{
				IsLinked();
				pNode->Remove();
				pNode = m_oRoot.m_pNext;
			}
		}

		bool IsInList(Node* pNode) const
		{
			for (Node* pCNode = m_oRoot.m_pNext; pCNode != &m_oRoot; pCNode = pCNode->m_pNext)
			{
				if (pCNode == pCNode) return true;
			}

			return false;
		}

		size_t Size() const
		{
			size_t size = 0;

			for (Node* pNode = m_oRoot.m_pNext; pNode != &m_oRoot; pNode = pNode->m_pNext)
			{
				size += 1;
			}

			return size;
		}

	protected:
		Node m_oRoot;
	};

	template <class T>
	class T2DList : public T2GenericDList
	{
	public:
		class Iterator
		{
		public:
			Iterator()
			{
				m_pPtr = TNULL;
			}

			Iterator(Node* pPtr)
			{
				m_pPtr = static_cast<T*>(pPtr);
			}

			Iterator(T* pPtr)
			{
				m_pPtr = pPtr;
			}

			bool operator==(const Iterator& other)
			{
				return m_pPtr == other.m_pPtr;
			}

			bool operator!=(const Iterator& other)
			{
				return m_pPtr != other.m_pPtr;
			}

			void operator=(const Iterator& other)
			{
				m_pPtr = other.m_pPtr;
			}

			void operator=(const T* pPtr)
			{
				m_pPtr = pPtr;
			}

			T* operator->() const
			{
				TASSERT(m_pPtr != TNULL);
				return m_pPtr;
			}

			operator T* () const
			{
				TASSERT(m_pPtr != TNULL);
				return static_cast<T*>(m_pPtr);
			}

			Iterator operator++(int)
			{
				TASSERT(m_pPtr != TNULL);
				Iterator old = m_pPtr;
				m_pPtr = static_cast<T*>(m_pPtr->Next());
				return old;
			}

			Iterator operator--(int)
			{
				TASSERT(m_pPtr != TNULL);
				Iterator old = m_pPtr;
				m_pPtr = static_cast<T*>(m_pPtr->Prev());
				return old;
			}

			Iterator operator++()
			{
				TASSERT(m_pPtr != TNULL);
				m_pPtr = m_pPtr->Next();
				return m_pPtr;
			}

			Iterator operator--()
			{
				TASSERT(m_pPtr != TNULL);
				m_pPtr = m_pPtr->Prev();
				return m_pPtr;
			}

		private:
			T* m_pPtr;
		};

	public:
		T2DList()
		{
			static_assert(std::is_base_of<T2GenericDList::Node, T>::value, "T must be a descendant of T2GenericDList::Node");
		}

		~T2DList()
		{
			TASSERT(IsEmpty());
		}

		Iterator Begin() const
		{
			return m_oRoot.Next();
		}

		const Node* End() const
		{
			return &m_oRoot;
		}

		void Begin(Iterator& begin) const
		{
			begin = m_oRoot.Next();
		}

		void End(const Node& end) const
		{
			end = &m_oRoot;
		}

		Iterator Front() const
		{
			return m_oRoot.Next();
		}

		Iterator Back() const
		{
			return m_oRoot.Prev();
		}

		void Delete(const Iterator& iter)
		{
			iter->Remove();
			delete static_cast<T*>(iter);
		}

		void DeleteAll()
		{
			for (auto it = Begin(); it != End(); it++)
			{
				Delete(it);
			}
		}

		void Erase(const Iterator& iter)
		{
			iter->Remove();
		}

		bool IsEmpty() const
		{
			return !m_oRoot.IsLinked();
		}

		void PushBack(T* pItem)
		{
			pItem->InsertBefore(&m_oRoot);
		}

		void PushFront(T* pItem)
		{
			pItem->InsertAfter(&m_oRoot);
		}

		T* PopBack()
		{
			Iterator node = Back();
			node->Remove();
			return node;
		}

		T* PopFront()
		{
			Iterator node = Front();
			node->Remove();
			return node;
		}
	};
}