#include "pch.h"
#include "ACowTippingMiniGameState.h"
#include "ARandom.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AAnimatableObjectType.h"
#include "Animation/AToshiAnimationInterface.h"
#include "Animation/AModel.h"
#include "Units/AAnimatedUnit.h"
#include "Units/ACharacterMovementComponent.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "FX/AParticleEffectManager.h"
#include "FX/AParticleSystem.h"
#include "Sound/ASoundManager.h"

#include <Render/TCameraObject.h>
#include <Render/TRenderInterface.h>
#include <Render/TRenderContext.h>
#include <Math/TSphere.h>
#include <Toshi/TString8.h>
#include <Toshi/TStringManager.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>

#include "Terrain/ATerrainInterface.h"
#include "SoundBank/cowtip.h"
#include "SoundBank/dagboss.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( yuk );
TPSTRING8_DECLARE( midyuk );
TPSTRING8_DECLARE( endyuk );
TPSTRING8_DECLARE( yukobject );
TPSTRING8_DECLARE( splat );
TPSTRING8_DECLARE( howl );
TPSTRING8_DECLARE( stop_lp );
TPSTRING8_DECLARE( run_lp );
TPSTRING8_DECLARE( run );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( walk_lp );
TPSTRING8_DECLARE( push_end );
TPSTRING8_DECLARE( scared );
TPSTRING8_DECLARE( stolen_coyote );
TPSTRING8_DECLARE( out_start );
TPSTRING8_DECLARE( distress_idle );
TPSTRING8_DECLARE( distress_pleased );
TPSTRING8_DECLARE( distress_pleased2 );

// NOTE: ACowTippingMiniGameState is split over several .cpp files, this one has the [Units] part.

// NOTE: several provisional fields are declared as TINT but hold floats or vectors (or are
// read as such), these helpers reinterpret them until the header types are settled.
template <class T>
static TFLOAT& AsFloat( T& a_rValue ) { return *TREINTERPRETCAST( TFLOAT*, &a_rValue ); }

template <class T>
static TVector4& AsVector4( T& a_rValue ) { return *TREINTERPRETCAST( TVector4*, &a_rValue ); }

static TINT   AsInt( TFLOAT a_fValue ) { return *TREINTERPRETCAST( TINT*, &a_fValue ); }
static TFLOAT BoolAsFloat( TBOOL a_bValue )
{
	TUINT32 uiValue = a_bValue;
	return *TREINTERPRETCAST( TFLOAT*, &uiValue );
}

// Projectile byte 0x49 is set when it hits something
static TUINT8& ProjectileHitFlag( ACowTippingMiniGameState::Projectile* a_pProjectile ) { return TREINTERPRETCAST( TUINT8*, &a_pProjectile->bRecalcVelocity )[ 1 ]; }

static T2SList<ACowTippingMiniGameState::Enemy>&     GetUnits13120( ACowTippingMiniGameState* a_pState ) { return *TREINTERPRETCAST( T2SList<ACowTippingMiniGameState::Enemy>*, &a_pState->m_llActiveEnemies ); }
static T2SList<ACowTippingMiniGameState::PopUpUnit>& GetUnits13268( ACowTippingMiniGameState* a_pState ) { return *TREINTERPRETCAST( T2SList<ACowTippingMiniGameState::PopUpUnit>*, &a_pState->m_llActiveRaccoons ); }
static T2SList<ACowTippingMiniGameState::PopUpUnit>& GetUnits13270( ACowTippingMiniGameState* a_pState ) { return *TREINTERPRETCAST( T2SList<ACowTippingMiniGameState::PopUpUnit>*, &a_pState->m_llActiveGophers ); }

static ACowTippingMiniGameState::Crate* GetCrateState( ACowTippingMiniGameState* a_pState, TINT a_iGroup, TINT a_iIndex )
{
	return &a_pState->m_aCrates[ a_iGroup ][ a_iIndex ];
}

static ACowTippingMiniGameState::Coin* GetCoinState( ACowTippingMiniGameState* a_pState, TINT a_iIndex )
{
	return &a_pState->m_aCoins[ a_iIndex ];
}

static TBOOL IsModelUpdatingSkeleton( AAnimatableObject* a_pObject )
{
	AModelInstance* pModel = a_pObject->GetModelInstance().Get();
	return pModel != TNULL && pModel->IsUpdatingSkeleton();
}

// $Barnyard: FUNCTION 004a6e60
TBOOL ACowTippingMiniGameState::IsUnitNotYukking( TINT a_iUnk1 )
{
	Enemy* pUnit = TREINTERPRETCAST( Enemy*, a_iUnk1 );

	if ( pUnit->pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( yuk ), 0.0f ) ) return TFALSE;
	if ( pUnit->pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( yuk ) ) ) return TFALSE;
	if ( pUnit->pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( midyuk ), 0.0f ) ) return TFALSE;
	if ( pUnit->pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( endyuk ) ) ) return TFALSE;
	if ( pUnit->pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( splat ), 0.0f ) ) return TFALSE;
	if ( pUnit->pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( splat ) ) ) return TFALSE;

	return TTRUE;
}

// $Barnyard: FUNCTION 004a71b0
void ACowTippingMiniGameState::TurnUnitTowardsPlayer( TINT a_iUnk1 )
{
	AAnimatedUnit* pUnit  = TREINTERPRETCAST( AAnimatedUnit*, a_iUnk1 );
	ASteer*        pSteer = pUnit->GetSteer();

	TVector4 vecDirection = *pSteer->GetDirection();
	TVector4 vecPosition  = *pSteer->GetPosition();

	// Turn towards 0x116c if not facing away from it
	TVector4 vecToTarget;
	vecToTarget.x = m_vCameraBasePos.x - vecPosition.x;
	vecToTarget.y = 0.0f;
	vecToTarget.z = m_vCameraBasePos.z - vecPosition.z;
	vecToTarget.w = AsFloat( m_vCameraBasePos.w );
	vecToTarget.Normalise();

	if ( TVector4::DotProduct3( vecDirection, vecToTarget ) > 0.2f )
		pSteer->SetDirection( vecToTarget );
}

// $Barnyard: FUNCTION 004a72a0
void ACowTippingMiniGameState::OnHitCollectable( Projectile* a_pProjectile, Coin* a_pUnk )
{
	ProjectileHitFlag( a_pProjectile ) = TTRUE;
	a_pUnk->iState                     = Coin::COINSTATE_COLLECTED;
	StartCollectEffect( a_pUnk->pObject, 1, 10.0f );
}

// $Barnyard: FUNCTION 004a72d0
void ACowTippingMiniGameState::SetObjectsLeft( TINT a_iUnk1 )
{
	m_iObjectsLeft = a_iUnk1;

	TFLOAT fProgress = TFLOAT( a_iUnk1 + 45 ) / TFLOAT( m_iNumObjects + 45 );
	TMath::Clip( fProgress, 0.0f, 1.0f );

	// NOTE: 0xf404 is inside m_oButtonSlamHud (+0x48c), AButtonSlamHudElement doesn't have this member yet
	*TREINTERPRETCAST( TFLOAT*, TREINTERPRETCAST( TUINT8*, &m_oButtonSlamHud ) + 0x48c ) = fProgress;
}

// $Barnyard: FUNCTION 004a73e0
TINT ACowTippingMiniGameState::ShowMessagePopup( const TWCHAR* a_wszText )
{
	if ( m_eTurnEnd != TURNEND_NONE )
		return m_eTurnEnd;

	m_oEndPopup.m_fDuration     = m_fEndPopupDuration;
	m_oEndPopup.m_fMoveDuration = m_fEndPopupMoveDuration;
	m_oEndPopup.m_eMode         = AGUI2AnythingPopup::MODE_FADE;
	m_oEndPopup.m_fDelay        = 0.0f;
	m_oEndPopup.m_fStartScale   = 1.4f;
	m_oEndPopup.m_fScale        = 1.5f;
	m_oEndPopup.m_fPulseSpeed   = 0.5f;
	m_oEndPopup.SetText( a_wszText, 0xffffffff, &m_GUIElement );

	return 0;
}

// $Barnyard: FUNCTION 004a7450
TINT ACowTippingMiniGameState::ShowEndPopup( void* a_pUnk1 )
{
	if ( m_eTurnEnd != TURNEND_NONE )
		return m_eTurnEnd;

	m_oTurnEndPopup.m_fDuration     = m_fTurnEndDelay;
	m_oTurnEndPopup.m_fMoveDuration = m_fTurnEndDelay * 0.2f;
	m_oTurnEndPopup.m_eMode         = AGUI2AnythingPopup::MODE_FADE;
	m_oTurnEndPopup.m_fDelay        = 0.0f;
	m_oTurnEndPopup.m_fStartScale   = 1.05f;
	m_oTurnEndPopup.m_fScale        = 1.5f;
	m_oTurnEndPopup.m_fPulseSpeed   = 0.5f;
	m_oTurnEndPopup.SetText( TSTATICCAST( const TWCHAR, a_pUnk1 ), 0xffffffff, &m_GUIElement );

	return 0;
}

// $Barnyard: FUNCTION 004a7900
TINT ACowTippingMiniGameState::AddLives1( TINT a_iUnk1, TINT a_iUnk2 )
{
	if ( m_eTurnEnd != TURNEND_NONE )
		return m_eTurnEnd;

	PlayerData* pPlayer   = TREINTERPRETCAST( PlayerData*, a_iUnk1 );
	TINT        iMaxLives = m_aStages[ m_iStage ].iMaxLives;
	TINT        iChange   = a_iUnk2;

	pPlayer->iLives += a_iUnk2;

	if ( pPlayer->iLives < 0 )
	{
		pPlayer->iLives = 0;

		// NOTE: retail adds the maximum here, not the change
		m_oLives1Hud.AddLives( iMaxLives );
		return 0;
	}

	if ( pPlayer->iLives > iMaxLives )
	{
		iChange         = iMaxLives - pPlayer->iLives;
		pPlayer->iLives = iMaxLives;
	}

	m_oLives1Hud.AddLives( iChange );
	return 0;
}

// $Barnyard: FUNCTION 004a7990
TINT ACowTippingMiniGameState::AddLifeAt( TINT a_iUnk1, TINT a_iUnk2 )
{
	if ( m_eTurnEnd != TURNEND_NONE )
		return m_eTurnEnd;

	PlayerData* pPlayer = TREINTERPRETCAST( PlayerData*, a_iUnk1 );
	pPlayer->iLives += 1;

	if ( pPlayer->iLives < 0 )
	{
		pPlayer->iLives = 0;
	}
	else if ( pPlayer->iLives > m_aStages[ m_iStage ].iMaxLives )
	{
		pPlayer->iLives = m_aStages[ m_iStage ].iMaxLives;
	}

	m_oLives1Hud.AddLife( *TREINTERPRETCAST( const TVector4*, a_iUnk2 ) );
	return 0;
}

