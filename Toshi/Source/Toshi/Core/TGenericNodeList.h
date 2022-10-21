#pragma once
namespace Toshi
{
	class TGenericNodeList
	{
	public:
		class TNode
		{
		public:
			TGenericNodeList* m_pList; // 0x4
			TNode* m_pNext; // 0x8
			TNode* m_pPrev; // 0xC

			inline TGenericNodeList* GetList() const { return m_pList; }
			inline TNode* Next() const { return m_pNext; }
			inline TNode* Prev() const { return m_pPrev; }
			inline bool IsLinked() const { return GetList() != TNULL; }
			inline TGenericNodeList* SetList(TGenericNodeList* a_pList) { m_pList = a_pList; return m_pList; }
		
			TNode()
			{
				m_pNext = this;
				m_pPrev = this;
				SetList(TNULL);
			}

			TNode(const TNode& a_pNode)
			{
				m_pList = a_pNode.GetList();
				m_pNext = a_pNode.Next();
				m_pPrev = a_pNode.Prev();
			}

			TNode operator=(const TNode& a_pNode)  { return TNode(a_pNode); }
		
		public:
			template<class T> friend class TNodeList;
			friend TGenericNodeList;
		};

		class Iterator
		{
			TNode* m_pIterator;

			inline bool IsNull() { return m_pIterator != TNULL; }
			inline bool operator!() { return IsNull(); }
			inline bool operator!=(Iterator& a_rIterator) const { return m_pIterator != a_rIterator.m_pIterator; }
			TNode& operator*() const { return *m_pIterator; }

			Iterator()
			{
				m_pIterator = TNULL;
			}

			Iterator(TNode* a_pNode)
			{
				m_pIterator = a_pNode;
			}

			Iterator(const Iterator& a_rIterator)
			{
				m_pIterator = a_rIterator.m_pIterator;
			}
		};
	};

	template <class T>
	class TNodeList : public TGenericNodeList
	{
	public:
		TNodeList() { }

		/*inline bool IsLinked() { return m_Root.IsLinked(); }
		inline T* GetFirst() { return static_cast<T*>(m_Root.Next()); }
		inline T& GetFirstRef() { return static_cast<T&>(*m_Root.Next()); }
		inline T* GetRoot() { return static_cast<T*>(&m_Root); }
		inline T& GetRootRef() { return static_cast<T&>(m_Root); }*/

	protected:
	};
}


