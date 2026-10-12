#include "pch.h"
#include "AObjectHash.h"
#include "AHashedObject.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0061f760
AObjectHash::AObjectHash( TINT a_iNumNodes )
{
	m_fTotalX                   = 0.0f;
	m_fTotalY                   = 0.0f;
	m_fNegativeMinX             = 0.0f;
	m_fNegativeMinY             = 0.0f;
	m_fNumCellsX                = 0;
	m_fNumCellsY                = 0;
	m_fMaxXOverNumCells         = 0.0f;
	m_fMaxYOverNumCells         = 0.0f;
	m_fDivideByMaxXOverNumCells = 0.0f;
	m_fDivideByMaxYOverNumCells = 0.0f;

	m_pAllCells = new CellObject[ a_iNumNodes ];
	for ( TINT i = 0; i < a_iNumNodes; i++ )
		m_FreeList.InsertHead( &m_pAllCells[ i ] );
}

// $Barnyard: FUNCTION 0061f920
// $Barnyard: FUNCTION 0061f880
AObjectHash::~AObjectHash()
{
	m_FreeList.RemoveAll();

	if ( m_pAllCells )
		delete[] m_pAllCells;
}

// $Barnyard: FUNCTION 0061f530
AObjectHash::CellObject* AObjectHash::AssignHash( AHashedObject* a_pObject )
{
	TVALIDPTR( a_pObject );
	RemoveHash( a_pObject );

	// This will be the return value
	AObjectHash::CellObject* pCellObject = TNULL;

	const TVector4& rPosition = a_pObject->GetPosition();
	TFLOAT          fRadius   = a_pObject->GetRadius();

	// Calculate current position on the grid
	TFLOAT fXMinRadius = ( rPosition.x - fRadius + m_fNegativeMinX ) * m_fDivideByMaxXOverNumCells;
	if ( fXMinRadius < 0.0f ) fXMinRadius -= 1.0f;

	TFLOAT fXMaxRadius = ( rPosition.x + fRadius + m_fNegativeMinX ) * m_fDivideByMaxXOverNumCells;
	if ( fXMaxRadius < 0.0f ) fXMaxRadius -= 1.0f;

	// NOTE: the grid covers the XZ plane, the Y members of the cache are used for the Z axis
	TFLOAT fYMinRadius = ( rPosition.z - fRadius + m_fNegativeMinY ) * m_fDivideByMaxYOverNumCells;
	if ( fYMinRadius < 0.0f ) fYMinRadius -= 1.0f;

	TFLOAT fYMaxRadius = ( rPosition.z + fRadius + m_fNegativeMinY ) * m_fDivideByMaxYOverNumCells;
	if ( fYMaxRadius < 0.0f ) fYMaxRadius -= 1.0f;

	// Loop through the grid and add the object to each cell it contacts
	for ( TINT iY = TINT( fYMinRadius ); iY < TINT( fYMaxRadius ) + 1; iY++ )
	{
		// Remember the previous cell (aka cell that is also contains this object)
		CellObject* pPreviousCell = pCellObject;

		for ( TINT iX = TINT( fXMinRadius ); iX < TINT( fXMaxRadius ) + 1; iX++ )
		{
			TNodeList<CellObject>* pCellBucket = &m_aCellBuckets[ ( iY & GRID_MASK ) + ( iX & GRID_MASK ) * GRID_SIZE ];

			pCellObject = pPreviousCell;

			if ( !m_FreeList.IsEmpty() )
			{
				// Get a cell object from the free list
				pCellObject = m_FreeList.Begin();
				m_FreeList.Remove( pCellObject );

				// Fill info about this object
				pCellObject->pPreviousCell = pPreviousCell;
				pCellObject->pHashedObject = a_pObject;

				// Insert this object to the cell bucket
				pCellBucket->InsertTail( pCellObject );
			}
			else
			{
				TERROR( "AObjectHash::AssignHash: Couldn't assign a hash since the free list is empty.\n" );
				TASSERT( TFALSE );
			}

			pPreviousCell = pCellObject;
		}
	}

	// Do the assignment of the cell object
	TASSERT( pCellObject != TNULL );
	a_pObject->m_pHashNode = pCellObject;

	return pCellObject;
}