// $Barnyard: FUNCTION 004a7bc0
TINT ACowTippingMiniGameState::GetUnitStartBit( void* a_pThis, TINT a_iUnk1 )
{
	Enemy* pUnit = TSTATICCAST( Enemy, a_pThis );

	if ( a_iUnk1 == Stage::STAGETYPE_JUNKYARD_BOSS || a_iUnk1 == Stage::STAGETYPE_JUNKYARD )
		return pUnit->iCow;

	return pUnit->iCowSide + 2 * pUnit->iCow;
}

// $Barnyard: FUNCTION 004a7cd0
TBOOL ACowTippingMiniGameState::IsPowerUpActive( TINT a_iUnk1 )
{
	// Timers at 0x0, 0x8, 0x10 of the player data
	return m_aPlayers[ m_iCurrentPlayer ].aPowerUps[ a_iUnk1 ].fTime > 0.0f;
}

// $Barnyard: FUNCTION 004a8d60
TINT ACowTippingMiniGameState::GetUnitHeldObject( TINT a_iUnk1 )
{
	PopUpUnit* pUnit      = TREINTERPRETCAST( PopUpUnit*, a_iUnk1 );
	TINT       iStageType = m_aStages[ m_iStage ].iStageType;

	if ( iStageType == Stage::STAGETYPE_JUNKYARD_BOSS || iStageType == Stage::STAGETYPE_JUNKYARD )
		return TREINTERPRETCAST( TINT, pUnit->pUnit->GetAnimatableObject() );

	return TREINTERPRETCAST( TINT, pUnit->pUnit->GetAnimatableObject()->FindAttachedObject( m_iRaccoonHandBone ) );
}

// $Barnyard: FUNCTION 004a91d0
TBOOL ACowTippingMiniGameState::MakeUnitRunAway( TINT a_iUnk1, TFLOAT a_fUnk2, TFLOAT a_fUnk3, TFLOAT a_fUnk4 )
{
	Enemy* pUnit  = TREINTERPRETCAST( Enemy*, a_iUnk1 );
	TINT   iState = pUnit->iRunning;

	if ( iState == 1 )
		return TTRUE;

	TRandom& rRandom = ARandom::GetSingleton()->m_oRandom;

	if ( pUnit->eType == Enemy::ENEMYTYPE_COYOTE )
	{
		AAnimatedUnit* pAnimatedUnit = pUnit->pUnit;

		if ( pAnimatedUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( howl ) ) ||
		     pUnit->pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( howl ), 0.0f ) )
		{
			return pUnit->pUnit->GetAnimatableObject()->PlayAnimation( TPS8( stop_lp ) );
		}

		pUnit->iRunning = 1;

		AInteriorSteer*              pSteer    = TSTATICCAST( AInteriorSteer, pAnimatedUnit->GetSteer() );
		ACharacterMovementComponent* pMovement = TSTATICCAST( ACharacterMovementComponent, pAnimatedUnit->FindComponent( &TGetClass( ACharacterMovementComponent ) ) );

		pMovement->SetRunningSpeed( ACharacterMovementComponent::APPLY_IF_MOVING, ( rRandom.GetFloatMinMax( 0.0f, m_fRunSpeedRandom ) + 1.0f ) * m_fCoyoteRunSpeed + a_fUnk4 + 1.0f );
		pSteer->SetMode( AInteriorSteer::MODE_NORMAL );

		pUnit->fWaitTime = rRandom.GetFloatMinMax( m_fCoyoteRunAwayTimeMin, m_fCoyoteRunAwayTimeMax );
		pUnit->fUnk24    = rRandom.GetFloatMinMax( m_fCoyoteRunAwayTimeMin, m_fCoyoteRunAwayTimeMax );

		return pAnimatedUnit->GetAnimatableObject()->PlayAnimation( TPS8( run_lp ) );
	}

	AAnimatedUnit* pAnimatedUnit = pUnit->pUnit;
	pUnit->iRunning              = 1;

	AInteriorSteer*              pSteer    = TSTATICCAST( AInteriorSteer, pAnimatedUnit->GetSteer() );
	ACharacterMovementComponent* pMovement = TSTATICCAST( ACharacterMovementComponent, pAnimatedUnit->FindComponent( &TGetClass( ACharacterMovementComponent ) ) );

	TFLOAT fMultiplier = 1.0f;
	if ( pUnit->iHits > m_aiUnitMaxHits[ pUnit->eType ] + 1 )
		fMultiplier = m_fRunSpeedScaleHurt;

	pMovement->SetRunningSpeed( ACharacterMovementComponent::APPLY_IF_MOVING, ( rRandom.GetFloatMinMax( 0.0f, m_fRunSpeedRandom ) + 0.25f ) * m_afUnitRunSpeed[ pUnit->eType ] * fMultiplier + m_fRunSpeedBonus + a_fUnk4 );
	pSteer->SetMode( AInteriorSteer::MODE_NORMAL );

	pUnit->fWaitTime = rRandom.GetFloatMinMax( m_fRunAwayWaitMin, m_fRunAwayWaitMax );
	pUnit->fUnk24    = rRandom.GetFloatMinMax( a_fUnk2, a_fUnk3 );

	// NOTE: retail returns the low byte of the previous state
	return TBOOL( iState & 0xff );
}

