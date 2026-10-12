#include "pch.h"
#include "ASteerKinematics2D.h"
#include "ASteer.h"
#include "Units/AUnit.h"
#include "Units/AVisibleUnit.h"
#include "Units/AKinematicsObstructionUnit.h"
#include "Physics/ACollisionObject.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005748b0, CreateObject 00576560, CreateObjectInPlace 005765c0
TDEFINE_CLASS( ASteerKinematics2D, 5, 0 );

// The movement is integrated with steps of this length
static constexpr TFLOAT TIME_STEP = 1.0f / 30.0f;

// $Barnyard: FUNCTION 00575110
ASteerKinematics2D::ASteerKinematics2D()
    : m_vPosition( TVector4::VEC_ZERO )
    , m_vFacing( TVector4::VEC_POSX )
    , m_fSpeed( 0.0f )
    , m_fMeasuredSpeed( 0.0f )
    , m_fDesiredSpeed( 0.0f )
    , m_fTimeToUpdate( 0.0f )
    , m_uiFlags( FLAGS_NONE )
    , m_vSeekTarget( TVector4::VEC_ZERO )
    , m_vSeekStart( TVector4::VEC_ZERO )
    , m_vSeekDirection( TVector4::VEC_ZERO )
    , m_vSeekFacing( TVector4::VEC_ZERO )
    , m_fTurnSpeed( 0.0f )
    , m_fGroundHeight( 0.0f )
    , m_bReturning( TFALSE )
    , m_fStuckTimer( 0.0f )
    , m_fAvoidTimer( 0.0f )
{
	m_fAvoidRadius  = 0.7f;
	m_fSeekOffsetSq = 1.0f;
	m_fUnkA8        = 1.0f;
	m_fUnkDC        = 1.0f;
	m_uiFlags |= FLAGS_UNK0;
}

// $Barnyard: FUNCTION 005748c0
// $Barnyard: FUNCTION 00575060
ASteerKinematics2D::~ASteerKinematics2D()
{
}

// $Barnyard: FUNCTION 00575490
TBOOL ASteerKinematics2D::Create( const TVector4& a_rcPosition, const TVector4& a_rcDirection, const ASteerTraits* a_pTraits, TFLOAT a_fSpeed )
{
	ASteerKinematics::Create( a_rcPosition, a_rcDirection, a_pTraits, a_fSpeed );

	m_vFacing        = a_rcDirection;
	m_vDesiredFacing = a_rcDirection;
	m_vPosition      = a_rcPosition;
	m_fSpeed         = a_fSpeed;
	m_fDesiredSpeed  = a_fSpeed;
	m_fTimeToUpdate  = 0.0f;
	m_uiFlags        = FLAGS_NONE;
	m_uiFlags        = FLAGS_UNK0;
	m_vSeekTarget    = a_rcPosition;
	m_fMeasuredSpeed = 0.0f;
	m_fGroundHeight  = m_vPosition.y;

	return TTRUE;
}

// $Barnyard: FUNCTION 00575260
TBOOL ASteerKinematics2D::CreateFrom( ASteer* a_pOther )
{
	ASteerKinematics::CreateFrom( a_pOther );

	ASteerKinematics* pOtherKinematics = a_pOther->GetKinematics();

	if ( pOtherKinematics )
	{
		if ( pOtherKinematics->GetClass() == GetClass() )
		{
			ASteerKinematics2D* pOther = TSTATICCAST( ASteerKinematics2D, pOtherKinematics );

			m_vFacing        = pOther->m_vFacing;
			m_vDesiredFacing = pOther->m_vDesiredFacing;
			m_vPosition      = pOther->m_vPosition;
			m_fSpeed         = pOther->m_fSpeed;
			m_fDesiredSpeed  = pOther->m_fDesiredSpeed;
			m_fTimeToUpdate  = pOther->m_fTimeToUpdate;
			m_uiFlags        = pOther->m_uiFlags;
			m_vSeekTarget    = pOther->m_vSeekTarget;
		}
		else
		{
			pOtherKinematics->CopyFacingDir( m_vFacing );
			m_vDesiredFacing = m_vFacing;
			pOtherKinematics->CopyPosition( m_vPosition );
			m_fSpeed        = pOtherKinematics->GetSpeed();
			m_fDesiredSpeed = pOtherKinematics->GetDesiredSpeed();
			m_fTimeToUpdate = 0.0f;
			m_vSeekTarget   = *pOtherKinematics->GetSeekTarget();
			m_uiFlags       = FLAGS_NONE;
			m_uiFlags       = FLAGS_UNK0;

			if ( pOtherKinematics->IsSeekingConstant() )
				m_uiFlags |= FLAGS_SEEKING;
		}
	}
	else
	{
		m_vFacing        = *a_pOther->GetDirection();
		m_vDesiredFacing = *a_pOther->GetDirection();
		m_vPosition      = *a_pOther->GetPosition();
		m_fSpeed         = a_pOther->GetSpeed();
		m_fDesiredSpeed  = a_pOther->GetSpeed();
		m_fTimeToUpdate  = 0.0f;
		m_uiFlags        = FLAGS_NONE;
		m_uiFlags        = FLAGS_UNK0;
		m_vSeekTarget    = m_vPosition;
	}

	m_fMeasuredSpeed = 0.0f;
	m_fSeekOffsetSq  = m_pTraits->m_fGoalToleranceSq;
	m_fGroundHeight  = m_vPosition.y;

	return TTRUE;
}

// $Barnyard: FUNCTION 00575000
TFLOAT ASteerKinematics2D::GetMeasuredSpeedRatio()
{
	return m_fMeasuredSpeed / m_pTraits->m_fMaxSpeed;
}

// $Barnyard: FUNCTION 005748e0
TFLOAT ASteerKinematics2D::GetMeasuredSpeed()
{
	return m_fMeasuredSpeed;
}

// $Barnyard: FUNCTION 00574a50
void ASteerKinematics2D::BlendMeasuredSpeed( TFLOAT a_fSpeed )
{
	m_fMeasuredSpeed = TMath::LERP( m_fMeasuredSpeed, a_fSpeed, 0.9f );
}

// $Barnyard: FUNCTION 00574940
const TVector4* ASteerKinematics2D::GetPosition()
{
	return &m_vPosition;
}

