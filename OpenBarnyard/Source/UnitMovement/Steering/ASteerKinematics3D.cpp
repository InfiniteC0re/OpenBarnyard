#include "pch.h"
#include "ASteerKinematics3D.h"
#include "ASteer.h"
#include "Units/AVisibleUnit.h"
#include "Units/AKinematicsObstructionUnit.h"
#include "Terrain/ATerrainInterface.h"
#include "Physics/ACollisionObject.h"
#include "Physics/ACollisionManager.h"

#include <Math/TVector2.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: CreateObject 00578a40 / CreateObjectInPlace 00578aa0
// NOTE: GetClass 00577240, CreateObject 00578a40, CreateObjectInPlace 00578aa0
TDEFINE_CLASS( ASteerKinematics3D );

// The movement is integrated with steps of this length
static constexpr TFLOAT TIME_STEP = 1.0f / 30.0f;

// NOTE: TVector4::Distance (006c7700) without the assert of TMath::Sqrt on 0, the distance is often 0 here
static TFLOAT GetDistance( const TVector4& a_rcVec1, const TVector4& a_rcVec2 )
{
	return sqrtf( TVector4::DistanceSq( a_rcVec1, a_rcVec2 ) );
}

// $Barnyard: FUNCTION 00577b20
ASteerKinematics3D::ASteerKinematics3D()
    : m_uiFlags( FLAGS_NONE )
    , m_vPosition( TVector4::VEC_ZERO )
    , m_fVerticalSpeed( 0.0f )
    , m_fGravityScale( 1.0f )
    , m_vFacing( TVector4::VEC_POSX )
    , m_fSpeed( 0.0f )
    , m_fMeasuredSpeed( 0.0f )
    , m_fDesiredSpeed( 0.0f )
    , m_vDesiredFacing( TVector4::VEC_POSX )
    , m_fTimeToUpdate( 0.0f )
    , m_fSeekOffsetSq( 1.0f )
    , m_fUnk64( 1.0f )
    , m_fTurnSpeed( 0.0f )
    , m_fAirTime( 0.0f )
    , m_vSeekTarget( TVector4::VEC_ZERO )
    , m_vSeekStart( TVector4::VEC_ZERO )
    , m_vSeekDirection( TVector4::VEC_ZERO )
    , m_vSeekFacing( TVector4::VEC_ZERO )
    , m_fPitch( 0.0f )
    , m_fRoll( 0.0f )
    , m_fTargetPitch( 0.0f )
    , m_fTargetRoll( 0.0f )
{
	m_uiFlags |= FLAGS_UNK0;
}

// $Barnyard: FUNCTION 00577250
// $Barnyard: FUNCTION 005778e0
ASteerKinematics3D::~ASteerKinematics3D()
{
}

// $Barnyard: FUNCTION 00577f40
TBOOL ASteerKinematics3D::Create( const TVector4& a_rcPosition, const TVector4& a_rcDirection, const ASteerTraits* a_pTraits, TFLOAT a_fSpeed )
{
	ASteerKinematics::Create( a_rcPosition, a_rcDirection, a_pTraits, a_fSpeed );

	m_vFacing        = a_rcDirection;
	m_vDesiredFacing = a_rcDirection;
	m_vPosition      = a_rcPosition;
	m_fSpeed         = a_fSpeed;
	m_fGravityScale  = 1.0f;
	m_fDesiredSpeed  = a_fSpeed;
	m_fVerticalSpeed = 0.0f;
	m_fTimeToUpdate  = 0.0f;
	m_uiFlags        = FLAGS_NONE;
	m_uiFlags |= FLAGS_UNK0;
	m_vSeekTarget = a_rcPosition;

	return TTRUE;
}

