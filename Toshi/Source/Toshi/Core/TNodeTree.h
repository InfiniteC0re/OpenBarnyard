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
			friend TNodeTree;

		public:
			TNode()
			{
				m_Root = TNULL;
				m_Next = this;
				m_Last = this;
				m_Parent = TNULL;
				m_Attached = TNULL;
			}

			TNode(void* dummyptr)
			{
				m_Root = TNULL;
				m_Next = (TNode*)dummyptr;
				m_Last = (TNode*)dummyptr;
				m_Parent = TNULL;
				m_Attached = TNULL;
			}

		protected:
			bool IsLinked() const
			{
				return m_Root != TNULL;
			}

			T* Parent() const
			{
				return m_Parent->Cast();
			}

			T* Next() const
			{
				return m_Next->Cast();
			}

			T* Last() const
			{
				return m_Last->Cast();
			}

			TNodeTree<T>* Root() const
			{
				return m_Root;
			}

			T* Attached() const
			{
				return m_Attached->Cast();
			}

			T* Cast()
			{
				return static_cast<T*>(this);
			}

		protected:
			TNodeTree<T>* m_Root;
			TNode* m_Next;
			TNode* m_Last;
			TNode* m_Parent;
			TNode* m_Attached;
		};

	public:
		TNodeTree()
		{
			m_Count = 0;
		}

		TNodeTree(void* dummyptr) : m_Root(dummyptr)
		{
			m_Count = 0;
		}

		TNode& Root()
		{
			return m_Root;
		}

		/**
		* Inserts node to the tree and links it.
		* 
		* @param parentNode The node that is going to be a parent.
		* @param sourceNode The node that should be linked.
		*/
		void Insert(TNode* parentNode, TNode* sourceNode)
		{
			// Toshi::TNodeTree<Toshi::TResource>::InsertNode - 00691aa0
			TASSERT(sourceNode->IsLinked() == TFALSE, "The source node shouldn't be linked");
			
			// Remove the source node from the tree
			Remove(*sourceNode, false);

			// Get the first attached to parent node
			TNode* firstAttached = parentNode->Attached();

			if (firstAttached != TNULL)
			{
				// Attach node to other attached nodes
				TNode* lastAttached = firstAttached->Last();
				
				lastAttached->m_Next = sourceNode;
				firstAttached->m_Last = sourceNode;

				sourceNode->m_Next = firstAttached;
				sourceNode->m_Last = lastAttached;
			}
			else
			{
				// Attach node as the first one
				parentNode->m_Attached = sourceNode;
			}

			sourceNode->m_Root = this;
			sourceNode->m_Parent = parentNode;
			m_Count += 1;
		}

		TNode* Remove(TNode& node, bool flag)
		{
			// Toshi::TNodeTree<Toshi::TResource>::Remove - 00691e70
			TNodeTree<T>* nodeRoot = node.Root();
			TNode* nodeParent = node.Parent();
			
			if (nodeRoot != TNULL)
			{
				// Check if the node belongs to the current tree
				if (nodeRoot != this)
				{
					return &node;
				}

				m_Count -= 1;
			}

			if (flag)
			{
				TNode* attachedNode = node.Attached();

				while (attachedNode != TNULL)
				{
					TNodeTree<T>* nodeRoot = node.Root();

					Remove(*attachedNode, false);
					InsertNode(node.Parent(), attachedNode);

					attachedNode = node.Attached();
					TIMPLEMENT_D("It seems to be unused and I hope it is. I don't know if it works and what it should do");
				}
			}

			if (nodeParent != TNULL)
			{
				// If it's the first attached to the root node, set it to next or just remove
				if (nodeParent->m_Attached == &node)
				{
					nodeParent->m_Attached = (node.m_Next != &node) ? node.m_Next : TNULL;
				}

				node.m_Parent = TNULL;
			}

			node.m_Last->m_Last = node.m_Next;
			node.m_Next->m_Parent = node.m_Last;
			node.m_Next = &node;
			node.m_Last = &node;
			node.m_Root = TNULL;
			return &node;
		}

		TNode* GetRoot()
		{
			return &m_Root;
		}
		
	protected:
		TNode m_Root; // 0x0
		int m_Count;  // 0x14
	};
}