// $Barnyard: FUNCTION 004a9d70
TBOOL ACowTippingMiniGameState::IsTargetInAim( void* a_pUnk1, TFLOAT* a_pUnk2, TFLOAT* a_pUnk3, TINT a_iUnk4, TFLOAT* a_pUnk5, TFLOAT a_fUnk6, TFLOAT a_fUnk7 )
{
	// a_pUnk1: aim direction, a_pUnk2: projectile position, a_pUnk3: target position,
	// a_iUnk4: unused, a_pUnk5: out distance, a_fUnk6: max distance, a_fUnk7: TBOOL in its low byte
	const TVector4& rcDirection = *TSTATICCAST( const TVector4, a_pUnk1 );
	const TVector4& rcPosition  = *TREINTERPRETCAST( const TVector4*, a_pUnk2 );
	const TVector4& rcTarget    = *TREINTERPRETCAST( const TVector4*, a_pUnk3 );
	const TVector4& rcCamera    = m_vCameraPos;
	TBOOL           bXZOnly     = *TREINTERPRETCAST( TUINT8*, &a_fUnk7 ) != 0;

	TVector4 vecToTarget;
	vecToTarget.Substract( rcTarget, rcCamera );
	vecToTarget.Normalise();

	TFLOAT fMinDot = m_fAimMinDot;
	if ( m_aPlayers[ m_iCurrentPlayer ].aPowerUps[ 2 ].fTime > 0.0f )
		fMinDot = m_fAimMinDotPowerUp;

	if ( !( TVector4::DotProduct3( rcDirection, vecToTarget ) > fMinDot ) )
		return TFALSE;

	TVector4 vecDirection;
	vecDirection.Substract( rcPosition, rcCamera );
	vecDirection.Normalise();

	TVector4 vecOffset;
	vecOffset.Multiply( vecDirection, m_fAimRayLength );

	TVector4 vecEnd;
	vecEnd.Add( rcPosition, vecOffset );

	TVector4 vecStart = rcCamera;
	vecOffset.Multiply( vecDirection, 2.0f );
	vecStart.Substract( vecOffset );

	if ( bXZOnly )
	{
		vecOffset.Multiply( vecDirection, 0.5f );

		TFLOAT fX      = rcTarget.x - ( vecOffset.x + rcPosition.x );
		TFLOAT fZ      = rcTarget.z - ( vecOffset.z + rcPosition.z );
		TFLOAT fDistSq = fZ * fZ + fX * fX;

		if ( fDistSq < a_fUnk6 * a_fUnk6 )
		{
			*a_pUnk5 = fDistSq;
			return TTRUE;
		}
	}
	else
	{
		TFLOAT fT;
		TFLOAT fDistSq = TMath::DistancePointToSegmentSq( rcTarget, vecStart, vecEnd, fT );

		if ( fDistSq < a_fUnk6 * a_fUnk6 )
		{
			TFLOAT fDist = TMath::Sqrt( fDistSq );

			if ( IsPowerUpActive( 2 ) )
			{
				// Has to be visible
				TRenderContext* pRenderContext = TRenderInterface::GetSingleton()->GetCurrentContext();
				TSphere         oSphere( rcTarget.x, rcTarget.y, rcTarget.z, 0.1f );

				TINT iClipFlags = TRenderContext::CullSphereToFrustum( oSphere, pRenderContext->GetWorldPlanes(), TRenderContext::CLIPFLAGS_ALL, -1 );
				if ( iClipFlags < 0 )
					return TFALSE;

				if ( iClipFlags != 0 && TRenderContext::CullSphereToFrustum( oSphere, pRenderContext->GetGuardPlanes(), iClipFlags, pRenderContext->GetGuardFlags() ) < 0 )
					return TFALSE;
			}

			TVector4 vecRay;
			vecRay.Substract( rcTarget, rcCamera );
			TFLOAT fRayLength = vecRay.NormaliseMag();

			TVector4 vecOrigin = rcCamera;
			vecOrigin.y -= 0.7f;

			if ( !ATerrainInterface::GetSingleton()->CollideRay( vecOrigin, vecRay, fRayLength, TCollisionCommon::TOSHICGROUPFLAG_NONE, TTRUE ) )
			{
				*a_pUnk5 = fDist;
				return TTRUE;
			}
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 004aa180
void ACowTippingMiniGameState::ShowScoreFloat( void* a_pUnk1, TINT a_iUnk2, TINT a_iUnk3 )
{
	// a_pUnk1: world position, a_iUnk2: score, a_iUnk3: bonus
	if ( a_iUnk2 == 0 )
		return;

	TINT iStageType = m_aStages[ m_iStage ].iStageType;
	if ( iStageType == Stage::STAGETYPE_JUNKYARD || iStageType == Stage::STAGETYPE_JUNKYARD_BOSS )
		return;

	const TVector4& rcPosition = *TSTATICCAST( const TVector4, a_pUnk1 );

	if ( ++m_iFloatTextIndex > 40 )
		m_iFloatTextIndex = 0;

	if ( a_iUnk2 < 0 )
	{
		TStringManager::String16Format( m_awszFloatTexts[ m_iFloatTextIndex ], 63, L"-%d", -a_iUnk2 );

		// NOTE: retail passes a 7th argument (1) that AddTimedFloat doesn't use
		m_oFloatHudElement.AddTimedFloat( m_awszFloatTexts[ m_iFloatTextIndex ], 0xffff0500, rcPosition, 1.0f, -2.6f, 1.0f );
		return;
	}

	if ( a_iUnk3 != 0 )
	{
		TVector4 vecPosition = rcPosition;
		vecPosition.y += 0.5f;
		vecPosition.x -= 1.0f;

		if ( ++m_iFloatTextIndex > 40 )
			m_iFloatTextIndex = 0;

		ALocale::FormatTags( m_awszFloatTexts[ m_iFloatTextIndex ], 63, 571, PPropertyValue( a_iUnk3 + a_iUnk2 ) );
		m_oFloatHudElement.AddTimedFloat( m_awszFloatTexts[ m_iFloatTextIndex ], 0xffffc800, vecPosition, 1.0f, -2.6f, 1.0f );

		if ( m_fStreakTimer <= 0.0f )
		{
			m_iStreak = 0;
		}
		else
		{
			m_iStreak += 1;

			if ( m_iStreakForExtraLife > 0 && m_iStreak >= m_iStreakForExtraLife && m_aPlayers[ m_iCurrentPlayer ].iLives < m_aStages[ m_iStage ].iMaxLives )
			{
				ShowMessagePopup( ALocaleManager::Interface()->GetString( 558 ) );
				AddLifeAt( TREINTERPRETCAST( TINT, &m_aPlayers[ m_iCurrentPlayer ] ), TREINTERPRETCAST( TINT, a_pUnk1 ) );
				m_fStreakTimer = m_fStreakTime;
				return;
			}
		}

		m_fStreakTimer = m_fStreakTime;
	}
	else
	{
		TStringManager::String16Format( m_awszFloatTexts[ m_iFloatTextIndex ], 63, L"+%d", a_iUnk2 );
		m_oFloatHudElement.AddTimedFloat( m_awszFloatTexts[ m_iFloatTextIndex ], 0xffffc800, rcPosition, 1.0f, -2.6f, 1.0f );
	}
}

// $Barnyard: FUNCTION 004aa650
void ACowTippingMiniGameState::PlaySplatEffect( void* a_pUnk1, TINT a_iUnk2 )
{
	const TVector4& rcPosition = *TSTATICCAST( const TVector4, a_pUnk1 );
	PlayerData&     rPlayer    = m_aPlayers[ m_iCurrentPlayer ];

	TFLOAT fScale = 0.8f;

	if ( rPlayer.aPowerUps[ 2 ].fTime > 0.0f )
		fScale = m_fScalePowerUp2 * 0.8f;
	else if ( rPlayer.aPowerUps[ 1 ].fTime > 0.0f )
		fScale = m_fScalePowerUp1 * 0.8f;

	AParticleEffectHandle* pEffect = AParticleEffectManager::GetSingleton()->PlayEffect( m_iTomatoSplashEffect, rcPosition, TVector4::VEC_NEGY );
	pEffect->GetInstance()->GetTransform().Scale( fScale, fScale, fScale );

	if ( m_aPlayers[ m_iCurrentPlayer ].aPowerUps[ 1 ].fTime > 0.0f )
	{
		pEffect            = AParticleEffectManager::GetSingleton()->PlayEffect( m_iTomatoSplashEffect, rcPosition, TVector4::VEC_NEGY );
		TFLOAT fSmallScale = fScale * 0.8f;
		pEffect->GetInstance()->GetTransform().Scale( fSmallScale, fSmallScale, fSmallScale );

		pEffect          = AParticleEffectManager::GetSingleton()->PlayEffect( m_iTomatoSplashEffect, rcPosition, TVector4::VEC_NEGY );
		TFLOAT fBigScale = fScale * 1.1f;
		pEffect->GetInstance()->GetTransform().Scale( fBigScale, fBigScale, fBigScale );
	}
}

// $Barnyard: FUNCTION 004aad40
TBOOL ACowTippingMiniGameState::SetScore( TINT a_iUnk1 )
{
	TINT iStageType = m_aStages[ m_iStage ].iStageType;

	if ( iStageType == Stage::STAGETYPE_JUNKYARD_BOSS || iStageType == Stage::STAGETYPE_JUNKYARD )
		return TTRUE;

	if ( m_iScoreForExtraLife > 0 )
	{
		TINT& rLastScore = m_aPlayers[ m_iCurrentPlayer ].iLastExtraLifeScore;

		if ( a_iUnk1 >= m_iScoreForExtraLife + rLastScore )
		{
			// Extra life every m_iScoreForExtraLife points
			rLastScore = a_iUnk1;
			ShowMessagePopup( ALocaleManager::Interface()->GetString( 595 ) );

			// NOTE: retail calls this through the singleton pointer (00782efc)
			AddLives1( TREINTERPRETCAST( TINT, &m_aPlayers[ m_iCurrentPlayer ] ), 1 );
		}
	}

	m_oPortrait.SetScore( a_iUnk1 );
	return TTRUE;
}

// $Barnyard: FUNCTION 004ab790
TINT ACowTippingMiniGameState::GetNumUnhit13268()
{
	T2SList<PopUpUnit>& rList  = GetUnits13268( this );
	TINT                iCount = rList.Size();

	T2_FOREACH( rList, it )
	{
		if ( it->iState == PopUpUnit::POPUPSTATE_HIT )
			iCount--;
	}

	return iCount;
}

// $Barnyard: FUNCTION 004ab990
TINT ACowTippingMiniGameState::FreeProjectile( void* a_pUnk1, TINT a_iUnk2 )
{
	Projectile*     pProjectile = TSTATICCAST( Projectile, a_pUnk1 );
	T2GenericSList& rActive     = *TREINTERPRETCAST( T2GenericSList*, &m_aProjectileLists[ a_iUnk2 ].llHeld );
	T2GenericSList& rFree       = *TREINTERPRETCAST( T2GenericSList*, &m_aProjectileLists[ a_iUnk2 ].llFree );

	rActive.Transfer( TREINTERPRETCAST( T2GenericSList::Node*, pProjectile ), rFree );

	pProjectile->vVelocity.Set( 0.0f, 0.0f, 0.0f, 1.0f );

	pProjectile->pObject->SetSkeletonUpdating( TFALSE, TTRUE );
	return 0;
}

// $Barnyard: FUNCTION 004ace20
TBOOL ACowTippingMiniGameState::HitFarmer()
{
	AAnimatableObject* pObject = m_pWildMikeBox;

	if ( IsModelUpdatingSkeleton( pObject ) && m_eRunnerState == RUNNERSTATE_PIPE_UP )
	{
		m_eRunnerState = RUNNERSTATE_COMING_OUT;
		pObject->PlayAnimation( TPS8( out_start ) );
		ASoundManager::GetSingleton()->PlayCue( soundbank::DAGBOSS_BOX02 );

		m_oRunnerIcon.SetShown( TFALSE );
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 004af2a0
TINT ACowTippingMiniGameState::DetachUnit( TINT a_iUnk1 )
{
	Enemy*             pUnit   = TREINTERPRETCAST( Enemy*, a_iUnk1 );
	AAnimatableObject* pParent = pUnit->pUnit->GetAnimatableObject()->GetParentObject();

	if ( pParent == TNULL )
		return 0;

	AAnimatableObjectManager::GetSingleton()->DetachObject( pUnit->pUnit->GetAnimatableObject(), pParent );

	// Face away from the up vector of the model
	TVector4 vecDirection;
	pUnit->pUnit->GetAnimatableObject()->GetModelInstance()->GetUpVector( vecDirection );

	vecDirection.y = 0.0f;
	vecDirection.x = -vecDirection.x;
	vecDirection.z = -vecDirection.z;
	vecDirection.Normalise();
	vecDirection.w = 1.0f;

	pUnit->pUnit->SetSteerDirection( vecDirection );
	return 1;
}

// $Barnyard: FUNCTION 004af390
TINT ACowTippingMiniGameState::StealUnitCarriedObject( TINT a_iUnk1 )
{
	Enemy* pUnit = TREINTERPRETCAST( Enemy*, a_iUnk1 );
	Cow*   pDC   = &m_aCows[ pUnit->iCow ];

	if ( pUnit->pUnit->GetAnimatableObject()->GetParentObject() == TNULL )
		return 0;

	DetachUnit( a_iUnk1 );
	pUnit->pUnit->GetAnimatableObject()->PlayAnimation( TPS8( run_lp ) );

	AAnimatableObject* pDCObject = pDC->pObject;
	pDCObject->KillAllAnimations();
	pDCObject->PlayAnimation( TPS8( stolen_coyote ) );

	if ( AAnimatableObjectManager::GetSingleton()->GetAttachedObject( pUnit->pUnit->GetAnimatableObject(), m_iCarryBone, 0 ) == TNULL )
	{
		AAnimatableObjectManager::GetSingleton()->AttachObject( pDCObject, pUnit->pUnit->GetAnimatableObject(), 0, m_iCarryBone, TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
	}

	*TREINTERPRETCAST( TUINT8*, &pDC->bActive ) = 0;
	return 0;
}

// $Barnyard: FUNCTION 004b00f0
void ACowTippingMiniGameState::SendUnitBackToStart( TINT a_iUnk1 )
{
	Enemy* pUnit = TREINTERPRETCAST( Enemy*, a_iUnk1 );

	if ( pUnit->eType == Enemy::ENEMYTYPE_COYOTE )
		return;

	if ( pUnit->eState == Enemy::ENEMYSTATE_PUSHING || pUnit->eState == Enemy::ENEMYSTATE_RETURNING || pUnit->eState == Enemy::ENEMYSTATE_TARGETED )
		return;

	if ( pUnit->pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( push_end ), 0.0f ) ) return;
	if ( pUnit->pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( push_end ) ) ) return;

	AAnimatedUnit*  pAnimatedUnit = pUnit->pUnit;
	AInteriorSteer* pSteer        = TSTATICCAST( AInteriorSteer, pAnimatedUnit->GetSteer() );
	pAnimatedUnit->GetAnimatableObject()->SetAnimControllersDisabled( TFALSE );

	TPString8Pool* pPool = TUtil::GetTPStringPool();
	TString8       strGoal;

	TINT iStageType = m_aStages[ m_iStage ].iStageType;
	if ( iStageType == Stage::STAGETYPE_JUNKYARD_BOSS || iStageType == Stage::STAGETYPE_JUNKYARD )
		strGoal.Format( "%s%da", "start", pUnit->iCow );
	else
		strGoal.Format( "%s%d%c", "start", pUnit->iCow + 1, TCHAR( 'a' + ( pUnit->iCowSide != 0 ) ) );

	if ( pAnimatedUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) ||
	     pAnimatedUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( run_lp ), 0.0f ) ||
	     pAnimatedUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( walk_lp ), 0.0f ) )
	{
		// Go back to the start goal
		pSteer->SetInteracting( 0 );
		pSteer->Command_CancelCurrentCommand();
		pSteer->Command_SeekGoal( TPString8( pPool, strGoal.GetString() ) );
		pSteer->SetMode( AInteriorSteer::MODE_NORMAL );
	}

	if ( pUnit->pUnit->GetAnimatableObject()->GetParentObject() != TNULL )
		DetachUnit( a_iUnk1 );

	MakeUnitRunAway( a_iUnk1, 0.0f, 0.0f, m_fYukRunSpeedBonus );

	pUnit->eState    = Enemy::ENEMYSTATE_RETURNING;
	pUnit->iHits     = 100;
	pUnit->fWaitTime = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 20.0f, 40.0f );

	if ( pUnit->bUnk4D && !pUnit->bUnk4F )
	{
		TINT iBit = GetUnitStartBit( pUnit, m_aStages[ m_iStage ].iStageType );

		// Bit array at 0x57b4 (30 bits)
		if ( iBit > -1 && iBit < 30 )
			TREINTERPRETCAST( TUINT8*, m_auiCowSideReservedBits )
		[iBit >> 3] |= 1 << ( iBit & 7 );

		pUnit->bUnk4D = TTRUE;
	}
}