// $Barnyard: FUNCTION 005749b0
TVector4 ASteerKinematics2D::Unk_0x28()
{
	TVector4 vVelocity = m_vFacing;
	vVelocity.Multiply( m_fSpeed );

	return vVelocity;
}

// $Barnyard: FUNCTION 00574950
TVector4 ASteerKinematics2D::GetVelocity()
{
	TVector4 vVelocity = m_vFacing;
	vVelocity.Multiply( m_fSpeed );

	return vVelocity;
}

// $Barnyard: FUNCTION 00574f80
const TVector4* ASteerKinematics2D::GetFacingDir()
{
	return &m_vFacing;
}

// $Barnyard: FUNCTION 00574930
TFLOAT ASteerKinematics2D::GetSpeed()
{
	return m_fSpeed;
}

// Folded with T2GUIElement::GetWidth (005748f0)
TFLOAT ASteerKinematics2D::GetDesiredSpeed()
{
	return m_fDesiredSpeed;
}

// $Barnyard: FUNCTION 00574920
const TVector4* ASteerKinematics2D::GetSeekTarget()
{
	return &m_vSeekTarget;
}

// $Barnyard: FUNCTION 00574c20
TBOOL ASteerKinematics2D::IsFacingAtDesiredFacing( TFLOAT a_fTolerance )
{
	return m_vDesiredFacing.z * m_vFacing.z + m_vDesiredFacing.x * m_vFacing.x > a_fTolerance;
}

// $Barnyard: FUNCTION 00574fd0
void ASteerKinematics2D::SetDesiredFacing( const TVector4& a_rcFacing )
{
	m_vDesiredFacing = a_rcFacing;
}

// NOTE: folded with SetDesiredFacing (00574fd0)
void ASteerKinematics2D::SetDesiredMovementDir( const TVector4& a_rcFacing )
{
	m_vDesiredFacing = a_rcFacing;
}

// Folded with ACollisionObjectMesh::GetObjectSphere (005748d0)
const TVector4* ASteerKinematics2D::GetDesiredFacing()
{
	return &m_vDesiredFacing;
}

// $Barnyard: FUNCTION 00574910
TFLOAT ASteerKinematics2D::GetGroundHeight()
{
	return m_fGroundHeight;
}

// $Barnyard: FUNCTION 005749f0
TVector4& ASteerKinematics2D::PredictPosition( TINT a_iTime, TVector4& a_rOut )
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

// $Barnyard: FUNCTION 00574900
TFLOAT ASteerKinematics2D::GetAvoidRadius()
{
	return m_fAvoidRadius;
}

