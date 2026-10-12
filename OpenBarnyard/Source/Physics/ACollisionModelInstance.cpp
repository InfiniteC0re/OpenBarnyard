#include "pch.h"
#include "ACollisionModelInstance.h"
#include "ACollisionModelSet.h"
#include "Terrain/ATerrainInterface.h"

#include <Math/TMath.h>

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
	return ( m_pTransformObject ) ? m_pTransformObject->GetScale().x : 1.0f;
}

// $Barnyard: FUNCTION 0061ad80
void ACollisionModelInstance::Update()
{
	for ( TINT i = 0; i < m_pCollisionModelSet->m_iNumMeshes; i++ )
	{
		// NOTE: retail calls an empty method of every collision model here (nullsub_9, with this instance, 0 and 20.0f)
	}
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

// $Barnyard: FUNCTION 0061adb0
TModelCollisionData* ACollisionModelInstance::GetCollisionData( TINT a_iModel ) const
{
	return m_pCollisionModelSet->m_vecCollModels[ a_iModel ]->GetCollisionData();
}

// $Barnyard: FUNCTION 0061add0
TBOOL ACollisionModelInstance::CollideRay( const TVector4& a_rcRayOrigin, const TVector4& a_rcRayDir, TFLOAT& a_rfMaxDistance, ACollisionModelRayHitList* a_pHitList, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups, TBOOL a_bCulling )
{
	TBOOL bCollided = TFALSE;

	for ( TINT i = 0; i < m_pCollisionModelSet->m_iNumMeshes; i++ )
	{
		bCollided |= m_pCollisionModelSet->m_vecCollModels[ i ]->CollideRay( this, a_rcRayOrigin, a_rcRayDir, a_rfMaxDistance, a_pHitList, a_eIgnoreGroups, a_bCulling );
	}

	return bCollided;
}

// $Barnyard: FUNCTION 0061af90
TBOOL ACollisionModelInstance::CollideSphere( const TVector4& a_rcSphere, ACollisionModelRayHitList* a_pHitList, TINT a_iCollGroup, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TBOOL bCollided = TFALSE;

	for ( TINT i = 0; i < m_pCollisionModelSet->m_iNumMeshes; i++ )
	{
		bCollided |= m_pCollisionModelSet->m_vecCollModels[ i ]->CollideSphere( this, a_rcSphere, a_pHitList, a_iCollGroup, a_eIgnoreGroups );
	}

	return bCollided;
}

// $Barnyard: FUNCTION 0061aff0
TBOOL ACollisionModelInstance::CollideSphere( const TVector4& a_rcSphere, TFLOAT& a_rfDistance, TVector4& a_rNormal, TINT a_iCollGroup, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	a_rfDistance = a_rcSphere.w;

	TBOOL bCollided = TFALSE;

	for ( TINT i = 0; i < m_pCollisionModelSet->m_iNumMeshes; i++ )
	{
		bCollided |= m_pCollisionModelSet->m_vecCollModels[ i ]->CollideSphere( this, a_rcSphere, a_rfDistance, a_rNormal, a_iCollGroup, a_eIgnoreGroups );
	}

	return bCollided;
}

// $Barnyard: FUNCTION 0061b060
TBOOL ACollisionModelInstance::PushOutSphere( const TVector4& a_rcSphere, TVector4& a_rOutPosition, TINT a_iCollGroup, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	a_rOutPosition = a_rcSphere;

	TFLOAT   fDistance;
	TVector4 vecNormal;
	TINT     iNumIterations = 5;

	if ( CollideSphere( a_rOutPosition, fDistance, vecNormal, a_iCollGroup, a_eIgnoreGroups ) )
	{
		do
		{
			if ( --iNumIterations == 0 )
				break;

			// Move the sphere out along the normal
			TVector4 vecOffset;
			vecOffset.Multiply( vecNormal, a_rcSphere.w - fDistance + 0.01f );

			a_rOutPosition.x = vecOffset.x + a_rcSphere.x;
			a_rOutPosition.y = vecOffset.y + a_rcSphere.y;
			a_rOutPosition.z = vecOffset.z + a_rcSphere.z;
			a_rOutPosition.w = a_rcSphere.w;
		} while ( CollideSphere( a_rOutPosition, fDistance, vecNormal, a_iCollGroup, a_eIgnoreGroups ) );
	}

	TBOOL bMoved     = !( a_rOutPosition.x == a_rcSphere.x && a_rOutPosition.y == a_rcSphere.y && a_rOutPosition.z == a_rcSphere.z && a_rOutPosition.w == a_rcSphere.w );
	a_rOutPosition.w = 1.0f;

	return bMoved;
}

// $Barnyard: FUNCTION 0061b1a0
TBOOL ACollisionModelInstance::CollideSweptSphere( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcDirection, TFLOAT& a_rfDistance, TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight, TFLOAT& a_rfPenetration, TBOOL& a_rbEdgeHit, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TBOOL bCollided = TFALSE;

	for ( TINT i = 0; i < m_pCollisionModelSet->m_iNumMeshes; i++ )
	{
		bCollided |= m_pCollisionModelSet->m_vecCollModels[ i ]->CollideSweptSphere( this, a_rcCenter, a_fRadius, a_rcDirection, a_rfDistance, a_rNormal, a_fMaxStepHeight, a_rfStepHeight, a_rfPenetration, a_rbEdgeHit, a_eIgnoreGroups );
	}

	return bCollided;
}

// $Barnyard: FUNCTION 0061b2f0
TBOOL ACollisionModelInstance::CollideSweptSphere( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcDirection, TFLOAT& a_rfDistance, TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight, TFLOAT& a_rfPenetration, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	// NOTE: retail writes the edge flag over the stack slot of a_eIgnoreGroups after pushing it
	TBOOL bEdgeHit;
	return CollideSweptSphere( a_rcCenter, a_fRadius, a_rcDirection, a_rfDistance, a_rNormal, a_fMaxStepHeight, a_rfStepHeight, a_rfPenetration, bEdgeHit, a_eIgnoreGroups );
}

// $Barnyard: FUNCTION 0061b2b0
TBOOL ACollisionModelInstance::CollideSweptSphere( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcDirection, TFLOAT& a_rfDistance, TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight, TFLOAT& a_rfPenetration )
{
	// NOTE: retail writes the edge flag over its own stack slot of a_rfPenetration
	TBOOL bEdgeHit;
	return CollideSweptSphere( a_rcCenter, a_fRadius, a_rcDirection, a_rfDistance, a_rNormal, a_fMaxStepHeight, a_rfStepHeight, a_rfPenetration, bEdgeHit, TCollisionCommon::TOSHICGROUPFLAG_NONE );
}

// $Barnyard: FUNCTION 0061b330
TBOOL ACollisionModelInstance::SlideSweptSphere( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcDirection, TFLOAT& a_rfDistance, TFLOAT a_fMaxStepHeight, TFLOAT a_fSpeedScale, TFLOAT& a_rfStepHeight, TVector4& a_rOutPosition, TFLOAT a_fMinSlope, TFLOAT a_fMaxSlope )
{
	// Move the sphere out of the triangles first
	TVector4 vecSphere( a_rcCenter.x, a_rcCenter.y, a_rcCenter.z, a_fRadius );
	TVector4 vecStart = a_rcCenter;
	PushOutSphere( vecSphere, vecStart, -1, TCollisionCommon::TOSHICGROUPFLAG_NONE );

	TFLOAT fTotalDistance = a_rfDistance;
	a_rfDistance          = a_rfDistance + 0.1f;

	TVector4 vecNormal;
	TFLOAT   fPenetration = 0.0f;
	TBOOL    bEdgeHit;
	TBOOL    bHit = CollideSweptSphere( vecStart, a_fRadius, a_rcDirection, a_rfDistance, vecNormal, a_fMaxStepHeight, a_rfStepHeight, fPenetration, bEdgeHit, 1 );

	if ( bHit == TTRUE && a_rfDistance > 0.0f && a_rfDistance - 0.1f < 0.0f )
	{
		a_rfDistance = 0.0f;
	}
	else
	{
		a_rfDistance = a_rfDistance - 0.1f;

		if ( !bHit )
		{
			TVector4 vecOffset;
			vecOffset.Multiply( a_rcDirection, a_rfDistance );

			a_rOutPosition.x = vecOffset.x + vecStart.x;
			a_rOutPosition.y = vecOffset.y + vecStart.y;
			a_rOutPosition.z = vecOffset.z + vecStart.z;
			a_rOutPosition.w = vecStart.w;
			return TFALSE;
		}

		if ( a_rfDistance < 0.0f )
		{
			// Inside of something, try to get out backwards
			TVector4 vecBackDirection( -a_rcDirection.x, -a_rcDirection.y, -a_rcDirection.z, a_rcDirection.w );
			TFLOAT   fBackDistance = -a_rfDistance;
			TVector4 vecBackNormal;
			TBOOL    bBackEdgeHit;

			if ( CollideSweptSphere( vecStart, a_fRadius, vecBackDirection, fBackDistance, vecBackNormal, a_fMaxStepHeight, a_rfStepHeight, fPenetration, bBackEdgeHit, TCollisionCommon::TOSHICGROUPFLAG_NONE ) )
			{
				TVector4 vecOffset;
				vecOffset.Multiply( vecBackDirection, fBackDistance );

				a_rOutPosition   = vecStart + vecOffset;
				a_rOutPosition.w = vecStart.w;
				return TTRUE;
			}
		}
	}

	TVector4 vecPosition      = vecStart;
	TVector4 vecMoveDirection = a_rcDirection;
	TFLOAT   fRemaining       = a_rfDistance;

	TFLOAT fPushBack = TVector4::DotProduct3( vecNormal, vecMoveDirection ) * 0.0001f;
	fPushBack        = ( fPushBack < 0.0f ) ? fPushBack : 0.0f;

	if ( fRemaining < 0.0f )
	{
		fPushBack          = 0.0f;
		fRemaining         = -fRemaining;
		vecMoveDirection.x = -vecMoveDirection.x;
		vecMoveDirection.y = -vecMoveDirection.y;
		vecMoveDirection.z = -vecMoveDirection.z;
		fTotalDistance     = fRemaining + fTotalDistance;
	}

	TFLOAT fDistance = fRemaining;
	TFLOAT fMove     = fPushBack + fRemaining;

	if ( !( 0.0f < fMove ) )
		fMove = 0.0f;

	// Move to the hit
	TVector4 vecOffset;
	vecOffset.Multiply( vecMoveDirection, fMove );

	TVector4 vecTarget( vecOffset.x + vecPosition.x, vecOffset.y + vecPosition.y, vecOffset.z + vecPosition.z, vecPosition.w );
	TVector4 vecPrevDirection = vecMoveDirection;

	TMath::SlideAlongPlane( a_rcDirection, vecNormal, a_fSpeedScale, fTotalDistance, fDistance + fPushBack, fRemaining, vecMoveDirection, bEdgeHit, 0.1f, 0.004f, 1.0f );

	if ( a_rcDirection.y > 0.9f )
		ATerrainInterface::ScaleSpeedBySlopeSoft( fRemaining, vecMoveDirection, a_fMinSlope, a_fMaxSlope );

	fTotalDistance   = fRemaining;
	vecPrevDirection = vecMoveDirection;
	fDistance        = fRemaining;

	TINT iNumIterations = 0;

	while ( bHit )
	{
		if ( fDistance <= 0.001f || fRemaining <= 0.001f || TVector4::DotProduct3( vecPrevDirection, vecMoveDirection ) <= -0.001f )
			break;

		if ( iNumIterations >= 15 )
			break;

		bHit = CollideSweptSphere( vecTarget, a_fRadius, vecMoveDirection, fDistance, vecNormal, a_fMaxStepHeight, a_rfStepHeight, fPenetration, bEdgeHit, TCollisionCommon::TOSHICGROUPFLAG_NONE );

		fPushBack = TVector4::DotProduct3( vecNormal, vecMoveDirection ) * 0.0001f;
		fPushBack = ( fPushBack < 0.0f ) ? fPushBack : 0.0f;

		if ( bHit )
		{
			fMove = fDistance + fPushBack;

			if ( fMove <= 0.0f )
			{
				vecOffset.Multiply( vecMoveDirection, fDistance );
				vecTarget.x = vecOffset.x + vecTarget.x;
				vecTarget.y = vecOffset.y + vecTarget.y;
				vecTarget.z = vecOffset.z + vecTarget.z;
			}
			else
			{
				vecOffset.Multiply( vecMoveDirection, fMove );
				vecTarget.x = vecOffset.x + vecTarget.x;
				vecTarget.y = vecOffset.y + vecTarget.y;
				vecTarget.z = vecOffset.z + vecTarget.z;

				vecPrevDirection = vecMoveDirection;

				TMath::SlideAlongPlane( a_rcDirection, vecNormal, a_fSpeedScale, fTotalDistance, fDistance + fPushBack, fRemaining, vecMoveDirection, bEdgeHit, 0.1f, 0.004f, 1.0f );

				if ( a_rcDirection.y > 0.5f )
					ATerrainInterface::ScaleSpeedBySlopeSoft( fRemaining, vecMoveDirection, a_fMinSlope, a_fMaxSlope );

				fDistance      = fPushBack + fRemaining;
				fTotalDistance = fRemaining;
			}
		}
		else
		{
			vecOffset.Multiply( vecMoveDirection, fDistance );
			vecTarget.x = vecOffset.x + vecTarget.x;
			vecTarget.y = vecOffset.y + vecTarget.y;
			vecTarget.z = vecOffset.z + vecTarget.z;
		}

		iNumIterations++;
	}

	// Too many iterations, stay at the start
	a_rOutPosition = ( iNumIterations == 15 ) ? vecStart : vecTarget;
	return TTRUE;
}

// $Barnyard: FUNCTION 0061aed0
TBOOL ACollisionModelInstance::CollideBox( const TOBBox& a_rcBox, ACollisionModelRayHitList* a_pHitList, TINT a_iCollGroup, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TBOOL bCollided = TFALSE;

	for ( TINT i = 0; i < m_pCollisionModelSet->m_iNumMeshes; i++ )
	{
		bCollided |= m_pCollisionModelSet->m_vecCollModels[ i ]->CollideBox( this, a_rcBox, a_pHitList, a_iCollGroup, a_eIgnoreGroups );
	}

	return bCollided;
}

// $Barnyard: FUNCTION 0061af30
TBOOL ACollisionModelInstance::CollideSweptBox( const TOBBox& a_rcPrevBox, const TOBBox& a_rcBox, ACollisionModelRayHitList* a_pHitList, TINT a_iCollGroup, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TBOOL bCollided = TFALSE;

	for ( TINT i = 0; i < m_pCollisionModelSet->m_iNumMeshes; i++ )
	{
		bCollided |= m_pCollisionModelSet->m_vecCollModels[ i ]->CollideSweptBox( this, a_rcPrevBox, a_rcBox, a_pHitList, a_iCollGroup, a_eIgnoreGroups );
	}

	return bCollided;
}