// $Barnyard: FUNCTION 004b0360
TINT ACowTippingMiniGameState::SendUnitBackToStartRunning( Enemy* a_pUnit )
{
	SendUnitBackToStart( TREINTERPRETCAST( TINT, a_pUnit ) );

	ANamedAnimationSetRef pAnimationSet = a_pUnit->pUnit->GetAnimatableObject()->GetObjectType()->FindAnimationSetOrDefault( TPS8( run ) );
	TBOOL                 bChanged      = a_pUnit->pUnit->GetAnimatableObject()->GetToshiAnimInterface().GetAnimationSet().Get() != pAnimationSet.Get();

	if ( bChanged )
	{
		a_pUnit->pUnit->GetAnimatableObject()->GetToshiAnimInterface().SetAnimationSet( pAnimationSet );

		// NOTE: the result isn't used
		TVector4 vecPosition;
		a_pUnit->pUnit->GetSteer()->CopyPosition( vecPosition );
	}

	return 0;
}

// $Barnyard: FUNCTION 004b0e30
TVector4* ACowTippingMiniGameState::GetUnitBonePos6( TVector4* a_pOutPos, Enemy* a_pUnit )
{
	TMatrix44 matBone;
	a_pUnit->pUnit->GetAnimatableObject()->GetModelInstance()->GetBoneTransform( m_iCoyotePelvisBone, matBone );

	*a_pOutPos = matBone.AsBasisVector4( BASISVECTOR_TRANSLATION );
	return a_pOutPos;
}

// $Barnyard: FUNCTION 004b0ee0
void* ACowTippingMiniGameState::GetUnitTargetPos( void* a_pUnk1, TINT a_iUnk2 )
{
	TVector4* pOutPos = TSTATICCAST( TVector4, a_pUnk1 );
	Enemy*    pUnit   = TREINTERPRETCAST( Enemy*, a_iUnk2 );

	if ( pUnit->eType == Enemy::ENEMYTYPE_COYOTE )
	{
		GetUnitBonePos6( pOutPos, pUnit );
		return pOutPos;
	}

	TMatrix44 matBone;
	pUnit->pUnit->GetAnimatableObject()->GetModelInstance()->GetBoneTransform( m_iKidPelvisBone, matBone );

	*pOutPos = matBone.AsBasisVector4( BASISVECTOR_TRANSLATION );
	return pOutPos;
}

// $Barnyard: FUNCTION 004b0fb0
TINT ACowTippingMiniGameState::GetUnit13268TargetPos( TINT a_iUnk1, void** a_pUnk2 )
{
	TVector4*  pOutPos = TREINTERPRETCAST( TVector4*, a_iUnk1 );
	PopUpUnit* pUnit   = TREINTERPRETCAST( PopUpUnit*, a_pUnk2 );

	TMatrix44 matBone;
	pUnit->pUnit->GetAnimatableObject()->GetModelInstance()->GetBoneTransform( m_iRaccoonPelvisBone, matBone );

	TVector4 vecPosition = matBone.AsBasisVector4( BASISVECTOR_TRANSLATION );

	TINT iStageType = m_aStages[ m_iStage ].iStageType;
	if ( iStageType == Stage::STAGETYPE_JUNKYARD || iStageType == Stage::STAGETYPE_JUNKYARD_BOSS )
		vecPosition.y -= 0.7f;

	*pOutPos = vecPosition;
	return a_iUnk1;
}

// $Barnyard: FUNCTION 004b1090
TVector4* ACowTippingMiniGameState::GetUnit13270TargetPos( TVector4* a_pOutPos, PopUpUnit* a_pUnit )
{
	TMatrix44 matBone;
	a_pUnit->pUnit->GetAnimatableObject()->GetModelInstance()->GetBoneTransform( m_iGopherPelvisBone, matBone );

	*a_pOutPos = matBone.AsBasisVector4( BASISVECTOR_TRANSLATION );
	return a_pOutPos;
}

// $Barnyard: FUNCTION 004b1140
void* ACowTippingMiniGameState::GetCratePos( Toshi::TVector4* a_pUnk1, TINT a_iUnk2 )
{
	Crate* pUnk = TREINTERPRETCAST( Crate*, a_iUnk2 );

	TVector4 vecPosition;
	pUnk->pObject->GetModelInstance()->GetPosition( vecPosition );

	*a_pUnk1 = vecPosition;
	return a_pUnk1;
}

// $Barnyard: FUNCTION 004b1620
ACowTippingMiniGameState::PopUpUnit* ACowTippingMiniGameState::FindAimed13270( const TVector4* a_pDir, const TVector4* a_pPos, TFLOAT* a_pOutDist, TFLOAT a_fUnk4, TFLOAT a_fMaxDist, TBOOL a_bUnk6 )
{
	TINT iStageType = m_aStages[ m_iStage ].iStageType;
	if ( iStageType == Stage::STAGETYPE_JUNKYARD_BOSS || iStageType == Stage::STAGETYPE_JUNKYARD )
		return TNULL;

	TINT       iUnk4   = AsInt( a_fUnk4 );
	PopUpUnit* pResult = TNULL;

	T2_FOREACH( GetUnits13270( this ), it )
	{
		TVector4 vecTarget;
		GetUnit13270TargetPos( &vecTarget, it );

		TFLOAT fDist;
		if ( IsTargetInAim( (void*)a_pDir, (TFLOAT*)a_pPos, (TFLOAT*)&vecTarget, iUnk4, &fDist, a_fMaxDist, BoolAsFloat( a_bUnk6 ) ) )
		{
			AAnimatedUnit* pAnimatedUnit = it->pUnit;

			if ( !pAnimatedUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( yuk ), 0.0f ) &&
			     !pAnimatedUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( yuk ) ) )
			{
				pResult     = it;
				a_fMaxDist  = fDist;
				*a_pOutDist = fDist;
			}
		}
	}

	return pResult;
}

// $Barnyard: FUNCTION 004b1740
void* ACowTippingMiniGameState::FindAimedCrate( TFLOAT* a_pUnk1, TFLOAT* a_pUnk2, TFLOAT* a_pUnk3, TFLOAT a_fUnk4, TFLOAT a_fUnk5, TFLOAT a_fUnk6 )
{
	// a_pUnk1: direction, a_pUnk2: position, a_pUnk3: out distance, a_fUnk4: unused (as TINT),
	// a_fUnk5: max distance, a_fUnk6: TBOOL in its low byte
	TINT iStageType = m_aStages[ m_iStage ].iStageType;
	if ( iStageType == Stage::STAGETYPE_JUNKYARD_BOSS || iStageType == Stage::STAGETYPE_JUNKYARD )
		return TNULL;

	TINT   iUnk4   = AsInt( a_fUnk4 );
	Crate* pResult = TNULL;

	for ( TINT i = 0; i < 6; i++ )
	{
		for ( TINT j = 4; j >= 0; j-- )
		{
			Crate* pUnk = GetCrateState( this, i, j );

			if ( pUnk->iState == Crate::CRATESTATE_ON_GROUND || pUnk->iState == Crate::CRATESTATE_FALLING || pUnk->iState == Crate::CRATESTATE_CARRIED )
			{
				TVector4 vecTarget;
				GetCratePos( &vecTarget, TREINTERPRETCAST( TINT, pUnk ) );

				TFLOAT fDist;
				if ( IsTargetInAim( a_pUnk1, a_pUnk2, (TFLOAT*)&vecTarget, iUnk4, &fDist, a_fUnk5, a_fUnk6 ) )
				{
					pResult  = pUnk;
					a_fUnk5  = fDist;
					*a_pUnk3 = fDist;
				}
			}
		}
	}

	return pResult;
}

// $Barnyard: FUNCTION 004b1850
void* ACowTippingMiniGameState::FindAimedCoin( TFLOAT* a_pUnk1, TFLOAT* a_pUnk2, void* a_pUnk3, void* a_pUnk4, void* a_pUnk5, TFLOAT a_fUnk6 )
{
	// a_pUnk1: direction, a_pUnk2: position, a_pUnk3: out distance (TFLOAT*), a_pUnk4: unused (TINT),
	// a_pUnk5: max distance (TFLOAT), a_fUnk6: TBOOL in its low byte
	TINT iStageType = m_aStages[ m_iStage ].iStageType;
	if ( iStageType == Stage::STAGETYPE_JUNKYARD_BOSS || iStageType == Stage::STAGETYPE_JUNKYARD )
		return TNULL;

	TFLOAT* pOutDist = TSTATICCAST( TFLOAT, a_pUnk3 );
	TINT    iUnk4    = TREINTERPRETCAST( TINT, a_pUnk4 );
	TFLOAT  fMaxDist = *TREINTERPRETCAST( TFLOAT*, &a_pUnk5 );
	Coin*   pResult  = TNULL;

	for ( TINT i = 0; i < 8; i++ )
	{
		Coin* pUnk = GetCoinState( this, i );

		if ( IsModelUpdatingSkeleton( pUnk->pObject ) && pUnk->iState != Coin::COINSTATE_COLLECTED )
		{
			TVector4 vecTarget;
			pUnk->pObject->GetModelInstance()->GetPosition( vecTarget );

			TFLOAT fDist;
			if ( IsTargetInAim( a_pUnk1, a_pUnk2, (TFLOAT*)&vecTarget, iUnk4, &fDist, fMaxDist, a_fUnk6 ) )
			{
				pResult   = pUnk;
				fMaxDist  = fDist;
				*pOutDist = fDist;
			}
		}
	}

	return pResult;
}

// $Barnyard: FUNCTION 004b19a0
ACowTippingMiniGameState::Coin* ACowTippingMiniGameState::HitTestCoins( const TVector4* a_pDir, Projectile* a_pProjectile, const TVector4* a_pPos, TFLOAT* a_pOutDist, TINT a_iUnk5, TFLOAT a_fMaxDist, TINT a_iList, t_HitCallbackCoin a_fnCallback, TBOOL a_bUnk9 )
{
	TINT iStageType = m_aStages[ m_iStage ].iStageType;
	if ( iStageType == Stage::STAGETYPE_JUNKYARD_BOSS || iStageType == Stage::STAGETYPE_JUNKYARD )
		return TNULL;

	Coin* pResult = TNULL;

	for ( TINT i = 0; i < 8; i++ )
	{
		Coin* pUnk = GetCoinState( this, i );

		if ( IsModelUpdatingSkeleton( pUnk->pObject ) && pUnk->iState != Coin::COINSTATE_COLLECTED )
		{
			TVector4 vecTarget;
			pUnk->pObject->GetModelInstance()->GetPosition( vecTarget );

			TFLOAT fDist;
			if ( IsTargetInAim( (void*)a_pDir, (TFLOAT*)a_pPos, (TFLOAT*)&vecTarget, a_iUnk5, &fDist, a_fMaxDist, BoolAsFloat( a_bUnk9 ) ) )
			{
				pResult     = pUnk;
				*a_pOutDist = a_fMaxDist;
				( this->*a_fnCallback )( a_pProjectile, pUnk );
			}
		}
	}

	return pResult;
}

