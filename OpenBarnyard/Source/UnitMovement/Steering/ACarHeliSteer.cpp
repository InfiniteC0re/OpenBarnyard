#include "pch.h"
#include "ACarHeliSteer.h"
#include "FSM/AFunctionTable.h"
#include "Units/AUnit.h"
#include "Units/AAnimatedUnit.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AModel.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0055c6e0, CreateObject 0055ca70, CreateObjectInPlace 0055cad0
TDEFINE_CLASS( ACarHeliSteer, 3, 0 );

// Has no functions of its own (dynamic initialiser 0070a260)
AFunctionTable ACarHeliSteer::ms_oFunctionTable( &TGetClass( ACarHeliSteer ), &ASteer::ms_oFunctionTable );

static TFLOAT s_fAcceleration = 30.0f; // 0x00774284

static constexpr TFLOAT s_fDegToRad = 0.017453292f;

// $Barnyard: FUNCTION 0055c6f0
ACarHeliSteer::ACarHeliSteer()
{
	m_fHeight    = 7.0f;
	m_fRadiusSq  = 100.0f;
	m_pTarget    = TNULL;
	m_bFollowing = TFALSE;
	m_vPosition  = TVector4::VEC_ZERO;
	m_vVelocity  = TVector4::VEC_ZERO;
	m_vDirection = TVector4::VEC_POSZ;
	m_vUp        = TVector4::VEC_NEGY;
	m_fSpeed     = 10.0f;
	m_fMaxSpeed  = 70.0f;
	m_fMinSpeed  = 2.0f;
	m_fBobHeight = 0.5f;
	m_fBobAngle  = 0.0f;
	m_vRight.CrossProduct( m_vDirection, m_vUp );
	m_oTransform = TMatrix44::IDENTITY;
}

// $Barnyard: FUNCTION 0055cb10
// $Barnyard: FUNCTION 0055c830
ACarHeliSteer::~ACarHeliSteer()
{
}

// $Barnyard: FUNCTION 0055c820
AFunctionTable* ACarHeliSteer::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 0055c8c0
TBOOL ACarHeliSteer::CreateFrom( ASteer* a_pOther, TINT a_iLOD )
{
	ASteer::CreateFrom( a_pOther, a_iLOD );

	a_pOther->CopyPosition( m_vPosition );
	a_pOther->CopyDirection( m_vDirection );
	a_pOther->CopyVelocity( m_vVelocity );

	return TTRUE;
}

// $Barnyard: FUNCTION 0055c840
TBOOL ACarHeliSteer::Create( AUnit* a_pOwner, const TVector4& a_rcPosition, const TVector4& a_rcDirection, TINT a_iLOD )
{
	ASteer::Create( a_pOwner, a_rcPosition, a_rcDirection, a_iLOD );

	m_vPosition  = a_rcPosition;
	m_vDirection = a_rcDirection;
	m_vVelocity  = TVector4::VEC_ZERO;

	return TTRUE;
}

