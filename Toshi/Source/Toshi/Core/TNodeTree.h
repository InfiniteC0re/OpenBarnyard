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

			T* Parent() const
			{
				return m_pParent->Cast();
			}

			T* Child(int index) const
			{
				return m_pChildren[index]->Cast();
			}

			T* Cast()
			{
				return static_cast<T*>(this);
			}
		};

		TNodeTree()
		{
			m_pRoot = TNULL;
		}

		T* Root() const
		{
			TASSERT(TTRUE == IsLinked(), "The tree doesn't have root");
			return m_pRoot->Cast();
		}
		
		bool IsLinked() const
		{
			return m_pRoot != TNULL;
		}

		void InsertRoot(TNode* a_pRoot)
		{
			m_pRoot = a_pRoot;
			a_pRoot = TNULL;
		}
		
		void RemoveRoot()
		{
			m_pRoot = TNULL;
		}

		TNodeTree& operator=(const TNodeTree& a_pNodeTree)
		{
			m_pRoot = a_pNodeTree.m_pRoot;
			return this;
		}
		
	protected:
		TNode* m_pRoot; // 0x0
	};
}


