#include "pch.h"
#include "ACollisionModel.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00618b40
ACollisionModel::ACollisionModel()
    : m_pModel( TNULL )
{
}

// $Barnyard: FUNCTION 0061a7b0
// $Barnyard: FUNCTION 00618fe0
ACollisionModel::~ACollisionModel()
{
}

// $Barnyard: FUNCTION 00618cb0
TBOOL ACollisionModel::Create( TINT a_iMeshIndex, Toshi::TModel* a_pModel )
{
	m_iMeshIndex = a_iMeshIndex;
	m_pModel     = a_pModel;

	TASSERT( !"Not implemented" );

	return TTRUE;
}
