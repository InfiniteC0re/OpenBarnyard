#include "pch.h"
#include "AUnitHash.h"
#include "AUnit.h"
#include "AUnitManager.h"
#include "UnitMovement/Steering/ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0040e8a0, CreateObject 0040f2d0, CreateObjectInPlace 0040f330
TDEFINE_CLASS( AUnitHash );

// DAT_00772784
TUINT AUnitHash::ms_uiQueryStamp = 1;

// $Barnyard: FUNCTION 0040f080
AUnitHash::AUnitHash()
{
	m_pOwner                    = TNULL;
	m_fTotalX                   = 0.0f;
	m_fTotalZ                   = 0.0f;
	m_fNegativeMinX             = 0.0f;
	m_fNegativeMinZ             = 0.0f;
	m_iNumCellsX                = 0;
	m_iNumCellsZ                = 0;
	m_fMaxXOverNumCells         = 0.0f;
	m_fMaxZOverNumCells         = 0.0f;
	m_fDivideByMaxXOverNumCells = 0.0f;
	m_fDivideByMaxZOverNumCells = 0.0f;

	for ( TINT i = 0; i < NUM_CELLS; i++ )
		m_FreeList.InsertHead( &m_aCells[ i ] );
}

// $Barnyard: FUNCTION 0040f370
// $Barnyard: FUNCTION 0040f220
AUnitHash::~AUnitHash()
{
	// Cells are embedded in m_aCells, so unlink them before the list destructor runs.
	// NOTE: like retail, the buckets and m_UsedList are destroyed with their default
	// destructor, which frees any cell still linked in them.
	m_FreeList.RemoveAll();
}

// $Barnyard: FUNCTION 0040e8b0
void AUnitHash::InitialiseCache( AUnitManager* a_pOwner, TFLOAT a_fMinX, TFLOAT a_fMinZ, TFLOAT a_fMaxX, TFLOAT a_fMaxZ, TINT a_iNumCellsX, TINT a_iNumCellsZ )
{
	m_pOwner                    = a_pOwner;
	m_fTotalX                   = a_fMaxX - a_fMinX;
	m_iNumCellsX                = a_iNumCellsX;
	m_iNumCellsZ                = a_iNumCellsZ;
	m_fTotalZ                   = a_fMaxZ - a_fMinZ;
	m_fNegativeMinX             = -a_fMinX;
	m_fNegativeMinZ             = -a_fMinZ;
	m_fMaxXOverNumCells         = a_fMaxX / a_iNumCellsX;
	m_fMaxZOverNumCells         = a_fMaxZ / a_iNumCellsZ;
	m_fDivideByMaxXOverNumCells = 1.0f / ( a_fMaxX / a_iNumCellsX );
	m_fDivideByMaxZOverNumCells = 1.0f / ( a_fMaxZ / a_iNumCellsZ );
}

// $Barnyard: FUNCTION 0040eaa0
void AUnitHash::RemoveHash( AUnit* a_pUnit )
{
	CellObject* pHashNode = a_pUnit->GetHashNode();

	while ( pHashNode != TNULL )
	{
		// Detach the node from its bucket and return it to the free list
		pHashNode->Remove();
		m_FreeList.InsertHead( pHashNode );

		pHashNode = pHashNode->pPreviousCell;
	}

	a_pUnit->SetHashNode( TNULL );
}

