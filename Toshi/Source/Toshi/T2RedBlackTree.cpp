#include "ToshiPCH.h"
#include "T2RedBlackTree.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

constinit T2GenericRedBlackTreeNode T2GenericRedBlackTree::ms_oNil;

// $Barnyard: FUNCTION 006cf1e0
T2GenericRedBlackTreeNode* T2GenericRedBlackTree::GetFirstNode() const
{
	T2GenericRedBlackTreeNode* pCurrentNode = &m_oRoot;
	T2GenericRedBlackTreeNode* pResult      = TNULL;

	while ( pCurrentNode != &ms_oNil )
	{
		pResult      = pCurrentNode;
		pCurrentNode = pResult->m_pLeft;
	}

	return pResult;
}

// $Barnyard: FUNCTION 006ceff0
T2GenericRedBlackTreeNode* T2GenericRedBlackTree::Insert( T2GenericRedBlackTreeNode* pNode )
{
	T2GenericRedBlackTreeNode* pNode_00;
	T2GenericRedBlackTreeNode* pTVar1;
	T2GenericRedBlackTreeNode* node;

	m_iNumElements += 1;
	pNode->red = 1;
	node       = pNode;

	while ( pNode_00 = node->m_pParent, pNode_00->red != 0 )
	{
		pTVar1 = pNode_00->m_pParent->m_pLeft;
		if ( pNode_00 == pTVar1 )
		{
			pTVar1 = pNode_00->m_pParent->m_pRight;

			if ( pTVar1->red == 0 )
			{
				if ( node == pNode_00->m_pRight )
				{
					LeftRotate( pNode_00 );
					node = pNode_00;
				}

				node->m_pParent->red            = 0;
				node->m_pParent->m_pParent->red = 1;
				RightRotate( node->m_pParent->m_pParent );
			}
			else
			{
				pNode_00->red                   = 0;
				pTVar1->red                     = 0;
				node->m_pParent->m_pParent->red = 1;
				node                            = node->m_pParent->m_pParent;
			}
		}
		else if ( pTVar1->red == 0 )
		{
			if ( node == pNode_00->m_pLeft )
			{
				RightRotate( pNode_00 );
				node = pNode_00;
			}

			node->m_pParent->red            = 0;
			node->m_pParent->m_pParent->red = 1;
			LeftRotate( node->m_pParent->m_pParent );
		}
		else
		{
			pNode_00->red                   = 0;
			pTVar1->red                     = 0;
			node->m_pParent->m_pParent->red = 1;
			node                            = node->m_pParent->m_pParent;
		}
	}

	m_oRoot.m_pLeft->red = 0;
	CheckValid();

	return pNode;
}

// $Barnyard: FUNCTION 006cf200
void T2GenericRedBlackTree::DeleteFixUp( T2GenericRedBlackTreeNode* pNode )
{
	TINT                       uVar2;
	T2GenericRedBlackTreeNode* pTVar1;
	T2GenericRedBlackTreeNode* pTVar3;
	T2GenericRedBlackTreeNode* pTVar4;

	pTVar4 = m_oRoot.m_pLeft;
	while ( pNode->red == 0 && pTVar4 != pNode )
	{
		pTVar3 = pNode->m_pParent;
		pTVar1 = pTVar3->m_pLeft;

		if ( pNode == pTVar1 )
		{
			pTVar1 = pTVar3->m_pRight;

			if ( pTVar1->red != 0 )
			{
				pTVar1->red           = 0;
				pNode->m_pParent->red = 1;
				LeftRotate( pNode->m_pParent );
				pTVar3 = pNode->m_pParent;
				pTVar1 = pTVar3->m_pRight;
			}

			uVar2 = pTVar1->m_pRight->red;
			if ( ( uVar2 == 0 ) && ( pTVar1->m_pLeft->red == 0 ) )
			{
				pTVar1->red = 1;
				pNode       = pNode->m_pParent;
			}
			else
			{
				if ( uVar2 == 0 )
				{
					pTVar1->m_pLeft->red = 0;
					pTVar1->red          = 1;
					RightRotate( pTVar1 );
					pTVar3 = pNode->m_pParent;
					pTVar1 = pTVar3->m_pRight;
				}

				pTVar1->red           = pTVar3->red;
				pNode->m_pParent->red = 0;
				pTVar1->m_pRight->red = 0;
				LeftRotate( pNode->m_pParent );
				pNode = pTVar4;
			}
		}
		else
		{
			if ( pTVar1->red != 0 )
			{
				pTVar1->red           = 0;
				pNode->m_pParent->red = 1;
				RightRotate( pNode->m_pParent );
				pTVar3 = pNode->m_pParent;
				pTVar1 = pTVar3->m_pLeft;
			}

			if ( pTVar1->m_pRight->red == 0 && pTVar1->m_pLeft->red == 0 )
			{
				pTVar1->red = 1;
				pNode       = pNode->m_pParent;
			}
			else
			{
				if ( pTVar1->m_pLeft->red == 0 )
				{
					pTVar1->m_pRight->red = 0;
					pTVar1->red           = 1;
					LeftRotate( pTVar1 );
					pTVar3 = pNode->m_pParent;
					pTVar1 = pTVar3->m_pLeft;
				}

				pTVar1->red           = pTVar3->red;
				pNode->m_pParent->red = 0;
				pTVar1->m_pLeft->red  = 0;
				RightRotate( pNode->m_pParent );
				pNode = pTVar4;
			}
		}
	}

	pNode->red = 0;
	CheckValid();
}

