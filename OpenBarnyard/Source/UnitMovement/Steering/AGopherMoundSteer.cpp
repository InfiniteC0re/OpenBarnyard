#include "pch.h"
#include "AGopherMoundSteer.h"
#include "Units/AAnimatedUnit.h"
#include "Animation/AAnimatableObjectType.h"
#include "Animation/ANamedAnimation.h"
#include "Animation/ANamedAnimationSet.h"
#include "Render/AShadowManager.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/golf.h"
#include "Terrain/ATerrainInterface.h"
#include "Physics/ACollisionModel.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005608a0, CreateObject 00560ba0, CreateObjectInPlace 00560c00
TDEFINE_CLASS( AGopherMoundSteer );

TPSTRING8_DECLARE( above );
TPSTRING8_DECLARE( default );
TPSTRING8_DECLARE( idle );
TPSTRING8_DECLARE( run );
TPSTRING8_DECLARE( strike );
TPSTRING8_DECLARE( inground_idle );
TPSTRING8_DECLARE( inground_surface );
TPSTRING8_DECLARE( inground_dive );
TPSTRING8_DECLARE( aboveground_surface );
TPSTRING8_DECLARE( aboveground_dive );
TPSTRING8_DECLARE( victory );
TPSTRING8_DECLARE( victory_end );
TPSTRING8_DECLARE( lose );
TPSTRING8_DECLARE( lose_end );

// $Barnyard: FUNCTION 005608b0
AGopherMoundSteer::AGopherMoundSteer()
    : m_vPosition( TVector4::VEC_ZERO )
    , m_vVelocity( TVector4::VEC_ZERO )
    , m_vImpulse( TVector4::VEC_ZERO )
    , m_vDirection( TVector4::VEC_POSX )
    , m_vObstacle( TVector4::VEC_ZERO )
    , m_fSpeed( 0.0f )
    , m_fRadius( 0.0f )
    , m_iTracksCue( -1 )
    , m_eState( STATE_UNDERGROUND )
    , m_bStayHidden( TTRUE )
{
	// NOTE: retail leaves m_fIdleTime uninitialised
#ifdef BARNYARD_COMMUNITY_PATCH
	m_fIdleTime = 0.0f;
#endif
}

// $Barnyard: FUNCTION 005609a0
// $Barnyard: FUNCTION 00560c40
AGopherMoundSteer::~AGopherMoundSteer()
{
}

// $Barnyard: FUNCTION 005609b0
TBOOL AGopherMoundSteer::Create( AUnit* a_pOwner, const TVector4& a_rcPosition, const TVector4& a_rcDirection, TINT a_iLOD )
{
	ASteer::Create( a_pOwner, a_rcPosition, a_rcDirection, a_iLOD );

	m_vDirection = a_rcDirection;
	m_vVelocity  = TVector4::VEC_ZERO;
	SetPosition( a_rcPosition );

	return TTRUE;
}

// $Barnyard: FUNCTION 00560a20
TBOOL AGopherMoundSteer::CreateFrom( ASteer* a_pOther, TINT a_iLOD )
{
	ASteer::CreateFrom( a_pOther, a_iLOD );

	a_pOther->CopyDirection( m_vDirection );
	a_pOther->CopyVelocity( m_vVelocity );
	a_pOther->CopyPosition( m_vPosition );
	SetPosition( m_vPosition );

	return TTRUE;
}