// $Barnyard: FUNCTION 0040eb20
AUnitHash::CellObject* AUnitHash::AssignHash( AUnit* a_pUnit )
{
	RemoveHash( a_pUnit );

	CellObject* pCellObject = TNULL;

	const TVector4& vecPos  = *a_pUnit->GetSteer()->GetPosition();
	TFLOAT          fRadius = a_pUnit->GetSteer()->GetCollisionRadius();

	// Calculate the covered cells
	TFLOAT fXMin = ( vecPos.x - fRadius + m_fNegativeMinX ) * m_fDivideByMaxXOverNumCells;
	if ( fXMin < 0.0f ) fXMin -= 1.0f;
	TINT iXMin = TINT( fXMin );

	TFLOAT fXMax = ( vecPos.x + fRadius + m_fNegativeMinX ) * m_fDivideByMaxXOverNumCells;
	if ( fXMax < 0.0f ) fXMax -= 1.0f;
	TINT iXMax = TINT( fXMax );

	TFLOAT fZMin = ( vecPos.z - fRadius + m_fNegativeMinZ ) * m_fDivideByMaxZOverNumCells;
	if ( fZMin < 0.0f ) fZMin -= 1.0f;
	TINT iZMin = TINT( fZMin );

	TFLOAT fZMax = ( vecPos.z + fRadius + m_fNegativeMinZ ) * m_fDivideByMaxZOverNumCells;
	if ( fZMax < 0.0f ) fZMax -= 1.0f;
	TINT iZMax = TINT( fZMax );

	// Add a cell object to each bucket the unit touches, chaining them through pPreviousCell
	for ( TINT iZ = iZMin; iZ < iZMax + 1; iZ++ )
	{
		for ( TINT iX = iXMin; iX < iXMax + 1; iX++ )
		{
			TNodeList<CellObject>* pCellBucket = &m_aCellBuckets[ ( iX & GRID_MASK ) * GRID_SIZE + ( iZ & GRID_MASK ) ];

			if ( !m_FreeList.IsEmpty() )
			{
				CellObject* pCell = m_FreeList.Begin();
				m_FreeList.Remove( pCell );

				pCell->pPreviousCell = pCellObject;
				pCell->pUnit         = a_pUnit;
				pCellBucket->InsertTail( pCell );

				pCellObject = pCell;
			}
		}
	}

	a_pUnit->SetHashNode( pCellObject );
	return pCellObject;
}

