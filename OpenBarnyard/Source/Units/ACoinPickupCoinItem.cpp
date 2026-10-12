#include "pch.h"
#include "ACoinPickupCoinItem.h"
#include "AUnitManager.h"
#include "AUnitTransform.h"
#include "Animation/AAnimatableObject.h"
#include "FX/AAniEffectManager.h"
#include "FX/ATrackBoneAniEffect.h"
#include "Physics/ACollisionObject.h"
#include "Render/AShadowManager.h"
#include "UnitMovement/Steering/ASteer.h"

#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: CreateObject 00534d90, CreateObjectInPlace 00534df0
TDEFINE_CLASS( ACoinPickupCoinItem );

TPSTRING8_DECLARE( c_spine );

// 0x00773f58, pulses per second of a flashing coin
static TFLOAT s_fFlashFrequency = 2.0f;

// 0x00773f5c, opacity a collected coin loses per second
static TFLOAT s_fFadeSpeed = 0.2f;

// 0x00773f60, radians per second the coin spins
static TFLOAT s_fSpinSpeed = 3.0f;

// $Barnyard: FUNCTION 00534d00
ACoinPickupCoinItem::ACoinPickupCoinItem()
    : m_pTarget( TNULL )
    , m_bCollected( TFALSE )
    , m_bCollectDone( TFALSE )
    , m_bPendingCollect( TTRUE )
    , m_fOpacity( 1.0f )
    , m_iValue( 0 )
    , m_bFlashing( TFALSE )
    , m_bUseTrackEffect( TFALSE )
    , m_fTime( 0.0f )
    , m_pTrackEffect( TNULL )
    , m_vStartPosition( TVector4::VEC_ZERO )
    , m_iUnkC4( -1 )
{
}

// NOTE: the deleting destructor (00534e30) is folded with APrecisionTargetItem's
ACoinPickupCoinItem::~ACoinPickupCoinItem()
{
}

// $Barnyard: FUNCTION 005350b0
TBOOL ACoinPickupCoinItem::Create( AUnitTransform* a_pTransform )
{
	if ( !AAnimatedUnit::Create( a_pTransform ) )
		return TFALSE;

	AddFlags( FLAGS_AI_ENABLED );
	m_fTime = 0.0f;

	AShadowManager::GetSingleton()->AddUnitShadow( this, 0.5f, -1, -0.05f, AShadowManager::FLAGS_UNIFORM_SCALE, AShadowManager::TYPE_DEFAULT );

	if ( a_pTransform->m_iModelIndex > 2 )
	{
		AAniEffectManager* pAniEffectManager = AAniEffectManager::GetSingleton();

		if ( pAniEffectManager && pAniEffectManager->ReserveEffects( 1 ) )
		{
			ATrackBoneAniEffect* pEffect = pAniEffectManager->CreateEffect<ATrackBoneAniEffect>();

			if ( pEffect )
			{
				m_pTrackEffect = pEffect;
				m_pTrackEffect->SetUnit( this );
				m_pTrackEffect->SetFadeDistance( 0.6f );
				m_pTrackEffect->SetMinDistance( 0.5f );
				m_pTrackEffect->SetActive( TFALSE );
				m_pTrackEffect->SetMaxSpeed( 40.0f );
				m_pTrackEffect->SetAcceleration( 20.0f );
			}
		}

		if ( m_pCollisionObject )
			m_pCollisionObject->SetType( ACollisionObject::TYPE_IGNORED ); // NOTE: replaces the type with the flag the cameras ignore
	}

	m_bCollected      = TFALSE;
	m_bCollectDone    = TFALSE;
	m_bPendingCollect = TTRUE;

	return TTRUE;
}

// $Barnyard: FUNCTION 00534e50
void ACoinPickupCoinItem::OnDestroy()
{
	if ( m_pTrackEffect )
	{
		AAniEffectManager::GetSingleton()->UnregisterEffect( m_pTrackEffect );
		AAniEffectManager::GetSingleton()->KillEffect( m_pTrackEffect );
		m_pTrackEffect = TNULL;
	}

	AAnimatedUnit::OnDestroy();
}