// $Barnyard: FUNCTION 00577c90
TBOOL ASteerKinematics3D::CreateFrom( ASteer* a_pOther )
{
	ASteerKinematics::CreateFrom( a_pOther );

	ASteerKinematics* pOtherKinematics = a_pOther->GetKinematics();

	if ( pOtherKinematics )
	{
		if ( pOtherKinematics->GetClass() == GetClass() )
		{
			// Same kind of kinematics, copy everything
			ASteerKinematics3D* pOther = TSTATICCAST( ASteerKinematics3D, pOtherKinematics );

			m_vFacing        = pOther->m_vFacing;
			m_vDesiredFacing = pOther->m_vDesiredFacing;
			m_vPosition      = pOther->m_vPosition;
			m_fVerticalSpeed = pOther->m_fVerticalSpeed;
			m_fGravityScale  = pOther->m_fGravityScale;
			m_fPitch         = pOther->m_fPitch;
			m_fRoll          = pOther->m_fRoll;
			m_fTargetPitch   = pOther->m_fTargetPitch;
			m_fTargetRoll    = pOther->m_fTargetRoll;
			m_fSpeed         = pOther->m_fSpeed;
			m_fDesiredSpeed  = pOther->m_fDesiredSpeed;
			m_fTimeToUpdate  = pOther->m_fTimeToUpdate;
			m_uiFlags        = pOther->m_uiFlags;
			m_vSeekTarget    = pOther->m_vSeekTarget;
			m_fSeekOffsetSq  = m_pTraits->m_fGoalToleranceSq;

			return TTRUE;
		}

		pOtherKinematics->CopyFacingDir( m_vFacing );
		m_vDesiredFacing = m_vFacing;
		pOtherKinematics->CopyPosition( m_vPosition );

		m_fSpeed         = pOtherKinematics->GetSpeed();
		m_fDesiredSpeed  = pOtherKinematics->GetDesiredSpeed();
		m_fTimeToUpdate  = 0.0f;
		m_vSeekTarget    = *pOtherKinematics->GetSeekTarget();
		m_fVerticalSpeed = 0.0f;
		m_fPitch         = 0.0f;
		m_fRoll          = 0.0f;
		m_fTargetPitch   = 0.0f;
		m_fTargetRoll    = 0.0f;
		m_fGravityScale  = 1.0f;
		m_uiFlags        = FLAGS_UNK0;

		if ( pOtherKinematics->IsSeekingConstant() )
			m_uiFlags |= FLAGS_SEEKING;

		m_fSeekOffsetSq = m_pTraits->m_fGoalToleranceSq;
		return TTRUE;
	}

	// The other steer has no kinematics
	m_vFacing        = *a_pOther->GetDirection();
	m_vDesiredFacing = *a_pOther->GetDirection();
	m_vPosition      = *a_pOther->GetPosition();
	m_fVerticalSpeed = 0.0f;
	m_fGravityScale  = 1.0f;
	m_fPitch         = 0.0f;
	m_fRoll          = 0.0f;
	m_fTargetPitch   = 0.0f;
	m_fTargetRoll    = 0.0f;
	m_fSpeed         = a_pOther->GetSpeed();
	m_fDesiredSpeed  = a_pOther->GetSpeed();
	m_fTimeToUpdate  = 0.0f;
	m_uiFlags        = FLAGS_UNK0;
	m_vSeekTarget    = m_vPosition;
	m_fSeekOffsetSq  = m_pTraits->m_fGoalToleranceSq;

	return TTRUE;
}

// $Barnyard: FUNCTION 005777c0
TFLOAT ASteerKinematics3D::GetMeasuredSpeedRatio()
{
	return m_fMeasuredSpeed / m_pTraits->m_fMaxSpeed;
}

// $Barnyard: FUNCTION 00577280
TFLOAT ASteerKinematics3D::GetMeasuredSpeed()
{
	return m_fMeasuredSpeed;
}

// $Barnyard: FUNCTION 005773d0
void ASteerKinematics3D::BlendMeasuredSpeed( TFLOAT a_fSpeed )
{
	m_fMeasuredSpeed = TMath::LERP( m_fMeasuredSpeed, a_fSpeed, 0.9f );
}

// $Barnyard: FUNCTION 005772b0
const TVector4* ASteerKinematics3D::GetPosition()
{
	return &m_vPosition;
}

// $Barnyard: FUNCTION 00577310
TVector4 ASteerKinematics3D::Unk_0x28()
{
	return GetVelocity();
}

// $Barnyard: FUNCTION 005772c0
TVector4 ASteerKinematics3D::GetVelocity()
{
	TFLOAT   fSpeed = GetSpeed();
	TVector4 vVelocity;
	vVelocity.Multiply( *GetFacingDir(), fSpeed );

	return vVelocity;
}

// $Barnyard: FUNCTION 00577260
const TVector4* ASteerKinematics3D::GetFacingDir()
{
	return &m_vFacing;
}

// Folded with T2GUIElement::GetHeight (00577290)
TFLOAT ASteerKinematics3D::GetSpeed()
{
	return m_fSpeed;
}

// $Barnyard: FUNCTION 005772a0
TFLOAT ASteerKinematics3D::GetDesiredSpeed()
{
	return m_fDesiredSpeed;
}

// $Barnyard: FUNCTION 00577340
TFLOAT ASteerKinematics3D::GetTurnSpeed()
{
	return m_fTurnSpeed;
}

// $Barnyard: FUNCTION 00577360
const TVector4* ASteerKinematics3D::GetSeekTarget()
{
	return &m_vSeekTarget;
}

// $Barnyard: FUNCTION 00577540
TBOOL ASteerKinematics3D::IsFacingAtDesiredFacing( TFLOAT a_fTolerance )
{
	return TVector4::DotProduct3( m_vDesiredFacing, m_vFacing ) > a_fTolerance;
}

// $Barnyard: FUNCTION 00577770
void ASteerKinematics3D::SetDesiredFacing( const TVector4& a_rcFacing )
{
	m_vDesiredFacing = a_rcFacing;
}

// $Barnyard: FUNCTION 00577270
const TVector4* ASteerKinematics3D::GetDesiredFacing()
{
	return &m_vDesiredFacing;
}

// $Barnyard: FUNCTION 00577370
TVector4& ASteerKinematics3D::PredictPosition( TINT a_iTime, TVector4& a_rOut )
{
	// NOTE: the time is a float, see ASteerKinematics::PredictPosition
	TFLOAT fTime = *TREINTERPRETCAST( TFLOAT*, &a_iTime );

	a_rOut = m_vFacing;
	a_rOut.Multiply( fTime * m_fSpeed );
	a_rOut.x = m_vPosition.x + a_rOut.x;
	a_rOut.y = m_vPosition.y + a_rOut.y;
	a_rOut.z = m_vPosition.z + a_rOut.z;

	return a_rOut;
}

// $Barnyard: FUNCTION 00577440
void ASteerKinematics3D::StopMoving()
{
	m_fSpeed = 0.0f;
}

