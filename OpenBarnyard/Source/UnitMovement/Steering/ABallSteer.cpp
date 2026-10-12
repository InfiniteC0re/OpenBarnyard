#include "pch.h"
#include "ABallSteer.h"
#include "ASteerTraits.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/golf.h"

#include "Terrain/ATerrainInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00556900, CreateObject 00556b60, CreateObjectInPlace 00556bc0 (the constructor is inlined in both)
TDEFINE_CLASS( ABallSteer, 3, 0 );

// $Barnyard: FUNCTION 00556910
ABallSteer::ABallSteer()
    : m_vecPosition( TVector4::VEC_ZERO )
    , m_vecMoveDirection( TVector4::VEC_POSY )
    , m_vecDirection( TVector4::VEC_POSX )
    , m_vecGroundNormal( TVector4::VEC_ZERO )
    , m_fSpeed( 0.0f )
    , m_fBounce( 0.35f )
    , m_fMinBounceRatio( 0.0f )
    , m_fRollingFriction( 0.0f )
    , m_fBounceFriction( 0.0f )
    , m_fGravity( 9.8f )
    , m_fStepTime( 0.0f )
    , m_eState( STATE_STOPPED )
    , m_iObstacleHitSound( -1 )
    , m_bPlayedHitSound( TFALSE )
    , m_bPlayHitSounds( TFALSE )
    , m_bPushOutIteratively( TFALSE )
{
}

// $Barnyard: FUNCTION 00556c00
// $Barnyard: FUNCTION 00556a10
ABallSteer::~ABallSteer()
{
}

// $Barnyard: FUNCTION 005575a0
TBOOL ABallSteer::CreateFrom( ASteer* a_pOther, TINT a_iLOD )
{
	ASteer::CreateFrom( a_pOther, a_iLOD );

	a_pOther->CopyPosition( m_vecPosition );
	PushOutOfTerrain();

	a_pOther->CopyDirection( m_vecDirection );
	a_pOther->CopyVelocity( m_vecMoveDirection );
	m_fSpeed         = m_vecMoveDirection.NormaliseMag();
	m_eState         = STATE_STOPPED;
	m_bPlayHitSounds = TFALSE;

	return TTRUE;
}

// $Barnyard: FUNCTION 005574f0
TBOOL ABallSteer::Create( AUnit* a_pOwner, const TVector4& a_rcPosition, const TVector4& a_rcDirection, TINT a_iLOD )
{
	ASteer::Create( a_pOwner, a_rcPosition, a_rcDirection, a_iLOD );

	m_vecPosition = a_rcPosition;

	if ( !a_rcPosition.IsEqual( TVector4::VEC_ZERO ) )
		PushOutOfTerrain();

	m_vecDirection     = a_rcDirection;
	m_vecMoveDirection = TVector4::VEC_POSY;
	m_fSpeed           = 0.0f;
	m_eState           = STATE_STOPPED;
	m_bPlayHitSounds   = TFALSE;

	return TTRUE;
}