// $Barnyard: FUNCTION 005351b0
void ACoinPickupCoinItem::UpdateAI( TFLOAT a_fDeltaTime )
{
	m_fTime += a_fDeltaTime;

	if ( IsVisible() == TTRUE )
	{
		TVector4 vPosition  = *GetSteer()->GetPosition();
		TVector4 vDirection = *GetSteer()->GetDirection();

		// Spin
		if ( !m_bCollectDone )
		{
			vDirection.AsVector3().RotateY( s_fSpinSpeed * a_fDeltaTime );
			GetSteer()->SetDirection( vDirection );
		}

		if ( m_bCollected == TTRUE && !m_bUseTrackEffect )
		{
			// Flies to the target and fades out
			TVector4 vVelocity = TVector4::VEC_NEGY;

			if ( m_pTarget )
			{
				vVelocity.Substract( *m_pTarget->GetSteer()->GetPosition(), *GetSteer()->GetPosition() );
				vVelocity.y = -0.2f;
				vVelocity.Normalise();
			}

			TVector4 vScaled;
			vScaled.Multiply( vVelocity, 4.0f );
			GetSteer()->SetVelocity( vScaled );

			if ( m_fOpacity <= 0.0f )
			{
				SetVisible( TFALSE );
			}
			else
			{
				GetAnimatableObject()->SetOpacity( m_fOpacity, TTRUE );
				m_fOpacity -= s_fFadeSpeed * a_fDeltaTime;
			}
		}

		if ( m_bFlashing == TTRUE )
		{
			TFLOAT fAngle = TMath::PI * s_fFlashFrequency * m_fTime;
			TMath::NormaliseAngle( fAngle );

			GetAnimatableObject()->SetOpacity( ( TMath::Cos( fAngle ) + 1.0f ) * 0.5f, TTRUE );
		}

		GetAnimatableObject()->GetModelInstance()->SetTransform( vPosition, vDirection );
	}

	// The track effect is over
	if ( m_bCollected == TTRUE && !m_bCollectDone )
	{
		if ( m_pTrackEffect && !m_pTrackEffect->IsActive() )
			m_bCollectDone = TTRUE;
	}

	AUnit::UpdateAI( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00534e90
void ACoinPickupCoinItem::SetCollected( TBOOL a_bCollected )
{
	m_bCollected   = a_bCollected;
	m_fOpacity     = 1.0f;
	m_bCollectDone = TFALSE;

	if ( m_bUseTrackEffect == TTRUE )
	{
		if ( m_pTrackEffect )
		{
			m_pTrackEffect->SetTarget( m_pTarget );
			m_pTrackEffect->SetTargetBone( TPS8( c_spine ) );
			m_pTrackEffect->SetActive( TTRUE );
		}
	}
	else if ( a_bCollected )
	{
		TVector4 vVelocity = TVector4::VEC_NEGY;

		if ( m_pTarget )
		{
			vVelocity.Substract( *m_pTarget->GetSteer()->GetPosition(), *GetSteer()->GetPosition() );
			vVelocity.y = -0.2f;
			vVelocity.Normalise();
		}

		TVector4 vScaled;
		vScaled.Multiply( vVelocity, 4.0f );
		GetSteer()->SetVelocity( vScaled );
	}
	else
	{
		// Back at its position
		SetVisible( TTRUE );
		GetSteer()->SetPosition( m_vStartPosition );
		GetAnimatableObject()->SetOpacity( m_fOpacity, TTRUE );
	}
}

// $Barnyard: FUNCTION 00535020
void ACoinPickupCoinItem::SetTarget( AUnit* a_pTarget )
{
	m_pTarget = a_pTarget;

	if ( m_pTrackEffect )
		m_pTrackEffect->SetTarget( a_pTarget );
}

// $Barnyard: FUNCTION 00535010
void ACoinPickupCoinItem::Destroy()
{
	AUnitManager::GetSingleton()->DestroyUnit( this );
}
