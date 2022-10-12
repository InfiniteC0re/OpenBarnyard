#pragma once
namespace Toshi
{
	class TGenericDList
	{
	public:
		class TNode
		{
		protected:
			TNode();
			// TNode(TUninitialized param1);

			inline TNode* Next() const { return m_pNext; }
			inline TNode* Prev() const { return m_pPrev; }
			inline bool IsLinked() const { return this != m_pNext; }

			void InsertAfter(TNode* node);
			void InsertBefore(TNode* node);
			void Remove();
			void Reset();

			TNode& operator=(const TNode& node);

		public:
			template<class T> friend class TDList;
			friend TGenericDList;

		public:
			template<typename T>
			inline T* As() { return static_cast<T*>(this); }

		protected:
			TNode* m_pNext;
			TNode* m_pPrev;
		};

	public:
		void InsertHead(TNode* node) { node->InsertAfter(&m_Root); }
		void InsertTail(TNode* node) { node->InsertBefore(&m_Root); }

		static void InsertSegmentAfter(TNode* node1, TNode* node2, TNode* node3)
		{
			node1->m_pNext = node3;
			node2->m_pPrev = node3->m_pPrev;
			node1->m_pNext->m_pPrev = node1;
			node2->m_pPrev->m_pNext = node2;
		}

		static void InsertSegmentBefore(TNode* node1, TNode* node2, TNode* node3)
		{
			node2->m_pPrev = node3;
			node1->m_pNext = node3->m_pNext;
			node2->m_pPrev->m_pNext = node2;
			node1->m_pNext->m_pPrev = node1;
		}

		void InsertSegmentAtHead(TNode* node1, TNode* node2)
		{
			node1->m_pNext = &m_Root;
			node2->m_pPrev = m_Root.m_pPrev;
			node1->m_pNext->m_pPrev = node1;
			node2->m_pPrev->m_pNext = node2;
		}

		void InsertSegmentAtTail(TNode* node1, TNode* node2)
		{
			node2->m_pPrev = &m_Root;
			node1->m_pNext = m_Root.m_pNext;
			node2->m_pPrev->m_pNext = node2;
			node1->m_pNext->m_pPrev = node1;
		}

	protected:
		TGenericDList();

	protected:
		TNode m_Root;
	};

	template <class T>
	class TDList : public TGenericDList
	{
	public:
		TDList() { }

		inline bool IsLinked()  { return m_Root.IsLinked(); }
		inline T* GetRoot()     { return m_Root.As<T>(); }
		inline T* GetFirst()    { return m_Root.Next()->As<T>(); }
		inline T& GetRootRef()  { return *GetRootRef(); }
		inline T& GetFirstRef() { return *GetFirst(); }
	};

	template <class T>
	using T2DList = TDList<T>;
}