// $Barnyard: FUNCTION 006cf460
T2GenericRedBlackTreeNode* T2GenericRedBlackTree::DeleteNode( T2GenericRedBlackTreeNode* pNode )
{
	T2GenericRedBlackTreeNode* pTVar1;
	T2GenericRedBlackTreeNode* y;
	T2GenericRedBlackTreeNode* pTVar3;

	y = pNode;
	if ( ( pNode->m_pLeft != &ms_oNil ) && ( pNode->m_pRight != &ms_oNil ) )
	{
		y = GetSuccessorOf( pNode );
	}

	pTVar3 = y->m_pLeft;
	if ( pTVar3 == &ms_oNil )
	{
		pTVar3 = y->m_pRight;
	}

	pTVar1 = y->m_pParent;

#ifdef BARNYARD_COMMUNITY_PATCH
	// This bug persists in all versions of the engine, including de Blob on Steam
	// Can cause softlock and other shit including memory stomps
	if ( pTVar3 != &ms_oNil ) pTVar3->m_pParent = pTVar1;
#else
	pTVar3->m_pParent = pTVar1;
#endif // BARNYARD_COMMUNITY_PATCH

	if ( &m_oRoot == pTVar1 )
	{
		m_oRoot.m_pLeft = pTVar3;
	}
	else
	{
		pTVar1 = y->m_pParent;

		if ( y == pTVar1->m_pLeft )
		{
			pTVar1->m_pLeft = pTVar3;
		}
		else
		{
			pTVar1->m_pRight = pTVar3;
		}
	}

	if ( y == pNode )
	{
		if ( y->red == 0 )
		{
			DeleteFixUp( pTVar3 );
		}

		CheckValid();
		TASSERT( 0 == ms_oNil.red );
		m_iNumElements -= 1;

		return y;
	}

	TASSERT( y != &ms_oNil );

	y->m_pLeft                 = pNode->m_pLeft;
	y->m_pRight                = pNode->m_pRight;
	y->m_pParent               = pNode->m_pParent;
	pNode->m_pRight->m_pParent = y;
	pNode->m_pLeft->m_pParent  = y;
	pTVar1                     = pNode->m_pParent;

	if ( pNode == pTVar1->m_pLeft )
	{
		pTVar1->m_pLeft = y;
	}
	else
	{
		pTVar1->m_pRight = y;
	}

	TINT oldRedValue = y->red;
	y->red           = pNode->red;

	if ( oldRedValue == 0 )
	{
		DeleteFixUp( pTVar3 );
	}

	CheckValid();
	TASSERT( 0 == ms_oNil.red );
	m_iNumElements -= 1;
	return pNode;
}