// $Barnyard: FUNCTION 004b1af0
ACowTippingMiniGameState::Enemy* ACowTippingMiniGameState::HitTest13120( const TVector4* a_pDir, Projectile* a_pProjectile, const TVector4* a_pPos, TFLOAT* a_pOutDist, TINT a_iUnk5, TFLOAT a_fMaxDist, TINT a_iList, t_HitCallback13120 a_fnCallback, TBOOL a_bUnk9 )
{
	Enemy* pResult = TNULL;

	T2_FOREACH( GetUnits13120( this ), it )
	{
		if ( it->eType != Enemy::ENEMYTYPE_COYOTE || it->iHits < 1 )
		{
			TVector4 vecTarget;
			GetUnitTargetPos( &vecTarget, TREINTERPRETCAST( TINT, it.Get() ) );

			TFLOAT fDist;
			if ( IsTargetInAim( (void*)a_pDir, (TFLOAT*)a_pPos, (TFLOAT*)&vecTarget, a_iUnk5, &fDist, a_fMaxDist, BoolAsFloat( a_bUnk9 ) ) )
			{
				*a_pOutDist = a_fMaxDist;
				pResult     = it;
				( this->*a_fnCallback )( TREINTERPRETCAST( TINT, a_pProjectile ), it.Get(), AsInt( a_fMaxDist ), a_iList );
			}
		}
	}

	return pResult;
}

// $Barnyard: FUNCTION 004b1bb0
ACowTippingMiniGameState::PopUpUnit* ACowTippingMiniGameState::HitTest13268( const TVector4* a_pDir, Projectile* a_pProjectile, const TVector4* a_pPos, TFLOAT* a_pOutDist, TINT a_iUnk5, TFLOAT a_fMaxDist, TINT a_iList, t_HitCallback13268 a_fnCallback, TBOOL a_bUnk9 )
{
	PopUpUnit* pResult = TNULL;

	T2_FOREACH( GetUnits13268( this ), it )
	{
		TVector4 vecTarget;
		GetUnit13268TargetPos( TREINTERPRETCAST( TINT, &vecTarget ), TREINTERPRETCAST( void**, it.Get() ) );

		TFLOAT fDist;
		if ( !IsTargetInAim( (void*)a_pDir, (TFLOAT*)a_pPos, (TFLOAT*)&vecTarget, a_iUnk5, &fDist, a_fMaxDist, BoolAsFloat( a_bUnk9 ) ) )
			continue;

		AAnimatedUnit* pAnimatedUnit = it->pUnit;
		TINT           iStageType    = m_aStages[ m_iStage ].iStageType;

		if ( iStageType == Stage::STAGETYPE_JUNKYARD_BOSS || iStageType == Stage::STAGETYPE_JUNKYARD )
		{
			if ( pAnimatedUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( yuk ), 0.0f ) ) continue;
			if ( pAnimatedUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( yuk ) ) ) continue;
			if ( pAnimatedUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( yukobject ), 0.0f ) ) continue;
			if ( pAnimatedUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( yukobject ) ) ) continue;
		}
		else
		{
			if ( pAnimatedUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( yuk ), 0.0f ) ) continue;
			if ( pAnimatedUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( yuk ) ) ) continue;
		}

		*a_pOutDist = a_fMaxDist;
		pResult     = it;
		( this->*a_fnCallback )( TREINTERPRETCAST( TINT, a_pProjectile ), it.Get(), AsInt( a_fMaxDist ), a_iList );
	}

	return pResult;
}

// $Barnyard: FUNCTION 004b1d40
ACowTippingMiniGameState::Crate* ACowTippingMiniGameState::HitTestCrates( const TVector4* a_pDir, Projectile* a_pProjectile, const TVector4* a_pPos, TFLOAT* a_pOutDist, TINT a_iUnk5, TFLOAT a_fMaxDist, TINT a_iList, t_HitCallbackCrate a_fnCallback, TBOOL a_bUnk9 )
{
	TINT iStageType = m_aStages[ m_iStage ].iStageType;
	if ( iStageType == Stage::STAGETYPE_JUNKYARD_BOSS || iStageType == Stage::STAGETYPE_JUNKYARD )
		return TNULL;

	Crate* pResult = TNULL;

	for ( TINT i = 0; i < 6; i++ )
	{
		for ( TINT j = 4; j >= 0; j-- )
		{
			Crate* pUnk = GetCrateState( this, i, j );

			if ( pUnk->iState == Crate::CRATESTATE_ON_GROUND || pUnk->iState == Crate::CRATESTATE_FALLING || pUnk->iState == Crate::CRATESTATE_CARRIED )
			{
				TVector4 vecTarget;
				GetCratePos( &vecTarget, TREINTERPRETCAST( TINT, pUnk ) );

				TFLOAT fDist;
				if ( IsTargetInAim( (void*)a_pDir, (TFLOAT*)a_pPos, (TFLOAT*)&vecTarget, a_iUnk5, &fDist, a_fMaxDist, BoolAsFloat( a_bUnk9 ) ) )
				{
					*a_pOutDist = a_fMaxDist;
					pResult     = pUnk;
					( this->*a_fnCallback )( a_pProjectile, pUnk, a_fMaxDist, a_iList );
				}
			}
		}
	}

	return pResult;
}

// $Barnyard: FUNCTION 004b2900
void ACowTippingMiniGameState::ScareUnitsNearProjectile( const TVector4* a_pPos, TFLOAT* a_pUnk2, TFLOAT a_fMaxDistXZ, TFLOAT a_fMaxDist )
{
	// Scares the units the projectile flies close to
	T2_FOREACH( GetUnits13120( this ), it )
	{
		Enemy* pUnit = it;

		TVector4 vecTarget;
		GetUnitTargetPos( &vecTarget, TREINTERPRETCAST( TINT, pUnit ) );

		TVector4 vecDirection;
		vecDirection.Substract( *a_pPos, m_vCameraPos );
		vecDirection.Normalise();

		TVector4 vecOffset;
		vecOffset.Multiply( vecDirection, m_fAimRayLength );

		TVector4 vecEnd;
		vecEnd.Add( *a_pPos, vecOffset );

		TVector4 vecStart = *a_pPos;
		vecOffset.Multiply( vecDirection, 2.0f );
		vecStart.Substract( vecOffset );

		TFLOAT fT;
		TFLOAT fDistSq = TMath::DistancePointToSegmentSq( vecTarget, vecStart, vecEnd, fT );

		if ( TMath::Sqrt( fDistSq ) < a_fMaxDist )
		{
			TVector4 vecCentre = *a_pPos;
			vecOffset.Multiply( vecDirection, 0.5f );
			vecCentre.x += vecOffset.x;
			vecCentre.y += vecOffset.y;
			vecCentre.z += vecOffset.z;

			if ( TVector4::DistanceXZ( vecCentre, vecTarget ) < a_fMaxDistXZ && IsUnitNotYukking( TREINTERPRETCAST( TINT, pUnit ) ) )
			{
				if ( pUnit->eType == Enemy::ENEMYTYPE_COYOTE )
				{
					if ( pUnit->iHits == 0 )
					{
						ANamedAnimationSetRef pAnimationSet = pUnit->pUnit->GetAnimatableObject()->GetObjectType()->FindAnimationSetOrDefault( TPS8( scared ) );

						if ( pUnit->pUnit->GetAnimatableObject()->GetToshiAnimInterface().GetAnimationSet().Get() != pAnimationSet.Get() )
							pUnit->pUnit->GetAnimatableObject()->GetToshiAnimInterface().SetAnimationSet( pAnimationSet );
					}
				}
				else
				{
					MakeUnitRunAway( TREINTERPRETCAST( TINT, pUnit ), m_fRunAwayTimeMin, m_fRunAwayTimeMax, 0.0f );
				}
			}
		}
	}
}

// $Barnyard: FUNCTION 004b30c0
void ACowTippingMiniGameState::CheckNearMiss13270( const TVector4* a_pPos, TFLOAT* a_pOutDist, TFLOAT a_fMaxDistXZ, TFLOAT a_fMaxDist )
{
	T2_FOREACH( GetUnits13270( this ), it )
	{
		TVector4 vecTarget;
		GetUnit13270TargetPos( &vecTarget, it );

		TVector4 vecDirection;
		vecDirection.Substract( *a_pPos, m_vCameraPos );
		vecDirection.Normalise();

		TVector4 vecOffset;
		vecOffset.Multiply( vecDirection, m_fUnk_00EDC );

		TVector4 vecEnd;
		vecEnd.Add( *a_pPos, vecOffset );

		TVector4 vecStart = *a_pPos;
		vecOffset.Multiply( vecDirection, 2.0f );
		vecStart.Substract( vecOffset );

		TFLOAT fT;
		TFLOAT fDistSq = TMath::DistancePointToSegmentSq( vecTarget, vecStart, vecEnd, fT );

		if ( TMath::Sqrt( fDistSq ) < a_fMaxDist )
		{
			TVector4 vecCentre = *a_pPos;
			vecOffset.Multiply( vecDirection, 0.5f );
			vecCentre.x += vecOffset.x;
			vecCentre.y += vecOffset.y;
			vecCentre.z += vecOffset.z;

			if ( TVector4::DistanceXZ( vecCentre, vecTarget ) < a_fMaxDistXZ )
			{
				AAnimatedUnit* pAnimatedUnit = it->pUnit;

				if ( !pAnimatedUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( yuk ), 0.0f ) &&
				     !pAnimatedUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( yuk ) ) )
				{
					*a_pOutDist = a_fMaxDist;
				}
			}
		}
	}
}