// $Barnyard: FUNCTION 005619b0
TBOOL AGopherMoundSteer::Update( TFLOAT a_fDeltaTime )
{
	AUnit* pUnit = m_oOwner->GetUnit();

	if ( !pUnit->IsDying() )
	{
		if ( TSTATICCAST( AVisibleUnit, pUnit )->IsVisible() )
			UpdateMovement( a_fDeltaTime );

		m_vImpulse = TVector4::VEC_ZERO;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00561430
TBOOL AGopherMoundSteer::UpdateMovement( TFLOAT a_fDeltaTime )
{
	AAnimatedUnit* pUnit = TSTATICCAST( AAnimatedUnit, m_oOwner->GetUnit() );

	TVector4 vNewPosition = m_vPosition;
	TVector4 vMove;
	vMove.Multiply( m_vImpulse, a_fDeltaTime );
	vMove.Multiply( vMove, 0.75f );
	vNewPosition.Add( vMove );

	TFLOAT fImpulse = m_vImpulse.NormaliseMag();

	if ( fImpulse > 0.01f )
		m_vDirection = m_vImpulse;

	if ( ATerrainInterface::IsSingletonCreated() )
		MoveWithCollision( m_vPosition, vNewPosition, a_fDeltaTime );

	TVector3 vDelta( vNewPosition.x - m_vPosition.x, 0.0f, vNewPosition.z - m_vPosition.z );
	m_vPosition = vNewPosition;
	m_fSpeed    = vDelta.Magnitude() / a_fDeltaTime;

	AAnimatableObject* pObject = pUnit->GetAnimatableObject();

	if ( m_eState == STATE_SURFACING &&
	     !pObject->IsAnimPlaying( TPS8( aboveground_surface ), 0.0f ) &&
	     !pObject->IsAnimPlaying( TPS8( strike ), 0.0f ) )
	{
		m_eState = STATE_ABOVEGROUND;
	}

	if ( m_eState == STATE_DIVING &&
	     !pObject->IsAnimPlaying( TPS8( aboveground_dive ), 0.0f ) &&
	     !pObject->IsAnimPlaying( TPS8( strike ), 0.0f ) )
	{
		m_eState = STATE_UNDERGROUND;
	}

	if ( m_eState == STATE_ABOVEGROUND )
	{
		UpdateAboveGroundAnimation( fImpulse );
		m_fIdleTime = 0.0f;
	}
	else
	{
		AAnimatableObject* pModel = pUnit->GetAnimatableObject();

		if ( m_fSpeed <= 0.0f )
		{
			ASoundManager::GetSingleton()->StopCue( m_iTracksCue );
			m_iTracksCue = -1;

			if ( !pModel->IsAnimPlaying( TPS8( idle ), 0.0f ) || m_bStayHidden )
			{
				m_fIdleTime = 0.0f;
			}
			else
			{
				m_fIdleTime += a_fDeltaTime;

				// Peek out after standing still for a second
				if ( m_fIdleTime > 1.0f )
					pModel->PlayAnimation( TPS8( inground_surface ) );
			}

			if ( m_bStayHidden && ( pModel->IsAnimPlaying( TPS8( inground_surface ), 0.0f ) || pModel->IsAnimPlaying( TPS8( inground_idle ), 0.0f ) ) )
				pModel->PlayAnimation( TPS8( inground_dive ) );
		}
		else
		{
			if ( m_iTracksCue == -1 && ( m_eState == STATE_UNDERGROUND || m_eState == STATE_DIVING ) )
				m_iTracksCue = ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::GOLF_GOPHER_TRACKS, m_vPosition );

			if ( pModel->IsAnimPlaying( TPS8( inground_surface ), 0.0f ) || pModel->IsAnimPlaying( TPS8( inground_idle ), 0.0f ) )
				pModel->PlayAnimation( TPS8( inground_dive ) );

			m_fIdleTime = 0.0f;
		}
	}

	TINT iSurface = GetSurfaceGroup( m_vPosition );

	if ( iSurface == TCollisionCommon::TOSHICGROUP_TIMBER )
	{
		// Come out of the ground
		if ( m_eState == STATE_UNDERGROUND )
		{
			m_eState = STATE_SURFACING;
			SetAnimationSet( TPS8( above ) );

			ASoundManager::GetSingleton()->StopCue( m_iTracksCue );
			m_iTracksCue = -1;

			pUnit->GetAnimatableObject()->PlayAnimation( TPS8( aboveground_surface ) );
		}
	}
	else if ( iSurface == TCollisionCommon::TOSHICGROUP_GRASS && m_eState == STATE_ABOVEGROUND )
	{
		// Dive back into the ground
		m_eState = STATE_DIVING;
		SetAnimationSet( TPS8( default ) );

		pUnit->GetAnimatableObject()->PlayAnimation( TPS8( aboveground_dive ) );
	}

	// The shadow is hidden while the gopher is underground
	TBOOL               bShadow  = TTRUE;
	AToshiAnimationRef* pAnimRef = pObject->GetToshiAnimInterface().GetAnim();

	if ( pAnimRef )
	{
		TPString8 strAnim = pAnimRef->GetNamedAnimation()->GetName();

		if ( strAnim != TPS8( aboveground_dive ) &&
		     strAnim != TPS8( victory ) &&
		     strAnim != TPS8( victory_end ) &&
		     strAnim != TPS8( lose ) &&
		     strAnim != TPS8( lose_end ) )
		{
			bShadow = pObject->GetToshiAnimInterface().GetAnimationSet()->GetName() == TPS8( above );
		}
	}

	AShadowManager::GetSingleton()->SetShadowEnabled( pUnit, bShadow );

	return TTRUE;
}

// $Barnyard: FUNCTION 00561180
void AGopherMoundSteer::UpdateAboveGroundAnimation( TFLOAT a_fImpulse )
{
	AAnimatableObject* pObject = TSTATICCAST( AAnimatedUnit, m_oOwner->GetUnit() )->GetAnimatableObject();

	if ( pObject->IsAnimPlaying( TPS8( idle ), 0.0f ) || pObject->IsAnimPlaying( TPS8( run ), 0.0f ) )
	{
		if ( m_fSpeed >= 0.01f )
		{
			AToshiAnimationInterface& rAnimInterface = pObject->GetToshiAnimInterface();
			AToshiAnimationRef*       pAnimRef       = AToshiAnimationInterface::GetAnimationRef( rAnimInterface.GetAnimRefs(), TPS8( run ) );

			if ( pAnimRef || ( pAnimRef = rAnimInterface.PlayAnim( TPS8( run ), -1.0f, TFALSE ) ) )
				rAnimInterface.SetAnimSpeed( pAnimRef, a_fImpulse * 0.25f );
		}
		else
		{
			pObject->PlayAnimation( TPS8( idle ) );
		}
	}
}

// $Barnyard: FUNCTION 00560c60
void AGopherMoundSteer::MoveWithCollision( TVector4& a_rPosition, TVector4& a_rNewPosition, TFLOAT a_fDeltaTime )
{
	TFLOAT fDX = a_rNewPosition.x - a_rPosition.x;
	TFLOAT fDZ = a_rNewPosition.z - a_rPosition.z;

	if ( fDZ * fDZ + fDX * fDX < 0.000001f )
		return;

	TVector4 vToObstacle = a_rNewPosition;
	vToObstacle.x -= m_vObstacle.x;
	vToObstacle.y -= m_vObstacle.y;
	vToObstacle.z -= m_vObstacle.z;

	TFLOAT fObstacleDistSq = vToObstacle.MagnitudeSqXZ();
	TFLOAT fPushOutDist    = -1.0f;
	TFLOAT fMinDist        = m_fRadius + 0.3f;

	if ( fMinDist * fMinDist > fObstacleDistSq )
		fPushOutDist = fMinDist;

	TVector4 vDirection = a_rNewPosition;
	vDirection.x -= a_rPosition.x;
	vDirection.y -= a_rPosition.y;
	vDirection.z -= a_rPosition.z;

	TFLOAT fDistance  = vDirection.NormaliseMag();
	TFLOAT fRemaining = fDistance;

	TVector4 vNormal;
	TFLOAT   fStepHeight;
	TFLOAT   fPenetration;

	// Slide along whatever the sphere hits
	for ( TINT i = 0;
	      ATerrainInterface::GetSingleton()->CollideSweptSphere( a_rPosition, m_fRadius, vDirection, fDistance, vNormal, 0.0f, fStepHeight, fPenetration, TCollisionCommon::TOSHICGROUPFLAG( 0x8c028 ) );
	      fRemaining = fDistance )
	{
		if ( ++i >= 16 )
			break;

		TVector4 vMoved;
		vMoved.Multiply( vDirection, fDistance );
		a_rPosition.x += vMoved.x;
		a_rPosition.y += vMoved.y;
		a_rPosition.z += vMoved.z;

		vDirection.Multiply( fRemaining - fDistance );

		TVector4 vProjected = vDirection;
		vDirection.AsVector3().Magnitude(); // NOTE: the result isn't used in retail
		vProjected.ProjectOnto( vNormal );

		vDirection.x -= vProjected.x;
		vDirection.y -= vProjected.y;
		vDirection.z -= vProjected.z;
		fDistance = vDirection.NormaliseMag();
	}

	TVector4 vMoved;
	vMoved.Multiply( vDirection, fDistance );
	a_rNewPosition.x = a_rPosition.x + vMoved.x;
	a_rNewPosition.y = a_rPosition.y + vMoved.y;
	a_rNewPosition.z = a_rPosition.z + vMoved.z;
	a_rNewPosition.w = a_rPosition.w;

	// Push the gopher out of the obstacle
	if ( fPushOutDist > 0.0f )
	{
		vToObstacle.AsVector3().Divide( TMath::Sqrt( fObstacleDistSq ) );
		a_rNewPosition = vToObstacle;
		a_rNewPosition.Multiply( fPushOutDist );
		a_rNewPosition.x += m_vObstacle.x;
		a_rNewPosition.y += m_vObstacle.y;
		a_rNewPosition.z += m_vObstacle.z;
	}

	ATerrainInterface::GetSingleton()->SetToGroundHeightTop( a_rNewPosition, TCollisionCommon::TOSHICGROUPFLAG_CAMERA );
}

// $Barnyard: FUNCTION 00560fc0
TINT AGopherMoundSteer::GetSurfaceGroup( const TVector4& a_rcPosition )
{
	if ( !ATerrainInterface::IsSingletonCreated() )
		return -1;

	TVector4 vOrigin = a_rcPosition;
	vOrigin.y -= 0.1f;

	// Cast a ray up, then down
	TFLOAT                    fMaxDist = 2.5f;
	ACollisionModelRayHitList oHitList;
	ATerrainInterface::GetSingleton()->CollideRay( vOrigin, TVector4::VEC_POSY, fMaxDist, &oHitList, TCollisionCommon::TOSHICGROUPFLAG_CAMERA, TFALSE );

	if ( oHitList.iNumHits == 0 )
		ATerrainInterface::GetSingleton()->CollideRay( vOrigin, TVector4::VEC_NEGY, fMaxDist, &oHitList, TCollisionCommon::TOSHICGROUPFLAG_CAMERA, TFALSE );

	if ( oHitList.iNumHits <= 0 )
		return -1;

	// Find the closest hit
	TINT   iClosest = -1;
	TFLOAT fClosest = 50.0f;

	for ( TINT i = 0; i < oHitList.iNumHits; i++ )
	{
		if ( oHitList.aHits[ i ].fDistance < fClosest )
		{
			iClosest = i;
			fClosest = oHitList.aHits[ i ].fDistance;
		}
	}

	// NOTE: retail doesn't check iClosest (every hit is closer than 50 in practice)
	ACollisionModelRayHitList::Hit& rHit      = oHitList.aHits[ iClosest ];
	TModelCollisionData*            pCollData = rHit.pInstance->GetCollisionData( rHit.iModel );

	return pCollData->GetCollGroup( pCollData->GetGroupForTriangleImp( rHit.uiFaceId ) ).eCollGroup;
}

// NOTE: inlined in retail (UpdateMovement, SetPosition)
void AGopherMoundSteer::SetAnimationSet( const TPString8& a_rcName )
{
	AAnimatableObject* pObject = TSTATICCAST( AAnimatedUnit, m_oOwner->GetUnit() )->GetAnimatableObject();
	pObject->GetToshiAnimInterface().SetAnimationSet( pObject->GetObjectType()->FindAnimationSetOrDefault( a_rcName ) );
}

// $Barnyard: FUNCTION 00561220
TBOOL AGopherMoundSteer::SetPosition( const TVector4& a_rcPosition )
{
	m_vPosition = a_rcPosition;

	AAnimatableObject* pObject = TSTATICCAST( AAnimatedUnit, m_oOwner->GetUnit() )->GetAnimatableObject();
	pObject->KillAllAnimations();

	if ( ATerrainInterface::IsSingletonCreated() )
	{
		ATerrainInterface::GetSingleton()->SetToGroundHeightTop( m_vPosition, TCollisionCommon::TOSHICGROUPFLAG_CAMERA );

		if ( GetSurfaceGroup( m_vPosition ) == TCollisionCommon::TOSHICGROUP_TIMBER )
		{
			m_eState = STATE_ABOVEGROUND;
			SetAnimationSet( TPS8( above ) );
		}
		else
		{
			m_eState = STATE_UNDERGROUND;
			SetAnimationSet( TPS8( default ) );
		}
	}
	else
	{
		m_eState = STATE_UNDERGROUND;
		SetAnimationSet( TPS8( default ) );
	}

	pObject->PlayAnimation( TPS8( idle ) );

	ASoundManager::GetSingleton()->StopCue( m_iTracksCue );
	m_iTracksCue = -1;

	return TTRUE;
}

// NOTE: same code as ABallSteer2::SetPosition3 (00560a80), folded in retail
TBOOL AGopherMoundSteer::SetPosition3( const TVector3& a_rcPosition )
{
	return SetPosition( TVector4( a_rcPosition.x, a_rcPosition.y, a_rcPosition.z, 1.0f ) );
}

// $Barnyard: FUNCTION 0056d190
void AGopherMoundSteer::SetDirection( const TVector4& a_rcDirection )
{
	m_vDirection = a_rcDirection;
}

// NOTE: same code as AThrowItemSteer::CopyPosition (0055fd50), folded in retail
void AGopherMoundSteer::CopyPosition( TVector4& a_rOutPosition )
{
	a_rOutPosition = m_vPosition;
}

// NOTE: same code as AThrowItemSteer::CopyPosition3 (00556a20), folded in retail
void AGopherMoundSteer::CopyPosition3( TVector3& a_rOutPosition )
{
	a_rOutPosition = m_vPosition.AsVector3();
}

// NOTE: same code as ASteerKinematics3D::GetPosition (005772b0), folded in retail
const TVector4* AGopherMoundSteer::GetPosition()
{
	return &m_vPosition;
}

// $Barnyard: FUNCTION 0056d070
void AGopherMoundSteer::CopyDirection( TVector4& a_rOutDirection )
{
	a_rOutDirection = m_vDirection;
}

// $Barnyard: FUNCTION 0056d050
void AGopherMoundSteer::CopyDirection3( TVector3& a_rOutDirection )
{
	a_rOutDirection = m_vDirection.AsVector3();
}

// $Barnyard: FUNCTION 0056d040
const TVector4* AGopherMoundSteer::GetDirection()
{
	return &m_vDirection;
}

// $Barnyard: FUNCTION 00560b10
void AGopherMoundSteer::CopyVelocity( TVector4& a_rOutVelocity )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	a_rOutVelocity.Set( m_vVelocity.x, m_vVelocity.y, m_vVelocity.z, 1.0f );
#else
	// NOTE: retail copies x into y
	a_rOutVelocity.Set( m_vVelocity.x, m_vVelocity.x, m_vVelocity.z, 1.0f );
#endif
}