// $Barnyard: FUNCTION 0055cb30
TBOOL ACarHeliSteer::Update( TFLOAT a_fDeltaTime )
{
	static TFLOAT s_fTurnSpeed       = TMath::PI;               // 0x00788bb0
	static TFLOAT s_fFacingTurnSpeed = TMath::PI * 0.16666667f; // 0x00788bac

	if ( m_bFollowing )
	{
		TVector4 vecMove;
		vecMove.Multiply( m_vVelocity, a_fDeltaTime );

		m_vPosition.x += vecMove.x;
		m_vPosition.y += vecMove.y;
		m_vPosition.z += vecMove.z;

		TVector4 vecTarget = *m_pTarget->GetSteer()->GetPosition();

		// Bob up and down above the target
		m_fBobAngle = a_fDeltaTime * 90.0f + m_fBobAngle;
		if ( m_fBobAngle > 360.0f )
			m_fBobAngle = 0.0f;

		m_vPosition.y = TMath::Sin( m_fBobAngle * s_fDegToRad ) * m_fBobHeight + ( vecTarget.y - m_fHeight );

		TFLOAT fDistanceSq = TVector4::DistanceSq( m_vPosition, vecTarget );

		if ( fDistanceSq > m_fRadiusSq )
		{
			// Speed up when far away, slow down when getting close
			if ( m_fRadiusSq + m_fRadiusSq < fDistanceSq )
				m_fSpeed = s_fAcceleration * a_fDeltaTime + m_fSpeed;
			else
				m_fSpeed = m_fSpeed - s_fAcceleration * a_fDeltaTime * 5.0f;

			TMath::Clip( m_fSpeed, m_fMinSpeed, m_fMaxSpeed );
			TMath::Clip( m_fSpeed, 0.0f, m_fMaxSpeed );
			SetSpeed( m_fSpeed );

			// Turn towards the target
			TVector4 vecToTarget;
			vecToTarget.Substract( vecTarget, m_vPosition );
			vecToTarget.y = 0.0f;
			vecToTarget.Normalise();

			TVector4 vecDirection = m_vVelocity;
			vecDirection.y        = 0.0f;
			vecDirection.Normalise();

			TVector4 vecAxis;
			vecAxis.CrossProduct( vecToTarget, vecDirection );
			vecAxis.Normalise();

			if ( TVector4::DotProduct3( vecToTarget, vecDirection ) < 0.5f )
			{
				TFLOAT fAngle = vecToTarget.GetAngleBetweenVector( vecDirection );
				TFLOAT fTurn  = s_fTurnSpeed * a_fDeltaTime;

				if ( m_fRadiusSq + m_fRadiusSq < fDistanceSq )
					fTurn = fTurn + fTurn;

				TMath::Clip( fTurn, 0.0f, fAngle );

				if ( vecAxis.y > 0.7f )
					m_vVelocity.AsVector3().RotateY( -fTurn );
				else
					m_vVelocity.AsVector3().RotateY( fTurn );
			}
		}
		else
		{
			m_fSpeed = m_fSpeed - s_fAcceleration * a_fDeltaTime * 5.0f;
			TMath::Clip( m_fSpeed, m_fMinSpeed, m_fMaxSpeed );
			SetSpeed( m_fSpeed );
		}

		// Turn the facing towards the velocity
		TVector4 vecAxis;
		vecAxis.CrossProduct( m_vVelocity, m_vDirection );
		vecAxis.Normalise();

		TFLOAT fAngle = m_vVelocity.GetAngleBetweenVector( m_vDirection );

		if ( fAngle > TMath::HALF_PI * 0.25f )
		{
			TFLOAT fTurn = s_fFacingTurnSpeed * a_fDeltaTime;
			TMath::Clip( fTurn, 0.0f, fAngle );

			if ( vecAxis.y > 0.7f )
				m_vDirection.AsVector3().RotateY( -fTurn );
			else
				m_vDirection.AsVector3().RotateY( fTurn );

			m_vDirection.Normalise();
		}
	}

	// Bank with the speed
	TVector4 vecVelocity = m_vVelocity;
	TFLOAT   fSpeed      = vecVelocity.NormaliseMag();

	TVector4 vecSide;
	vecSide.CrossProduct( vecVelocity, TVector4::VEC_NEGY );

	TFLOAT fBank = TMath::Sin( TMath::HALF_PI / m_fMaxSpeed * fSpeed );
	fBank        = fBank + fBank;
	TMath::Clip( fBank, 0.0f, 0.76f );

	vecVelocity.y = fBank;

	m_vUp.CrossProduct( vecSide, vecVelocity );
	m_vUp.Normalise();
	m_vRight.CrossProduct( m_vDirection, m_vUp );
	m_vRight.Normalise();
	m_vDirection.CrossProduct( m_vUp, m_vRight );
	m_vDirection.Normalise();

	m_oTransform                                       = TMatrix44::IDENTITY;
	m_oTransform.AsBasisVector3( BASISVECTOR_RIGHT )   = m_vRight.AsVector3();
	m_oTransform.AsBasisVector3( BASISVECTOR_UP )      = m_vDirection.AsVector3();
	m_oTransform.AsBasisVector3( BASISVECTOR_FORWARD ) = m_vUp.AsVector3();
	m_oTransform.GetTranslation()                      = m_vPosition;

	TSTATICCAST( AAnimatedUnit, &GetParentUnit() )->GetAnimatableObject()->GetModelInstance()->SetTransform( m_oTransform );

	return TTRUE;
}