// $Barnyard: FUNCTION 0061f4d0
void AObjectHash::RemoveHash( AHashedObject* a_pObject )
{
	TVALIDPTR( a_pObject );
	CellObject* pHashNode = a_pObject->m_pHashNode;

	while ( pHashNode != TNULL )
	{
		// Detach the node and add it to the free list
		pHashNode->Remove();
		m_FreeList.InsertHead( pHashNode );

		// Go for the next node that is attached to it
		pHashNode = pHashNode->pPreviousCell;
	}

	// Remove hash from the object
	a_pObject->m_pHashNode = TNULL;
}

// $Barnyard: FUNCTION 0061f2f0
void AObjectHash::Debug_DrawUsedBBox()
{
	TFLOAT fMaxX = -FLT_MAX;
	TFLOAT fMaxZ = -FLT_MAX;
	TFLOAT fMinX = FLT_MAX;
	TFLOAT fMinZ = FLT_MAX;

	for ( TUINT i = 0; i < GRID_SIZE; i++ )
	{
		for ( TUINT k = 0; k < GRID_SIZE; k++ )
		{
			TNodeList<CellObject>& rCellBucket = m_aCellBuckets[ i + k * GRID_SIZE ];

			T2_FOREACH( rCellBucket, it )
			{
				AHashedObject* pHashedObject = it->pHashedObject;

				TVector4 vecPos  = pHashedObject->GetPosition();
				TFLOAT   fRadius = pHashedObject->GetRadius();

				fMinX = TMath::Min( fMinX, vecPos.x - fRadius );
				fMinZ = TMath::Min( fMinZ, vecPos.z - fRadius );
				fMaxX = TMath::Max( fMaxX, vecPos.x + fRadius );
				fMaxZ = TMath::Max( fMaxZ, vecPos.z + fRadius );
			}
		}
	}

#ifdef TOSHI_DEBUG

	// [6/15/2025 InfiniteC0re] TODO: draw the bounding box

#endif // TOSHI_DEBUG
}

// $Barnyard: FUNCTION 0061f040
void AObjectHash::InitialiseCache( TFLOAT a_fMinX, TFLOAT a_fMinY, TFLOAT a_fMaxX, TFLOAT a_fMaxY, TINT a_iNumCellsX, TINT a_iNumCellsY )
{
	m_fNumCellsX                = a_iNumCellsX;
	m_fNumCellsY                = a_iNumCellsY;
	m_fTotalX                   = a_fMaxX - a_fMinX;
	m_fTotalY                   = a_fMaxY - a_fMinY;
	m_fNegativeMinX             = -a_fMinX;
	m_fNegativeMinY             = -a_fMinY;
	m_fMaxXOverNumCells         = a_fMaxX / a_iNumCellsX;
	m_fMaxYOverNumCells         = a_fMaxY / a_iNumCellsY;
	m_fDivideByMaxXOverNumCells = 1.0f / ( a_fMaxX / a_iNumCellsX );
	m_fDivideByMaxYOverNumCells = 1.0f / ( a_fMaxY / a_iNumCellsY );
}

