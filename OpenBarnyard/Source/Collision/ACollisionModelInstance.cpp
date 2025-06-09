#include "pch.h"
#include "ACollisionModelInstance.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0061ac70
ACollisionModelInstance::ACollisionModelInstance()
    : m_pCollisionModelSet( TNULL )
    , m_pSceneObject( TNULL )
    , m_iUnk( -1 )
    , m_pTransformObject( TNULL )
{
}

ACollisionModelInstance::~ACollisionModelInstance()
{
}

// $Barnyard: FUNCTION 0061ac90
void ACollisionModelInstance::Create( ACollisionModelSet* a_pModelSet, Toshi::TSceneObject* a_pSceneObject )
{
	m_pCollisionModelSet = a_pModelSet;
	m_pSceneObject       = a_pSceneObject;
	m_iUnk               = -1;
	m_pTransformObject   = &a_pSceneObject->GetTransform();

	// Force transform mode to matrix
	if ( m_pTransformObject->GetMode() != TTransformObject::Mode::Matrix )
	{
		TMatrix44 matTransform;
		m_pTransformObject->GetLocalMatrixImp( matTransform );
		m_pTransformObject->SetMatrix( matTransform );
	}
}
