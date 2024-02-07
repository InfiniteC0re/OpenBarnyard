#pragma once
#include "T2Iterator.h"

namespace Toshi {

	template <class T>
	class TNodeList
	{
	public:
		class TNode
		{
		public:
			friend TNodeList;
			T2_DEFINE_ITERATOR(T, TNode);

		public:
			TNode()
			{
				m_Next = this;
				m_Prev = this;
				m_List = TNULL;
			}

			TNode(const TNode& node)
			{
				m_Next = node.m_Next;
				m_Prev = node.m_Prev;
				m_List = node.m_List;
			}

			void Remove()
			{
				m_List->Remove(*this);
			}

			T* Next() const
			{
				return m_Next->As<T>();
			}

			T* Prev() const
			{
				return m_Prev->As<T>();
			}

			TBOOL IsLinked() const
			{
				return m_List != TNULL;
			}

			void SetList(TNodeList* list)
			{
				m_List = list;
			}

			TNodeList* GetList() const
			{
				return m_List;
			}

			template<class T>
			T* As()
			{
				return static_cast<T*>(this);
			}

			TNode& operator=(const TNode& node)
			{
				m_Next = node.m_Next;
				m_Prev = node.m_Prev;
				m_List = node.m_List;
				return *this;
			}

		protected:
			TNodeList* m_List;
			TNode* m_Next;
			TNode* m_Prev;
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

		void InsertAfter(TNode& insertAfter, TNode& newNode)
		{
			TASSERT(!newNode.IsLinked());
			newNode.SetList(this);
			newNode.m_Next = insertAfter.m_Next;
			newNode.m_Prev = &insertAfter;
			insertAfter.m_Next->m_Prev = &newNode;
			insertAfter.m_Next = &newNode;
			m_Count++;
		}

		void InsertBefore(TNode& insertBefore, TNode& newNode)
		{
			TASSERT(!newNode.IsLinked());
			newNode.SetList(this);
			newNode.m_Next = &insertBefore;
			newNode.m_Prev = insertBefore.m_Prev;
			insertBefore.m_Prev->m_Next = &newNode;
			insertBefore.m_Prev = &newNode;
			m_Count++;
		}

		TNode* Remove(TNode& rNode)
		{
			TASSERT(rNode.GetList() == this);
			rNode.SetList(TNULL);
			rNode.m_Prev->m_Next = rNode.m_Next;
			rNode.m_Next->m_Prev = rNode.m_Prev;
			rNode.m_Next = &rNode;
			rNode.m_Prev = &rNode;
			m_Count--;
			TASSERT(rNode.IsLinked() == TFALSE);
			return &rNode;
		}

		void RemoveAll()
		{
			while (!IsEmpty())
			{
				Remove(*m_Head.Next());
			}
		}

		T* RemoveHead()
		{
			if (!IsEmpty())
			{
				return Remove(*m_Head.Next())->As<T>();
			}

			return TNULL;
		}

		T* RemoveTail()
		{
			if (!IsEmpty())
			{
				return Remove(*m_Head.Prev())->As<T>();
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
				Delete(*m_Head.Next());
			}
		}

		void Delete(T& node)
		{
			Remove(node);
			
			delete& node;
		}

		void InsertHead(T& node)
		{
			InsertAfter(m_Head, node);
		}

		void InsertTail(T& node)
		{
			InsertBefore(m_Head, node);
		}

		TBOOL IsEmpty() const
		{
			return m_Head.Next() == End();
		}

		TBOOL IsValid(const T* node) const
		{
			return node != TNULL && node->m_List == this;
		}

		size_t Count() const
		{
			return m_Count;
		}

		TNode::Iterator_t Begin() const
		{
			return m_Head.m_Next;
		}

		TNode::Iterator_t Head() const
		{
			return m_Head.m_Next;
		}

		TNode::Iterator_t Tail() const
		{
			return m_Head.m_Prev;
		}

		TNode::Iterator_t End() const
		{
			return &m_Head;
		}

	protected:
		mutable TNode m_Head;
		size_t m_Count;
	};
}