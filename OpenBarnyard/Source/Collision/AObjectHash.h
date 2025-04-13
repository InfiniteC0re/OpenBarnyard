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

private:
	//...
	Node*                  m_pAllNodes;
	Toshi::TNodeList<Node> m_FreeList;
	Toshi::TNodeList<Node> m_UsedList;
	//...
	Toshi::TNodeList<Node> m_aLists[ NUM_LISTS ];
	//...
};
