#include "pch.h"
#include "AProjectileFireCameraHelper.h"
#include "ACameraManager.h"
#include "Units/AUnit.h"
#include "Units/AVisibleUnit.h"
#include "UnitMovement/Steering/ASteer.h"
#include "Physics/ACollisionManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AProjectileFireCameraHelper );

// $Barnyard: FUNCTION 00466120
AProjectileFireCameraHelper::AProjectileFireCameraHelper()
    : m_fUnk14( 2.0f )
    , m_vAim( TVector2::VEC_ZERO )
    , m_fSpeed( 1.0f )
    , m_bLookingAtTarget( TFALSE )
    , m_fDistance( 5.5f )
    , m_fSideOffset( 0.0f )
    , m_fHeight( 3.5f )
    , m_fTargetHeight( 2.0f )
    , m_uiCollisionMask( 0x80000 )
{
}

// $Barnyard: FUNCTION 00466070
// $Barnyard: FUNCTION 00466180
AProjectileFireCameraHelper::~AProjectileFireCameraHelper()
{
}

// $Barnyard: FUNCTION 004667a0
TBOOL AProjectileFireCameraHelper::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( !m_pTarget )
		return TFALSE;

	TVector4 vPosition;
	TVector4 vDirection;
	GetCameraTransform( vPosition, vDirection, a_fDeltaTime );

	m_pCamera->GetMatrix().GetTranslation() = vPosition;
	LookAtDirection( vDirection );

	m_vLastTargetPos = *m_pTarget->GetUnit()->GetSteer()->GetPosition();
	return TTRUE;
}

// $Barnyard: FUNCTION 00465f80
void AProjectileFireCameraHelper::OnCreate()
{
	TVector4 vDirection;

	if ( ACameraManager::GetSingleton()->GetCurrentCamera() )
	{
		// Look down
		const TVector4& rcUp = m_pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_UP );
		vDirection.Set( -rcUp.x, -rcUp.y, -rcUp.z, 1.0f );
	}
	else
	{
		m_pCamera->GetMatrix().GetTranslation() = TVector4( 0.0f, -10.0f, 0.0f, 1.0f );
		vDirection                              = TVector4::VEC_POSX;
	}

	LookAtDirection( vDirection );
}

// $Barnyard: FUNCTION 00466080
TFLOAT AProjectileFireCameraHelper::GetTargetMovedDistanceXZ() const
{
	if ( !m_pTarget || !m_pTarget->GetUnit() )
		return 0.0f;

	TVector4 vTargetPos = *m_pTarget->GetUnit()->GetSteer()->GetPosition();
	TVector4 vLastPos   = m_vLastTargetPos;
	vTargetPos.y        = 0.0f;
	vLastPos.y          = 0.0f;

	return ( vLastPos - vTargetPos ).Magnitude();
}

// $Barnyard: FUNCTION 004661a0
void AProjectileFireCameraHelper::GetCameraTransform( TVector4& a_rOutPosition, TVector4& a_rOutDirection, TFLOAT a_fDeltaTime )
{
	AUnit*   pUnit      = m_pTarget->GetUnit();
	TVector4 vCameraPos = m_pCamera->GetMatrix().GetTranslation();
	TVector4 vTargetPos = *pUnit->GetSteer()->GetPosition();
	TVector4 vDirection = *pUnit->GetSteer()->GetDirection();

	TSTATICCAST( AVisibleUnit, pUnit )->SetVisible( TTRUE );

	// Aim up/down around the side axis of the target
	TVector4 vSide;
	vSide.CrossProduct( vDirection, TVector4::VEC_NEGY );
	vDirection.RotateAroundAxis( vSide, TMath::HALF_PI * m_vAim.x );
	vDirection.Multiply( -1.0f );

	TVector4 vSideOffset;
	vSideOffset.Multiply( vSide, m_fSideOffset );
	vDirection.x += vSideOffset.x;
	vDirection.y += vSideOffset.y;
	vDirection.z += vSideOffset.z;
	vDirection.Normalise();

	// NOTE: the result is unused in the original
	GetTargetMovedDistanceXZ();

	// Aim left/right around the up axis
	vDirection.RotateAroundAxis( TVector4::VEC_NEGY, -( TMath::HALF_PI * m_vAim.y ) );

	TVector4 vOffset;
	vOffset.Multiply( vDirection, m_fDistance );

	TVector4 vIdealPos;
	vIdealPos.x = vOffset.x + vTargetPos.x;
	vIdealPos.y = vOffset.y + vTargetPos.y - m_fHeight;
	vIdealPos.z = vOffset.z + vTargetPos.z;
	vIdealPos.w = vOffset.w;

	vTargetPos.y -= m_fTargetHeight;

	// Move the camera towards the ideal position
	TVector4 vToIdeal( vIdealPos.x - vCameraPos.x, vIdealPos.y - vCameraPos.y, vIdealPos.z - vCameraPos.z, vTargetPos.w );
	TVector4 vVelocity;
	vVelocity.Multiply( vToIdeal, m_fSpeed );

	TVector4 vMove;
	vMove.Multiply( vVelocity, a_fDeltaTime );
	vCameraPos.x += vMove.x;
	vCameraPos.y += vMove.y;
	vCameraPos.z += vMove.z;

	TVector4 vToTarget( vTargetPos.x - vCameraPos.x, vTargetPos.y - vCameraPos.y, vTargetPos.z - vCameraPos.z, vTargetPos.w );
	TFLOAT   fDistance = vToTarget.NormaliseMag();

	TVector4 vRayDirection;
	vRayDirection.Multiply( vToTarget, -1.0f );

	TVector4 vIdealToTarget( vTargetPos.x - vIdealPos.x, vTargetPos.y - vIdealPos.y, vTargetPos.z - vIdealPos.z, vTargetPos.w );
	vIdealToTarget.Normalise();

	m_bLookingAtTarget = TVector4::DotProduct3( vIdealToTarget, vToTarget ) > 0.99f;

	// Don't let the camera go through the terrain
	ARayData oRay( vTargetPos, vRayDirection, fDistance );
	TFLOAT   fHitDistance;

	if ( ACollisionManager::GetSingleton()->CollideRayWithTerrain( pUnit, oRay, fHitDistance, TCollisionCommon::TOSHICGROUPFLAG( m_uiCollisionMask ) ) )
	{
		fHitDistance *= 0.99f;

		TVector4 vHitOffset;
		vHitOffset.Multiply( vRayDirection, fHitDistance );
		vCameraPos.x = vHitOffset.x + vTargetPos.x;
		vCameraPos.y = vHitOffset.y + vTargetPos.y;
		vCameraPos.z = vHitOffset.z + vTargetPos.z;
		vCameraPos.w = vTargetPos.w;
	}

	m_vLastTargetPos = *m_pTarget->GetUnit()->GetSteer()->GetPosition();

	a_rOutPosition  = vCameraPos;
	a_rOutDirection = vToTarget;
}