void T2GenericRedBlackTree::LeftRotate( T2GenericRedBlackTreeNode* pNode )
{
	T2GenericRedBlackTreeNode* pTVar1;
	T2GenericRedBlackTreeNode* pTVar2;

	pTVar2          = pNode->m_pRight;
	pNode->m_pRight = pTVar2->m_pLeft;

	if ( pTVar2->m_pLeft != &ms_oNil )
	{
		pTVar2->m_pLeft->m_pParent = pNode;
	}

	pTVar2->m_pParent = pNode->m_pParent;
	pTVar1            = pNode->m_pParent;

	if ( pNode == pTVar1->m_pLeft )
	{
		pTVar1->m_pLeft = pTVar2;
	}
	else
	{
		pTVar1->m_pRight = pTVar2;
	}

	pTVar2->m_pLeft  = pNode;
	pNode->m_pParent = pTVar2;
	CheckValid();
}

void T2GenericRedBlackTree::RightRotate( T2GenericRedBlackTreeNode* pNode )
{
	T2GenericRedBlackTreeNode* pTVar1;
	T2GenericRedBlackTreeNode* pTVar2;

	pTVar2         = pNode->m_pLeft;
	pNode->m_pLeft = pTVar2->m_pRight;

	if ( pTVar2->m_pRight != &ms_oNil )
	{
		pTVar2->m_pRight->m_pParent = pNode;
	}

	pTVar2->m_pParent = pNode->m_pParent;
	pTVar1            = pNode->m_pParent;

	if ( pNode == pTVar1->m_pLeft )
	{
		pTVar1->m_pLeft = pTVar2;
	}
	else
	{
		pTVar1->m_pRight = pTVar2;
	}

	pTVar2->m_pRight = pNode;
	pNode->m_pParent = pTVar2;
	CheckValid();
}

// $Barnyard: FUNCTION 006cf190
T2GenericRedBlackTreeNode* T2GenericRedBlackTree::GetSuccessorOf( const T2GenericRedBlackTreeNode* pNode )
{
	T2GenericRedBlackTreeNode* pTVar1;
	T2GenericRedBlackTreeNode* pTVar2;
	bool                       bVar3;

	pTVar1 = pNode->m_pRight;
	if ( pTVar1 == &ms_oNil )
	{
		pTVar1 = pNode->m_pParent;
		pTVar2 = pTVar1;
		if ( pNode == pTVar1->m_pRight )
		{
			do {
				pTVar1 = pTVar2->m_pParent;
				bVar3  = pTVar2 == pTVar1->m_pRight;
				pTVar2 = pTVar1;
			} while ( bVar3 );
		}
		if ( pTVar1->m_pParent == pTVar1 )
		{
			pTVar1 = &ms_oNil;
		}
	}
	else
	{
		pTVar2 = pTVar1->m_pLeft;
		if ( pTVar2 != &ms_oNil )
		{
			do {
				pTVar1 = pTVar2;
				pTVar2 = pTVar2->m_pLeft;
			} while ( pTVar2 != &ms_oNil );
			return pTVar1;
		}
	}
	return pTVar1;
}

T2GenericRedBlackTreeNode* T2GenericRedBlackTree::GetPredecessorOf( const T2GenericRedBlackTreeNode* pNode )
{
	T2GenericRedBlackTreeNode* pTVar1;
	T2GenericRedBlackTreeNode* pTVar2 = pNode->m_pLeft;
	TBOOL                      bVar3;

	if ( pTVar2 == &ms_oNil )
	{
		pTVar2 = pNode->m_pParent;
		if ( pNode == pTVar2->m_pLeft )
		{
			do
			{
				pTVar1 = pTVar2->m_pParent;

				if ( pTVar1 == pTVar2 )
				{
					return &ms_oNil;
				}

				bVar3  = pTVar2 == pTVar1->m_pLeft;
				pTVar2 = pTVar1;
			} while ( bVar3 );

			return pTVar1;
		}
	}
	else
	{
		pTVar1 = pTVar2->m_pRight;

		if ( pTVar2->m_pRight != &ms_oNil )
		{
			do
			{
				pTVar2 = pTVar1;
				pTVar1 = pTVar2->m_pRight;
			} while ( pTVar2->m_pRight != &ms_oNil );
		}
	}

	return pTVar2;
}

TOSHI_NAMESPACE_END
