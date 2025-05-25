#pragma once
#include <Toshi/TNodeList.h>

class AObjectHash
{
public:
	inline static constexpr TUINT NUM_LISTS = 4096;

	struct Node
	    : public Toshi::TNodeList<Node>::TNode
	{
		void* pUnk1 = TNULL;
		void* pUnk2 = TNULL;
	};

public:
	AObjectHash( TINT a_iNumNodes );
	virtual ~AObjectHash();

protected:
	//...
	Node*                  m_pAllNodes;
	Toshi::TNodeList<Node> m_FreeList;
	Toshi::TNodeList<Node> m_UsedList;
	//...
	TFLOAT m_fTotalX;
	TFLOAT m_fTotalY;
	TFLOAT m_fNegativeMinX;
	TFLOAT m_fNegativeMinY;
	TFLOAT m_fMaxXOverNumCells;
	TFLOAT m_fMaxYOverNumCells;
	TFLOAT m_fMaxXOverNumCellsOverOne;
	TFLOAT m_fMaxYOverNumCellsOverOne;
	TINT   m_fNumCellsX;
	TINT   m_fNumCellsY;
	//...
	Toshi::TNodeList<Node> m_aLists[ NUM_LISTS ];
	//...
};