// $Barnyard: FUNCTION 004b4060
TINT ACowTippingMiniGameState::AddUnitScore( Enemy* a_pUnit, TINT a_iScore, TINT a_iBonus, Projectile* a_pProjectile )
{
	TINT iScore = a_iScore;
	TINT iBonus = a_iBonus;

	// Projectile type 4 doubles the score
	if ( a_iScore > 0 && a_pProjectile != TNULL && a_pProjectile->iGrowType == 4 )
	{
		iScore = 2 * a_iScore;
		iBonus = 2 * a_iBonus;
	}

	TVector4 vecPosition;
	a_pUnit->pUnit->GetAnimatableObject()->GetModelInstance()->GetPosition( vecPosition );
	vecPosition.y -= 3.5f;

	TVector4 vecOffset;
	vecOffset.Multiply( m_vRailDirection, 0.6f );
	vecPosition.Substract( vecOffset );

	PlayerData* pPlayer = &m_aPlayers[ m_iCurrentPlayer ];
	if ( pPlayer->iScore + iScore < 0 )
		iScore = -pPlayer->iScore;

	ShowScoreFloat( &vecPosition, iScore, iBonus );

	pPlayer = &m_aPlayers[ m_iCurrentPlayer ];
	pPlayer->iScore += iScore + iBonus;
	if ( pPlayer->iScore < 0 )
		pPlayer->iScore = 0;

	// NOTE: retail calls this through the singleton pointer (00782efc)
	SetScore( pPlayer->iScore );

	if ( a_pUnit->eType == Enemy::ENEMYTYPE_COYOTE )
		m_iCoyoteScore += iScore;
	else
		m_iEnemyScore += iScore;

	m_iBonusScore += iBonus;
	return m_iBonusScore;
}

// $Barnyard: FUNCTION 004b41f0
TINT ACowTippingMiniGameState::AddUnit13268Score( PopUpUnit* a_pUnit, TINT a_iScore, TINT a_iBonus )
{
	TINT iScore = a_iScore;
	TINT iBonus = a_iBonus;

	// NOTE: retail reads the player through the singleton pointer (00782efc)
	if ( a_iScore > 0 && m_aPlayers[ m_iCurrentPlayer ].aPowerUps[ 4 ].fTime > 0.0f )
	{
		iScore = 2 * a_iScore;
		iBonus = 2 * a_iBonus;
	}

	TVector4 vecPosition;
	a_pUnit->pUnit->GetAnimatableObject()->GetModelInstance()->GetPosition( vecPosition );
	vecPosition.y -= 3.5f;

	TVector4 vecOffset;
	vecOffset.Multiply( m_vRailDirection, 0.6f );
	vecPosition.Substract( vecOffset );

	PlayerData* pPlayer = &m_aPlayers[ m_iCurrentPlayer ];
	if ( pPlayer->iScore + iScore < 0 )
		iScore = -pPlayer->iScore;

	ShowScoreFloat( &vecPosition, iScore, iBonus );

	pPlayer = &m_aPlayers[ m_iCurrentPlayer ];
	pPlayer->iScore += iScore + iBonus;
	if ( pPlayer->iScore < 0 )
		pPlayer->iScore = 0;

	SetScore( pPlayer->iScore );

	m_iRaccoonScore += iScore;
	m_iBonusScore += iBonus;
	return m_iBonusScore;
}

// $Barnyard: FUNCTION 004b4390
TBOOL ACowTippingMiniGameState::AddUnit13270Score( PopUpUnit* a_pUnit, TINT a_iScore, TINT a_iBonus )
{
	TINT iScore = a_iScore;
	TINT iBonus = a_iBonus;

	if ( a_iScore > 0 && m_aPlayers[ m_iCurrentPlayer ].aPowerUps[ 4 ].fTime > 0.0f )
	{
		iScore = 2 * a_iScore;
		iBonus = 2 * a_iBonus;
	}

	TVector4 vecPosition;
	a_pUnit->pUnit->GetAnimatableObject()->GetModelInstance()->GetPosition( vecPosition );
	vecPosition.y -= 4.0f;

	TVector4 vecOffset;
	vecOffset.Multiply( m_vRailDirection, 0.6f );
	vecPosition.Substract( vecOffset );

	ShowScoreFloat( &vecPosition, iScore, iBonus );

	PlayerData* pPlayer = &m_aPlayers[ m_iCurrentPlayer ];
	pPlayer->iScore += iScore + iBonus;
	if ( pPlayer->iScore < 0 )
		pPlayer->iScore = 0;

	return SetScore( pPlayer->iScore );
}

// $Barnyard: FUNCTION 004b44f0
TINT ACowTippingMiniGameState::AddCrateScore( Crate* a_pUnk, TINT a_iScore, TINT a_iBonus )
{
	TINT iScore = a_iScore;
	TINT iBonus = a_iBonus;

	if ( a_iScore > 0 && m_aPlayers[ m_iCurrentPlayer ].aPowerUps[ 4 ].fTime > 0.0f )
	{
		iScore = 2 * a_iScore;
		iBonus = 2 * a_iBonus;
	}

	TVector4 vecPosition;
	a_pUnk->pObject->GetModelInstance()->GetPosition( vecPosition );
	vecPosition.y -= 4.0f;

	TVector4 vecOffset;
	vecOffset.Multiply( m_vRailDirection, 0.6f );
	vecPosition.Substract( vecOffset );

	PlayerData* pPlayer = &m_aPlayers[ m_iCurrentPlayer ];
	if ( pPlayer->iScore + iScore < 0 )
		iScore = -pPlayer->iScore;

	ShowScoreFloat( &vecPosition, iScore, iBonus );

	pPlayer = &m_aPlayers[ m_iCurrentPlayer ];
	pPlayer->iScore += iScore + iBonus;
	if ( pPlayer->iScore < 0 )
		pPlayer->iScore = 0;

	SetScore( pPlayer->iScore );

	m_iCrateScore += iScore;
	m_iBonusScore += iBonus;
	return m_iBonusScore;
}

// $Barnyard: FUNCTION 004b4be0
void ACowTippingMiniGameState::PlayProjectileSplat( void** a_pUnk1, TINT a_iUnk2 )
{
	Projectile* pProjectile = TREINTERPRETCAST( Projectile*, a_pUnk1 );

	TVector4 vecPosition;
	pProjectile->pObject->GetModelInstance()->GetPosition( vecPosition );

	TVector4 vecDirection;
	vecDirection.Substract( vecPosition, m_vCameraBasePos );

	if ( vecDirection.NormaliseMag() > 0.5f )
		vecPosition.y -= 0.1f;

	PlaySplatEffect( &vecPosition, TREINTERPRETCAST( TINT, &vecDirection ) );
}

// $Barnyard: FUNCTION 004b4ce0
TINT ACowTippingMiniGameState::PlayProjectileSplatWithSound( Projectile* a_pProjectile, void* a_pUnk2 )
{
	TVector4 vecPosition;
	a_pProjectile->pObject->GetModelInstance()->GetPosition( vecPosition );

	TVector4 vecDirection;
	vecDirection.Substract( vecPosition, m_vCameraBasePos );

	if ( vecDirection.NormaliseMag() > 0.5f )
		vecPosition.y -= 0.1f;

	PlaySplatEffect( &vecPosition, TREINTERPRETCAST( TINT, &vecDirection ) );
	return ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::COWTIP_BOX02, vecPosition );
}

// $Barnyard: FUNCTION 004b5780
TINT ACowTippingMiniGameState::AddLifeAtUnit( PlayerData* a_pPlayer, Enemy* a_pUnit )
{
	if ( m_eTurnEnd != TURNEND_NONE )
		return m_eTurnEnd;

	TVector4 vecPosition;
	GetUnitTargetPos( &vecPosition, TREINTERPRETCAST( TINT, a_pUnit ) );
	vecPosition.y -= 1.5f;

	return AddLifeAt( TREINTERPRETCAST( TINT, a_pPlayer ), TREINTERPRETCAST( TINT, &vecPosition ) );
}