// $Barnyard: FUNCTION 00578db0
void ASteerKinematics3D::Update( TFLOAT a_fDeltaTime, AKinematicsObstructionLists* a_pObstructions, AKinematicsObstruction** a_ppIgnored, TINT a_iNumIgnored, UPDATEFLAGS a_uiFlags )
{
	UpdateTilt( a_fDeltaTime );

	TVector4 vPrevPosition = m_vPosition;

	if ( m_uiFlags & FLAGS_FROZEN )
		return;

	if ( m_uiFlags & ( FLAGS_SEEKING | FLAGS_SEEK_FACING ) )
		UpdateSeek( a_fDeltaTime, a_pObstructions, a_ppIgnored, a_iNumIgnored, a_uiFlags );

	const ASteerTraits* pTraits = m_pTraits;
	m_fTimeToUpdate += a_fDeltaTime;

	TFLOAT fStep;
	while ( GetNextTimeStep( a_fDeltaTime, &fStep ) )
	{
		TFLOAT fSpeedFactor = UpdateTurning( a_fDeltaTime, fStep );
		TFLOAT fRateFactor  = ( m_uiFlags & FLAGS_HEAVY_CARRY ) ? pTraits->m_fHeavyCarrySpeedFactor : 1.0f;
		TFLOAT fTargetSpeed = fSpeedFactor * fRateFactor * m_fDesiredSpeed;

		if ( m_fSpeed < 0.0f )
			fRateFactor *= -1.0f;

		if ( TMath::Abs( fTargetSpeed ) <= TMath::Abs( m_fSpeed ) )
		{
			// Decelerate
			m_fSpeed -= fRateFactor * pTraits->m_fDec * fStep;

			if ( m_fSpeed < fTargetSpeed )
				m_fSpeed = fTargetSpeed;
		}
		else
		{
			// Accelerate
			m_fSpeed += fRateFactor * pTraits->m_fAcc * fStep;

			if ( m_fSpeed > fTargetSpeed )
				m_fSpeed = fTargetSpeed;
		}

		TVector4 vMovement = GetVelocity();
		vMovement.Multiply( fStep );

		m_vPosition.x += vMovement.x;
		m_vPosition.y += vMovement.y;
		m_vPosition.z += vMovement.z;
	}

	UpdateObstruction( a_fDeltaTime );

	if ( ( m_uiFlags & FLAGS_SEEK_FACING ) && !( m_uiFlags & FLAGS_SEEKING ) )
	{
		TFLOAT fDistanceSq = TVector4::DistanceSq( m_vPosition, m_vSeekTarget );

		if ( IsFacingAtDesiredFacing( 0.99f ) && fDistanceSq < 0.0001f )
		{
			if ( TVector4::DotProduct3( m_vSeekFacing, m_vFacing ) < 0.98f )
			{
				// Turn to the final facing first
				SetDesiredFacing( m_vSeekFacing );
			}
			else
			{
				// Arrived
				m_uiFlags &= ~FLAGS_SEEK_FACING;
				SetPosition( m_vSeekTarget );
				SetFacing( m_vSeekFacing );
				SetSpeed( 0.0f );
			}
		}
		else
		{
			SetSpeed( GetDistance( m_vPosition, vPrevPosition ) / a_fDeltaTime );
		}
	}

	TFLOAT fPrevDistanceSq = TVector4::DistanceSq( vPrevPosition, m_vPosition );

	if ( !( m_uiFlags & FLAGS_SEEK_FACING ) || TVector4::DistanceSq( m_vSeekTarget, m_vPosition ) >= 0.0625f )
		UpdateObstructions( a_fDeltaTime, a_pObstructions, a_ppIgnored, a_iNumIgnored );

	if ( TMath::Abs( TVector4::DistanceSq( vPrevPosition, m_vPosition ) - fPrevDistanceSq ) < FLT_EPSILON )
		m_uiFlags &= ~FLAGS_MOVED;
	else
		m_uiFlags |= FLAGS_MOVED;
}

// $Barnyard: FUNCTION 00577450
void ASteerKinematics3D::ApplyGravity( TFLOAT a_fDeltaTime )
{
	// NOTE: +y is down
	TFLOAT fAcceleration = a_fDeltaTime * m_fGravityScale * 9.8f;

	m_vPosition.y = ( 0.5f * fAcceleration + m_fVerticalSpeed ) * a_fDeltaTime + m_vPosition.y;
	m_fVerticalSpeed += fAcceleration;
	m_fAirTime += a_fDeltaTime;
}

// $Barnyard: FUNCTION 00578ae0
void ASteerKinematics3D::UpdateFacing( TFLOAT a_fDeltaTime )
{
	TFLOAT fStep;

	if ( GetNextTimeStep( a_fDeltaTime, &fStep ) )
		UpdateTurning( a_fDeltaTime, fStep );
}

