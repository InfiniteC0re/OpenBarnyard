#pragma once
#include "Toshi/Typedefs.h"

namespace Toshi
{
	template <class T>
	class TNodeTree
	{
	public:
		class TNode
		{
			
		public:
			TNode* m_pParent;
			TNode** m_pChildren;
			T* m_value;
			inline TNode* Parent() const { return m_pParent; }
			inline TNode* Child(int param_1) const { return m_pChildren[param_1]; }
		};

		TNode* m_pRoot; // 0x0

		inline TNodeTree() { m_pRoot = TNULL; }

		inline TNode* Root() { return m_pRoot; }
		inline void InsertRoot(TNode* a_pRoot) { m_pRoot = a_pRoot; a_pRoot = TNULL; }
		inline void RemoveRoot() { m_pRoot = TNULL; }

		inline TNodeTree& operator=(const TNodeTree& a_pNodeTree) { m_pRoot = a_pNodeTree.m_pRoot; return this; }

	
	};
}


