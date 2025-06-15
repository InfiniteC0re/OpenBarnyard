#include "pch.h"
#include "ACollisionModelInstance.h"
#include "ACollisionModelSet.h"

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
void ACollisionModelInstance::Create( ACollisionModelSet* a_pModelSet, TSceneObject* a_pSceneObject )
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

// $Barnyard: FUNCTION 0061ace0
void ACollisionModelInstance::GetTransformForBone( TINT a_iBone, TMatrix44& a_rOutMatrix )
{
	if ( a_iBone == TBONE_INVALID )
	{
		m_pTransformObject->GetLocalMatrixImp( a_rOutMatrix );
	}
	else
	{
		TMatrix44 matBoneTransform;
		m_pSceneObject->GetSkeletonInstance()->GetBoneTransformCurrent( a_iBone, matBoneTransform );

		m_pTransformObject->GetLocalMatrixImp( a_rOutMatrix );
		a_rOutMatrix.Multiply( matBoneTransform );
	}
}

// $Barnyard: FUNCTION 0061ad60
TFLOAT ACollisionModelInstance::GetScale() const
{
	return ( m_pTransformObject ) ? m_pTransformObject->GetScale().x : 0.0f;
}

// $Barnyard: FUNCTION 0061ae40
TBOOL ACollisionModelInstance::CollideRay( const TVector4& a_rcRayOrigin, const TVector4& a_rcRayDir, TFLOAT& a_rfMaxDistance, TCollisionCommon::TOSHICGROUPFLAG a_eCGroupsMask, TBOOL a_bCulling )
{
	TBOOL bCollided = TFALSE;

	for ( TINT i = 0; i < m_pCollisionModelSet->m_iNumMeshes; i++ )
	{
		if ( m_pCollisionModelSet->m_vecCollModels[ i ]->CollideRay( this, a_rcRayOrigin, a_rcRayDir, a_rfMaxDistance, a_eCGroupsMask, a_bCulling ) )
		{
			bCollided = TTRUE;
		}
	}

	return bCollided;
}
