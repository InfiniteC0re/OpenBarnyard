#include "pch.h"
#include "AInstanceManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 005dfd60
AInstanceManager::AInstanceManager()
    : m_ppKeyLibs( TNULL )
    , m_pModels( TNULL )
    , m_pSomeBuffer( TNULL )
    , m_iInstanceLibIndex( -1 )
    , m_ppCollObjectModelNodes( TNULL )
    , m_pCollisionModelInstances( TNULL )
    , m_uiModelsFlags( 0 )
    , m_iNumKeyLibraries( 0 )
    , m_flUnk1( 30.0f )
    , m_flUnk2( 85.0f )
    , m_flUnk3( 75.0f )
{
	m_iNumModels = NUM_MODELS;
	m_pModels    = new Model[ NUM_MODELS ];

	for ( TSIZE i = 0; i < TARRAYSIZE( m_aS1 ); i++ )
		m_llFreeS1.PushFront( &m_aS1[ i ] );

	m_pSomeBuffer = TMalloc( m_iNumModels << 3 );
}

AInstanceManager::~AInstanceManager()
{
}

// $Barnyard: FUNCTION 005e00f0
void AInstanceManager::DestroyCollisionSets()
{
	while ( !m_vecCollisionSets.IsEmpty() )
	{
		ACollisionModelSet* pModelSet = *m_vecCollisionSets.Back();
		delete pModelSet;

		m_vecCollisionSets.PopBack();
	}

	m_uiModelsFlags &= ~2;
}

// $Barnyard: FUNCTION 005e0050
void AInstanceManager::ClearList( List& a_rList )
{
	while ( !a_rList.llCollModelNodes.IsEmpty() )
	{
		auto pLast = a_rList.llCollModelNodes.Back().Get();
		a_rList.llCollModelNodes.PopBack();

		ACollisionObjectModel* pModel = &pLast->GetNodeValue();

		m_llFreeCollisionModelNodes.PushBack( pLast );
		if ( pModel->IsCollisionObjectLinked() )
			pModel->UnlinkCollisionObject();
	}

	a_rList.iUnknown = 0;
}

// $Barnyard: FUNCTION 005e0140
void AInstanceManager::SetupCollisions( TBOOL a_bCreateCollisionSets )
{
	const TBOOL bHasCollisionSets = ( ( m_uiModelsFlags & 2 ) == 2 );

	// Check if new flag value is different
	if ( bHasCollisionSets == a_bCreateCollisionSets ) return;

	// Update flags
	if ( bHasCollisionSets ) m_uiModelsFlags |= 2;
	else m_uiModelsFlags &= ~2;

	if ( bHasCollisionSets )
	{
		if ( m_llFreeCollisionModelInstances.IsEmpty() )
		{
			// Need to (re)allocate model instance nodes
			if ( m_pCollisionModelInstances ) delete[] m_pCollisionModelInstances;

			// Create nodes
			m_pCollisionModelInstances = new ACollisionModelInstanceNode[ 34 ];

			// Add all to the free list
			for ( TINT i = 0; i < 34; i++ )
				m_llFreeCollisionModelInstances.PushFront( &m_pCollisionModelInstances[ i ] );
		}

		// Create collision sets for the models
		for ( TINT i = 0; i < m_iNumModels; i++ )
		{
			TModel*    pModel         = m_pModels[ i ].pManagedModel->GetModel();
			const TINT iNumCollMeshes = pModel->GetNumCollisionMeshes();

			if ( iNumCollMeshes > 0 )
			{
				ACollisionModelSet* pCollModelSet = new ACollisionModelSet;
				pCollModelSet->Create( pModel );

				m_vecCollisionSets.PushBack( pCollModelSet );
			}
		}
	}
	else
	{
		// Doesn't need collision sets anymore, destroy them
		m_llFreeCollisionModelInstances.Clear();
		DestroyCollisionSets();
	}
}

// T2SList:
// $Barnyard: FUNCTION 006c7e10
// $Barnyard: FUNCTION 006c7e60
// $Barnyard: FUNCTION 006c7e30