// $Barnyard: FUNCTION 0061ef30
TBOOL AObjectHash::AddQueryResult( AHashedObject* a_pObject, const TVector4& a_rcPos, TFLOAT a_fRadius, TBOOL a_b3D, TINT* a_pNumFound, TINT a_iMaxNum )
{
	if ( a_pObject->IsNoQuery() || TUINT( TUINTPTR( a_pObject->GetQueryStamp() ) ) == ms_uiQueryStamp )
		return TTRUE;

	// Don't test this object again during this query
	a_pObject->SetQueryStamp( (void*)TUINTPTR( ms_uiQueryStamp ) );

	TVector4 vecPosition = a_pObject->GetPosition();
	TFLOAT   fRadius     = a_pObject->GetRadius();

	TFLOAT fDX = vecPosition.x - a_rcPos.x;
	TFLOAT fDistanceSq;

	if ( a_b3D )
	{
		TFLOAT fDY  = vecPosition.y - a_rcPos.y;
		TFLOAT fDZ  = vecPosition.z - a_rcPos.z;
		fDistanceSq = fDY * fDY + fDZ * fDZ + fDX * fDX;
	}
	else
	{
		TFLOAT fDZ  = vecPosition.z - a_rcPos.z;
		fDistanceSq = fDZ * fDZ + fDX * fDX;
	}

	TFLOAT fTotalRadiusSq = ( fRadius + a_fRadius ) * ( fRadius + a_fRadius );

	if ( fDistanceSq >= fTotalRadiusSq )
		return TTRUE;

	m_aResults[ *a_pNumFound ].fUnk    = fTotalRadiusSq - fRadius * fRadius;
	m_aResults[ *a_pNumFound ].pObject = a_pObject;

	return ++( *a_pNumFound ) != a_iMaxNum;
}

// $Barnyard: FUNCTION 0061f0e0
AObjectHash::QueryResult* AObjectHash::FindObjects( const TVector4& a_rcPos, TFLOAT a_fRadius, TINT* a_pNumFound, TBOOL a_b3D, TINT a_iMaxNum )
{
	TUINT uiOldStamp = ms_uiQueryStamp++;

	// The stamp wrapped around
	if ( uiOldStamp > ms_uiQueryStamp )
		ms_uiQueryStamp = 1;

	*a_pNumFound = 0;

	T2_FOREACH( m_UsedList, it )
	{
		if ( !AddQueryResult( it->pHashedObject, a_rcPos, a_fRadius, a_b3D, a_pNumFound, a_iMaxNum ) )
			return m_aResults;
	}

	// Calculate the covered cells
	TFLOAT fXMin = ( a_rcPos.x - a_fRadius + m_fNegativeMinX ) * m_fDivideByMaxXOverNumCells;
	if ( fXMin < 0.0f ) fXMin -= 1.0f;
	TINT iXMin = TINT( fXMin );

	TFLOAT fXMax = ( a_fRadius + a_rcPos.x + m_fNegativeMinX ) * m_fDivideByMaxXOverNumCells;
	if ( fXMax < 0.0f ) fXMax -= 1.0f;
	TINT iXMax = TINT( fXMax );

	TFLOAT fZMin = ( a_rcPos.z - a_fRadius + m_fNegativeMinY ) * m_fDivideByMaxYOverNumCells;
	if ( fZMin < 0.0f ) fZMin -= 1.0f;
	TINT iZMin = TINT( fZMin );

	TFLOAT fZMax = ( a_fRadius + a_rcPos.z + m_fNegativeMinY ) * m_fDivideByMaxYOverNumCells;
	if ( fZMax < 0.0f ) fZMax -= 1.0f;
	TINT iZMax = TINT( fZMax );

	for ( TINT iZ = iZMin; iZ < iZMax + 1; iZ++ )
	{
		for ( TINT iX = iXMin; iX < iXMax + 1; iX++ )
		{
			TNodeList<CellObject>& rCellBucket = m_aCellBuckets[ ( iX & GRID_MASK ) * GRID_SIZE + ( iZ & GRID_MASK ) ];

			T2_FOREACH( rCellBucket, it )
			{
				if ( !AddQueryResult( it->pHashedObject, a_rcPos, a_fRadius, a_b3D, a_pNumFound, a_iMaxNum ) )
					return m_aResults;
			}
		}
	}

	return m_aResults;
}