// $Barnyard: FUNCTION 004bacc0
TBOOL ACowTippingMiniGameState::OnUnitHit( void* a_pUnk1, TINT a_iUnk2, TINT a_iUnk3, TINT a_iUnk4 )
{
	// a_pUnk1: unit, a_iUnk2: hits, a_iUnk3: projectile, a_iUnk4: TBOOL in its low byte
	Enemy*      pUnit       = TSTATICCAST( Enemy, a_pUnk1 );
	Projectile* pProjectile = TREINTERPRETCAST( Projectile*, a_iUnk3 );

	TINT iOldHits = pUnit->iHits;
	pUnit->iHits  = a_iUnk2 + iOldHits;

	m_aPlayers[ m_iCurrentPlayer ].iHits += 1;
	m_aPlayers[ m_iCurrentPlayer ].iTurnHits += 1;

	AAnimatedUnit*  pAnimatedUnit = pUnit->pUnit;
	AInteriorSteer* pSteer        = TSTATICCAST( AInteriorSteer, pAnimatedUnit->GetSteer() );
	TFLOAT          fSpeed        = pSteer->GetSpeed();

	// Running units give a bonus
	TBOOL bBonus = ( pUnit->eState == Enemy::ENEMYSTATE_ACTIVE || pUnit->eState == Enemy::ENEMYSTATE_RETURNING ) && pSteer->GetMode() != AInteriorSteer::MODE_STILL && fSpeed > m_fMovingBonusMinSpeed;

	TINT iStageType = m_aStages[ m_iStage ].iStageType;

	if ( iStageType == Stage::STAGETYPE_JUNKYARD || iStageType == Stage::STAGETYPE_JUNKYARD_BOSS )
	{
		if ( pUnit->pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( push_end ), 0.0f ) ||
		     pUnit->pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( push_end ) ) )
		{
			StealUnitCarriedObject( TREINTERPRETCAST( TINT, pUnit ) );
		}
	}

	if ( pUnit->eState == Enemy::ENEMYSTATE_PUSH_END )
		SendUnitBackToStart( TREINTERPRETCAST( TINT, pUnit ) );
	else if ( pUnit->eState != Enemy::ENEMYSTATE_RETURNING )
		pUnit->eState = Enemy::ENEMYSTATE_ACTIVE;

	// NOTE: retail reads m_aiUnitMaxHits through the singleton pointer (00782efc)
	TINT iMaxHits = ( pUnit->eType >= Enemy::ENEMYTYPE_COYOTE ) ? 1 : m_aiUnitMaxHits[ pUnit->eType ];

	if ( pUnit->eType == Enemy::ENEMYTYPE_COYOTE )
		pUnit->fWaitTime = 1.0f;

	pUnit->pUnit->GetAnimatableObject()->SetAnimControllersDisabled( TFALSE );

	const TPString8* pAnimation;

	if ( pUnit->iHits < iMaxHits || pUnit->eState == Enemy::ENEMYSTATE_RETURNING )
	{
		if ( iOldHits < iMaxHits )
			AddUnitScore( pUnit, m_aiUnitSplatScore[ pUnit->eType ], bBonus ? m_iMovingHitBonus : 0, pProjectile );

		pAnimatedUnit->GetAnimatableObject()->KillAllAnimations();
		pAnimation = &TPS8( splat );
	}
	else
	{
		if ( iOldHits < iMaxHits )
		{
			if ( pUnit->eType == Enemy::ENEMYTYPE_COYOTE )
			{
				AddUnitScore( pUnit, m_iCoyoteHitScore, 0, pProjectile );

				PlayerData* pPlayer = &m_aPlayers[ m_iCurrentPlayer ];
				pPlayer->iCoyoteHits += 1;

				// Extra life every m_iCoyoteHitsForExtraLife of these
				if ( m_iCoyoteHitsForExtraLife > 0 && pPlayer->iCoyoteHits % m_iCoyoteHitsForExtraLife == 0 && pPlayer->iLives < m_aStages[ m_iStage ].iMaxLives )
				{
					ShowMessagePopup( ALocaleManager::Interface()->GetString( 566 ) );
					AddLifeAtUnit( &m_aPlayers[ m_iCurrentPlayer ], pUnit );
				}
			}
			else
			{
				SendUnitBackToStartRunning( pUnit );
				AddUnitScore( pUnit, m_aiUnitTipScore[ pUnit->eType ], bBonus ? m_iMovingHitBonus : 0, pProjectile );
			}
		}

		pAnimatedUnit->GetAnimatableObject()->KillAllAnimations();
		pAnimation = &TPS8( yuk );
	}

	pAnimatedUnit->GetAnimatableObject()->PlayAnimation( *pAnimation );
	TurnUnitTowardsPlayer( TREINTERPRETCAST( TINT, pAnimatedUnit ) );

	if ( ( iStageType == Stage::STAGETYPE_JUNKYARD || iStageType == Stage::STAGETYPE_JUNKYARD_BOSS ) )
	{
		if ( pUnit->pUnit->GetAnimatableObject()->GetParentObject() != TNULL )
			DetachUnit( TREINTERPRETCAST( TINT, pUnit ) );

		if ( TUINT8( a_iUnk4 ) == 0 )
		{
			AAnimatableObject* pObject = m_pHanna;

			if ( pObject->IsAnimationPlanned( TPS8( distress_idle ) ) || pObject->IsAnimPlaying( TPS8( distress_idle ), 0.0f ) )
			{
				if ( pObject->IsAnimationPlanned( TPS8( distress_pleased ) ) || pObject->IsAnimPlaying( TPS8( distress_pleased ), 0.0f ) )
					pObject->FinishAnimation( TPS8( distress_pleased ) );

				pObject->PlayAnimation( TPS8( distress_pleased ) );
			}

			AAnimatedUnit* pDistressUnit = TREINTERPRETCAST( AAnimatedUnit*, m_pOtis );

			if ( pDistressUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( distress_idle ) ) ||
			     pDistressUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( distress_idle ), 0.0f ) )
			{
				if ( pDistressUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( distress_pleased ) ) ||
				     pDistressUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( distress_pleased ), 0.0f ) )
				{
					pDistressUnit->GetAnimatableObject()->FinishAnimation( TPS8( distress_pleased ) );
				}
				else if ( pDistressUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( distress_pleased2 ) ) || pDistressUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( distress_pleased2 ), 0.0f ) )
				{
					pDistressUnit->GetAnimatableObject()->FinishAnimation( TPS8( distress_pleased2 ) );
				}

				const TPString8& rcPleased = ARandom::GetSingleton()->m_oRandom.GetInt( 1 ) ? TPS8( distress_pleased2 ) : TPS8( distress_pleased );
				pDistressUnit->GetAnimatableObject()->PlayAnimation( rcPleased );
			}
		}
	}

	TBOOL bResult = pSteer->SetMode( AInteriorSteer::MODE_STILL );

	if ( iStageType == Stage::STAGETYPE_JUNKYARD || iStageType == Stage::STAGETYPE_JUNKYARD_BOSS || pUnit->eType == Enemy::ENEMYTYPE_COYOTE )
	{
		pUnit->eState = Enemy::ENEMYSTATE_FADING;

		TVector4 vecPosition = *pUnit->pUnit->GetSteer()->GetPosition();
		vecPosition.y -= 0.5f;

		// NOTE: retail returns the low byte of the effect handle here
		bResult = AParticleEffectManager::GetSingleton()->PlayEffect( m_iDustPuffEffect, vecPosition, TVector4::VEC_NEGY ) != TNULL;
	}

	return bResult;
}

// $Barnyard: FUNCTION 004bb1b0
TBOOL ACowTippingMiniGameState::OnProjectileHitUnit13120( TINT a_iUnk1, void* a_pUnk2, TINT a_iUnk3, TINT a_iUnk4 )
{
	// a_iUnk1: projectile, a_pUnk2: unit (m_llActiveEnemies list), a_iUnk3: distance (unused), a_iUnk4: projectile list
	Projectile* pProjectile = TREINTERPRETCAST( Projectile*, a_iUnk1 );
	TINT        iType       = pProjectile->iGrowType;

	ProjectileHitFlag( pProjectile ) = TTRUE;

	if ( iType == 2 )
		return OnUnitHit( a_pUnk2, m_iGrowType2Hits, a_iUnk1, 0 );

	if ( iType == 1 )
		return OnUnitHit( a_pUnk2, m_iGrowType1Hits, a_iUnk1, 0 );

	return OnUnitHit( a_pUnk2, m_aProjectileLists[ a_iUnk4 ].iHits, a_iUnk1, 0 );
}

// $Barnyard: FUNCTION 004bb510
void ACowTippingMiniGameState::OnProjectileHitUnit13270( Projectile* a_pProjectile, PopUpUnit* a_pUnit, TFLOAT a_fDist, TINT a_iList )
{
	ProjectileHitFlag( a_pProjectile ) = TTRUE;

	AAnimatedUnit* pAnimatedUnit = a_pUnit->pUnit;
	m_aPlayers[ m_iCurrentPlayer ].iHits += 1;
	m_aPlayers[ m_iCurrentPlayer ].iTurnHits += 1;

	TINT iHits;
	if ( a_pProjectile->iGrowType == 2 )
		iHits = m_iGrowType2Hits;
	else if ( a_pProjectile->iGrowType == 1 )
		iHits = m_iGrowType1Hits;
	else
		iHits = m_aProjectileLists[ a_iList ].iHits;

	a_pUnit->iHits += iHits;
	a_pUnit->iState = PopUpUnit::POPUPSTATE_ACTIVE;
	AddUnit13270Score( a_pUnit, m_iGopherHitScore, 0 );

	pAnimatedUnit->GetAnimatableObject()->KillAllAnimations();
	pAnimatedUnit->GetAnimatableObject()->PlayAnimation( TPS8( yuk ) );
	TurnUnitTowardsPlayer( TREINTERPRETCAST( TINT, a_pUnit->pUnit ) );

	a_pUnit->iState = PopUpUnit::POPUPSTATE_HIT;
}

// $Barnyard: FUNCTION 004bb5f0
TBOOL ACowTippingMiniGameState::TestProjectileVs13270( TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3 )
{
	// a_iUnk1: projectile list, a_iUnk2: position, a_iUnk3: projectile
	const TVector4& rcPosition  = *TREINTERPRETCAST( const TVector4*, a_iUnk2 );
	Projectile*     pProjectile = TREINTERPRETCAST( Projectile*, a_iUnk3 );

	TVector4 vecDirection;
	vecDirection.Substract( rcPosition, m_vCameraPos );
	vecDirection.Normalise();

	TFLOAT     fDist;
	PopUpUnit* pUnit = FindAimed13270( &vecDirection, &rcPosition, &fDist, m_fGopherHitRadius, m_fGopherHitDistance, TTRUE );

	if ( pUnit != TNULL )
	{
		TVector4 vecTarget;
		GetUnit13270TargetPos( &vecTarget, pUnit );
		vecTarget.y -= m_fEnemyHeight * 0.5f;

		if ( m_iGopherHitsEnabled > 0 && fDist < m_fGopherHitDistance )
		{
			OnProjectileHitUnit13270( pProjectile, pUnit, fDist, a_iUnk1 );
			PlayProjectileSplat( TREINTERPRETCAST( void**, pProjectile ), TREINTERPRETCAST( TINT, pUnit ) );
			FreeProjectile( pProjectile, a_iUnk1 );
			return TTRUE;
		}

		TVector4 vecVelocity = pProjectile->vVelocity;
		vecVelocity.Normalise();

		if ( fDist < m_fGopherNearMissDistance )
		{
			// NOTE: the result isn't used in retail
			TVector4 vecOffset;
			vecOffset.Multiply( vecVelocity, m_fGopherNearMissDistance );
			TVector4 vecHalfOffset;
			vecHalfOffset.Multiply( vecOffset, 0.5f );
			TVector4 vecPoint;
			vecPoint.Add( vecTarget, vecHalfOffset );
			TVector4::DistanceXZ( rcPosition, vecPoint );
		}
	}

	CheckNearMiss13270( &rcPosition, &fDist, m_fGopherNearMissDistanceXZ, m_fGopherNearMissDistance );
	return TFALSE;
}

// $Barnyard: FUNCTION 004bb820
TINT ACowTippingMiniGameState::OnProjectileHitCrate( Projectile* a_pProjectile, Crate* a_pUnk, TFLOAT a_fDist, TINT a_iList )
{
	ProjectileHitFlag( a_pProjectile ) = TTRUE;
	m_aPlayers[ m_iCurrentPlayer ].iHits += 1;
	m_aPlayers[ m_iCurrentPlayer ].iTurnHits += 1;

	TINT iHits;
	if ( a_pProjectile->iGrowType == 2 )
		iHits = m_iGrowType2Hits;
	else if ( a_pProjectile->iGrowType == 1 )
		iHits = m_iGrowType1Hits;
	else
		iHits = m_aProjectileLists[ a_iList ].iHits;

	a_pUnk->iHits += iHits;

	if ( a_pUnk->iHits < m_iCrateMaxHits )
		return AddCrateScore( a_pUnk, m_iCrateHitScore, 0 );

	AAnimatableObject* pObject = a_pUnk->pObject;

	if ( pObject->GetParentObject() != TNULL )
		AAnimatableObjectManager::GetSingleton()->DetachObject( pObject, pObject->GetParentObject() );

	AddCrateScore( a_pUnk, m_iCrateHitScore, 0 );

	pObject->GetModelInstance()->SetOpacity( 1.0f );
	pObject->GetModelInstance()->SetSkeletonUpdating( TTRUE );
	a_pUnk->iState = Crate::CRATESTATE_COLLECTED;

	TVector4 vecPosition;
	pObject->GetModelInstance()->GetPosition( vecPosition );

	AParticleEffectManager::GetSingleton()->PlayEffect( m_iDustPuffEffect, vecPosition, TVector4::VEC_NEGY );
	return 0;
}

