#pragma once
#include "Toshi2/T2Iterator.h"

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

			TNode(const TNode& node)
			{
				m_Next = node.m_Next;
				m_Prev = node.m_Prev;
				m_List = node.m_List;
			}

			T* Next() const
			{
				return m_Next->As<T>();
			}

			T* Prev() const
			{
				return m_Prev->As<T>();
			}

			bool IsLinked() const
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

		TNode* Remove(TNode& node)
		{
			TASSERT(&node != TNULL);
			node.SetList(TNULL);
			node.m_Prev->m_Next = node.m_Next;
			node.m_Next->m_Prev = node.m_Prev;
			node.m_Next = &node;
			node.m_Prev = &node;
			m_Count--;

			return &node;
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
				Delete(*m_Head.Next());
			}
		}

		void Delete(TNode& node)
		{
			TASSERT(&node != TNULL);
			Remove(node);

			delete &node;
		}

		void InsertHead(TNode& node)
		{
			InsertAfter(m_Head, node);
		}

		void InsertTail(TNode& node)
		{
			InsertBefore(m_Head, node);
		}

		bool IsEmpty() const
		{
			return m_Head.Next() == &m_Head;
		}

		bool IsValid(const TNode* node) const
		{
			return node != TNULL && node->m_List == this;
		}

		TNode* Head() const
		{
			return m_Head.Next();
		}

		TNode* Tail() const
		{
			return m_Head.Prev();
		}

		size_t Count() const
		{
			return m_Count;
		}

		T2Iterator<T, TNode> Begin() const
		{
			return m_Head.m_Next;
		}

		T2Iterator<T, TNode> End() const
		{
			return &m_Head;
		}

	protected:
		TNode m_Head;
		size_t m_Count;
	};
}