// $Barnyard: FUNCTION 00577900
void ASteerKinematics3D::MoveTo( TFLOAT a_fDeltaTime, const TVector4* a_pPrevPosition, const TVector4* a_pPosition )
{
	TFLOAT fTurnSpeed = TMath::Abs( m_fTurnSpeed );
	m_vPosition       = *a_pPosition;

	if ( m_fDesiredSpeed <= 0.00001f && fTurnSpeed <= 0.0001f )
	{
		TVector4 vPrevPosition = *a_pPrevPosition;
		m_fMeasuredSpeed       = TMath::LERP( m_fMeasuredSpeed, GetDistance( m_vPosition, vPrevPosition ) / a_fDeltaTime, 0.9f );
		return;
	}

	TVector4 vPrevPosition = *a_pPrevPosition;
	TFLOAT   fSpeed        = GetDistance( m_vPosition, vPrevPosition ) / a_fDeltaTime;

	// Turning on the spot still plays the walk animation
	TFLOAT fAnimatedSpeed = m_pTraits->GetAnimatedSpeed( ASteer::MOVEMENT_WALK );
	TFLOAT fTurnMinSpeed  = 10.0f * fTurnSpeed;
	TFLOAT fMinSpeed      = ( fAnimatedSpeed < fTurnMinSpeed ) ? fAnimatedSpeed : fTurnMinSpeed;
	TFLOAT fTargetSpeed   = ( fSpeed < fMinSpeed ) ? fMinSpeed : fSpeed;

	if ( fTargetSpeed > m_fMeasuredSpeed )
	{
		m_fMeasuredSpeed += a_fDeltaTime * 20.0f;

		if ( m_fMeasuredSpeed > fTargetSpeed )
			m_fMeasuredSpeed = fTargetSpeed;
	}
	else if ( fTargetSpeed < m_fMeasuredSpeed )
	{
		m_fMeasuredSpeed -= a_fDeltaTime * 5.0f;

		if ( m_fMeasuredSpeed < fTargetSpeed )
			m_fMeasuredSpeed = fTargetSpeed;
	}
}

// $Barnyard: FUNCTION 00577570
TBOOL ASteerKinematics3D::TurnTowards( const TVector4& a_rcTarget )
{
	TVector4 vDirection = a_rcTarget - m_vPosition;
	vDirection.w        = a_rcTarget.w;
	vDirection.y        = 0.0f;

	TFLOAT fDistanceSq = vDirection.MagnitudeSq();

	if ( fDistanceSq < m_fSeekOffsetSq )
		return TTRUE;

	vDirection.Divide( TMath::Sqrt( fDistanceSq ) );
	m_vDesiredFacing = vDirection;

	return TFALSE;
}

// $Barnyard: FUNCTION 00577640
TBOOL ASteerKinematics3D::TurnTowardsWithin( const TVector4& a_rcTarget, TINT a_iUnused, TFLOAT a_fToleranceSq, TFLOAT a_fMinDistanceSq )
{
	TVector4 vDirection = a_rcTarget - m_vPosition;
	vDirection.w        = a_rcTarget.w;
	vDirection.y        = 0.0f;

	TFLOAT fDistanceSq = vDirection.MagnitudeSq();

	if ( fDistanceSq >= a_fMinDistanceSq )
	{
		vDirection.Divide( TMath::Sqrt( fDistanceSq ) );
		m_vDesiredFacing = vDirection;
	}

	return fDistanceSq <= a_fToleranceSq;
}

// $Barnyard: FUNCTION 005773f0
TBOOL ASteerKinematics3D::SetPosition( const TVector4& a_rcPosition )
{
	m_vPosition = a_rcPosition;
	return TTRUE;
}

// $Barnyard: FUNCTION 00577730
void ASteerKinematics3D::SetFacing( const TVector4& a_rcFacing )
{
	m_vFacing        = a_rcFacing;
	m_vDesiredFacing = a_rcFacing;
}

// $Barnyard: FUNCTION 00577420
void ASteerKinematics3D::SetSpeed( TFLOAT a_fSpeed )
{
	m_fSpeed        = a_fSpeed;
	m_fDesiredSpeed = a_fSpeed;
}

// $Barnyard: FUNCTION 00577430
void ASteerKinematics3D::SetDesiredSpeed( TFLOAT a_fSpeed )
{
	m_fDesiredSpeed = a_fSpeed;
}

// $Barnyard: FUNCTION 00577790
void ASteerKinematics3D::SetDesiredMovementDir( const TVector4& a_rcFacing )
{
	m_vDesiredFacing.x = a_rcFacing.x;
	m_vDesiredFacing.z = a_rcFacing.z;
}

// $Barnyard: FUNCTION 00577ae0
void ASteerKinematics3D::Unk_0x9c( TBOOL a_bEnable )
{
	if ( a_bEnable )
		m_uiFlags |= FLAGS_UNK0;
	else
		m_uiFlags &= ~FLAGS_UNK0;
}

// $Barnyard: FUNCTION 00577b00
void ASteerKinematics3D::SetFrozen( TBOOL a_bFreeze )
{
	if ( a_bFreeze )
		m_uiFlags |= FLAGS_FROZEN;
	else
		m_uiFlags &= ~FLAGS_FROZEN;
}

// $Barnyard: FUNCTION 00578300
TBOOL ASteerKinematics3D::SeekAlongLine( const TVector4& a_rcStart, const TVector4& a_rcTarget )
{
	TVector4 vDirection = a_rcTarget - a_rcStart;
	vDirection.w        = a_rcTarget.w;
	vDirection.y        = 0.0f;

	if ( vDirection.MagnitudeSq() <= 0.000001f )
		return TFALSE;

	m_vSeekTarget   = a_rcTarget;
	m_vSeekStart    = a_rcStart;
	m_vSeekTarget.y = m_vSeekStart.y;

	m_vSeekDirection   = m_vSeekTarget - m_vSeekStart;
	m_vSeekDirection.w = m_vSeekTarget.w;
	m_vSeekDirection.Normalise();

	m_uiFlags = ( m_uiFlags & ~( FLAGS_SEEKING | FLAGS_SEEK_FACING ) ) | FLAGS_SEEKING;
	return TTRUE;
}