// $Barnyard: FUNCTION 00574ab0
TBOOL ASteerKinematics2D::GetNextTimeStep( TFLOAT a_fDeltaTime, TFLOAT* a_pOutStep )
{
	if ( m_fTimeToUpdate > TIME_STEP && m_fTimeToUpdate < TIME_STEP * 1.3f )
	{
		// Slightly more than a step, split it in two halves
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

	TBOOL bResult;

	if ( m_fTimeToUpdate >= 0.0001f )
	{
		*a_pOutStep = m_fTimeToUpdate;
		bResult     = TTRUE;
	}
	else
	{
		m_fTimeToUpdate = 0.0f;
		*a_pOutStep     = 0.0f;
		bResult         = TFALSE;
	}

	m_fTimeToUpdate -= *a_pOutStep;
	return bResult;
}

// $Barnyard: FUNCTION 00575550
TFLOAT ASteerKinematics2D::UpdateTurning( TFLOAT a_fDeltaTime, TFLOAT a_fStep )
{
	static TFLOAT s_fTolerance = 1.0f * TMath::PI / 180.0f; // 0x00789678

	TFLOAT fDot          = m_vDesiredFacing.z * m_vFacing.z + m_vDesiredFacing.x * m_vFacing.x;
	TFLOAT fTurningAcc   = m_pTraits->m_fTurningAcc;
	TFLOAT fTurningDec   = m_pTraits->m_fTurningDec;
	TFLOAT fMaxTurnSpeed = m_pTraits->m_fNHolonomicRate;
	TFLOAT fFactor       = 1.0f;
	TFLOAT fAngle        = TMath::ATan2( -( m_vDesiredFacing.z * m_vFacing.x - m_vFacing.z * m_vDesiredFacing.x ), fDot );

	if ( ( m_uiFlags & FLAGS_SEEK_SLOWDOWN ) && ( m_uiFlags & FLAGS_SEEKING ) )
	{
		// Slow down when facing away or getting close to the target
		fFactor = fDot;
		TMath::Clip( fFactor, 0.0f, 1.0f );

		TFLOAT fDistance = TVector4::DistanceXZ( m_vSeekTarget, m_vPosition );

		if ( fDistance < 1.0f )
		{
			TFLOAT fSlowdown = TMath::LERP( -( ( 1.0f - fDistance / 1.0f ) * 3.0f ), 1.0f, fFactor );
			TFLOAT fZero     = 0.0f;
			fFactor          = ( fSlowdown <= 0.0f ) ? fZero : fSlowdown;
		}
	}

	if ( -s_fTolerance <= fAngle )
	{
		if ( fAngle <= s_fTolerance )
		{
			m_fTurnSpeed = 0.0f;
		}
		else
		{
			// Turn speed that still allows stopping at the desired facing
			TFLOAT fStopSpeedSq = fTurningDec * fAngle + fTurningDec * fAngle;
			TFLOAT fLimit       = ( fMaxTurnSpeed * fMaxTurnSpeed >= fStopSpeedSq ) ? TMath::Sqrt( fStopSpeedSq ) : fMaxTurnSpeed;

			if ( fLimit >= m_fTurnSpeed )
			{
				m_fTurnSpeed += ( ( m_fTurnSpeed >= 0.0f ) ? fTurningAcc : fTurningDec ) * a_fStep;

				if ( m_fTurnSpeed > fLimit )
					m_fTurnSpeed = fLimit;
			}
			else
			{
				m_fTurnSpeed -= fTurningDec * a_fStep;

				if ( m_fTurnSpeed < fLimit )
					m_fTurnSpeed = fLimit;
			}
		}
	}
	else
	{
		TFLOAT fStopSpeedSq = fTurningDec * fAngle * -2.0f;
		TFLOAT fLimit       = -( ( fMaxTurnSpeed * fMaxTurnSpeed >= fStopSpeedSq ) ? TMath::Sqrt( fStopSpeedSq ) : fMaxTurnSpeed );

		if ( fLimit <= m_fTurnSpeed )
		{
			m_fTurnSpeed -= ( ( m_fTurnSpeed <= 0.0f ) ? fTurningAcc : fTurningDec ) * a_fStep;

			if ( m_fTurnSpeed < fLimit )
				m_fTurnSpeed = fLimit;
		}
		else
		{
			m_fTurnSpeed += fTurningDec * a_fStep;

			if ( m_fTurnSpeed > fLimit )
				m_fTurnSpeed = fLimit;
		}
	}

	if ( m_fTurnSpeed != 0.0f )
	{
		m_vFacing.AsVector3().RotateY( a_fStep * m_fTurnSpeed );
		m_vFacing.Normalise();
	}

	return fFactor;
}

// $Barnyard: FUNCTION 00576600
void ASteerKinematics2D::UpdateFacing( TFLOAT a_fDeltaTime )
{
	TFLOAT fStep;

	if ( GetNextTimeStep( a_fDeltaTime, &fStep ) )
		UpdateTurning( a_fDeltaTime, fStep );
}

// $Barnyard: FUNCTION 00574b60
void ASteerKinematics2D::MoveTo( TFLOAT a_fDeltaTime, const TVector4* a_pPrevPosition, const TVector4* a_pPosition )
{
	m_vPosition     = *a_pPosition;
	m_fGroundHeight = m_vPosition.y;

	// Smooth out small steps
	if ( TMath::Abs( m_vPosition.y - a_pPrevPosition->y ) <= 0.5f )
		m_vPosition.y = TMath::LERP( m_vPosition.y, a_pPrevPosition->y, 0.4f );

	TVector4 vMovement = m_vPosition;
	vMovement.x -= a_pPrevPosition->x;
	vMovement.y = 0.0f;
	vMovement.z -= a_pPrevPosition->z;

	// NOTE: TVector4::MagnitudeXZ (006c7520) without the assert of TMath::Sqrt on 0, the distance is often 0 here
	m_fMeasuredSpeed = TMath::LERP( m_fMeasuredSpeed, sqrtf( vMovement.MagnitudeSqXZ() ) / a_fDeltaTime, 0.9f );
}

// $Barnyard: FUNCTION 00574c50
TBOOL ASteerKinematics2D::TurnTowards( const TVector4& a_rcTarget )
{
	TVector4 vToTarget( a_rcTarget.x - m_vPosition.x, 0.0f, a_rcTarget.z - m_vPosition.z, a_rcTarget.w );
	TFLOAT   fDistanceSq = vToTarget.MagnitudeSq();

	if ( fDistanceSq < m_fSeekOffsetSq )
		return TTRUE;

	vToTarget.Divide( TMath::Sqrt( fDistanceSq ) );
	m_vDesiredFacing = vToTarget;

	return TFALSE;
}

// $Barnyard: FUNCTION 00574d00
TBOOL ASteerKinematics2D::TurnTowardsWithin( const TVector4& a_rcTarget, TINT a_iUnused, TFLOAT a_fToleranceSq, TFLOAT a_fMinDistanceSq )
{
	TVector4 vToTarget( a_rcTarget.x - m_vPosition.x, 0.0f, a_rcTarget.z - m_vPosition.z, a_rcTarget.w );
	TFLOAT   fDistanceSq = vToTarget.MagnitudeSq();

	if ( fDistanceSq >= a_fMinDistanceSq )
	{
		vToTarget.Divide( TMath::Sqrt( fDistanceSq ) );
		m_vDesiredFacing = vToTarget;
	}

	return fDistanceSq <= a_fToleranceSq;
}

// $Barnyard: FUNCTION 00574a70
TBOOL ASteerKinematics2D::SetPosition( const TVector4& a_rcPosition )
{
	m_vPosition     = a_rcPosition;
	m_fGroundHeight = m_vPosition.y;

	return TTRUE;
}

// $Barnyard: FUNCTION 00574f90
void ASteerKinematics2D::SetFacing( const TVector4& a_rcFacing )
{
	m_vFacing        = a_rcFacing;
	m_vDesiredFacing = a_rcFacing;
}

// $Barnyard: FUNCTION 00574aa0
void ASteerKinematics2D::SetSpeed( TFLOAT a_fSpeed )
{
	m_fSpeed        = a_fSpeed;
	m_fDesiredSpeed = a_fSpeed;
}

// $Barnyard: FUNCTION 005750d0
void ASteerKinematics2D::Unk_0x9c( TBOOL a_bEnable )
{
	if ( a_bEnable )
		m_uiFlags |= FLAGS_UNK0;
	else
		m_uiFlags &= ~FLAGS_UNK0;
}

// $Barnyard: FUNCTION 005750f0
void ASteerKinematics2D::SetFrozen( TBOOL a_bFreeze )
{
	if ( a_bFreeze )
		m_uiFlags |= FLAGS_FROZEN;
	else
		m_uiFlags &= ~FLAGS_FROZEN;
}

// $Barnyard: FUNCTION 00575840
TBOOL ASteerKinematics2D::SeekAlongLine( const TVector4& a_rcStart, const TVector4& a_rcTarget )
{
	TVector4 vLine( a_rcTarget.x - a_rcStart.x, 0.0f, a_rcTarget.z - a_rcStart.z, a_rcTarget.w );

	if ( vLine.MagnitudeSq() <= 1.0e-6f )
		return TFALSE;

	m_vSeekTarget   = a_rcTarget;
	m_vSeekStart    = a_rcStart;
	m_vSeekTarget.y = m_vSeekStart.y;

	m_vSeekDirection.x = m_vSeekTarget.x - m_vSeekStart.x;
	m_vSeekDirection.y = m_vSeekTarget.y - m_vSeekStart.y;
	m_vSeekDirection.z = m_vSeekTarget.z - m_vSeekStart.z;
	m_vSeekDirection.w = m_vSeekTarget.w;
	m_vSeekDirection.Normalise();

	m_uiFlags = ( m_uiFlags & ~( FLAGS_SEEKING | FLAGS_SEEK_FACING ) ) | FLAGS_SEEKING;
	return TTRUE;
}

// $Barnyard: FUNCTION 00574dd0
TBOOL ASteerKinematics2D::Seek( const TVector4& a_rcTarget )
{
	TVector4 vStart = m_vPosition;
	return SeekAlongLine( vStart, a_rcTarget );
}

// $Barnyard: FUNCTION 005757e0
TBOOL ASteerKinematics2D::SeekFacing( const TVector4& a_rcTarget, const TVector4& a_rcFacing, TFLOAT a_fUnk )
{
	m_vSeekFacing = a_rcFacing;
	m_vSeekTarget = a_rcTarget;

	TBOOL bResult = Seek( a_rcTarget );
	m_uiFlags |= FLAGS_SEEK_FACING;

	return bResult;
}

// $Barnyard: FUNCTION 00575960
void ASteerKinematics2D::Halt()
{
	m_uiFlags &= ~FLAGS_SEEKING;
	m_uiFlags &= ~FLAGS_SEEK_FACING;
}

// $Barnyard: FUNCTION 00575970
TBOOL ASteerKinematics2D::IsSeekingConstant()
{
	return ( m_uiFlags & ( FLAGS_SEEKING | FLAGS_SEEK_FACING ) ) != 0;
}

// $Barnyard: FUNCTION 005750a0
TBOOL ASteerKinematics2D::HasMoved()
{
	return ( m_uiFlags & FLAGS_MOVED ) != 0;
}

// $Barnyard: FUNCTION 00574ff0
void ASteerKinematics2D::ResetSeekOffset()
{
	m_fSeekOffsetSq = m_pTraits->m_fGoalToleranceSq;
}

// $Barnyard: FUNCTION 005750b0
void ASteerKinematics2D::SetSeekSlowdown( TBOOL a_bEnable )
{
	if ( a_bEnable )
		m_uiFlags |= FLAGS_SEEK_SLOWDOWN;
	else
		m_uiFlags &= ~FLAGS_SEEK_SLOWDOWN;
}

// $Barnyard: FUNCTION 00575010
void ASteerKinematics2D::SaveStuckPosition()
{
	m_vStuckPosition = *GetPosition();
	m_vStuckFacing   = *GetFacingDir();
}

// $Barnyard: FUNCTION 00575080
TBOOL ASteerKinematics2D::IsAvoiding()
{
	return ( m_uiFlags & ( FLAGS_BLOCKED | FLAGS_AVOIDING | FLAGS_AVOIDED ) ) != 0;
}

// $Barnyard: FUNCTION 00576500
void ASteerKinematics2D::SetReturnEnabled( TBOOL a_bEnabled )
{
	if ( !a_bEnabled )
	{
		m_uiFlags |= FLAGS_NO_RETURN;
		m_bReturning  = TFALSE;
		m_fStuckTimer = 0.0f;
	}
	else
	{
		m_uiFlags &= ~FLAGS_NO_RETURN;
	}
}

// $Barnyard: FUNCTION 00576530
TBOOL ASteerKinematics2D::ResetAvoiding()
{
	m_uiFlags &= ~FLAGS_BLOCKED;
	m_uiFlags &= ~FLAGS_AVOIDING;
	m_uiFlags &= ~FLAGS_AVOIDED;
	m_fAvoidTimer      = 0.0f;
	m_fAvoidCheckTimer = 0.0f;

	return TFALSE;
}

// $Barnyard: FUNCTION 00575980
TBOOL ASteerKinematics2D::GetSeekFacing( TFLOAT a_fDeltaTime, TFLOAT* a_pOutDistance, TVector4* a_pOutFacing )
{
	TBOOL bPassed = TFALSE;

	GetFacingDir();
	GetFacingDir();

	// Follow the line on the height of its start
	TVector4 vPosition = *GetPosition();
	m_vSeekTarget.y    = m_vSeekStart.y;
	vPosition.y        = m_vSeekStart.y;

	*a_pOutFacing = GetSeekDirection( m_vSeekStart, m_vSeekTarget, vPosition, *GetFacingDir(), a_pOutDistance, &bPassed, ( m_uiFlags & FLAGS_SEEK_SLOWDOWN ) != 0 );

	const TVector4* pPosition = GetPosition();

	if ( !bPassed )
	{
		TFLOAT fZ = m_vSeekTarget.z - pPosition->z;
		TFLOAT fX = m_vSeekTarget.x - pPosition->x;

		if ( fZ * fZ + fX * fX > m_fSeekOffsetSq )
			return !a_pOutFacing->IsEqual( TVector4::VEC_ZERO );
	}

	m_uiFlags &= ~FLAGS_SEEKING;
	return TFALSE;
}

// $Barnyard: FUNCTION 00576630
TBOOL ASteerKinematics2D::UpdateSeek( TFLOAT a_fDeltaTime, AKinematicsObstructionLists* a_pObstructions, AKinematicsObstruction** a_ppIgnored, TINT a_iNumIgnored, UPDATEFLAGS a_uiFlags )
{
	if ( GetSpeed() > TMath::TFLOAT_EPSILON && ( m_uiFlags & FLAGS_SEEKING ) )
	{
		TFLOAT   fDistance;
		TVector4 vFacing = TVector4::VEC_POSX;

		if ( GetSeekFacing( a_fDeltaTime, &fDistance, &vFacing ) )
		{
			if ( !( a_uiFlags & UPDATEFLAGS_NO_AVOID ) )
			{
				AvoidObstructions( a_fDeltaTime, &vFacing, a_pObstructions, a_ppIgnored, a_iNumIgnored );

				if ( ( m_uiFlags & FLAGS_CHECK_BLOCKED ) &&
				     IsBlocked( a_fDeltaTime, &vFacing, a_pObstructions, a_ppIgnored, a_iNumIgnored ) &&
				     !( m_uiFlags & ( FLAGS_BLOCKED | FLAGS_AVOIDED ) ) &&
				     !m_pSteer->GetParentUnit().HasInteraction() )
				{
					m_uiFlags |= FLAGS_BLOCKED;
					m_fAvoidTimer = 3.0f;
				}
			}

			SetDesiredFacing( vFacing );
		}
		else if ( !( m_uiFlags & FLAGS_SEEK_FACING ) )
		{
			// Keep turning towards the direction of the line
			TVector4 vDesiredFacing = *GetDesiredFacing();
			vDesiredFacing.Lerp4( vDesiredFacing, m_vSeekDirection, 0.9f * a_fDeltaTime );
			vDesiredFacing.Normalise();

			SetDesiredFacing( vDesiredFacing );
		}
	}

	if ( !( m_uiFlags & FLAGS_SEEKING ) && ( m_uiFlags & FLAGS_SEEK_FACING ) )
	{
		// Walk straight to the target
		SetSpeed( 0.0f );

		TVector4 vToTarget;
		vToTarget.Substract( m_vSeekTarget, m_vPosition );

		TVector2 vToTargetXZ( vToTarget.x, vToTarget.z );
		TFLOAT   fDistance = vToTargetXZ.Magnitude();
		TFLOAT   fStep     = 1.0f * a_fDeltaTime;

		if ( fDistance <= fStep )
		{
			SetPosition( m_vSeekTarget );
			SetDesiredFacing( m_vSeekFacing );
			return TTRUE;
		}

		TFLOAT fOneOverDistance = 1.0f / fDistance;
		vToTargetXZ.x *= fOneOverDistance;
		vToTargetXZ.y *= fOneOverDistance;

		TVector4 vFacing( vToTargetXZ.x, 0.0f, vToTargetXZ.y, 1.0f );
		TVector4 vStep( vToTargetXZ.x * fStep, 0.0f, vToTargetXZ.y * fStep, 1.0f );

		TVector4 vNewPosition;
		vNewPosition.Add( m_vPosition, vStep );

		SetPosition( vNewPosition );
		SetDesiredFacing( vFacing );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00575a80
TBOOL ASteerKinematics2D::AvoidObstructions( TFLOAT a_fDeltaTime, TVector4* a_pFacing, AKinematicsObstructionLists* a_pObstructions, AKinematicsObstruction** a_ppIgnored, TINT a_iNumIgnored )
{
	if ( !a_pObstructions )
		return TFALSE;

	TVector4 vPush      = TVector4::VEC_ZERO;
	TINT     iNumPushes = 0;

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

			if ( bIgnored || !pObstruction || !( pObstruction->GetObstructionType() & ( AKinematicsObstruction::TYPE_INTERACTION | AKinematicsObstruction::TYPE_UNIT ) ) )
				continue;

			const TVector4* pObstructionPos = pObstruction->GetPosition();
			TVector4        vPosition       = *GetPosition();
			TVector4        vFacing         = *a_pFacing;

			TVector4 vToObstruction( pObstructionPos->x - vPosition.x, pObstructionPos->y - vPosition.y, pObstructionPos->z - vPosition.z, pObstructionPos->w );
			TFLOAT   fDistanceSq = TVector4::DotProduct3( vToObstruction, vToObstruction );

			if ( fDistanceSq > 0.0f && fDistanceSq < 49.0f )
			{
				vToObstruction.Divide( TMath::Sqrt( fDistanceSq ) );

				// Only the ones in front
				if ( TVector4::DotProduct3( vToObstruction, vFacing ) > 0.0f )
				{
					TFLOAT fWeight = 1.0f;

					if ( pObstruction->GetSpeed() < 0.1f )
						fWeight = 2.0f;

					if ( pObstruction->GetObstructionType() & AKinematicsObstruction::TYPE_INTERACTION )
					{
						TVector4 vNormalised( vToObstruction.x, vToObstruction.y, vToObstruction.z, vToObstruction.w );
						fWeight += fWeight;
						vNormalised.Normalise();
					}

					TVector4 vAway;
					vAway.Multiply( vToObstruction, -( ( 25.0f - fDistanceSq ) * 0.04f ) * fWeight );

					iNumPushes++;
					vPush.x += vAway.x;
					vPush.y += vAway.y;
					vPush.z += vAway.z;
				}
			}
		}
	}

	if ( iNumPushes > 0 )
	{
		m_fAvoidRadius = ( m_vAvoidPush.x * m_vAvoidPush.x + m_vAvoidPush.y * m_vAvoidPush.y >= 0.7f ) ? 1.2f : 0.7f;

		TFLOAT fPushSq = vPush.MagnitudeSq();

		if ( fPushSq > 0.0f )
			vPush.Divide( TMath::Sqrt( fPushSq ) );

		vPush.Multiply( 0.5f );
		vPush.x += a_pFacing->x;
		vPush.y = 0.0f;
		vPush.z += a_pFacing->z;

		TFLOAT fFacingSq = vPush.MagnitudeSq();

		if ( fFacingSq > 0.0f )
		{
			*a_pFacing = vPush;
			a_pFacing->Divide( TMath::Sqrt( fFacingSq ) );
		}
	}

	m_vAvoidPush = TVector2::VEC_ZERO;
	return TFALSE;
}

// $Barnyard: FUNCTION 00575ea0
TBOOL ASteerKinematics2D::IsBlocked( TFLOAT a_fDeltaTime, TVector4* a_pFacing, AKinematicsObstructionLists* a_pObstructions, AKinematicsObstruction** a_ppIgnored, TINT a_iNumIgnored )
{
	if ( !a_pObstructions )
		return TFALSE;

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

			if ( bIgnored || !pObstruction || !( pObstruction->GetObstructionType() & AKinematicsObstruction::TYPE_UNIT ) )
				continue;

			AUnit* pUnit        = TSTATICCAST( AKinematicsObstructionUnit, pObstruction )->GetUnit();
			TBOOL  bInteracting = pUnit ? pUnit->HasInteraction() : TFALSE;

			const TVector4* pObstructionPos = pObstruction->GetPosition();
			TVector4        vPosition       = *GetPosition();
			TVector4        vFacing         = *a_pFacing;

			TVector4 vToObstruction( pObstructionPos->x - vPosition.x, pObstructionPos->y - vPosition.y, pObstructionPos->z - vPosition.z, pObstructionPos->w );
			TFLOAT   fDistanceSq = TVector4::DotProduct3( vToObstruction, vToObstruction );

			if ( fDistanceSq > 0.0f && fDistanceSq < 6.25f )
			{
				vToObstruction.Divide( TMath::Sqrt( fDistanceSq ) );

				if ( TVector4::DotProduct3( vToObstruction, vFacing ) > 0.8f )
				{
					TVector4 vDirection = pObstruction->GetDirection();
					TFLOAT   fDot       = TVector4::DotProduct3( vFacing, vDirection );

					// Standing and talking or coming towards us
					if ( pObstruction->GetSpeed() < 0.01f && bInteracting == TTRUE )
						return TTRUE;

					if ( fDot > 0.2f )
						return TTRUE;
				}
			}
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00576890
void ASteerKinematics2D::UpdateObstructions( TFLOAT a_fDeltaTime, AKinematicsObstructionLists* a_pObstructions, AKinematicsObstruction** a_ppIgnored, TINT a_iNumIgnored )
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

			if ( !bIgnored && pObstruction && ( pObstruction->GetShape() & ( AKinematicsObstruction::SHAPE_CAPSULE | AKinematicsObstruction::SHAPE_CIRCLE ) ) && ( pObstruction->GetObstructionType() & ( AKinematicsObstruction::TYPE_INTERACTION | AKinematicsObstruction::TYPE_UNK1 | AKinematicsObstruction::TYPE_UNIT ) ) )
			{
				AKinematicsObstruction::SHAPE iType = pObstruction->GetShape();

				if ( iType == AKinematicsObstruction::SHAPE_CIRCLE )
					PushOutOfUnit( a_fDeltaTime, pObstruction );
				else if ( iType == AKinematicsObstruction::SHAPE_CAPSULE )
					PushOutOfBox( a_fDeltaTime, pObstruction );
			}
		}
	}

	// Pushed around while trying to move, wait for a bit
	if ( ( m_uiFlags & FLAGS_MOVED ) && m_fMeasuredSpeed < 0.3f && !( m_uiFlags & ( FLAGS_BLOCKED | FLAGS_AVOIDED ) ) && !m_pSteer->GetParentUnit().HasInteraction() )
	{
		m_uiFlags |= FLAGS_BLOCKED;
		m_fAvoidTimer = 3.0f;
	}
}

