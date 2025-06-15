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

	TFLOAT fYMinRadius = ( rPosition.y - fRadius + m_fNegativeMinY ) * m_fDivideByMaxYOverNumCells;
	if ( fYMinRadius < 0.0f ) fYMinRadius -= 1.0f;

	TFLOAT fYMaxRadius = ( rPosition.y + fRadius + m_fNegativeMinY ) * m_fDivideByMaxYOverNumCells;
	if ( fYMaxRadius < 0.0f ) fYMaxRadius -= 1.0f;

	// Loop through the grid and add the object to each cell it contacts
	for ( TINT iY = TINT( fYMinRadius ); iY < TINT( fYMaxRadius ) + 1; iY++ )
	{
		// Remember the previous cell (aka cell that is also contains this object)
		CellObject* pPreviousCell = pCellObject;

		for ( TINT iX = TINT( fXMinRadius ); iX < TINT( fXMaxRadius ) + 1; iX++ )
		{
			TNodeList<CellObject>* pCellBucket = &m_aCellBuckets[ iY % GRID_SIZE + ( iX % GRID_SIZE ) * GRID_SIZE ];
			
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
	m_fNumCellsX               = a_iNumCellsX;
	m_fNumCellsY               = a_iNumCellsY;
	m_fTotalX                  = a_fMaxX - a_fMinX;
	m_fTotalY                  = a_fMaxY - a_fMinY;
	m_fNegativeMinX            = -a_fMinX;
	m_fNegativeMinY            = -a_fMinY;
	m_fMaxXOverNumCells        = a_fMaxX / a_iNumCellsX;
	m_fMaxYOverNumCells        = a_fMaxY / a_iNumCellsY;
	m_fDivideByMaxXOverNumCells = 1.0f / ( a_fMaxX / a_iNumCellsX );
	m_fDivideByMaxYOverNumCells = 1.0f / ( a_fMaxY / a_iNumCellsY );
}