// $Barnyard: FUNCTION 00577710
TBOOL ASteerKinematics3D::Seek( const TVector4& a_rcTarget )
{
	return SeekAlongLine( m_vPosition, a_rcTarget );
}

// $Barnyard: FUNCTION 005782a0
TBOOL ASteerKinematics3D::SeekFacing( const TVector4& a_rcTarget, const TVector4& a_rcFacing, TFLOAT a_fUnk )
{
	m_vSeekFacing = a_rcFacing;
	m_vSeekTarget = a_rcTarget;

	TBOOL bResult = Seek( a_rcTarget );
	m_uiFlags |= FLAGS_SEEK_FACING;

	return bResult;
}

// $Barnyard: FUNCTION 00578430
void ASteerKinematics3D::Halt()
{
	m_uiFlags &= ~FLAGS_SEEKING;
	m_uiFlags &= ~FLAGS_SEEK_FACING;
}

// $Barnyard: FUNCTION 00578440
TBOOL ASteerKinematics3D::IsSeekingConstant()
{
	return ( m_uiFlags & ( FLAGS_SEEKING | FLAGS_SEEK_FACING ) ) != 0;
}

// $Barnyard: FUNCTION 00577a80
TBOOL ASteerKinematics3D::HasMoved()
{
	return ( m_uiFlags & FLAGS_MOVED ) != 0;
}

// $Barnyard: FUNCTION 005777b0
void ASteerKinematics3D::ResetSeekOffset()
{
	m_fSeekOffsetSq = m_pTraits->m_fGoalToleranceSq;
}

// $Barnyard: FUNCTION 00577ac0
void ASteerKinematics3D::SetSeekSlowdown( TBOOL a_bEnable )
{
	if ( a_bEnable )
		m_uiFlags |= FLAGS_SEEK_SLOWDOWN;
	else
		m_uiFlags &= ~FLAGS_SEEK_SLOWDOWN;
}

// $Barnyard: FUNCTION 00577350
TFLOAT ASteerKinematics3D::GetAirTime()
{
	return m_fAirTime;
}

// $Barnyard: FUNCTION 005777d0
void ASteerKinematics3D::ResetAirTime()
{
	m_fAirTime = 0.0f;
}

// $Barnyard: FUNCTION 005777e0
TFLOAT ASteerKinematics3D::GetVerticalSpeed()
{
	return m_fVerticalSpeed;
}

// $Barnyard: FUNCTION 00578a10
TBOOL ASteerKinematics3D::GetTilt( TFLOAT* a_pPitch, TFLOAT* a_pRoll )
{
	*a_pPitch = m_fPitch;
	*a_pRoll  = m_fRoll;

	return !( m_uiFlags & FLAGS_NO_TILT );
}

// $Barnyard: FUNCTION 00577a90
void ASteerKinematics3D::SetCarryingHeavy( TBOOL a_bCarrying )
{
	if ( a_bCarrying )
		m_uiFlags |= FLAGS_HEAVY_CARRY;
	else
		m_uiFlags &= ~FLAGS_HEAVY_CARRY;
}

// $Barnyard: FUNCTION 005777f0
void ASteerKinematics3D::SetGravityScale( TFLOAT a_fScale, TFLOAT a_fUnused )
{
	m_fGravityScale = a_fScale;
}

// $Barnyard: FUNCTION 00577800
void ASteerKinematics3D::SetTilt( TFLOAT a_fPitch, TFLOAT a_fRoll, TBOOL a_bImmediate )
{
	m_fTargetPitch = a_fPitch;
	m_fTargetRoll  = a_fRoll;

	if ( a_bImmediate )
	{
		m_fPitch = a_fPitch;
		m_fRoll  = a_fRoll;
	}
}

// $Barnyard: FUNCTION 00577490
TBOOL ASteerKinematics3D::GetNextTimeStep( TFLOAT a_fDeltaTime, TFLOAT* a_pOutStep )
{
	if ( m_fTimeToUpdate > TIME_STEP && m_fTimeToUpdate < TIME_STEP * 1.3f )
	{
		// Split what's slightly more than one step in two halves
		*a_pOutStep = m_fTimeToUpdate * 0.5f;
		m_fTimeToUpdate -= *a_pOutStep;
		return TTRUE;
	}

	if ( m_fTimeToUpdate >= TIME_STEP )
	{
		*a_pOutStep = TIME_STEP;
		m_fTimeToUpdate -= *a_pOutStep;
		return TTRUE;
	}

	if ( m_fTimeToUpdate >= 0.0001f )
	{
		*a_pOutStep = m_fTimeToUpdate;
		m_fTimeToUpdate -= *a_pOutStep;
		return TTRUE;
	}

	m_fTimeToUpdate = 0.0f;
	*a_pOutStep     = 0.0f;
	m_fTimeToUpdate -= *a_pOutStep;
	return TFALSE;
}