// $Barnyard: FUNCTION 00557720
TBOOL ABallSteer::Update( TFLOAT a_fDeltaTime )
{
	m_bPlayedHitSound = TFALSE;
	m_fStepTime += a_fDeltaTime;

	// NOTE: the whole accumulated time is simulated at once, STEP_TIME is only the minimum
	if ( m_fStepTime >= STEP_TIME )
	{
		if ( m_eState > STATE_STOPPED && m_eState <= STATE_FLYING )
			Step( m_fStepTime );

		m_fStepTime = 0.0f;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005576c0
TBOOL ABallSteer::Step( TFLOAT a_fDeltaTime )
{
	if ( m_eState == STATE_ROLLING )
	{
		Roll( a_fDeltaTime );
		return TTRUE;
	}

	if ( m_eState == STATE_FLYING )
	{
		// Gravity
		m_vecMoveDirection.Multiply( m_fSpeed );
		m_vecMoveDirection.y = a_fDeltaTime * m_fGravity + m_vecMoveDirection.y;
		m_fSpeed             = m_vecMoveDirection.NormaliseMag();

		Fly( a_fDeltaTime );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00556dd0
TBOOL ABallSteer::Fly( TFLOAT a_fDeltaTime )
{
	TFLOAT   fDeltaTime = a_fDeltaTime;
	TFLOAT   fTimeLeft;
	TFLOAT   fDistance;
	TVector4 vecNormal;
	TVector4 vecNormalVelocity;

	while ( TTRUE )
	{
		TFLOAT fStepHeight;
		TFLOAT fPenetration;

		fDistance = fDeltaTime * m_fSpeed;

		if ( !ATerrainInterface::GetSingleton()->CollideSweptSphere( m_vecPosition, m_pTraits->m_fRadius, m_vecMoveDirection, fDistance, vecNormal, 0.0f, fStepHeight, fPenetration, IGNORE_GROUPS ) )
		{
			// Nothing in the way
			TVector4 vecMove;
			vecMove.Multiply( m_vecMoveDirection, fDistance );
			m_vecPosition.x += vecMove.x;
			m_vecPosition.y += vecMove.y;
			m_vecPosition.z += vecMove.z;

			return TTRUE;
		}

		if ( m_bPlayHitSounds && !m_bPlayedHitSound )
		{
			TFLOAT fVolume = TVector4::DotProduct3( m_vecMoveDirection, vecNormal ) * m_fSpeed * -0.2f;

			if ( fVolume > 1.0f )
				fVolume = 1.0f;

			if ( fVolume > 0.2f )
			{
				m_bPlayedHitSound = TTRUE;

				// Check if it's the ground or an obstacle (TOSHICGROUP_NOFRICTION) by colliding again without the obstacles
				// NOTE: uses a_fDeltaTime and not fDeltaTime of this iteration
				TFLOAT   fGroundDistance = a_fDeltaTime * m_fSpeed;
				TVector4 vecGroundNormal;

				if ( ATerrainInterface::GetSingleton()->CollideSweptSphere( m_vecPosition, m_pTraits->m_fRadius, m_vecMoveDirection, fGroundDistance, vecGroundNormal, 0.0f, fStepHeight, fPenetration, IGNORE_GROUPS | TCollisionCommon::TOSHICGROUPFLAG_NOFRICTION ) &&
				     fGroundDistance <= fDistance )
				{
					ASoundManager::GetSingleton()->PlayCueAtPositionEx( soundbank::GOLF_HIT_GROUND, m_vecPosition, fVolume, TFALSE, 0.0f, -1 );
				}
				else
				{
					ASoundManager::GetSingleton()->PlayCueAtPositionEx( m_iObstacleHitSound, m_vecPosition, fVolume, TFALSE, 0.0f, -1 );
				}
			}
		}

		// Move up to the hit
		TVector4 vecMove;
		vecMove.Multiply( m_vecMoveDirection, fDistance );
		m_vecPosition.x += vecMove.x;
		m_vecPosition.y += vecMove.y;
		m_vecPosition.z += vecMove.z;

		// NOTE: retail computes the time left from a_fDeltaTime and not from fDeltaTime of this iteration
		fTimeLeft = a_fDeltaTime - fDistance / m_fSpeed;

		// Split the velocity into the parts along the normal and along the surface
		m_vecMoveDirection.Multiply( m_fSpeed );
		vecNormalVelocity = m_vecMoveDirection;
		vecNormalVelocity.ProjectOnto( vecNormal );
		m_vecMoveDirection.x -= vecNormalVelocity.x;
		m_vecMoveDirection.y -= vecNormalVelocity.y;
		m_vecMoveDirection.z -= vecNormalVelocity.z;

		if ( m_fBounceFriction > 0.0f )
			m_vecMoveDirection.Multiply( m_fBounceFriction );

		if ( vecNormalVelocity.MagnitudeSq() > 1.0f || m_fSpeed > 2.0f )
		{
			// Bounce back
			TVector4 vecBounce;
			vecBounce.Multiply( vecNormalVelocity, m_fBounce );
			m_vecMoveDirection.x -= vecBounce.x;
			m_vecMoveDirection.y -= vecBounce.y;
			m_vecMoveDirection.z -= vecBounce.z;
		}
		else
		{
			vecNormalVelocity = TVector4::VEC_ZERO;
		}

		m_fSpeed = m_vecMoveDirection.NormaliseMag();

		// Stop bouncing once the bounce is too small
		if ( fDistance > 0.0001f &&
		     ( vecNormalVelocity.Magnitude() * ( m_fBounce / m_fSpeed ) < m_fMinBounceRatio ||
		       vecNormalVelocity.Magnitude() * m_fBounce < 0.05f ) )
		{
			break;
		}

		fDeltaTime = fTimeLeft;
	}

	// Undo the bounce, it rolls along the surface now
	m_vecMoveDirection.Multiply( m_fSpeed );

	TVector4 vecBounce;
	vecBounce.Multiply( vecNormalVelocity, m_fBounce );
	m_vecMoveDirection.x += vecBounce.x;
	m_vecMoveDirection.y += vecBounce.y;
	m_vecMoveDirection.z += vecBounce.z;
	m_fSpeed = m_vecMoveDirection.NormaliseMag();

	m_vecGroundNormal = vecNormal;
	m_eState          = STATE_ROLLING;

	return Roll( fTimeLeft );
}

// $Barnyard: FUNCTION 00557210
TBOOL ABallSteer::Roll( TFLOAT a_fDeltaTime )
{
	TFLOAT   fDistance = 0.05f;
	TVector4 vecNormal;
	TFLOAT   fStepHeight;
	TFLOAT   fPenetration;

	if ( !ATerrainInterface::GetSingleton()->CollideSweptSphere( m_vecPosition, m_pTraits->m_fRadius, TVector4::VEC_POSY, fDistance, vecNormal, 0.0f, fStepHeight, fPenetration, IGNORE_GROUPS ) )
	{
		// Nothing below, fall
		m_eState = STATE_FLYING;
		m_vecMoveDirection.Multiply( m_fSpeed );
		m_vecMoveDirection.y = a_fDeltaTime * m_fGravity + m_vecMoveDirection.y;
		m_fSpeed             = m_vecMoveDirection.NormaliseMag();

		return Fly( a_fDeltaTime );
	}

	// Split the gravity into the parts into the ground and along the ground
	TVector4 vecAcceleration;
	vecAcceleration.Multiply( TVector4::VEC_POSY, m_fGravity );

	TVector4 vecGravityNormal = vecAcceleration;
	vecGravityNormal.ProjectOnto( m_vecGroundNormal );
	vecAcceleration.x -= vecGravityNormal.x;
	vecAcceleration.y -= vecGravityNormal.y;
	vecAcceleration.z -= vecGravityNormal.z;

	// Friction, can't do more than stopping the ball
	TFLOAT fMaxFriction = TVector4::DotProduct3( vecGravityNormal, TVector4::VEC_POSY ) * m_fRollingFriction;
	TFLOAT fFriction    = TVector4::DotProduct3( vecAcceleration, m_vecMoveDirection ) + m_fSpeed / a_fDeltaTime;

	if ( fFriction >= fMaxFriction )
		fFriction = fMaxFriction;

	TVector4 vecFriction;
	vecFriction.Multiply( m_vecMoveDirection, fFriction );
	vecAcceleration.x -= vecFriction.x;
	vecAcceleration.y -= vecFriction.y;
	vecAcceleration.z -= vecFriction.z;

	m_vecMoveDirection.Multiply( m_fSpeed );

	TVector4 vecAdd;
	vecAdd.Multiply( vecAcceleration, a_fDeltaTime );
	m_vecMoveDirection.x += vecAdd.x;
	m_vecMoveDirection.y += vecAdd.y;
	m_vecMoveDirection.z += vecAdd.z;
	m_fSpeed = m_vecMoveDirection.NormaliseMag();

	if ( m_fSpeed < 0.01f && m_fRollingFriction > 0.0f )
	{
		m_fSpeed = 0.0f;
		m_eState = STATE_STOPPED;
		return TTRUE;
	}

	fDistance = m_fSpeed * a_fDeltaTime;

	if ( ATerrainInterface::GetSingleton()->CollideSweptSphere( m_vecPosition, m_pTraits->m_fRadius, m_vecMoveDirection, fDistance, vecNormal, 0.0f, fStepHeight, fPenetration, IGNORE_GROUPS ) )
	{
		// Hit something
		m_eState = STATE_FLYING;
		return Fly( a_fDeltaTime );
	}

	TVector4 vecMove;
	vecMove.Multiply( m_vecMoveDirection, m_fSpeed );
	vecMove.Multiply( a_fDeltaTime );
	m_vecPosition.x += vecMove.x;
	m_vecPosition.y += vecMove.y;
	m_vecPosition.z += vecMove.z;

	return TTRUE;
}

// $Barnyard: FUNCTION 00556c20
void ABallSteer::PushOutOfTerrain()
{
	if ( !ATerrainInterface::IsSingletonCreated() )
		return;

	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
	TFLOAT             fRadius  = m_pTraits->m_fRadius;

	if ( m_bPushOutIteratively )
	{
		// Move it out along the normal of the hit, up to 4 times
		TFLOAT   fDistance = fRadius;
		TVector4 vecSphere( m_vecPosition.x, m_vecPosition.y, m_vecPosition.z, fRadius );
		TVector4 vecNormal;

		for ( TINT i = 5; pTerrain->CollideSphere( vecSphere, fDistance, vecNormal, -1, IGNORE_GROUPS ); )
		{
			if ( --i == 0 )
				break;

			TVector4 vecMove;
			vecMove.Multiply( vecNormal, fRadius - fDistance + 0.01f );
			vecSphere.x += vecMove.x;
			vecSphere.y += vecMove.y;
			vecSphere.z += vecMove.z;
			vecSphere.w = fRadius;
			fDistance   = fRadius;
		}

		m_vecPosition   = vecSphere;
		m_vecPosition.w = 1.0f;
	}
	else
	{
		TVector4 vecSphere( m_vecPosition.x, m_vecPosition.y, m_vecPosition.z, fRadius );
		TVector4 vecPosition;

		if ( pTerrain->PushOutSphere( vecSphere, vecPosition, -1, IGNORE_GROUPS ) )
			m_vecPosition = vecPosition;
	}
}

// $Barnyard: FUNCTION 00557660
TBOOL ABallSteer::SetPosition( const TVector4& a_rcPosition )
{
	m_vecPosition = a_rcPosition;
	PushOutOfTerrain();

	m_vecMoveDirection = TVector4::VEC_POSY;
	m_fSpeed           = 0.0f;
	m_eState           = STATE_STOPPED;

	return TTRUE;
}

// $Barnyard: FUNCTION 00557600
TBOOL ABallSteer::SetPosition3( const TVector3& a_rcPosition )
{
	m_vecPosition.Set( a_rcPosition.x, a_rcPosition.y, a_rcPosition.z, 1.0f );
	PushOutOfTerrain();

	m_vecMoveDirection = TVector4::VEC_POSY;
	m_fSpeed           = 0.0f;
	m_eState           = STATE_STOPPED;

	return TTRUE;
}

// $Barnyard: FUNCTION 00556b40
void ABallSteer::SetDirection( const TVector4& a_rcDirection )
{
	m_vecDirection = a_rcDirection;
}

// NOTE: same code as AThrowItemSteer::CopyPosition (0055fd50), folded in retail
void ABallSteer::CopyPosition( TVector4& a_rOutPosition )
{
	a_rOutPosition = m_vecPosition;
}

// NOTE: same code as AThrowItemSteer::CopyPosition3 (00556a20), folded in retail
void ABallSteer::CopyPosition3( TVector3& a_rOutPosition )
{
	a_rOutPosition = m_vecPosition.AsVector3();
}

// NOTE: same code as ASteerKinematics3D::GetPosition (005772b0), folded in retail
const TVector4* ABallSteer::GetPosition()
{
	return &m_vecPosition;
}

// NOTE: same code as AThrowItemSteer::CopyDirection (00556a60), folded in retail
void ABallSteer::CopyDirection( TVector4& a_rOutDirection )
{
	a_rOutDirection = m_vecDirection;
}

// NOTE: same code as AThrowItemSteer::CopyDirection3 (00556a40), folded in retail
void ABallSteer::CopyDirection3( TVector3& a_rOutDirection )
{
	a_rOutDirection = m_vecDirection.AsVector3();
}

// NOTE: same code as AThrowItemSteer::GetDirection (00572640), folded in retail
const TVector4* ABallSteer::GetDirection()
{
	return &m_vecDirection;
}

// $Barnyard: FUNCTION 00556ac0
void ABallSteer::CopyVelocity( TVector4& a_rOutVelocity )
{
	TVector4 vecVelocity;
	vecVelocity.Multiply( m_vecMoveDirection, m_fSpeed );
	a_rOutVelocity = vecVelocity;
}

// $Barnyard: FUNCTION 00556a80
TVector4 ABallSteer::GetVelocity()
{
	TVector4 vecVelocity;
	vecVelocity.Multiply( m_vecMoveDirection, m_fSpeed );
	return vecVelocity;
}

// NOTE: shared getter of the float at +0x58 (0066bdd0), folded in retail
TFLOAT ABallSteer::GetSpeed()
{
	return m_fSpeed;
}

// $Barnyard: FUNCTION 00556b00
void ABallSteer::SetVelocity( const TVector4& a_rcVelocity )
{
	m_vecMoveDirection = a_rcVelocity;
	m_fSpeed           = m_vecMoveDirection.NormaliseMag();
	m_eState           = STATE_FLYING;
}
