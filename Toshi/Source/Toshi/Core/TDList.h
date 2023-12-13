#pragma once
#include "Toshi2/T2Iterator.h"

namespace Toshi
{
	class TGenericDList
	{
	public:
		class TNode
		{
		protected:
			TNode()
			{
				Reset();
			}

			~TNode()
			{
				m_Next->m_Prev = m_Prev;
				m_Prev->m_Next = m_Next;
				m_Next = this;
				m_Prev = this;
			}

			TNode* Next() const
			{
				return m_Next;
			}

			TNode* Prev() const
			{
				return m_Prev;
			}

		public:
			TNode& operator=(const TNode& node)
			{
				m_Next = node.m_Next;
				m_Prev = node.m_Prev;
				return *this;
			}

			TBOOL IsLinked() const { return this != m_Next; }
			void Reset() { m_Next = this; m_Prev = this; }

			void InsertAfter(TNode* node)
			{
				TASSERT(!IsLinked(), "TNode::InsertAfter - TNode shouldn't be linked");

				m_Prev = node;
				m_Next = node->m_Next;
				node->m_Next = this;
				m_Next->m_Prev = this;
			}

			void InsertBefore(TNode* node)
			{
				TASSERT(!IsLinked(), "TNode::InsertBefore - TNode shouldn't be linked");

				m_Next = node;
				m_Prev = node->m_Prev;
				node->m_Prev = this;
				m_Prev->m_Next = this;
			}

			void Remove()
			{
				m_Prev->m_Next = m_Next;
				m_Next->m_Prev = m_Prev;
				Reset();
			}

		public:
			template<class T> friend class TDList;
			template<class T, class Node> friend class T2Iterator;
			friend TGenericDList;

		public:
			template<typename T>
			T* As() { return static_cast<T*>(this); }

		public:
			TNode* m_Next;
			TNode* m_Prev;
		};

	public:
		void InsertHead(TNode* node) { node->InsertAfter(&m_Root); }
		void InsertTail(TNode* node) { node->InsertBefore(&m_Root); }
		void RemoveHead()            { if (!IsEmpty()) m_Root.Next()->Remove(); }
		void RemoveTail()            { if (!IsEmpty()) m_Root.Prev()->Remove(); }
		TBOOL IsEmpty()               { return m_Root.Next() == &m_Root; }
		TNode* Head()                { return m_Root.Next(); }
		TNode* Tail()                { return m_Root.Prev(); }
		TNode* Begin()               { return m_Root.Next(); }
		TNode* End()                 { return &m_Root; }

		static void InsertSegmentAfter(TNode* node1, TNode* node2, TNode* node3)
		{
			node1->m_Next = node3;
			node2->m_Prev = node3->m_Prev;
			node1->m_Next->m_Prev = node1;
			node2->m_Prev->m_Next = node2;
		}

		static void InsertSegmentBefore(TNode* node1, TNode* node2, TNode* node3)
		{
			node2->m_Prev = node3;
			node1->m_Next = node3->m_Next;
			node2->m_Prev->m_Next = node2;
			node1->m_Next->m_Prev = node1;
		}

		void InsertSegmentAtHead(TNode* node1, TNode* node2)
		{
			node1->m_Next = &m_Root;
			node2->m_Prev = m_Root.m_Prev;
			node1->m_Next->m_Prev = node1;
			node2->m_Prev->m_Next = node2;
		}

		void InsertSegmentAtTail(TNode* node1, TNode* node2)
		{
			node2->m_Prev = &m_Root;
			node1->m_Next = m_Root.m_Next;
			node2->m_Prev->m_Next = node2;
			node1->m_Next->m_Prev = node1;
		}

		void RemoveAll()
		{
			auto pNode = m_Root.Next();

			while (pNode != &m_Root)
			{
				pNode->Remove();
				pNode = m_Root.Next();
			}
		}

	protected:
		TGenericDList()
		{
			m_Root.m_Next = &m_Root;
			m_Root.m_Prev = &m_Root;
		}

		~TGenericDList() { RemoveAll(); }

	public:
		TNode m_Root;
	};

	class TGenericPriList
	{
	public:
		class TNode
		{
		protected:
			TNode()
			{
				Reset();
			}