// $Barnyard: FUNCTION 00578000
TFLOAT ASteerKinematics3D::UpdateTurning( TFLOAT a_fDeltaTime, TFLOAT a_fStep )
{
	const ASteerTraits* pTraits = m_pTraits;

	TFLOAT fDot         = TVector4::DotProduct3( m_vDesiredFacing, m_vFacing );
	TFLOAT fTurningAcc  = pTraits->m_fTurningAcc;
	TFLOAT fTurningDec  = pTraits->m_fTurningDec;
	TFLOAT fMaxTurnRate = pTraits->m_fNHolonomicRate;
	TFLOAT fAngle       = TMath::ATan2( -( m_vDesiredFacing.z * m_vFacing.x - m_vFacing.z * m_vDesiredFacing.x ), fDot );

	// NOTE: a static initialised on first use in retail (0x007896b0)
	static TFLOAT s_fMinAngle = 0.017453292f;

	TFLOAT fSpeedFactor = 1.0f;

	if ( ( m_uiFlags & FLAGS_SEEK_SLOWDOWN ) && ( m_uiFlags & FLAGS_SEEKING ) )
	{
		// Slow down when not facing the target
		fSpeedFactor = fDot;
		TMath::Clip( fSpeedFactor, 0.0f, 1.0f );

		TFLOAT fDistance = GetDistance( m_vSeekTarget, m_vPosition );

		if ( fDistance < 1.0f )
		{
			// Even more when close to it
			TFLOAT fFactor = TMath::LERP( -( ( 1.0f - fDistance / 1.0f ) * 3.0f ), 1.0f, fSpeedFactor );
			fSpeedFactor   = ( 0.0f < fFactor ) ? fFactor : 0.0f;
		}
	}

	if ( fAngle < -s_fMinAngle )
	{
		// Turn right, the turning speed is limited so that it can stop in time
		TFLOAT fStopSpeedSq = fTurningDec * fAngle * -2.0f;
		TFLOAT fTarget      = -( ( fMaxTurnRate * fMaxTurnRate >= fStopSpeedSq ) ? TMath::Sqrt( fStopSpeedSq ) : fMaxTurnRate );

		if ( fTarget <= m_fTurnSpeed )
		{
			m_fTurnSpeed -= ( ( m_fTurnSpeed <= 0.0f ) ? fTurningAcc : fTurningDec ) * a_fStep;

			if ( m_fTurnSpeed < fTarget )
				m_fTurnSpeed = fTarget;
		}
		else
		{
			m_fTurnSpeed += fTurningDec * a_fStep;

			if ( m_fTurnSpeed > fTarget )
				m_fTurnSpeed = fTarget;
		}
	}
	else if ( fAngle > s_fMinAngle )
	{
		// Turn left
		TFLOAT fStopSpeedSq = fTurningDec * fAngle;
		fStopSpeedSq += fStopSpeedSq;

		TFLOAT fTarget = ( fMaxTurnRate * fMaxTurnRate >= fStopSpeedSq ) ? TMath::Sqrt( fStopSpeedSq ) : fMaxTurnRate;

		if ( fTarget >= m_fTurnSpeed )
		{
			m_fTurnSpeed += ( ( m_fTurnSpeed >= 0.0f ) ? fTurningAcc : fTurningDec ) * a_fStep;

			if ( m_fTurnSpeed > fTarget )
				m_fTurnSpeed = fTarget;
		}
		else
		{
			m_fTurnSpeed -= fTurningDec * a_fStep;

			if ( m_fTurnSpeed < fTarget )
				m_fTurnSpeed = fTarget;
		}
	}
	else
	{
		// Close enough, snap to the desired facing
		m_vFacing    = m_vDesiredFacing;
		m_fTurnSpeed = 0.0f;
		m_vFacing.Normalise();

		return fSpeedFactor;
	}

	m_vFacing.AsVector3().RotateY( a_fStep * m_fTurnSpeed );
	m_vFacing.Normalise();

	return fSpeedFactor;
}

// $Barnyard: FUNCTION 00577830
void ASteerKinematics3D::UpdateTilt( TFLOAT a_fDeltaTime )
{
	TFLOAT fChange = 1.0f * a_fDeltaTime;

	if ( m_fTargetPitch < m_fPitch )
	{
		m_fPitch -= fChange;
		if ( m_fPitch < m_fTargetPitch ) m_fPitch = m_fTargetPitch;
	}
	else if ( m_fTargetPitch > m_fPitch )
	{
		m_fPitch += fChange;
		if ( m_fPitch > m_fTargetPitch ) m_fPitch = m_fTargetPitch;
	}

	if ( m_fTargetRoll < m_fRoll )
	{
		m_fRoll -= fChange;
		if ( m_fRoll < m_fTargetRoll ) m_fRoll = m_fTargetRoll;
	}
	else if ( m_fTargetRoll > m_fRoll )
	{
		m_fRoll += fChange;
		if ( m_fRoll > m_fTargetRoll ) m_fRoll = m_fTargetRoll;
	}
}

