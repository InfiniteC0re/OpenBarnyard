#pragma once

namespace Toshi
{
	template <class T>
	class TNodeList
	{
	public:
		class TNode
		{
		public:
			friend TNodeList;

		public:
			TNode()
			{
				m_Next = this;
				m_Prev = this;
				m_List = nullptr;
			}

			TNode* Next()
			{
				return m_Next;
			}

			TNode* Prev()
			{
				return m_Prev;
			}

			bool IsLinked()
			{
				return m_List != TNULL;
			}

			void SetList(TNodeList* list)
			{
				m_List = list;
			}

			template<class T>
			T* As()
			{
				return static_cast<T*>(this);
			}

		protected:
			TNodeList* m_List;
			TNode* m_Next;
			TNode* m_Prev;
		};

		class Iterator
		{
		public:
			Iterator(TNode* pPtr)
			{
				m_pPtr = pPtr;
			}

			Iterator(T* pPtr)
			{
				m_pPtr = static_cast<TNode*>(pPtr);
			}

			void operator=(const Iterator& other)
			{
				m_pPtr = other.m_pPtr;
			}

			bool operator==(const Iterator& other) const
			{
				return m_pPtr == other.m_pPtr;
			}

			T* operator->() const
			{
				return static_cast<T*>(m_pPtr);
			}

			operator T* () const
			{
				return static_cast<T*>(m_pPtr);
			}

			Iterator operator++()
			{
				m_pPtr = m_pPtr->Next();
				return m_pPtr;
			}

			Iterator operator--()
			{
				m_pPtr = m_pPtr->Prev();
				return m_pPtr;
			}

		private:
			TNode* m_pPtr;
		};

	public:
		TNodeList()
		{
			m_Count = 0;
		}

		~TNodeList()
		{
			DeleteAll();
		}

		void InsertAfter(TNode* insertAfter, TNode* newNode)
		{
			TASSERT(!newNode->IsLinked());
			newNode->SetList(this);
			newNode->m_Next = insertAfter->m_Next;
			newNode->m_Prev = insertAfter;
			insertAfter->m_Next->m_Prev = newNode;
			insertAfter->m_Next = newNode;
			m_Count += 1;
		}

		void InsertBefore(TNode* insertBefore, TNode* newNode)
		{
			TASSERT(!newNode->IsLinked());
			newNode->SetList(this);
			newNode->m_Next = insertBefore;
			newNode->m_Prev = insertBefore->m_Prev;
			insertBefore->m_Prev->m_Next = newNode;
			insertBefore->m_Prev = newNode;
			m_Count += 1;
		}

		TNode* Remove(TNode* node)
		{
			TASSERT(node != TNULL);
			node->SetList(TNULL);
			node->m_Prev->m_Next = node->m_Next;
			node->m_Next->m_Prev = node->m_Prev;
			node->m_Next = node;
			node->m_Prev = node;
			m_Count -= 1;

			return node;
		}

		TNode* RemoveHead()
		{
			if (!IsEmpty())
			{
				return Remove(m_Head.Next());
			}

			return TNULL;
		}

		TNode* RemoveTail()
		{
			if (!IsEmpty())
			{
				return Remove(m_Head.Prev());
			}

			return TNULL;
		}

		void DeleteHead()
		{
			Delete(Head());
		}

		void DeleteTail()
		{
			Delete(Tail());
		}

		void DeleteAll()
		{
			while (!IsEmpty())
			{
				Delete(m_Head.Next());
			}
		}

		void Delete(TNode* node)
		{
			TASSERT(node != TNULL);
			Remove(node);

			static_cast<T*>(node)->~T();
		}

		void InsertHead(TNode* node)
		{
			InsertAfter(&m_Head, node);
		}

		void InsertTail(TNode* node)
		{
			InsertBefore(&m_Head, node);
		}

		bool IsEmpty()
		{
			return m_Head.Next() == &m_Head;
		}

		bool IsValid(const TNode* node)
		{
			return node != TNULL && node->m_List == this;
		}

		TNode* Head()
		{
			return m_Head.Next();
		}

		TNode* Tail()
		{
			return m_Head.Prev();
		}

		size_t Count() const
		{
			return m_Count;
		}

		Iterator Begin() const
		{
			return m_Head.m_Next;
		}

		Iterator End()
		{
			return &m_Head;
		}

	protected:
		TNode m_Head;
		size_t m_Count;
	};
}