// $Barnyard: FUNCTION 005760f0
TBOOL ASteerKinematics2D::PushOutOfUnit( TFLOAT a_fDeltaTime, AKinematicsObstruction* a_pObstruction )
{
	TBOOL bInteracting = TFALSE;

	if ( a_pObstruction->GetObstructionType() & AKinematicsObstruction::TYPE_UNIT )
	{
		AUnit* pUnit = TSTATICCAST( AKinematicsObstructionUnit, a_pObstruction )->GetUnit();

		if ( pUnit )
		{
			bInteracting = pUnit->HasInteraction();

			// Units only block units they overlap with vertically
			if ( pUnit->IsA( &TGetClass( AVisibleUnit ) ) )
			{
				TVector4          vUnitPosition    = *pUnit->GetSteer()->GetPosition();
				ACollisionObject* pCollisionObject = TSTATICCAST( AVisibleUnit, pUnit )->GetCollisionObject();

				if ( pCollisionObject )
				{
					TFLOAT fOwnHeight = TSTATICCAST( AVisibleUnit, &m_pSteer->GetParentUnit() )->GetCollisionObject()->GetHeightScale();

					if ( vUnitPosition.y - pCollisionObject->GetHeightScale() > GetPosition()->y ||
					     GetPosition()->y - fOwnHeight > vUnitPosition.y )
					{
						return TFALSE;
					}
				}
			}
		}
	}

	// While talking, only the ones that are talking too push us
	TBOOL bPushable = TTRUE;

	if ( m_pSteer->GetParentUnit().HasInteraction() )
		bPushable = bInteracting;

	AUnit* pPlayer = ( a_pObstruction->GetObstructionType() & AKinematicsObstruction::TYPE_UNIT ) ? TSTATICCAST( AKinematicsObstructionUnit, a_pObstruction )->GetUnit() : TNULL;

	if ( pPlayer && pPlayer->IsPlayer() )
	{
		// The player only pushes us while standing still and we're moving
		ASteer* pPlayerSteer = pPlayer->GetSteer();

		if ( !pPlayerSteer || pPlayerSteer->GetSpeed() >= 0.01f || GetSpeed() <= 0.01f )
			return TFALSE;
	}
	else if ( !bPushable )
	{
		return TFALSE;
	}

	TFLOAT fOwnRadius   = m_pTraits->m_fRadius * 1.02f;
	TFLOAT fOtherRadius = a_pObstruction->GetRadius() * 1.02f;

	const TVector4* pOther    = a_pObstruction->GetPosition();
	TVector4        vPosition = *GetPosition();

	TVector2 vAway( vPosition.x - pOther->x, vPosition.z - pOther->z );
	TFLOAT   fDistanceSq = vAway.x * vAway.x + vAway.y * vAway.y;

	TFLOAT fMinGap = 0.09f;
	TFLOAT fGap    = ( fOtherRadius - fOwnRadius + 0.1f ) * 1.25f;
	TFLOAT fRadius = fOtherRadius + ( ( fGap <= 0.09f ) ? fMinGap : fGap ) * 2.0f + fOwnRadius;

	if ( fRadius * fRadius <= fDistanceSq || fDistanceSq == 0.0f )
		return TFALSE;

	TVector4 vVelocity = GetVelocity();
	TVector2 vDirection( vVelocity.x, vVelocity.z );

	TFLOAT fPenetration = fRadius - TMath::Sqrt( fDistanceSq );
	vAway.Normalize();
	vDirection.Normalize();

	// Split the push into the part along the movement and the part to the side
	TFLOAT fCross  = vDirection.x * vAway.y - vDirection.y * vAway.x;
	TFLOAT fAlong  = ( vDirection.y * vAway.y + vDirection.x * vAway.x ) * fPenetration * 0.1f;
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

	// Always push to a side
	if ( TMath::Abs( fCross ) < 0.5f )
	{
		if ( fCross >= 0.1f )
			fCross = 1.5f - fCross;
		else
			fCross = -1.5f - fCross;
	}

	TFLOAT fSide = fCross * fPenetration * 0.1f;
	vSide.x *= fSide;
	vSide.y *= fSide;

	TVector4 vNewPosition = m_vPosition;
	TFLOAT   fAlongScale;
	TFLOAT   fSideScale;

	if ( bInteracting == TTRUE || ( fPenetration > 0.1f && ( a_pObstruction->GetObstructionType() & ( AKinematicsObstruction::TYPE_UNK1 | AKinematicsObstruction::TYPE_UNIT ) ) ) )
	{
		fAlongScale = a_fDeltaTime * 30.0f;
		fSideScale  = fAlongScale;
	}
	else if ( !( a_pObstruction->GetObstructionType() & AKinematicsObstruction::TYPE_INTERACTION ) )
	{
		fAlongScale = a_fDeltaTime * 0.5f;
		fSideScale  = fAlongScale;
	}
	else
	{
		fAlongScale = ( 1.0f - fPenetration / fRadius ) * a_fDeltaTime * 3.0f;
		fSideScale  = fAlongScale + fAlongScale;
	}

	vNewPosition.z = fAlongZ * fAlongScale + vNewPosition.z;
	vNewPosition.x = vSide.x * fSideScale + fAlongX * fAlongScale + vNewPosition.x;
	vNewPosition.z = vSide.y * fSideScale + vNewPosition.z;

	SetPosition( vNewPosition );
	return TTRUE;
}

