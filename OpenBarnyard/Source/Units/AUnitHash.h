#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TNodeList.h>
#include <Math/TVector4.h>

class AUnit;
class AUnitManager;

//-----------------------------------------------------------------------------
// Purpose: Spatial hash of every unit, owned by AUnitManager. Same algorithm as
// AObjectHash (64x64 wrapping grid of buckets over the XZ plane), but with a
// fixed pool of cells and a fixed result buffer used by proximity queries.
//-----------------------------------------------------------------------------
class AUnitHash : public Toshi::TObject
{
public:
	TDECLARE_CLASS( AUnitHash, Toshi::TObject );

	inline static constexpr TUINT GRID_SIZE   = 64;
	inline static constexpr TUINT GRID_MASK   = GRID_SIZE - 1;
	inline static constexpr TUINT NUM_BUCKETS = GRID_SIZE * GRID_SIZE;
	inline static constexpr TINT  NUM_CELLS   = 420;
	inline static constexpr TINT  MAX_RESULTS = 420;

	// NOTE: retail constructs these with AObjectHash::CellObject::CellObject (identical code folded).
	struct CellObject
	    : public Toshi::TNodeList<CellObject>::TNode
	{
		AUnit*      pUnit         = TNULL; // 0xc
		CellObject* pPreviousCell = TNULL; // 0x10
	};

	struct QueryResult
	{
		AUnit* pUnit; // 0x0
		// ( fUnitRadius + fQueryRadius )^2 - fUnitRadius^2
		TFLOAT fUnk; // 0x4
	};

	// Optional unit filter used by the queries. Called as __stdcall (callee pops the argument).
	using FilterFn = TBOOL( __stdcall* )( AUnit* a_pUnit );

public:
	AUnitHash();
	~AUnitHash();

	void InitialiseCache( AUnitManager* a_pOwner, TFLOAT a_fMinX, TFLOAT a_fMinZ, TFLOAT a_fMaxX, TFLOAT a_fMaxZ, TINT a_iNumCellsX, TINT a_iNumCellsZ );

	CellObject* AssignHash( AUnit* a_pUnit );
	void        RemoveHash( AUnit* a_pUnit );

	// Collects units whose radius overlaps the sphere/circle (a_b3D: XYZ distance, otherwise XZ).
	// a_bAllUnits walks AUnitManager's unit list instead of the hash buckets.
	// Returns the internal result buffer, *a_pNumFound receives the number of entries.
	QueryResult* FindUnits( const Toshi::TVector4& a_rcPos, TFLOAT a_fRadius, TINT* a_pNumFound, TBOOL a_b3D, TINT a_iMaxNum, TBOOL a_bAllUnits, FilterFn a_fnFilter );

private:
	// Adds the unit to the result buffer if it overlaps. Returns TFALSE once a_iMaxNum results are collected.
	TBOOL AddQueryResult( AUnit* a_pUnit, const Toshi::TVector4& a_rcPos, TFLOAT a_fRadius, TBOOL a_b3D, TINT* a_pNumFound, TINT a_iMaxNum );

	// Brute force query through every unit of AUnitManager.
	void FindUnitsInList( const Toshi::TVector4& a_rcPos, TFLOAT a_fRadius, TINT* a_pNumFound, TBOOL a_b3D, TINT a_iMaxNum, FilterFn a_fnFilter );

private:
	// Incremented on every query and stored in each visited unit (AUnit::SetQueryStamp) so a unit
	// that spans several buckets is only tested once.
	static TUINT ms_uiQueryStamp;

private:
	// Parameters of the last FindUnitsInList query
	TINT*                        m_pNumFound;                   // 0x4
	TBOOL                        m_b3D;                         // 0x8
	TINT                         m_iMaxNum;                     // 0xc
	Toshi::TVector4              m_vecPos;                      // 0x10
	TFLOAT                       m_fRadius;                     // 0x20
	TBOOL                        m_bMaxReached;                 // 0x24
	AUnitManager*                m_pOwner;                      // 0x28
	CellObject                   m_aCells[ NUM_CELLS ];         // 0x2c
	Toshi::TNodeList<CellObject> m_FreeList;                    // 0x20fc
	Toshi::TNodeList<CellObject> m_UsedList;                    // 0x210c
	TFLOAT                       m_fTotalX;                     // 0x211c
	TFLOAT                       m_fTotalZ;                     // 0x2120
	TFLOAT                       m_fNegativeMinX;               // 0x2124
	TFLOAT                       m_fNegativeMinZ;               // 0x2128
	TINT                         m_iNumCellsX;                  // 0x212c
	TINT                         m_iNumCellsZ;                  // 0x2130
	TFLOAT                       m_fMaxXOverNumCells;           // 0x2134
	TFLOAT                       m_fMaxZOverNumCells;           // 0x2138
	TFLOAT                       m_fDivideByMaxXOverNumCells;   // 0x213c
	TFLOAT                       m_fDivideByMaxZOverNumCells;   // 0x2140
	Toshi::TNodeList<CellObject> m_aCellBuckets[ NUM_BUCKETS ]; // 0x2144
	QueryResult                  m_aResults[ MAX_RESULTS ];     // 0x12144
};

TSTATICASSERT( sizeof( AUnitHash ) == 0x12e64 );
