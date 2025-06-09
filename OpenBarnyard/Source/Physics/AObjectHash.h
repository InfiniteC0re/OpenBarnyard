#pragma once
#include <Toshi/TNodeList.h>

class AHashedObject;

//-----------------------------------------------------------------------------
// Purpose: A performance optimization that splits the whole map into different
// chunks, allowing not to perform complex calculations on collision objects
// that do not collide with each other when they are in different cells.
// The alhorithm is also called "Spatial Hashing".
//-----------------------------------------------------------------------------
class AObjectHash
{
public:
	inline static constexpr TUINT GRID_SIZE   = 64;
	inline static constexpr TUINT NUM_BUCKETS = GRID_SIZE * GRID_SIZE;

	struct CellObject
	    : public Toshi::TNodeList<CellObject>::TNode
	{
		AHashedObject* pHashedObject   = TNULL;
		CellObject*    pPreviousCell = TNULL;
	};

public:
	AObjectHash( TINT a_iNumNodes );
	virtual ~AObjectHash();

	void InitialiseCache( TFLOAT a_fMinX, TFLOAT a_fMinY, TFLOAT a_fMaxX, TFLOAT a_fMaxY, TINT a_iNumCellsX, TINT a_iNumCellsY );

	CellObject* AssignHash( AHashedObject* a_pObject );
	void RemoveHash( AHashedObject* a_pObject );

protected:
	//...
	CellObject*                  m_pAllCells;
	Toshi::TNodeList<CellObject> m_FreeList;
	Toshi::TNodeList<CellObject> m_UsedList;
	//...
	TFLOAT m_fTotalX;
	TFLOAT m_fTotalY;
	TFLOAT m_fNegativeMinX;
	TFLOAT m_fNegativeMinY;
	TFLOAT m_fMaxXOverNumCells;
	TFLOAT m_fMaxYOverNumCells;
	TFLOAT m_fDivideByMaxXOverNumCells;
	TFLOAT m_fDivideByMaxYOverNumCells;
	TINT   m_fNumCellsX;
	TINT   m_fNumCellsY;
	//...
	Toshi::TNodeList<CellObject> m_aCellBuckets[ NUM_BUCKETS ];
	//...
};