// $Barnyard: FUNCTION 00574e10
TBOOL ASteerKinematics2D::PushOutOfBox( TFLOAT a_fDeltaTime, AKinematicsObstruction* a_pObstruction )
{
	const TVector4* pPosition = GetPosition();
	TVector2        vPositionXZ( pPosition->x, pPosition->z );
	TFLOAT          fPenetration;

	if ( !a_pObstruction->CollideCircle( 0.0f, vPositionXZ, m_pTraits->m_fRadius, &fPenetration ) )
		return TFALSE;

	TVector4        vPosition = *GetPosition();
	const TVector4* pOther    = a_pObstruction->GetPosition();

	TVector4 vAway( vPosition.x - pOther->x, vPosition.y - pOther->y, vPosition.z - pOther->z, vPosition.w );
	vAway.Normalise();

	TVector4 vPush;
	vPush.Multiply( vAway, fPenetration );

	TVector4 vNewPosition( vPush.x + vPosition.x, vPush.y + vPosition.y, vPush.z + vPosition.z, vPosition.w );
	SetPosition( vNewPosition );

	return TTRUE;
}

// $Barnyard: FUNCTION 005769e0
void ASteerKinematics2D::Update( TFLOAT a_fDeltaTime, AKinematicsObstructionLists* a_pObstructions, AKinematicsObstruction** a_ppIgnored, TINT a_iNumIgnored, UPDATEFLAGS a_uiFlags )
{
	TVector4 vPrevPosition = m_vPosition;
	TVector4 vPrevFacing   = m_vFacing;

	if ( m_uiFlags & FLAGS_FROZEN )
		return;

	if ( !( a_uiFlags & UPDATEFLAGS_NO_OBSTRUCTION_STOP ) )
	{
		if ( m_uiFlags & FLAGS_BLOCKED )
		{
			// Wait, then start avoiding
			m_fAvoidTimer -= a_fDeltaTime;

			if ( m_fAvoidTimer < 0.0f )
			{
				m_uiFlags &= ~FLAGS_BLOCKED;
				m_uiFlags |= FLAGS_AVOIDING;
				m_uiFlags |= FLAGS_AVOIDED;
				m_fAvoidTimer      = ( m_uiFlags & FLAGS_CHECK_BLOCKED ) ? 3.0f : 5.0f;
				m_fAvoidCheckTimer = 0.0f;
			}

			m_fSpeed = 0.0f;
			return;
		}

		if ( m_uiFlags & FLAGS_AVOIDING )
		{
			m_fAvoidTimer -= a_fDeltaTime;

			TVector4 vFacing = *GetFacingDir();
			m_fAvoidCheckTimer += a_fDeltaTime;

			TBOOL bStop;

			if ( m_fAvoidCheckTimer > 0.3f )
			{
				m_fAvoidCheckTimer -= 0.3f;
				bStop = !IsBlocked( a_fDeltaTime, &vFacing, a_pObstructions, a_ppIgnored, a_iNumIgnored );
			}
			else
			{
				bStop = TFALSE;
			}

			if ( bStop || m_fAvoidTimer < 0.0f || m_pSteer->GetParentUnit().HasInteraction() )
			{
				m_uiFlags &= ~FLAGS_AVOIDING;
				m_fAvoidTimer = 3.0f;
			}

			m_fSpeed = 0.0f;
			return;
		}

		if ( m_uiFlags & FLAGS_AVOIDED )
		{
			m_fAvoidTimer -= a_fDeltaTime;

			if ( m_fAvoidTimer < 0.0f )
				m_uiFlags &= ~FLAGS_AVOIDED;
		}
	}

	if ( m_uiFlags & ( FLAGS_SEEKING | FLAGS_SEEK_FACING ) )
		UpdateSeek( a_fDeltaTime, a_pObstructions, a_ppIgnored, a_iNumIgnored, a_uiFlags );

	m_fTimeToUpdate += a_fDeltaTime;
	const ASteerTraits* pTraits = m_pTraits;

	TFLOAT fStep;

	if ( GetNextTimeStep( a_fDeltaTime, &fStep ) )
	{
		TFLOAT fZero = 0.0f;

		do
		{
			TFLOAT fTargetSpeed = UpdateTurning( a_fDeltaTime, fStep ) * m_fDesiredSpeed;
			fTargetSpeed        = ( fZero < fTargetSpeed ) ? fTargetSpeed : fZero;

			if ( fTargetSpeed > m_fSpeed )
			{
				m_fSpeed += fStep * pTraits->m_fAcc;

				if ( m_fSpeed > fTargetSpeed )
					m_fSpeed = fTargetSpeed;
			}
			else
			{
				m_fSpeed -= fStep * pTraits->m_fDec;

				if ( m_fSpeed < fTargetSpeed )
					m_fSpeed = fTargetSpeed;
			}

			TFLOAT fDistance = fStep * m_fSpeed;
			m_vPosition.x    = fDistance * m_vFacing.x + m_vPosition.x;
			m_vPosition.z    = fDistance * m_vFacing.z + m_vPosition.z;
		} while ( GetNextTimeStep( a_fDeltaTime, &fStep ) );
	}

	if ( ( m_uiFlags & FLAGS_SEEK_FACING ) && !( m_uiFlags & FLAGS_SEEKING ) )
	{
		TFLOAT fX = m_vSeekTarget.x - m_vPosition.x;
		TFLOAT fZ = m_vSeekTarget.z - m_vPosition.z;

		if ( IsFacingAtDesiredFacing( 0.99f ) && fZ * fZ + fX * fX < 0.0001f )
		{
			if ( m_vSeekFacing.z * m_vFacing.z + m_vSeekFacing.x * m_vFacing.x >= 0.98f )
			{
				// Arrived
				m_uiFlags &= ~FLAGS_SEEK_FACING;
				SetPosition( m_vSeekTarget );
				SetFacing( m_vSeekFacing );
				SetSpeed( 0.0f );
			}
			else
			{
				SetDesiredFacing( m_vSeekFacing );
			}
		}
		else
		{
			SetSpeed( TVector4::DistanceXZ( m_vPosition, vPrevPosition ) / a_fDeltaTime );
		}
	}

	TFLOAT   fMovedSq = ( m_vPosition.z - vPrevPosition.z ) * ( m_vPosition.z - vPrevPosition.z ) + ( m_vPosition.x - vPrevPosition.x ) * ( m_vPosition.x - vPrevPosition.x );
	TVector4 vMovement( m_vPosition.x - vPrevPosition.x, m_vPosition.y - vPrevPosition.y, m_vPosition.z - vPrevPosition.z, m_vPosition.w );
	TFLOAT   fMovementSq  = vMovement.MagnitudeSqXZ();
	TFLOAT   fStuckDistSq = 0.0f;

	if ( !( m_uiFlags & ( FLAGS_SEEKING | FLAGS_SEEK_FACING ) ) && 0.0f == GetSpeed() && 0.0f == fMovedSq )
	{
		if ( !( m_uiFlags & FLAGS_NO_RETURN ) && m_fStuckTimer > 0.0f )
		{
			// Got pushed away while standing, walk back
			TVector4 vToStuck;
			vToStuck.Substract( m_vStuckPosition, vPrevPosition );
			TFLOAT fDistanceSq = vToStuck.MagnitudeSqXZ();

			if ( fDistanceSq > 0.49f || m_bReturning )
			{
				vToStuck.Substract( m_vStuckPosition, vPrevPosition );
				GetFacingDir();
				vToStuck.y = 0.0f;

				// NOTE: TMath::Sqrt asserts on 0, retail gets 0 when the unit is back where it got stuck
				TFLOAT fDistance = ( vToStuck.MagnitudeSq() != 0.0f ) ? vToStuck.Magnitude() : 0.0f;

				if ( fDistance > 0.0f )
				{
					vToStuck.Divide( fDistance );
					fStuckDistSq = fDistanceSq;

					if ( fDistance > 1.0f )
						fDistance = 1.0f;

					TVector4 vStep = vToStuck;
					vStep.Multiply( a_fDeltaTime );
					vStep.Multiply( 2.0f );
					vStep.Multiply( fDistance );

					TVector4 vNewPosition;
					vNewPosition.Add( vStep, vPrevPosition );
					SetPosition( vNewPosition );
				}

				fMovedSq     = TVector4::DistanceSqXZ( vPrevPosition, m_vPosition );
				m_bReturning = TTRUE;
			}
		}
	}
	else
	{
		m_fStuckTimer = 0.0f;
	}

	if ( !( a_uiFlags & UPDATEFLAGS_NO_PUSH ) )
	{
		TFLOAT fX = m_vPosition.x - m_vSeekTarget.x;
		TFLOAT fZ = m_vPosition.z - m_vSeekTarget.z;

		if ( !( m_uiFlags & FLAGS_SEEK_FACING ) || fZ * fZ + fX * fX >= 0.0625f )
			UpdateObstructions( a_fDeltaTime, a_pObstructions, a_ppIgnored, a_iNumIgnored );
	}

	TFLOAT fX = m_vPosition.x - vPrevPosition.x;
	TFLOAT fZ = m_vPosition.z - vPrevPosition.z;

	if ( TMath::Abs( fZ * fZ + fX * fX - fMovedSq ) >= TMath::TFLOAT_EPSILON )
	{
		// Something pushed us
		m_uiFlags |= FLAGS_MOVED;

		if ( 0.0f == GetSpeed() && 0.0f == fMovementSq )
		{
			if ( m_fStuckTimer <= 0.0f )
			{
				m_vStuckPosition = vPrevPosition;
				m_vStuckFacing   = vPrevFacing;
			}

			m_fStuckTimer = 10.0f;
		}

		return;
	}

	m_uiFlags &= ~FLAGS_MOVED;

	if ( ( m_uiFlags & FLAGS_NO_RETURN ) || !m_bReturning )
		return;

	if ( fStuckDistSq < 0.4f )
	{
		// Back where it stood
		SetDesiredFacing( m_vStuckFacing );
		m_bReturning = TFALSE;
		return;
	}

	TVector4 vToStuck;
	vToStuck.Substract( m_vStuckPosition, vPrevPosition );
	vToStuck.y = 0.0f;
	vToStuck.Normalise();

	if ( fStuckDistSq > 0.6f )
	{
		TVector4 vBlend;
		vBlend.Substract( m_vStuckFacing, vToStuck );
		vBlend.Multiply( a_fDeltaTime );

		TVector4 vFacing;
		vFacing.Add( vBlend, vToStuck );
		vFacing.Normalise();

		SetDesiredFacing( vFacing );
	}
	else
	{
		TVector4 vBlend;
		vBlend.Substract( vToStuck, m_vStuckFacing );
		vBlend.Multiply( a_fDeltaTime );

		TVector4 vFacing;
		vFacing.Add( vBlend, m_vStuckFacing );
		vFacing.Normalise();

		SetDesiredFacing( m_vStuckFacing );
	}
}
