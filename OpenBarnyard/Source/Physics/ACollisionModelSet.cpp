#include "pch.h"
#include "ACollisionModelSet.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0061a5c0
ACollisionModelSet::ACollisionModelSet()
    : m_pModel( TNULL )
    , m_iNumMeshes( 0 )
    , field5_0x90( 0 )
{
}

// $Barnyard: FUNCTION 0061a5e0
// $Barnyard: FUNCTION 0061ac50
ACollisionModelSet::~ACollisionModelSet()
{
    T2_FOREACH( m_vecCollModels, it )
    {
		delete it;
    }

	m_vecCollModels.Clear();
}

// $Barnyard: FUNCTION 0061a640
TBOOL ACollisionModelSet::Create( Toshi::TModel* a_pModel )
{
	m_pModel = a_pModel;

    TBOOL bSuccess = TTRUE;

    for ( TINT i = 0; i < a_pModel->GetNumCollisionMeshes(); i++ )
    {
		ACollisionModel* pCollisionModel = new ACollisionModel();

        TBOOL bCreated = pCollisionModel->Create( i, a_pModel );
		bSuccess &= bCreated;

		TASSERT( bCreated == TTRUE );

        m_vecCollModels.PushBack( pCollisionModel );
    }

    m_iNumMeshes = a_pModel->GetNumCollisionMeshes();
	TASSERT( a_pModel->GetNumCollisionMeshes() == m_vecCollModels.Size() );

    field5_0x90 = ( m_iNumMeshes < 2 ) ? 0 : 1;
	return bSuccess;
}
