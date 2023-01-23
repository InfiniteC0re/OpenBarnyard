#pragma once
#include "T2Allocator.h"

namespace Toshi
{
	class T2GenericRedBlackTreeNode
	{
	public:
		friend class T2GenericRedBlackTree;

	public:
		constexpr T2GenericRedBlackTreeNode()
		{
			red = 0;
			m_pNode2 = this;
			m_pNode1 = this;
			m_pNode3 = this;
		}

		T2GenericRedBlackTreeNode(T2GenericRedBlackTreeNode* pNil)
		{
			red = 0;
			m_pNode2 = pNil;
			m_pNode1 = pNil;
			m_pNode3 = pNil;
		}

		virtual ~T2GenericRedBlackTreeNode() = default;

	private:
		int red;
		T2GenericRedBlackTreeNode* m_pNode2;
		T2GenericRedBlackTreeNode* m_pNode1;
		T2GenericRedBlackTreeNode* m_pNode3;
	};

	class T2GenericRedBlackTree
	{
	public:
		T2GenericRedBlackTree(T2Allocator* pAllocator) : m_oRoot(ms_oNil)
		{
			m_pAllocator = pAllocator;
			m_iNumElements = 0;
		}

		~T2GenericRedBlackTree()
		{
			TASSERT(m_iNumElements == 0);
		}

		T2GenericRedBlackTreeNode* GetFirstNode();
		T2GenericRedBlackTreeNode* Insert(T2GenericRedBlackTreeNode* pNode);
		void DeleteFixUp(T2GenericRedBlackTreeNode* pNode);
		T2GenericRedBlackTreeNode* DeleteNode(T2GenericRedBlackTreeNode* pNode);

		void LeftRotate(T2GenericRedBlackTreeNode* pNode);
		void RightRotate(T2GenericRedBlackTreeNode* pNode);
		static T2GenericRedBlackTreeNode* GetSuccessorOf(T2GenericRedBlackTreeNode* pNode);
		static T2GenericRedBlackTreeNode* GetPredecessorOf(T2GenericRedBlackTreeNode* pNode);

	private:
		void CheckValid()
		{
			TASSERT(ms_oNil.red == 0);
			TASSERT(m_oRoot.red == 0);
		}

	private:
		static constinit T2GenericRedBlackTreeNode ms_oNil;

	private:
		T2Allocator* m_pAllocator;
		T2GenericRedBlackTreeNode m_oRoot;
		size_t m_iNumElements;
	};
}