#pragma once
namespace Toshi
{
	class TGenericDList
	{
	public:
		class TNode
		{
		protected:
			TNode() { Reset(); }

			TNode* Next() const { return m_Next; }
			TNode* Prev() const { return m_Prev; }

			TNode& operator=(const TNode& node)
			{
				m_Next = node.m_Next;
				m_Prev = node.m_Prev;
				return *this;
			}

		public:
			bool IsLinked() const { return this != m_Next; }
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
		bool IsEmpty()               { return m_Root.Next() == &m_Root; }
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
		TGenericDList() = default;
		~TGenericDList() { RemoveAll(); }

	public:
		TNode m_Root;
	};

	template <class T>
	class TDList : public TGenericDList
	{
	public:
		TDList() { }

		T* Head()         { return TGenericDList::Head()->As<T>(); }
		T* Tail()         { return TGenericDList::Tail()->As<T>(); }
		T* Begin()        { return TGenericDList::Begin()->As<T>(); }
		T* End()          { return TGenericDList::End()->As<T>(); }
		bool IsEmpty()    { return TGenericDList::IsEmpty(); }
		bool IsLinked()   { return m_Root.IsLinked(); }
		void RemoveHead() { TGenericDList::RemoveHead(); }
		void RemoveTail() { TGenericDList::RemoveTail(); }
	};

	template <class T>
	using T2DList = TDList<T>;
}


