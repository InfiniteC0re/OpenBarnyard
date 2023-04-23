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

		protected:
			TNode()
			{
				m_Tree = TNULL;
				m_Next = this;
				m_Prev = this;
				m_Parent = TNULL;
				m_Attached = TNULL;
			}

		public:
			bool IsLinked() const
			{
				return m_Tree != TNULL;
			}

			bool IsChildOfDefaultRoot() const
			{
				TASSERT(IsLinked() == TTRUE);
				return m_Parent == &Tree()->m_Root;
			}

			T* Parent() const
			{
				return m_Parent->Cast();
			}

			T* Next() const
			{
				return m_Next->Cast();
			}

			T* Prev() const
			{
				return m_Prev->Cast();
			}

			TNodeTree<T>* Tree() const
			{
				return m_Tree;
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
			TNodeTree<T>* m_Tree;
			TNode* m_Next;
			TNode* m_Prev;
			TNode* m_Parent;
			TNode* m_Attached;
		};

	public:
		TNodeTree()
		{
			m_Count = 0;
		}

		~TNodeTree()
		{
			DeleteAll();
			TASSERT(IsLinked() == TFALSE);
		}

		/**
		* Inserts node as a child of another node.
		* 
		* @param parentNode Pointer to the parent node.
		* @param sourceNode Pointer to the node you want to insert.
		*/
		void Insert(TNode* parentNode, TNode* sourceNode)
		{
			// Toshi::TNodeTree<Toshi::TResource>::Insert - 00691aa0
			TASSERT(sourceNode->IsLinked() == TFALSE, "The source node shouldn't be linked");
			
			// Remove the source node from the tree
			Remove(*sourceNode, false);

			// Get the first attached to parent node
			TNode* firstAttached = parentNode->Attached();

			if (firstAttached != TNULL)
			{
				// Attach node to other attached nodes
				TNode* lastAttached = firstAttached->Prev();
				
				lastAttached->m_Next = sourceNode;
				firstAttached->m_Prev = sourceNode;

				sourceNode->m_Next = firstAttached;
				sourceNode->m_Prev = lastAttached;
			}
			else
			{
				// Attach node as the first one
				parentNode->m_Attached = sourceNode;
			}

			sourceNode->m_Tree = this;
			sourceNode->m_Parent = parentNode;
			m_Count += 1;
		}

		/**
		* Inserts node to the default tree.
		*
		* @param sourceNode Pointer to the node you want to insert.
		*/
		void InsertAtRoot(TNode* sourceNode)
		{
			Insert(GetRoot(), sourceNode);
		}

		TNode* Remove(TNode& node, bool flag = false)
		{
			// Toshi::TNodeTree<Toshi::TResource>::Remove - 00691e70
			TNodeTree<T>* nodeRoot = node.Tree();
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
					TNodeTree<T>* nodeRoot = node.Tree();

					Remove(*attachedNode, false);
					Insert(node.Parent(), attachedNode);

					attachedNode = node.Attached();
					TIMPLEMENT_D("It seems to be unused and I hope it is. I don't know if it works and what it should do");
				}
			}

			if (nodeParent != TNULL)
			{
				// If it's the first attached to the root node, set it to next or just remove
				if (nodeParent->Attached() == &node)
				{
					nodeParent->m_Attached = (node.Next() != &node) ? node.Next() : TNULL;
				}

				node.m_Parent = TNULL;
			}

			node.m_Prev->m_Prev = node.m_Next;
			node.m_Next->m_Parent = node.m_Prev;
			node.m_Next = &node;
			node.m_Prev = &node;
			node.m_Tree = TNULL;
			return &node;
		}

		TNode* Remove(TNode* node, bool flag = false)
		{
			return Remove(*node, flag);
		}

		void DeleteRecurse(TNode* node)
		{
			while (node != TNULL)
			{
				TNode* next = (node->Next() != node) ? node->Next() : TNULL;

				if (node->Attached() != TNULL)
				{
					DeleteRecurse(node->Attached());
				}

				if (node->Tree() == this)
				{
					m_Count -= 1;
				}

				if (node->Tree() == TNULL || node->Tree() == this)
				{
					TNode* nodeParent = node->Parent();

					if (nodeParent != TNULL)
					{
						// If it's the first attached to the root node, set it to next or just remove
						if (nodeParent->Attached() == node)
						{
							nodeParent->m_Attached = (node->Next() != node) ? node->Next() : TNULL;
						}

						node->m_Parent = TNULL;
					}

					node->m_Prev->m_Parent = node->m_Next;
					node->m_Next->m_Attached = node->m_Prev;
					node->m_Next = node;
					node->m_Prev = node;
					node->m_Tree = TNULL;
				}

				delete node;
				node = next;
			}
		}
		
		void DeleteAll()
		{
			TNode* node = GetRoot()->Attached();

			while (node != TNULL)
			{
				Remove(node, false);
				DeleteRecurse(node);
				node = GetRoot()->Attached();
			}

			TASSERT(Count() == 0);
		}

		TNode* GetRoot()
		{
			return &m_Root;
		}

		T* AttachedToRoot()
		{
			return m_Root.Attached();
		}

		size_t Count() const
		{
			return m_Count;
		}

		bool IsLinked() const
		{
			return m_Root.IsLinked();
		}
		
	protected:
		TNode m_Root;   // 0x0
		size_t m_Count; // 0x14
	};
}