// $Barnyard: FUNCTION 00578b10
TBOOL ASteerKinematics3D::UpdateSeek( TFLOAT a_fDeltaTime, AKinematicsObstructionLists* a_pObstructions, AKinematicsObstruction** a_ppIgnored, TINT a_iNumIgnored, UPDATEFLAGS a_uiFlags )
{
	// NOTE: retail passes all the arguments of Update but only uses the delta time
	if ( m_uiFlags & FLAGS_SEEKING )
	{
		TFLOAT   fDistance;
		TVector4 vFacing;

		if ( GetSeekFacing( a_fDeltaTime, &fDistance, &vFacing ) == TTRUE )
		{
			SetDesiredFacing( vFacing );
		}
		else if ( !( m_uiFlags & FLAGS_SEEK_FACING ) )
		{
			// Passed the target, keep going along the line
			TVector4 vDesiredFacing = *GetDesiredFacing();
			vDesiredFacing.Lerp4( vDesiredFacing, m_vSeekDirection, 0.9f * a_fDeltaTime );
			vDesiredFacing.Normalise();

			SetDesiredFacing( vDesiredFacing );
		}
	}

	if ( !( m_uiFlags & FLAGS_SEEKING ) && ( m_uiFlags & FLAGS_SEEK_FACING ) )
	{
		// Slide to the target
		SetSpeed( 0.0f );

		TVector4 vToTarget = m_vSeekTarget - m_vPosition;
		TVector2 vToTarget2D( vToTarget.x, vToTarget.z );

		TFLOAT fDistance    = vToTarget2D.Magnitude();
		TFLOAT fMaxDistance = 1.0f * a_fDeltaTime;

		if ( fDistance <= fMaxDistance )
		{
			SetPosition( m_vSeekTarget );
			SetDesiredFacing( m_vSeekFacing );
			return TTRUE;
		}

		TFLOAT fInvDistance = 1.0f / fDistance;
		vToTarget2D.x *= fInvDistance;
		vToTarget2D.y *= fInvDistance;

		TVector4 vFacing( vToTarget2D.x, 0.0f, vToTarget2D.y, 0.0f );

		TVector4 vPosition = m_vPosition;
		vPosition.x += vToTarget2D.x * fMaxDistance;
		vPosition.z += vToTarget2D.y * fMaxDistance;

		SetPosition( vPosition );
		SetDesiredFacing( vFacing );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00578450
TBOOL ASteerKinematics3D::GetSeekFacing( TFLOAT a_fDeltaTime, TFLOAT* a_pOutDistance, TVector4* a_pOutFacing )
{
	TBOOL bPassed = TFALSE;

	// NOTE: retail calls GetFacingDir twice here and ignores the results
	TVector4 vPosition = *GetPosition();
	m_vSeekTarget.y    = m_vSeekStart.y;
	vPosition.y        = m_vSeekStart.y;

	*a_pOutFacing = GetSeekDirection( m_vSeekStart, m_vSeekTarget, vPosition, *GetFacingDir(), a_pOutDistance, &bPassed, ( m_uiFlags & FLAGS_SEEK_SLOWDOWN ) != 0 );

	TVector4 vCurrentPosition = *GetPosition();
	TFLOAT   fDistanceSq      = TVector4::DistanceSq( vCurrentPosition, m_vSeekTarget );

	if ( bPassed != TTRUE && fDistanceSq > m_fSeekOffsetSq )
		return !a_pOutFacing->IsEqual( TVector4::VEC_ZERO );

	// Reached the target
	m_uiFlags &= ~FLAGS_SEEKING;
	return TFALSE;
}

// $Barnyard: FUNCTION 00578840
void ASteerKinematics3D::UpdateObstruction( TFLOAT a_fDeltaTime )
{
	if ( !( m_uiFlags & FLAGS_OBSTRUCTION ) )
		return;

	TFLOAT fDistance   = 1.0f;
	TFLOAT fStepHeight = 0.0f;

	// Sweep a sphere in front of the steer against the terrain
	TVector4 vCenter = *GetPosition();
	vCenter.y -= 0.6f;

	TVector4 vEndPosition;
	TBOOL    bTerrainHit = ATerrainInterface::GetSingleton()->SlideSweptSphere( vCenter, 0.4f, *GetFacingDir(), fDistance, 0.15f, 1.0f, fStepHeight, vEndPosition, -0.13f, 0.4f );

	TFLOAT fPushSpeed = 5.0f;

	if ( !bTerrainHit )
	{
		// ...and against the other collision objects
		ACollisionObject* pCollisionObject = TSTATICCAST( AVisibleUnit, &m_pSteer->GetParentUnit() )->GetCollisionObject();

		if ( !pCollisionObject )
			return;

		const TVector4& rcObjectPosition = pCollisionObject->GetObjectPosition();
		TVector4        vObjectCenter( rcObjectPosition.x, rcObjectPosition.y, rcObjectPosition.z, 1.0f );

		fDistance = 1.1f;

		TVector4 vDirection = *GetFacingDir();
		TVector4 vNormal;

		ACollisionObject* pHitObject = ACollisionManager::GetSingleton()->CollideSweptSphereObjects( pCollisionObject, vNormal, fDistance, vDirection, vObjectCenter, ACollisionObject::TYPE_IGNORED, 0.0f, TNULL, 0 );

		if ( !pHitObject )
			return;

		// Slower against units
		if ( pHitObject->GetType() & ACollisionObject::TYPE_UNIT )
			fPushSpeed = 1.0f;
	}

	TFLOAT fPush = fPushSpeed * a_fDeltaTime;

	if ( fPush > 0.5f )
		fPush = 0.5f;

	// Move back
	TVector4 vPush;
	vPush.Multiply( *GetFacingDir(), fPush );

	m_vPosition.x -= vPush.x;
	m_vPosition.y -= vPush.y;
	m_vPosition.z -= vPush.z;
}

// $Barnyard: FUNCTION 00578cd0
void ASteerKinematics3D::UpdateObstructions( TFLOAT a_fDeltaTime, AKinematicsObstructionLists* a_pObstructions, AKinematicsObstruction** a_ppIgnored, TINT a_iNumIgnored )
{
	if ( !a_pObstructions )
		return;

	for ( TINT i = 0; i < a_pObstructions->iNumLists; i++ )
	{
		AKinematicsObstructionLists::List* pList = a_pObstructions->apLists[ i ];

		for ( TINT k = 0; k < pList->iNumObstructions; k++ )
		{
			AKinematicsObstruction* pObstruction = pList->apObstructions[ k ];

			TBOOL bIgnored = TFALSE;

			for ( TINT j = 0; j < a_iNumIgnored; j++ )
			{
				if ( pObstruction == a_ppIgnored[ j ] )
				{
					bIgnored = TTRUE;
					break;
				}
			}

			if ( !bIgnored && pObstruction &&
			     ( pObstruction->GetShape() & AKinematicsObstruction::SHAPE_CIRCLE ) &&
			     ( pObstruction->GetObstructionType() & AKinematicsObstruction::TYPE_UNIT ) &&
			     pObstruction->GetShape() == AKinematicsObstruction::SHAPE_CIRCLE )
			{
				PushOutOfObstruction( a_fDeltaTime, pObstruction );
			}
		}
	}
}

// $Barnyard: FUNCTION 00578560
void ASteerKinematics3D::PushOutOfObstruction( TFLOAT a_fDeltaTime, AKinematicsObstruction* a_pObstruction )
{
	// Units only block units they overlap with vertically
	if ( a_pObstruction->GetObstructionType() & AKinematicsObstruction::TYPE_UNIT )
	{
		AUnit* pUnit = TSTATICCAST( AKinematicsObstructionUnit, a_pObstruction )->GetUnit();

		if ( pUnit && pUnit->IsA( &TGetClass( AVisibleUnit ) ) )
		{
			ACollisionObject* pCollisionObject = TSTATICCAST( AVisibleUnit, pUnit )->GetCollisionObject();

			if ( !pCollisionObject )
				return;

			TVector4 vUnitPosition = *pUnit->GetSteer()->GetPosition();
			TFLOAT   fOwnHeight    = TSTATICCAST( AVisibleUnit, &m_pSteer->GetParentUnit() )->GetCollisionObject()->GetHeightScale();

			if ( vUnitPosition.y - pCollisionObject->GetHeightScale() > GetPosition()->y )
				return;

			if ( GetPosition()->y - fOwnHeight > vUnitPosition.y )
				return;
		}
	}

	TFLOAT                 fOwnRadius   = m_pTraits->m_fRadius;
	TFLOAT                 fOtherRadius = a_pObstruction->GetRadius();
	const Toshi::TVector4* pOther       = a_pObstruction->GetPosition();

	TVector4 vPosition = *GetPosition();
	TVector2 vAway( vPosition.x - pOther->x, vPosition.z - pOther->z );

	TFLOAT fDistanceSq = vAway.x * vAway.x + vAway.y * vAway.y;
	TFLOAT fRadius     = fOtherRadius + fOwnRadius;

	if ( fRadius * fRadius > fDistanceSq && fDistanceSq > 0.0f )
	{
		TVector4 vVelocity = GetVelocity();
		TVector2 vDirection( vVelocity.x, vVelocity.z );

		TFLOAT fPenetration = fRadius - TMath::Sqrt( fDistanceSq );
		vAway.Normalize();
		vDirection.Normalize();

		// Split the push into the part along the movement and the part to the side
		TFLOAT fCross  = vDirection.x * vAway.y - vDirection.y * vAway.x;
		TFLOAT fAlong  = ( vDirection.x * vAway.x + vDirection.y * vAway.y ) * 0.5f * fPenetration;
		TFLOAT fAlongX = vDirection.x * fAlong;
		TFLOAT fAlongZ = fAlong * vDirection.y;

		// TVector2::Rotate (006ccf50), rotates by -PI/2
		TVector2 vSide = vDirection;
		{
			TFLOAT fSin, fCos;
			TMath::SinCos( -TMath::HALF_PI, fSin, fCos );

			TFLOAT fY = fCos * vSide.y - fSin * vSide.x;
			vSide.x   = fCos * vSide.x + fSin * vSide.y;
			vSide.y   = fY;
		}

		TVector4 vNewPosition = m_vPosition;
		TFLOAT   fSide        = 0.5f * fCross * fPenetration;
		vSide.x *= fSide;
		vSide.y *= fSide;

		if ( a_pObstruction->GetObstructionType() & AKinematicsObstruction::TYPE_UNIT )
		{
			TFLOAT fScale = a_fDeltaTime * 60.0f;

			if ( fScale >= 1.0f )
				fScale = 1.0f;

			vNewPosition.z = fAlongZ * fScale + vNewPosition.z;
			vNewPosition.x = vSide.x * fScale + fAlongX * fScale + vNewPosition.x;
			vNewPosition.z = vSide.y * fScale + vNewPosition.z;

			SetPosition( vNewPosition );
		}
	}
}
