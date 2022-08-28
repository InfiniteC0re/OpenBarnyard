#pragma once
namespace Toshi
{
	//template <typename T, int>
	class TGenericDList
	{
		TGenericDList();

	public: 
		class TNode
		{
		protected:
			TNode();
			// TNode(TUninitialized param1);

			inline TNode* Next() const { return m_pNextNode; }
			inline TNode* Prev() const { return m_pPrevNode; }
			inline bool IsLinked() const { return this != m_pNextNode; }

			void InsertAfter(TNode*);
			void InsertBefore(TNode*);
			void Remove();
			void Reset();

			TNode& operator=(const TNode& param_1);

			TNode* m_pNextNode; // 0x0
			TNode* m_pPrevNode; // 0x4
		};
	};


}


