#include "pch.h"
#include "AHudTrackBoneAniEffect.h"
#include "Units/AAnimatedUnit.h"
#include "Animation/AAnimatableObject.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/Steering/AStationarySteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00630790, CreateObject 00630900, CreateObjectInPlace 00630960
TDEFINE_CLASS( AHudTrackBoneAniEffect );

// $Barnyard: FUNCTION 006307a0
AHudTrackBoneAniEffect::AHudTrackBoneAniEffect()
    : m_bFirstUpdate( TTRUE )
    , m_iTargetBone( -1 )
    , m_iUnitBone( -1 )
    , m_fSpeed( 5.0f )
    , m_fMaxSpeed( 1.0f )
    , m_fAcceleration( 10.0f )
    , m_fUnk24( 1.0f )
    , m_fMinDistance( 0.2f )
    , m_fFadeDistance( 0.0f )
    , m_fTargetScale( -1.0f )
    , m_fStartScale( -1.0f )
    , m_fStartDistance( 1.0f )
    , m_qUnk3C( TQuaternion::IDENTITY )
{
}

// $Barnyard: FUNCTION 006309a0
// $Barnyard: FUNCTION 00630820
AHudTrackBoneAniEffect::~AHudTrackBoneAniEffect()
{
	m_oUnit   = TNULL;
	m_oTarget = TNULL;
}

// NOTE: folded with ATrackBoneAniEffect::GetUnit (00632e20)
AUnit* AHudTrackBoneAniEffect::GetUnit()
{
	return m_oUnit->GetUnit();
}

// $Barnyard: FUNCTION 00630ba0
TBOOL AHudTrackBoneAniEffect::Update( TFLOAT a_fDeltaTime )
{
	if ( !m_oUnit || !m_oUnit->GetUnit() )
	{
		Finish();
		return TFALSE;
	}

	AAnimatedUnit* pUnit   = TSTATICCAST( AAnimatedUnit, m_oUnit ? m_oUnit->GetUnit() : TNULL );
	AAnimatedUnit* pTarget = TSTATICCAST( AAnimatedUnit, m_oTarget ? m_oTarget->GetUnit() : TNULL );

	if ( pUnit && pTarget )
	{
		// Where to go
		TVector4 vTargetPos;

		if ( m_iTargetBone == -1 )
		{
			vTargetPos = *pTarget->GetSteer()->GetPosition();
		}
		else
		{
			TMatrix44 mBoneTransform;
			pTarget->GetModelInstance()->GetBoneTransform( m_iTargetBone, mBoneTransform );
			vTargetPos = mBoneTransform.AsBasisVector4( BASISVECTOR_TRANSLATION );
		}

		// Where the unit (or its bone) is
		TVector4 vPos;
		TVector4 vBoneOffset = TVector4::VEC_ZERO;

		if ( m_iUnitBone == -1 )
		{
			vPos = *pUnit->GetSteer()->GetPosition();
		}
		else
		{
			TMatrix44 mBoneTransform;
			pUnit->GetAnimatableObject()->GetModelInstance()->GetBoneTransform( m_iUnitBone, mBoneTransform );
			vPos = mBoneTransform.AsBasisVector4( BASISVECTOR_TRANSLATION );

			vBoneOffset.Substract( *pUnit->GetSteer()->GetPosition(), vPos );
		}

		TVector4 vDirection;
		vDirection.Substract( vTargetPos, vPos );
		TFLOAT fDistance = vDirection.NormaliseMag();

		if ( m_bFirstUpdate )
		{
			pUnit->ChangeSteer( &TGetClass( AStationarySteer ) );
			pUnit->GetAnimatableObject()->SetAnimControllersDisabled( TFALSE );
			pUnit->GetAnimatableObject()->SetVisible( TTRUE );
			pUnit->GetAnimatableObject()->SetSkeletonUpdating( TTRUE, TTRUE );
			m_bFirstUpdate   = TFALSE;
			m_fStartDistance = fDistance;
		}

		m_fSpeed += a_fDeltaTime * m_fAcceleration;
		TMath::Clip( m_fSpeed, 0.0f, m_fMaxSpeed );

		TVector4 vNewPos;

		if ( a_fDeltaTime * m_fSpeed <= fDistance )
		{
			TVector4 vMove;
			vMove.Multiply( vDirection, m_fSpeed );
			vMove.Multiply( a_fDeltaTime );
			vNewPos.Add( vPos, vMove );
		}
		else
		{
			vNewPos = vTargetPos;
		}

		TFLOAT fDistanceLeft = ( vNewPos - vTargetPos ).Magnitude();

		vNewPos.x += vBoneOffset.x;
		vNewPos.y += vBoneOffset.y;
		vNewPos.z += vBoneOffset.z;
		pUnit->GetSteer()->SetPosition( vNewPos );

		if ( fDistanceLeft < m_fMinDistance )
		{
			pUnit->GetAnimatableObject()->SetSkeletonUpdating( TFALSE, TTRUE );
			Finish();
			return TFALSE;
		}

		if ( m_fTargetScale != -1.0f )
		{
			TFLOAT fProgress = ( m_fStartDistance - fDistanceLeft ) / m_fStartDistance;

			if ( fProgress >= 0.0f )
			{
				TFLOAT fScale = fProgress * ( m_fTargetScale - m_fStartScale ) + m_fStartScale;

				if ( fScale > 0.0f )
					pUnit->GetAnimatableObject()->GetModelInstance()->SetScale( TVector4( fScale, fScale, fScale, 1.0f ) );
			}
		}

		if ( m_fFadeDistance > 0.0f )
		{
			TFLOAT fFade = fDistanceLeft - m_fMinDistance;

			if ( fFade < m_fFadeDistance )
			{
				fFade = TMath::Max( fFade, 0.0f );
				pUnit->GetModelInstance()->SetOpacity( fFade / m_fFadeDistance );
			}
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00631020
void AHudTrackBoneAniEffect::Setup( AUnit* a_pUnit, AUnit* a_pTarget, const TPString8& a_rcBone, TFLOAT a_fMaxSpeed, TFLOAT a_fFadeDistance )
{
	m_fFadeDistance = a_fFadeDistance;
	SetUnit( a_pUnit );
	SetTarget( a_pTarget );
	SetTargetBone( a_rcBone );
	m_fMaxSpeed = a_fMaxSpeed;
}

// $Barnyard: FUNCTION 00630a40
void AHudTrackBoneAniEffect::SetUnit( AUnit* a_pUnit )
{
	m_oUnit       = a_pUnit->GetSelfProxy();
	m_fStartScale = TSTATICCAST( AAnimatedUnit, a_pUnit )->GetAnimatableObject()->GetModelInstance()->GetScale().x;
}

// $Barnyard: FUNCTION 00630b10
void AHudTrackBoneAniEffect::SetTarget( AUnit* a_pTarget )
{
	m_oTarget = a_pTarget->GetSelfProxy();
}

// $Barnyard: FUNCTION 006309c0
void AHudTrackBoneAniEffect::SetTargetBone( const TPString8& a_rcBone )
{
	AAnimatedUnit* pTarget = TSTATICCAST( AAnimatedUnit, m_oTarget ? m_oTarget->GetUnit() : TNULL );
	m_iTargetBone          = pTarget->GetAnimatableObject()->GetModelInstance()->GetBoneIndex( a_rcBone );
}
