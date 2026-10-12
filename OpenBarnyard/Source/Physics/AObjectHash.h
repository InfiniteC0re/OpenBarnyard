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
	inline static constexpr TUINT GRID_MASK   = GRID_SIZE - 1;
	inline static constexpr TUINT NUM_BUCKETS = GRID_SIZE * GRID_SIZE;
	inline static constexpr TINT  MAX_RESULTS = 20;

	struct CellObject
	    : public Toshi::TNodeList<CellObject>::TNode
	{
		// $Barnyard: FUNCTION 0061f0c0
		CellObject() {}

		AHashedObject* pHashedObject = TNULL;
		CellObject*    pPreviousCell = TNULL;
	};

	struct QueryResult
	{
		AHashedObject* pObject; // 0x0
		// ( fObjectRadius + fQueryRadius )^2 - fObjectRadius^2
		TFLOAT fUnk; // 0x4
	};

public:
	AObjectHash( TINT a_iNumNodes );
	virtual ~AObjectHash();

	void InitialiseCache( TFLOAT a_fMinX, TFLOAT a_fMinY, TFLOAT a_fMaxX, TFLOAT a_fMaxY, TINT a_iNumCellsX, TINT a_iNumCellsY );

	CellObject* AssignHash( AHashedObject* a_pObject );
	void        RemoveHash( AHashedObject* a_pObject );

	// Collects objects whose radius overlaps the sphere/circle (a_b3D: XYZ distance, otherwise XZ).
	// Returns the internal result buffer, *a_pNumFound receives the number of entries.
	QueryResult* FindObjects( const Toshi::TVector4& a_rcPos, TFLOAT a_fRadius, TINT* a_pNumFound, TBOOL a_b3D, TINT a_iMaxNum );

	// It's purpose probably was to draw something in debug mode, but the function wasn't completely removed, so it does nothing but calculates the BBox
	void Debug_DrawUsedBBox();

private:
	// Adds the object to the result buffer if it overlaps. Returns TFALSE once a_iMaxNum results are collected.
	TBOOL AddQueryResult( AHashedObject* a_pObject, const Toshi::TVector4& a_rcPos, TFLOAT a_fRadius, TBOOL a_b3D, TINT* a_pNumFound, TINT a_iMaxNum );

	// Incremented on every query and stored in each visited object (AHashedObject::SetQueryStamp) so an object
	// that spans several buckets is only tested once.
	inline static TUINT ms_uiQueryStamp = 0;

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
	QueryResult                  m_aResults[ MAX_RESULTS ];
};