// $Barnyard: FUNCTION 004bba20
TBOOL ACowTippingMiniGameState::TestProjectileVsCrates( void* a_pUnk1, TINT a_iUnk2, void** a_pUnk3 )
{
	// a_pUnk1: projectile list, a_iUnk2: position, a_pUnk3: projectile
	TINT            iList       = TREINTERPRETCAST( TINT, a_pUnk1 );
	const TVector4& rcPosition  = *TREINTERPRETCAST( const TVector4*, a_iUnk2 );
	Projectile*     pProjectile = TREINTERPRETCAST( Projectile*, a_pUnk3 );

	TVector4 vecDirection;
	vecDirection.Substract( rcPosition, m_vCameraPos );
	vecDirection.Normalise();

	TFLOAT fDist;
	Crate* pUnk = TSTATICCAST( Crate, FindAimedCrate( (TFLOAT*)&vecDirection, (TFLOAT*)&rcPosition, &fDist, m_fCrateHitRadius, m_fCrateHitDistance, BoolAsFloat( TTRUE ) ) );

	if ( pUnk != TNULL )
	{
		TVector4 vecTarget;
		GetCratePos( &vecTarget, TREINTERPRETCAST( TINT, pUnk ) );

		if ( fDist < m_fCrateHitDistance )
		{
			OnProjectileHitCrate( pProjectile, pUnk, fDist, iList );
			PlayProjectileSplatWithSound( pProjectile, pUnk );
			FreeProjectile( pProjectile, iList );
			return TTRUE;
		}
	}

	TINT iStageType = m_aStages[ m_iStage ].iStageType;

	if ( iStageType == Stage::STAGETYPE_JUNKYARD_BOSS || iStageType == Stage::STAGETYPE_JUNKYARD )
	{
		AAnimatableObject* pObject = m_pWildMikeBox;

		if ( IsModelUpdatingSkeleton( pObject ) )
		{
			TVector4 vecTarget;
			pObject->GetModelInstance()->GetPosition( vecTarget );
			vecTarget.y -= 0.3f;

			if ( IsTargetInAim( &vecDirection, (TFLOAT*)&rcPosition, (TFLOAT*)&vecTarget, *TREINTERPRETCAST( TINT*, &m_fShooterRadius ), &fDist, m_fShooterRadius, BoolAsFloat( TTRUE ) ) )
			{
				HitFarmer();
				PlayProjectileSplatWithSound( pProjectile, pUnk );
				FreeProjectile( pProjectile, iList );
			}
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 004bbc10
TBOOL ACowTippingMiniGameState::TestProjectileVsCoins( TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3 )
{
	// a_iUnk1: projectile list, a_iUnk2: position, a_iUnk3: projectile
	const TVector4& rcPosition  = *TREINTERPRETCAST( const TVector4*, a_iUnk2 );
	Projectile*     pProjectile = TREINTERPRETCAST( Projectile*, a_iUnk3 );

	TVector4 vecDirection;
	vecDirection.Substract( rcPosition, m_vCameraPos );
	vecDirection.Normalise();

	TFLOAT fDistSq;
	Coin*  pUnk = TSTATICCAST( Coin, FindAimedCoin( (TFLOAT*)&vecDirection, (TFLOAT*)&rcPosition, &fDistSq, *TREINTERPRETCAST( void**, &m_fCoinHitRadius ), *TREINTERPRETCAST( void**, &m_fCoinHitDistance ), BoolAsFloat( TTRUE ) ) );

	if ( pUnk == TNULL )
		return TFALSE;

	// NOTE: the result isn't used in retail
	TVector4 vecTarget;
	pUnk->pObject->GetModelInstance()->GetPosition( vecTarget );
	vecTarget.y -= m_fEnemyHeight * 0.5f;

	if ( fDistSq >= m_fCoinHitDistance * m_fCoinHitDistance )
		return TFALSE;

	// Same as OnHitCollectable
	ProjectileHitFlag( pProjectile ) = TTRUE;
	pUnk->iState                     = Coin::COINSTATE_COLLECTED;
	StartCollectEffect( pUnk->pObject, 1, 10.0f );

	PlayProjectileSplatWithSound( pProjectile, pUnk );
	FreeProjectile( pProjectile, a_iUnk1 );
	return TTRUE;
}

// $Barnyard: FUNCTION 004bc040
void* ACowTippingMiniGameState::SendAllUnitsBackToStart()
{
	T2_FOREACH( GetUnits13120( this ), it )
	{
		SendUnitBackToStart( TREINTERPRETCAST( TINT, it.Get() ) );
	}

	return TNULL;
}

// $Barnyard: FUNCTION 004bcb60
TINT ACowTippingMiniGameState::EndGame()
{
	if ( m_eTurnEnd != TURNEND_NONE )
		return m_eTurnEnd;

	ShowEndPopup( (void*)ALocaleManager::Interface()->GetString( 588 ) );
	m_eTurnEnd = TURNEND_GAME_END;
	SendAllUnitsBackToStart();
	m_fTurnEndTimer = m_fTurnEndDelay;

	TINT iStageType = m_aStages[ m_iStage ].iStageType;
	if ( iStageType == Stage::STAGETYPE_JUNKYARD_BOSS || iStageType == Stage::STAGETYPE_JUNKYARD )
		ASoundManager::GetSingleton()->PlayCue( soundbank::DAGBOSS_OTIS_RESCUE2 );

	m_fLevelProgress = 1.0f;
	m_oHudRoot.Hide();

	m_oRunnerIcon.SetShown( TFALSE );
	m_oElementPopup.SetShown( TFALSE );
	m_oRaccoonWarning.SetShown( TFALSE );
	return 0;
}

// $Barnyard: FUNCTION 004c16e0
TINT ACowTippingMiniGameState::RemoveObjects( TINT a_iUnk1 )
{
	if ( m_aStages[ m_iStage ].iStageType == Stage::STAGETYPE_JUNKYARD_BOSS )
	{
		SetObjectsLeft( m_iObjectsLeft - a_iUnk1 );

		if ( m_iObjectsLeft <= 0 )
			return EndGame();

		return m_iObjectsLeft;
	}

	return 0;
}

// $Barnyard: FUNCTION 004c1720
TINT ACowTippingMiniGameState::OnProjectileHitUnit13268( TINT a_iUnk1, void* a_pUnk2, TINT a_iUnk3, TINT a_iUnk4 )
{
	// a_iUnk1: projectile, a_pUnk2: unit (m_llActiveRaccoons list), a_iUnk3: distance (unused), a_iUnk4: projectile list
	Projectile* pProjectile = TREINTERPRETCAST( Projectile*, a_iUnk1 );
	PopUpUnit*  pUnit       = TSTATICCAST( PopUpUnit, a_pUnk2 );

	m_aPlayers[ m_iCurrentPlayer ].iHits += 1;
	m_aPlayers[ m_iCurrentPlayer ].iTurnHits += 1;
	ProjectileHitFlag( pProjectile ) = TTRUE;

	TINT iHits;
	if ( pProjectile->iGrowType == 2 )
		iHits = m_iGrowType2Hits;
	else if ( pProjectile->iGrowType == 1 )
		iHits = m_iGrowType1Hits;
	else
		iHits = m_aProjectileLists[ a_iUnk4 ].iHits;

	pUnit->iHits += iHits;
	pUnit->iState = PopUpUnit::POPUPSTATE_ACTIVE;
	AddUnit13268Score( pUnit, m_iRaccoonHitScore, 0 );

	AAnimatedUnit* pAnimatedUnit = pUnit->pUnit;

	if ( m_aStages[ m_iStage ].iStageType == Stage::STAGETYPE_JUNKYARD_BOSS )
	{
		AAnimatableObject* pObject = TREINTERPRETCAST( AAnimatableObject*, GetUnitHeldObject( TREINTERPRETCAST( TINT, pUnit ) ) );

		if ( AAnimatableObjectManager::GetSingleton()->GetAttachedObject( pObject, m_iSlingshotBone, 0 ) != TNULL )
		{
			pAnimatedUnit->GetAnimatableObject()->KillAllAnimations();
			pAnimatedUnit->GetAnimatableObject()->PlayAnimation( TPS8( yukobject ) );
			RemoveObjects( m_iObjectsLostOnYukObject );
		}
		else
		{
			pAnimatedUnit->GetAnimatableObject()->KillAllAnimations();
			pAnimatedUnit->GetAnimatableObject()->PlayAnimation( TPS8( yuk ) );
			RemoveObjects( m_iObjectsLostOnYuk );
		}
	}
	else
	{
		pUnit->pUnit->GetAnimatableObject()->KillAllAnimations();
		pAnimatedUnit->GetAnimatableObject()->PlayAnimation( TPS8( yuk ) );
	}

	TurnUnitTowardsPlayer( TREINTERPRETCAST( TINT, pUnit->pUnit ) );
	pUnit->iState = PopUpUnit::POPUPSTATE_HIT;

	// NOTE: AButtonSlamHudElement::StartFade takes the two floats (1.0f) as TINT for now
	m_oButtonSlamHud.StartFade( 0.0f, 0x3f800000, 0x3f800000 );

	TINT iCount = GetNumUnhit13268();
	if ( iCount == 0 )
	{
		m_oRaccoonWarning.SetShown( TFALSE );
	}

	return iCount;
}

// $Barnyard: FUNCTION 004c2f00
TBOOL ACowTippingMiniGameState::TestProjectileVsAll( TINT a_iUnk1, TFLOAT* a_pUnk2, TINT a_iUnk3 )
{
	// a_iUnk1: projectile list, a_pUnk2: position, a_iUnk3: projectile
	const TVector4* pPosition   = TREINTERPRETCAST( const TVector4*, a_pUnk2 );
	Projectile*     pProjectile = TREINTERPRETCAST( Projectile*, a_iUnk3 );

	// NOTE: retail passes the address of its own a_pUnk2 slot here and reuses it below
	TFLOAT fDist;
	ScareUnitsNearProjectile( pPosition, &fDist, m_fScareDistanceXZ, m_fEnemyNearMissDistance );

	if ( pProjectile->iGrowType == 1 )
	{
		// NOTE: not normalised
		TVector4 vecDirection;
		vecDirection.Substract( *pPosition, m_vCameraPos );

		HitTest13268( &vecDirection, pProjectile, pPosition, &fDist, AsInt( m_fRaccoonHitRadius * m_fHitTestScale ), m_fRaccoonHitDistance * m_fHitTestScale, a_iUnk1, &ACowTippingMiniGameState::OnProjectileHitUnit13268, TTRUE );
		HitTest13120( &vecDirection, pProjectile, pPosition, &fDist, AsInt( m_fEnemyHitRadius * m_fHitTestScale ), m_fEnemyHitDistance * m_fHitTestScale, a_iUnk1, &ACowTippingMiniGameState::OnProjectileHitUnit13120, TTRUE );
		HitTestCrates( &vecDirection, pProjectile, pPosition, &fDist, AsInt( m_fHitTestScale * m_fCrateHitRadius ), m_fCrateHitDistance * m_fHitTestScale, a_iUnk1, &ACowTippingMiniGameState::OnProjectileHitCrate, TTRUE );
		HitTestCoins( &vecDirection, pProjectile, pPosition, &fDist, AsInt( m_fHitTestScale * m_fCrateHitRadius ), m_fCrateHitDistance * m_fHitTestScale, a_iUnk1, &ACowTippingMiniGameState::OnHitCollectable, TTRUE );
	}

	return TFALSE;
}