// $Barnyard: FUNCTION 0040e960
TBOOL AUnitHash::AddQueryResult( AUnit* a_pUnit, const TVector4& a_rcPos, TFLOAT a_fRadius, TBOOL a_b3D, TINT* a_pNumFound, TINT a_iMaxNum )
{

	if ( a_pUnit->GetFlags() & AUnit::FLAGS_NO_QUERY )
		return TTRUE;

	// Already visited by this query?
	if ( a_pUnit->GetQueryStamp() == ms_uiQueryStamp )
		return TTRUE;

	a_pUnit->SetQueryStamp( ms_uiQueryStamp );

	TVector4 vecUnitPos;
	a_pUnit->GetSteer()->CopyPosition( vecUnitPos );
	TFLOAT fUnitRadius = a_pUnit->GetSteer()->GetCollisionRadius();

	TFLOAT fDistanceSq;
	TFLOAT fDX = vecUnitPos.x - a_rcPos.x;

	if ( !a_b3D )
	{
		TFLOAT fDZ  = vecUnitPos.z - a_rcPos.z;
		fDistanceSq = fDZ * fDZ + fDX * fDX;
	}
	else
	{
		TFLOAT fDY  = vecUnitPos.y - a_rcPos.y;
		TFLOAT fDZ  = vecUnitPos.z - a_rcPos.z;
		fDistanceSq = fDY * fDY + fDZ * fDZ + fDX * fDX;
	}

	TFLOAT fRadiusSum   = fUnitRadius + a_fRadius;
	TFLOAT fRadiusSumSq = fRadiusSum * fRadiusSum;

	if ( fDistanceSq < fRadiusSumSq )
	{
		m_aResults[ *a_pNumFound ].fUnk  = fRadiusSumSq - fUnitRadius * fUnitRadius;
		m_aResults[ *a_pNumFound ].pUnit = a_pUnit;

		// Stop the query once the buffer is full
		if ( ++( *a_pNumFound ) == a_iMaxNum )
			return TFALSE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0040ed10
void AUnitHash::FindUnitsInList( const TVector4& a_rcPos, TFLOAT a_fRadius, TINT* a_pNumFound, TBOOL a_b3D, TINT a_iMaxNum, FilterFn a_fnFilter )
{
	m_b3D         = a_b3D;
	m_pNumFound   = a_pNumFound;
	m_iMaxNum     = a_iMaxNum;
	m_vecPos      = a_rcPos;
	m_fRadius     = a_fRadius;
	m_bMaxReached = TFALSE;

	TNodeList<AUnit>& rUnits = AUnitManager::GetSingleton()->GetUnits();

	T2_FOREACH( rUnits, it )
	{
		AUnit* pUnit = it.Get();

		if ( !pUnit->IsDying() &&
		     ( a_fnFilter == TNULL || a_fnFilter( pUnit ) ) &&
		     !AddQueryResult( pUnit, m_vecPos, m_fRadius, m_b3D, m_pNumFound, m_iMaxNum ) )
		{
			m_bMaxReached = TTRUE;
			return;
		}
	}
}

// $Barnyard: FUNCTION 0040edd0
AUnitHash::QueryResult* AUnitHash::FindUnits( const TVector4& a_rcPos, TFLOAT a_fRadius, TINT* a_pNumFound, TBOOL a_b3D, TINT a_iMaxNum, TBOOL a_bAllUnits, FilterFn a_fnFilter )
{
	TUINT uiOldStamp = ms_uiQueryStamp++;

	if ( uiOldStamp > ms_uiQueryStamp )
	{
		// The stamp wrapped around, reset it in every unit
		ms_uiQueryStamp = 1;

		if ( m_pOwner )
			m_pOwner->SetUnitsQueryStamp( 0 );
	}

	*a_pNumFound = 0;

	if ( a_bAllUnits )
	{
		FindUnitsInList( a_rcPos, a_fRadius, a_pNumFound, a_b3D, a_iMaxNum, a_fnFilter );
		return m_aResults;
	}

	T2_FOREACH( m_UsedList, it )
	{
		AUnit* pUnit = it->pUnit;

		if ( ( a_fnFilter == TNULL || a_fnFilter( pUnit ) ) &&
		     !AddQueryResult( pUnit, a_rcPos, a_fRadius, a_b3D, a_pNumFound, a_iMaxNum ) )
			return m_aResults;
	}

	// Calculate the covered cells
	TFLOAT fXMin = ( a_rcPos.x - a_fRadius + m_fNegativeMinX ) * m_fDivideByMaxXOverNumCells;
	if ( fXMin < 0.0f ) fXMin -= 1.0f;
	TINT iXMin = TINT( fXMin );

	TFLOAT fXMax = ( a_fRadius + a_rcPos.x + m_fNegativeMinX ) * m_fDivideByMaxXOverNumCells;
	if ( fXMax < 0.0f ) fXMax -= 1.0f;
	TINT iXMax = TINT( fXMax );

	TFLOAT fZMin = ( a_rcPos.z - a_fRadius + m_fNegativeMinZ ) * m_fDivideByMaxZOverNumCells;
	if ( fZMin < 0.0f ) fZMin -= 1.0f;
	TINT iZMin = TINT( fZMin );

	TFLOAT fZMax = ( a_fRadius + a_rcPos.z + m_fNegativeMinZ ) * m_fDivideByMaxZOverNumCells;
	if ( fZMax < 0.0f ) fZMax -= 1.0f;
	TINT iZMax = TINT( fZMax );

	for ( TINT iZ = iZMin; iZ < iZMax + 1; iZ++ )
	{
		for ( TINT iX = iXMin; iX < iXMax + 1; iX++ )
		{
			TNodeList<CellObject>& rCellBucket = m_aCellBuckets[ ( iX & GRID_MASK ) * GRID_SIZE + ( iZ & GRID_MASK ) ];

			T2_FOREACH( rCellBucket, it )
			{
				AUnit* pUnit = it->pUnit;

				if ( ( a_fnFilter == TNULL || a_fnFilter( pUnit ) ) &&
				     !AddQueryResult( pUnit, a_rcPos, a_fRadius, a_b3D, a_pNumFound, a_iMaxNum ) )
					return m_aResults;
			}
		}
	}

	return m_aResults;
}