			TNode* Next() const { return m_Next; }
			TNode* Prev() const { return m_Prev; }

		public:
			TNode& operator=(const TNode& node)
			{
				m_Next = node.m_Next;
				m_Prev = node.m_Prev;
				SetPriority(node.GetPriority());
				return *this;
			}

			void Reset()
			{
				m_Next = this;
				m_Prev = this;
				SetPriority(0);
			}

			void InsertAfter(TNode* node)
			{
				TASSERT(!IsLinked(), "TNode::InsertAfter - TNode shouldn't be linked");

				m_iPriority = -0x8000;
				m_Prev = node;
				m_Next = node->m_Next;
				node->m_Next = this;
				m_Next->m_Prev = this;
			}

			void InsertBefore(TNode* node)
			{
				TASSERT(!IsLinked(), "TNode::InsertBefore - TNode shouldn't be linked");

				m_iPriority = 0x7FFF;
				m_Next = node;
				m_Prev = node->m_Prev;
				node->m_Prev = this;
				m_Prev->m_Next = this;
			}

			void Remove()
			{
				m_Prev->m_Next = m_Next;
				m_Next->m_Prev = m_Prev;
				m_Next = this; 
				m_Prev = this;
			}

			void SetPriority(int priority)
			{
				m_iPriority = priority;
			}

			int GetPriority() const
			{
				return m_iPriority;
			}

			TBOOL IsLinked() const
			{
				return this != m_Next;
			}

		public:
			template<class T> friend class TPriList;
			template<class T, class Node> friend class T2Iterator;
			friend TGenericPriList;

		public:
			template<typename T>
			T* As() { return static_cast<T*>(this); }

			template<typename T>
			T* operator*() { return static_cast<T*>(this); }

		public:
			TNode* m_Next;
			TNode* m_Prev;
			int m_iPriority;
		};

	public:
		void InsertHead(TNode* node) { node->InsertAfter(&GetRoot()); }
		void InsertTail(TNode* node) { node->InsertBefore(&GetRoot()); }
		void RemoveHead() { if (!IsEmpty()) m_Next->Remove(); }
		void RemoveTail() { if (!IsEmpty()) m_Prev->Remove(); }
		TBOOL IsEmpty() { return m_Next == &GetRoot(); }
		TNode* Head() { return m_Next; }
		TNode* Tail() { return m_Prev; }
		TNode* Begin() { return m_Next; }
		TNode* End() { return &GetRoot(); }

		void Insert(TNode* node, int iPriority)
		{
			node->SetPriority(iPriority);
			Insert(node);
		}

		void Insert(TNode* node)
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

		void RemoveAll()
		{
			auto pNode = m_Next;

			while (pNode != &GetRoot())
			{
				pNode->Remove();
				pNode = m_Next;
			}
		}

	protected:
		TGenericPriList()
		{
			m_Next = &GetRoot();
			m_Prev = &GetRoot();
		}

		~TGenericPriList() { RemoveAll(); }

		TNode& GetRoot()
		{
			return *TREINTERPRETCAST(TNode*, this);
		}

	public:
		TNode* m_Next;
		TNode* m_Prev;
	};

	template <class T>
	class TPriList : public TGenericPriList
	{
	public:
		TPriList() { }

		T2Iterator<T, TNode> Begin() { return TGenericPriList::Begin(); }
		T2Iterator<T, TNode> End()   { return TGenericPriList::End(); }
		TBOOL IsLinked()              { return GetRoot().IsLinked(); }
	};

	template <class T>
	class TDList : public TGenericDList
	{
	public:
		TDList() { }

		T* Head()                    { return TGenericDList::Head()->As<T>(); }
		T* Tail()                    { return TGenericDList::Tail()->As<T>(); }
		T2Iterator<T, TNode> Begin() { return TGenericDList::Begin()->As<T>(); }
		T2Iterator<T, TNode> End()   { return TGenericDList::End()->As<T>(); }
		TBOOL IsEmpty()               { return TGenericDList::IsEmpty(); }
		TBOOL IsLinked()              { return m_Root.IsLinked(); }
		void RemoveHead()            { TGenericDList::RemoveHead(); }
		void RemoveTail()            { TGenericDList::RemoveTail(); }
	};
}