// $Barnyard: FUNCTION 0055c950
TBOOL ACarHeliSteer::SetPosition( const TVector4& a_rcPosition )
{
	m_vPosition = a_rcPosition;
	return TTRUE;
}

// NOTE: same code as ACarSteer::SetPosition3, folded in retail (0055c910)
TBOOL ACarHeliSteer::SetPosition3( const TVector3& a_rcPosition )
{
	TVector4 vPosition( a_rcPosition.x, a_rcPosition.y, a_rcPosition.z, 1.0f );
	SetPosition( vPosition );

	return TTRUE;
}

// $Barnyard: FUNCTION 0055ca50
void ACarHeliSteer::SetDirection( const TVector4& a_rcDirection )
{
	m_vDirection = a_rcDirection;
}

// NOTE: same code as AProjectileItemSteer::CopyDirection, folded in retail (0056e1a0)
void ACarHeliSteer::CopyPosition( TVector4& a_rOutPosition )
{
	a_rOutPosition = m_vPosition;
}

// NOTE: same code as AProjectileItemSteer::CopyDirection3, folded in retail (0055c980)
void ACarHeliSteer::CopyPosition3( TVector3& a_rOutPosition )
{
	a_rOutPosition = m_vPosition.AsVector3();
}

// NOTE: same code as AProjectileItemSteer::GetDirection, folded in retail (0056e190)
const TVector4* ACarHeliSteer::GetPosition()
{
	return &m_vPosition;
}

// NOTE: same code as AKidBikeSteer's getter, folded in retail (00566ae0)
void ACarHeliSteer::CopyDirection( TVector4& a_rOutDirection )
{
	a_rOutDirection = m_vDirection;
}

// $Barnyard: FUNCTION 0055c9a0
void ACarHeliSteer::CopyDirection3( TVector3& a_rOutDirection )
{
	a_rOutDirection = m_vDirection.AsVector3();
}

// NOTE: same code as AKidBikeSteer's getter, folded in retail (00566b00)
const TVector4* ACarHeliSteer::GetDirection()
{
	return &m_vDirection;
}

// $Barnyard: FUNCTION 0055ca30
void ACarHeliSteer::CopyVelocity( TVector4& a_rOutVelocity )
{
	a_rOutVelocity = m_vVelocity;
}

// $Barnyard: FUNCTION 0055ca00
TVector4 ACarHeliSteer::GetVelocity()
{
	return m_vVelocity;
}

// $Barnyard: FUNCTION 0055c9c0
TFLOAT ACarHeliSteer::GetSpeed()
{
	return m_vVelocity.AsVector3().Magnitude();
}

// $Barnyard: FUNCTION 0055c9d0
void ACarHeliSteer::SetSpeed( TFLOAT a_fSpeed )
{
	m_fSpeed = a_fSpeed;
	m_vVelocity.Normalise();
	m_vVelocity.Multiply( m_fSpeed );
}

// NOTE: same code as ASteerKinematics3D::SetDesiredFacing, folded in retail (00577770)
void ACarHeliSteer::SetVelocity( const TVector4& a_rcVelocity )
{
	m_vVelocity = a_rcVelocity;
}