// NOTE: same code as AThrowItemSteer::GetVelocity (00560ac0), folded in retail
TVector4 AGopherMoundSteer::GetVelocity()
{
	return m_vVelocity;
}

// NOTE: same code as ASteerKinematics3D::GetSpeed (00577340), folded in retail
TFLOAT AGopherMoundSteer::GetSpeed()
{
	return m_fSpeed;
}

// $Barnyard: FUNCTION 00560af0
void AGopherMoundSteer::SetVelocity( const TVector4& a_rcVelocity )
{
	m_vVelocity = a_rcVelocity;
}

// $Barnyard: FUNCTION 00560b30
void AGopherMoundSteer::AddImpulse( const TVector4& a_rcImpulse )
{
	m_vImpulse.x += a_rcImpulse.x;
	m_vImpulse.y += a_rcImpulse.y;
	m_vImpulse.z += a_rcImpulse.z;
}

// $Barnyard: FUNCTION 00560b60
void AGopherMoundSteer::SetObstacle( const TVector4& a_rcCentre, TFLOAT a_fRadius )
{
	m_vObstacle = a_rcCentre;
	m_fRadius   = a_fRadius;
}

// $Barnyard: FUNCTION 00560b90
TBOOL AGopherMoundSteer::IsUnderground()
{
	return m_eState == STATE_UNDERGROUND;
}
