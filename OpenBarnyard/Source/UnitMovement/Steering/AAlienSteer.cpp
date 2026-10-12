#include "pch.h"
#include "AAlienSteer.h"
#include "ASteerKinematics.h"
#include "Animation/AAlienAnimController.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: CreateObject 005563b0 / CreateObjectInPlace 00556410
TDEFINE_CLASS( AAlienSteer );

// NOTE: TVector4::NormaliseXZ (006c7540, normalises in the XZ plane and returns the length there)
static TFLOAT NormaliseXZ( TVector4& a_rVector )
{
	a_rVector.y = 0.0f;

	TFLOAT fLength = a_rVector.x * a_rVector.x + a_rVector.z * a_rVector.z;

	if ( fLength == 0.0f )
	{
		a_rVector.x = 0.0f;
		a_rVector.z = 1.0f;
	}
	else
	{
		fLength     = TMath::Sqrt( fLength );
		a_rVector.x = 1.0f / fLength * a_rVector.x;
		a_rVector.z = 1.0f / fLength * a_rVector.z;
	}

	return fLength;
}

// $Barnyard: FUNCTION 00555d60
AAlienSteer::AAlienSteer()
    : m_bStrafing( TFALSE )
    , m_bStrafingUpdated( TFALSE )
    , m_bTurningToTarget( TFALSE )
    , m_bRolling( TFALSE )
    , m_vDesiredVelocity( TVector4::VEC_POSZ )
{
}

// $Barnyard: FUNCTION 00555e60
// $Barnyard: FUNCTION 00556450
AAlienSteer::~AAlienSteer()
{
}

// $Barnyard: FUNCTION 00556470
TBOOL AAlienSteer::Update( TFLOAT a_fDeltaTime )
{
	if ( !m_bStrafing && !m_bRolling )
		return APlayerSteer::Update( a_fDeltaTime );

	m_bStrafingUpdated = TTRUE;
	m_bTurningToTarget = TFALSE;

	// NOTE: a static initialised on first use in retail (0x00788abc)
	static TFLOAT s_fMaxTurnSpeed = TMath::PI * 1.5f;

	TVector4 vToTarget = m_vTarget - *GetPosition();
	vToTarget.w        = m_vTarget.w;

	TFLOAT fDistance = NormaliseXZ( vToTarget );

	if ( fDistance <= 0.0f )
		vToTarget = TVector4::VEC_POSZ;

	// Turn towards the target at a limited rate
	TVector4 vFacing = *GetKinematics()->GetFacingDir();
	NormaliseXZ( vFacing );

	TFLOAT fAngle  = vFacing.GetAngleBetweenVector( vToTarget );
	m_fFacingAngle = fAngle;

	if ( fAngle > 0.0f )
	{
		m_bTurningToTarget = TTRUE;

		TFLOAT fTurn = s_fMaxTurnSpeed * a_fDeltaTime;

		if ( fTurn > fAngle )
			fTurn = fAngle;

		TVector4 vCross;
		vCross.CrossProduct( vToTarget, vFacing );

		if ( vCross.y > 0.0f )
			vFacing.AsVector3().RotateY( -fTurn );
		else
			vFacing.AsVector3().RotateY( fTurn );
	}

	GetKinematics()->SetFacing( vFacing );
	GetKinematics()->SetDesiredFacing( vToTarget );

	if ( m_bRolling )
	{
		CircleTarget( m_vRollVelocity, fDistance, a_fDeltaTime * 10.0f );

		TVector4 vVelocity;
		vVelocity.Multiply( m_vRollVelocity, 10.0f );
		APlayerSteer::SetDesiredVelocity( vVelocity );
	}
	else if ( m_fDesiredSpeed > 0.0f )
	{
		CircleTarget( m_vDesiredVelocity, fDistance, a_fDeltaTime * m_fDesiredSpeed );
		TVector4 vVelocity;
		vVelocity.Multiply( m_vDesiredVelocity, m_fDesiredSpeed );
		APlayerSteer::SetDesiredVelocity( vVelocity );
		m_fDesiredSpeed = 0.0f;
	}

	// The movement uses the desired facing, the model keeps facing the target
	TVector4 vTurnedFacing = *GetKinematics()->GetFacingDir();
	GetKinematics()->SetFacing( *GetKinematics()->GetDesiredFacing() );

	TBOOL bResult = APlayerSteer::Update( a_fDeltaTime );
	GetKinematics()->SetFacing( vTurnedFacing );

	return bResult;
}

// $Barnyard: FUNCTION 00556840
TBOOL AAlienSteer::SetDesiredVelocity( const TVector4& a_rcVelocity )
{
	if ( !m_bStrafing && !m_bRolling )
		return APlayerSteer::SetDesiredVelocity( a_rcVelocity );

	// Applied by the next Update
	m_vDesiredVelocity = a_rcVelocity;
	m_fDesiredSpeed    = NormaliseXZ( m_vDesiredVelocity );
	GetStrafeDirection( m_vDesiredVelocity );

	return TTRUE;
}

// $Barnyard: FUNCTION 00556110
TBOOL AAlienSteer::StopAndFace( const TVector4& a_rcFacing )
{
	return APlayerSteer::StopAndFace( a_rcFacing );
}

// $Barnyard: FUNCTION 005567c0
TBOOL AAlienSteer::SetState( STATE a_eState )
{
	if ( a_eState != STATE_JUMPING )
		return APlayerSteer::SetState( a_eState );

	if ( m_bRolling )
		return TTRUE;

	// Jumping while strafing starts a combat roll
	if ( !m_bStrafing || !CanRoll() )
		return APlayerSteer::SetState( STATE_JUMPING );

	m_bRolling      = TTRUE;
	m_vRollVelocity = m_vDesiredVelocity;

	return TTRUE;
}

