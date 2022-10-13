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
			// TNode(TUninitialized param1);

			inline TNode* Next() const { return m_Next; }
			inline TNode* Prev() const { return m_Prev; }
			inline bool IsLinked() const { return this != m_Next; }
			inline void Reset() { m_Next = this; m_Prev = this; }

			void InsertAfter(TNode* node);
			void InsertBefore(TNode* node);
			void Remove();

			TNode& operator=(const TNode& node);

		public:
			template<class T> friend class TDList;
			friend TGenericDList;

		public:
			template<typename T>
			inline T* As() { return static_cast<T*>(this); }

		protected:
			TNode* m_Next;
			TNode* m_Prev;
		};

	public:
		inline void InsertHead(TNode* node) { node->InsertAfter(&m_Root); }
		inline void InsertTail(TNode* node) { node->InsertBefore(&m_Root); }
		inline void RemoveHead()            { if (!IsEmpty()) m_Root.Next()->Remove(); }
		inline void RemoveTail()            { if (!IsEmpty()) m_Root.Prev()->Remove(); }
		inline bool IsEmpty()               { return m_Root.Next() == &m_Root; }
		inline TNode* Head()                { return m_Root.Next(); }
		inline TNode* Tail()                { return m_Root.Prev(); }
		inline TNode* Begin()               { return m_Root.Next(); }
		inline TNode* End()                 { return &m_Root; }

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
		TGenericDList();
		~TGenericDList() { RemoveAll(); }

	protected:
		TNode m_Root;
	};

	template <class T>
	class TDList : public TGenericDList
	{
	public:
		TDList() { }

		inline T* Head()         { return TGenericDList::Head()->As<T>(); }
		inline T* Tail()         { return TGenericDList::Tail()->As<T>(); }
		inline T* Begin()        { return TGenericDList::Begin()->As<T>(); }
		inline T* End()          { return TGenericDList::End()->As<T>(); }
		inline bool IsEmpty()    { return TGenericDList::IsEmpty(); }
		inline bool IsLinked()   { return m_Root.IsLinked(); }
		inline void RemoveHead() { TGenericDList::RemoveHead(); }
		inline void RemoveTail() { TGenericDList::RemoveTail(); }
	};

	template <class T>
	using T2DList = TDList<T>;
}


