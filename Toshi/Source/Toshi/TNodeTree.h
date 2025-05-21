#pragma once
#include "Toshi/Typedefs.h"

TOSHI_NAMESPACE_START

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
			m_Tree   = TNULL;
			m_Next   = (T*)this;
			m_Prev   = (T*)this;
			m_Parent = TNULL;
			m_Child  = TNULL;
		}

	public:
		TBOOL IsChildOfDefaultRoot() const
		{
			TASSERT( IsLinked() == TTRUE );
			return m_Parent == (T*)( &Tree()->m_Root );
		}

		TBOOL         IsLinked() const { return m_Tree != TNULL; }
		T*            Parent() const { return m_Parent; }
		T*            Next() const { return m_Next; }
		T*            Prev() const { return m_Prev; }
		TNodeTree<T>* Tree() const { return m_Tree; }
		T*            Child() const { return m_Child; }

	protected:
		TNodeTree<T>* m_Tree;
		T*            m_Next;
		T*            m_Prev;
		T*            m_Parent;
		T*            m_Child;
	};

public:
	TNodeTree()
	{
		m_Count = 0;
	}

	~TNodeTree()
	{
		DeleteAll();
		TASSERT( IsLinked() == TFALSE );
	}

	// $Barnyard: FUNCTION 00403980
	/**
		* Inserts node as a child of another node.
		* 
		* @param parentNode Pointer to the parent node.
		* @param sourceNode Pointer to the node you want to insert.
		*/
	void Insert( T* parentNode, T* sourceNode )
	{
		// Toshi::TNodeTree<Toshi::TResource>::Insert - 00691aa0
		TASSERT( sourceNode->IsLinked() == TFALSE, "The source node shouldn't be linked" );

		// Remove the source node from the tree
		Remove( *sourceNode, TFALSE );

		// Get the first attached to parent node
		T* firstAttached = parentNode->Child();

		if ( firstAttached != TNULL )
		{
			// Attach node to other attached nodes
			T* lastAttached = firstAttached->Prev();

			lastAttached->m_Next  = sourceNode;
			firstAttached->m_Prev = sourceNode;

			sourceNode->m_Next = firstAttached;
			sourceNode->m_Prev = lastAttached;
		}
		else
		{
			// Attach node as the first one
			parentNode->m_Child = sourceNode;
		}

		sourceNode->m_Tree   = this;
		sourceNode->m_Parent = parentNode;
		m_Count += 1;
	}

	/**
		* Inserts node to the default tree.
		*
		* @param sourceNode Pointer to the node you want to insert.
		*/
	void InsertAtRoot( T* sourceNode )
	{
		Insert( GetRoot(), sourceNode );
	}

	// $Barnyard: FUNCTION 004030d0
	/**
		 * Tries to remove sourceNode from the tree and inserts it to the parentNode or to the root
		 */
	void ReInsert( T* parentNode, T* sourceNode )
	{
		Remove( sourceNode, TFALSE );

		if ( parentNode == TNULL )
		{
			if ( this != TNULL )
			{
				InsertAtRoot( sourceNode );
				return;
			}
		}

		Insert( parentNode, sourceNode );
	}

	T* Remove( T& node, TBOOL flag = TFALSE )
	{
		// Toshi::TNodeTree<Toshi::TResource>::Remove - 00691e70
		TNodeTree<T>* nodeRoot   = node.Tree();
		T*            nodeParent = node.Parent();

		if ( nodeRoot != TNULL )
		{
			// Check if the node belongs to the current tree
			if ( nodeRoot != this )
			{
				return &node;
			}

			m_Count -= 1;
		}

		if ( flag )
		{
			T* attachedNode = node.Child();

			while ( attachedNode != TNULL )
			{
				TNodeTree<T>* nodeRoot = node.Tree();

				Remove( *attachedNode, TFALSE );
				Insert( node.Parent(), attachedNode );

				attachedNode = node.Child();
				TIMPLEMENT_D( "It seems to be unused and I hope it is. I don't know if it works and what it should do" );
			}
		}

		if ( nodeParent != TNULL )
		{
			// If it's the first attached to the root node, set it to next or just remove
			if ( nodeParent->Child() == &node )
			{
				nodeParent->m_Child = ( node.Next() != &node ) ? node.Next() : TNULL;
			}

			node.m_Parent = TNULL;
		}

		node.m_Prev->m_Next = node.m_Next;
		node.m_Next->m_Prev = node.m_Prev;
		node.m_Next         = &node;
		node.m_Prev         = &node;
		node.m_Tree         = TNULL;
		return &node;
	}

	T* Remove( T* node, TBOOL flag = TFALSE )
	{
		return Remove( *node, flag );
	}

	void DeleteRecurse( T* node )
	{
		while ( node != TNULL )
		{
			T* next = ( node->Next() != node ) ? node->Next() : TNULL;

			if ( node->Child() != TNULL )
			{
				DeleteRecurse( node->Child() );
			}

			if ( node->Tree() == this )
			{
				m_Count -= 1;
			}

			if ( node->Tree() == TNULL || node->Tree() == this )
			{
				T* nodeParent = node->Parent();

				if ( nodeParent != TNULL )
				{
					// If it's the first attached to the root node, set it to next or just remove
					if ( nodeParent->Child() == node )
					{
						nodeParent->m_Child = ( node->Next() != node ) ? node->Next() : TNULL;
					}

					node->m_Parent = TNULL;
				}

				node->m_Prev->m_Parent = node->m_Next;
				node->m_Next->m_Child  = node->m_Prev;
				node->m_Next           = node;
				node->m_Prev           = node;
				node->m_Tree           = TNULL;
			}

			delete node;
			node = next;
		}
	}

	void DeleteAll()
	{
		T* node = GetRoot()->Child();

		while ( node != TNULL )
		{
			Remove( node, TFALSE );
			DeleteRecurse( node );
			node = GetRoot()->Child();
		}

		TASSERT( Count() == 0 );
	}

	T* GetRoot()
	{
		return TSTATICCAST( T, &m_Root );
	}

	T* ChildOfRoot()
	{
		return m_Root.Child();
	}

	size_t Count() const
	{
		return m_Count;
	}

	TBOOL IsLinked() const
	{
		return m_Root.IsLinked();
	}

protected:
	TNode  m_Root;  // 0x0
	size_t m_Count; // 0x14
};

TOSHI_NAMESPACE_END