// $Barnyard: FUNCTION 005560e0
TBOOL AAlienSteer::EndState( STATE a_eState )
{
	if ( a_eState == STATE_JUMPING && m_bRolling )
		return TTRUE;

	return APlayerSteer::EndState( a_eState );
}

// $Barnyard: FUNCTION 00556010
void AAlienSteer::SetStrafing( TBOOL a_bStrafing )
{
	if ( a_bStrafing != m_bStrafing )
	{
		m_bStrafing        = a_bStrafing;
		m_bStrafingUpdated = TFALSE;
		m_fStrafeAngle     = 0.0f;
		m_fDesiredSpeed    = 0.0f;

		if ( !a_bStrafing )
			GetKinematics()->SetSpeed( 0.0f );
	}
}

// $Barnyard: FUNCTION 00556060
TBOOL AAlienSteer::IsStrafingOrRolling() const
{
	return m_bStrafing || m_bRolling;
}

// $Barnyard: FUNCTION 00556080
TBOOL AAlienSteer::IsStrafingActive() const
{
	return ( m_bStrafing && m_bStrafingUpdated ) || m_bRolling;
}

// $Barnyard: FUNCTION 005560b0
void AAlienSteer::SetTarget( const TVector4& a_rcTarget )
{
	m_vTarget = a_rcTarget;
}

// $Barnyard: FUNCTION 00556120
const TVector4* AAlienSteer::GetDesiredVelocity() const
{
	return &m_vDesiredVelocity;
}

// $Barnyard: FUNCTION 00556130
TBOOL AAlienSteer::IsTurning() const
{
	return m_bTurningToTarget;
}

// $Barnyard: FUNCTION 00556140
TBOOL AAlienSteer::CanRoll()
{
	if ( GetKinematics()->GetSpeed() < 0.01f )
		return TFALSE;

	TVector4 vFacing          = *GetKinematics()->GetFacingDir();
	TVector4 vDesiredVelocity = m_vDesiredVelocity;

	// Only sideways
	TINT iDirection = AAlienAnimController::GetDirectionIndex( vFacing, vDesiredVelocity );
	return iDirection == 2 || iDirection == 6;
}

// $Barnyard: FUNCTION 005561e0
TBOOL AAlienSteer::IsRolling() const
{
	return m_bRolling;
}

// $Barnyard: FUNCTION 005561f0
void AAlienSteer::StopRoll()
{
	m_bRolling = TFALSE;
	GetKinematics()->SetSpeed( 0.0f );
}

// $Barnyard: FUNCTION 00556210
TFLOAT AAlienSteer::GetFacingAngle() const
{
	return m_fFacingAngle;
}

// $Barnyard: FUNCTION 00556220
void AAlienSteer::GetStrafeDirection( TVector4& a_rDirection )
{
	// NOTE: a static initialised on first use in retail (0x00788ab4)
	static TFLOAT s_fQuarterAngle = TMath::TWO_PI * 0.125f;

	TVector4 vToTarget = m_vTarget - *GetPosition();
	vToTarget.w        = m_vTarget.w;
	NormaliseXZ( vToTarget );

	TFLOAT fAngle = vToTarget.GetAngleBetweenVector( a_rDirection );

	// Ignore small changes of the input
	if ( TMath::Abs( m_fStrafeAngle - fAngle ) > 0.1f )
		m_fStrafeAngle = fAngle;
	else
		fAngle = m_fStrafeAngle;

	if ( fAngle >= s_fQuarterAngle )
	{
		if ( fAngle < s_fQuarterAngle * 3.0f )
		{
			// Sideways, circle around the target
			TVector4 vCross;
			vCross.CrossProduct( a_rDirection, vToTarget );

			if ( vCross.y > 0.0f )
				vToTarget.AsVector3().RotateY( -TMath::HALF_PI );
			else
				vToTarget.AsVector3().RotateY( TMath::HALF_PI );
		}
		else
		{
			// Backwards, away from the target
			vToTarget.x = -vToTarget.x;
			vToTarget.y = -vToTarget.y;
			vToTarget.z = -vToTarget.z;
		}
	}

	a_rDirection = vToTarget;
}

// $Barnyard: FUNCTION 00555e70
void AAlienSteer::CircleTarget( TVector4& a_rDirection, TFLOAT a_fRadius, TFLOAT a_fDistance )
{
	TVector4 vFromTarget = *GetPosition() - m_vTarget;
	vFromTarget.w        = GetPosition()->w;

	TFLOAT fAngle = a_fDistance / a_fRadius;

	TVector4 vCross;
	vCross.CrossProduct( vFromTarget, a_rDirection );

	if ( vCross.y <= 0.0f )
		vFromTarget.AsVector3().RotateY( -fAngle );
	else
		vFromTarget.AsVector3().RotateY( fAngle );

	TVector4 vNewPosition;
	vNewPosition.x = vFromTarget.x + m_vTarget.x;
	vNewPosition.y = vFromTarget.y + m_vTarget.y;
	vNewPosition.z = vFromTarget.z + m_vTarget.z;
	vNewPosition.w = m_vTarget.w;

	TVector4 vDirection = vNewPosition - *GetPosition();
	vDirection.w        = vNewPosition.w;

	// Only follow the circle while it doesn't bend the direction too much
	if ( TMath::PI * 0.1f > vDirection.GetAngleBetweenVector( a_rDirection ) )
	{
		a_rDirection = vDirection;
		a_rDirection.Normalise();
	}
}
