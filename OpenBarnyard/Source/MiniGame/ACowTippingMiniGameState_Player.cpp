#include "pch.h"
#include "ACowTippingMiniGameState.h"
#include "ARandom.h"
#include "Locale/ALocaleManager.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Units/AAnimatedUnit.h"
#include "Units/AUnitPlayer.h"
#include "Units/Projectiles/AProjectile.h"
#include "Units/Projectiles/AAimedSquirtProjectileLauncher.h"
#include "Units/Projectiles/ASimpleProjectileManager.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/Steering/AStationarySteer.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/dagf.h"
#include "Input/AInputHandler.h"
#include "FX/AParticleEffectManager.h"
#include "Cameras/ACameraManager.h"
#include "Render/ARenderer.h"
#include "Terrain/ATerrainInterface.h"

#include <Math/TMathInline.h>
#include <Math/TMath.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: same as the static LerpClamped3 of APhysics.cpp (006b6700)
static TVector4 LerpClamped3( TVector4 a_vStart, TVector4 a_vEnd, TFLOAT a_fProgress )
{
	TFLOAT afResult[ 3 ];

	for ( TINT i = 0; i < 3; i++ )
	{
		TFLOAT fStart = a_vStart.AsArray()[ i ];
		TFLOAT fEnd   = a_vEnd.AsArray()[ i ];
		TFLOAT fValue = ( fEnd - fStart ) * a_fProgress + fStart;

		if ( ( fStart < fEnd && fValue > fEnd ) || ( fStart > fEnd && fValue < fEnd ) )
			fValue = fEnd;

		afResult[ i ] = fValue;
	}

	return TVector4( afResult[ 0 ], afResult[ 1 ], afResult[ 2 ], a_vStart.w );
}

TPSTRING8_DECLARE( Tomatoes );
TPSTRING8_DECLARE( cb_rlocator );
TPSTRING8_DECLARE( distress_idle );
TPSTRING8_DECLARE( down );
TPSTRING8_DECLARE( draw );
TPSTRING8_DECLARE( fire );
TPSTRING8_DECLARE( getup );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( idle_lp2 );
TPSTRING8_DECLARE( reload );
TPSTRING8_DECLARE( shoot );
TPSTRING8_DECLARE( sl_idle_lp );
TPSTRING8_DECLARE( spin );
TPSTRING8_DECLARE( spin_idle );
TPSTRING8_DECLARE( splat );
TPSTRING8_DECLARE( tawnt );
TPSTRING8_DECLARE( tip_1rac_end );
TPSTRING8_DECLARE( up );
TPSTRING8_DECLARE( yuk );
TPSTRING8_DECLARE( yukobject );

// NOTE: ACowTippingMiniGameState is split over several .cpp files, this one has the [Player] part.

//-----------------------------------------------------------------------------
// NOTE: most fields of the state are still declared as provisional TINT/TFLOAT
// scalars. These macros view them as what they really are until the header gets
// the final types.
//-----------------------------------------------------------------------------
#define CT_FLOAT( FIELD )      ( *TREINTERPRETCAST( TFLOAT*, &( FIELD ) ) )
#define CT_VEC4( FIELD )       ( *TREINTERPRETCAST( Toshi::TVector4*, &( FIELD ) ) )
#define CT_PTR( TYPE, FIELD )  ( TREINTERPRETCAST( TYPE*, ( FIELD ) ) )
#define CT_LIST( FIELD )       ( *TREINTERPRETCAST( Toshi::T2GenericSList*, &( FIELD ) ) )
#define CT_AT( TYPE, OFFSET )  ( TREINTERPRETCAST( TYPE*, TREINTERPRETCAST( TUINT8*, this ) + ( OFFSET ) ) )
#define CT_UNIT( NODE, FIELD ) ( TREINTERPRETCAST( AAnimatedUnit*, ( NODE )->FIELD ) )


// NOTE: passes the bits of a float to a provisional TINT parameter of another part's method
#define CT_FLOAT_BITS( FIELD ) ( *TREINTERPRETCAST( const TINT*, &( FIELD ) ) )

static TINT CT_FloatBits( TFLOAT a_fValue )
{
	return *TREINTERPRETCAST( TINT*, &a_fValue );
}

// NOTE: inlined in retail, every function checks the type of the current stage this way
static TBOOL CT_IsStageType23( ACowTippingMiniGameState* a_pState )
{
	TINT iType = a_pState->m_aStages[ a_pState->m_iStage ].iStageType;
	return iType == ACowTippingMiniGameState::Stage::STAGETYPE_JUNKYARD_BOSS || iType == ACowTippingMiniGameState::Stage::STAGETYPE_JUNKYARD;
}

// $Barnyard: FUNCTION 004a6fe0
TBOOL ACowTippingMiniGameState::ShowShootHint()
{
	if ( !m_bShootHintShown )
	{
		// NOTE: the result is not used
		ALocaleManager::Interface()->GetString( 590 );

		m_oRaccoonWarning.m_fPulseSpeed = 10.0f;
		m_bShootHintShown               = TTRUE;
		return TFALSE;
	}

	return m_bShootHintShown;
}

// $Barnyard: FUNCTION 004a70d0
void* ACowTippingMiniGameState::FindCrateByObject( TINT a_iUnk1 )
{
	Crate( *aRecords )[ NUM_CRATES_PER_GROUP ] = m_aCrates;

	for ( TINT i = 0; i < 6; i++ )
	{
		for ( TINT k = 4; k >= 0; k-- )
		{
			if ( TREINTERPRETCAST( TINT, aRecords[ i ][ k ].pObject ) == a_iUnk1 )
				return &aRecords[ i ][ k ];
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 004a73b0
TINT ACowTippingMiniGameState::SelectAmmoList()
{
	// Picks the last projectile list that still has something in it
	m_iProjectileType = 2;

	do
	{
		if ( --m_iProjectileType < 0 )
			break;

	} while ( m_aProjectileLists[ m_iProjectileType ].iAmmo == 0 );

	return m_iProjectileType;
}

// Returns one of the vectors of an Cow record (at +0x68)
// $Barnyard: FUNCTION 004a7560
static TVector4* FUN_004a7560( TVector4* a_pOut, ACowTippingMiniGameState::Cow* a_pRecord, TINT a_iIndex )
{
	*a_pOut = TREINTERPRETCAST( TVector4*, TREINTERPRETCAST( TUINT8*, a_pRecord ) + 0x68 )[ a_iIndex ];
	return a_pOut;
}

// $Barnyard: FUNCTION 004a86d0
void ACowTippingMiniGameState::ResetCrate( void* a_pUnk1 )
{
	Crate* pRecord = TSTATICCAST( Crate, a_pUnk1 );

	AAnimatableObjectManager::GetSingleton()->DetachObject( pRecord->pObject, pRecord->pObject->GetParentObject() );
	pRecord->pObject->SetOpacity( 1.0f, TTRUE );
	pRecord->pObject->SetSkeletonUpdating( TTRUE, TTRUE );
	pRecord->iState    = Crate::CRATESTATE_FALLING;
	pRecord->fFallTime = 1.0f;
}

// $Barnyard: FUNCTION 004a8f70
TBOOL ACowTippingMiniGameState::SetAmmoCount( TINT a_iUnk1, TINT a_iUnk2 )
{
	TINT iCount = a_iUnk2;

	if ( iCount > m_iMaxAmmo )
		iCount = m_iMaxAmmo;

	if ( a_iUnk1 == m_iProjectileType && AAnimatableObjectManager::GetSingleton()->GetAttachedObject( m_pLauncher, m_iLauncherGunEndBone, 0 ) )
		iCount--;

	m_aProjectileLists[ a_iUnk1 ].iAmmo = iCount;
	return TTRUE;
}

// $Barnyard: FUNCTION 004a9460
TINT ACowTippingMiniGameState::GetAmmoCount( TINT a_iUnk1 )
{
	if ( m_iProjectileType < 0 )
		return 0;

	ProjectileList* pList  = &m_aProjectileLists[ a_iUnk1 ];
	TINT            iCount = pList->iAmmo;

	if ( iCount > m_iMaxAmmo )
		pList->iAmmo = m_iMaxAmmo;

	if ( a_iUnk1 == m_iProjectileType && AAnimatableObjectManager::GetSingleton()->GetAttachedObject( m_pLauncher, m_iLauncherGunEndBone, 0 ) )
	{
		iCount++;

		if ( pList->iAmmo == m_iMaxAmmo )
			pList->iAmmo--;
	}

	return iCount;
}

// $Barnyard: FUNCTION 004aa8c0
TBOOL ACowTippingMiniGameState::UpdateTomatoesAnim( TFLOAT a_fUnk1 )
{
	AAnimatableObject* pObject = m_pLauncher;

	// Moves the "Tomatoes" animation towards the amount of tomatoes left
	TFLOAT fCurrent = pObject->GetAnimationProgress( TPS8( Tomatoes ) );
	TINT   iLeft    = GetAmmoCount( m_iProjectileType );
	TINT   iMax     = m_iMaxAmmo;
	TFLOAT fTarget  = 0.0f;

	if ( iMax != 0 )
		fTarget = 1.0f - TFLOAT( iLeft ) / TFLOAT( iMax );

	TFLOAT fProgress = a_fUnk1 * 1.5f;
	TMath::Clip( fProgress, 0.0f, 1.0f );

	TFLOAT fValue = ( fTarget - fCurrent ) * fProgress + fCurrent;
	return pObject->SetAnimationTime( TPS8( Tomatoes ), pObject->GetAnimationDuration( TPS8( Tomatoes ) ) * fValue );
}

// $Barnyard: FUNCTION 004ab340
void ACowTippingMiniGameState::FlagAimableUnits( TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3, TINT a_iUnk4, TINT a_iUnk5 )
{
	// NOTE: the parameters are not used
	T2GenericSList& rList1 = CT_LIST( m_llActiveEnemies );

	for ( T2GenericSList::Node* pIt = rList1.Begin(); pIt != rList1.End(); pIt = pIt->Next() )
	{
		Enemy* pNode = TREINTERPRETCAST( Enemy*, pIt );

		if ( TREINTERPRETCAST( TUINT8*, pNode )[ 0x4e ] == 0 && ( pNode->eType != Enemy::ENEMYTYPE_COYOTE || pNode->iHits < 1 ) )
			CT_UNIT( pNode, pUnit )->AddFlags( AUnit::FLAGS_MILK_TARGET );
	}

	T2GenericSList& rList2 = CT_LIST( m_llActiveRaccoons );

	for ( T2GenericSList::Node* pIt = rList2.Begin(); pIt != rList2.End(); pIt = pIt->Next() )
	{
		Raccoon* pNode = TREINTERPRETCAST( Raccoon*, pIt );

		if ( TUINT8( pNode->bTargeted ) == 0 )
			CT_UNIT( pNode, pUnit )->AddFlags( AUnit::FLAGS_MILK_TARGET );
	}
}

// $Barnyard: FUNCTION 004ab580
void ACowTippingMiniGameState::RemoveFadedUnit( TINT a_iUnk1 )
{
	Gopher*        pNode = TREINTERPRETCAST( Gopher*, a_iUnk1 );
	AAnimatedUnit* pUnit = CT_UNIT( pNode, pUnit );

	pUnit->SetVisible( TFALSE );
	pUnit->ChangeSteer( &TGetClass( AStationarySteer ) );
	pUnit->SetHashed( TFALSE );
	pUnit->SetNoQuery( TTRUE );
	pNode->iHits = 0;
	CT_UNIT( pNode, pUnit )->GetAnimatableObject()->SetAnimControllersDisabled( TTRUE );

	CT_LIST( m_llActiveGophers ).Transfer( TREINTERPRETCAST( T2GenericSList::Node*, pNode ), CT_LIST( m_llFreeGophers ) );
}

// $Barnyard: FUNCTION 004ab690
TINT ACowTippingMiniGameState::FindClosestEnemyXZ( TINT a_iUnk1, TFLOAT a_fUnk2 )
{
	const TVector4& rcPosition = *TREINTERPRETCAST( TVector4*, a_iUnk1 );
	TFLOAT          fClosest   = a_fUnk2;
	Enemy*          pClosest   = TNULL;

	T2GenericSList& rList = CT_LIST( m_llActiveEnemies );

	for ( T2GenericSList::Node* pIt = rList.Begin(); pIt != rList.End(); pIt = pIt->Next() )
	{
		Enemy* pNode = TREINTERPRETCAST( Enemy*, pIt );

		TVector4 vUnitPosition = *CT_UNIT( pNode, pUnit )->GetSteer()->GetPosition();
		TFLOAT   fDistance     = TVector4::DistanceXZ( rcPosition, vUnitPosition );

		// NOTE: retail reads the table through the singleton pointer (00782efc), it's this object
		TINT  iMax  = ( pNode->eType >= Enemy::ENEMYTYPE_COYOTE ) ? 1 : m_aiUnitMaxHits[ pNode->eType ];
		TBOOL bDone = iMax <= pNode->iHits;

		if ( pNode->iNumTargeting <= m_iAmmoPerTarget && IsUnitNotYukking( TREINTERPRETCAST( TINT, pNode ) ) && !bDone && fDistance < fClosest )
		{
			fClosest = fDistance;
			pClosest = pNode;
		}
	}

	return TREINTERPRETCAST( TINT, pClosest );
}

// $Barnyard: FUNCTION 004ab9e0
void ACowTippingMiniGameState::UpdateCowTipping( TFLOAT a_fUnk1 )
{
	TRandom& rRandom = ARandom::GetSingleton()->m_oRandom;

	TFLOAT fTotalTip = 0.0f;
	TBOOL  bSpinning = TFALSE;

	for ( TINT i = 0; i < m_iNumCows; i++ )
	{
		Cow* pRecord = &m_aCows[ i ];

		if ( TUINT8( pRecord->bActive ) == 0 )
			continue;

		AAnimatableObject* pObject = pRecord->pObject;
		TFLOAT&            rfTip   = pRecord->fTipProgress;
		TFLOAT&            rfUnk1  = pRecord->fUnk1C;
		TINT&              riSpins = pRecord->iSpins;
		TFLOAT&            rfTimer = pRecord->fHitTime;

		TVector4 vUnk1;
		TVector4 vUnk2;
		TVector4 vUnk3;
		TVector4 vUnk4;
		GetCowHitPoints( pRecord, &vUnk1, &vUnk2, &vUnk3, &vUnk4 );

		if ( rfTip > m_fTipThreshold )
		{
			bSpinning = TTRUE;

			if ( riSpins >= m_iCowSpinsToSleep )
			{
				if ( !pObject->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) &&
				     !pObject->IsAnimationPlanned( TPS8( idle_lp ) ) &&
				     !pObject->IsAnimPlaying( TPS8( getup ), 0.0f ) &&
				     !pObject->IsAnimationPlanned( TPS8( getup ) ) &&
				     ( pObject->IsAnimationFinished( TPS8( tip_1rac_end ) ) ||
				       pObject->IsAnimationPlanned( TPS8( spin_idle ) ) ||
				       pObject->IsAnimationPlanned( TPS8( spin ) ) ) )
				{
					pObject->PlayAnimation( TPS8( getup ) );
					rfTip     = 0.0f;
					bSpinning = TFALSE;
				}
			}
			else
			{
				if ( m_bUnk_13409 && pObject->IsAnimationFinished( TPS8( tip_1rac_end ) ) )
				{
					m_bUnk_13409 = TFALSE;

					if ( riSpins == 0 )
					{
						AInputHandler::RumbleParams oRumble = { { m_fShootRumbleStrength, 0.0f, 0.0f, m_fShootRumbleUnk, 0.0f, 1.0f, 1.0f }, 1 };
						AInputHandler::GetSingleton()->SetRumble( AInputHandler::GetSingleton()->GetMainController(), &oRumble );
					}
				}

				if ( pObject->IsAnimPlaying( TPS8( spin_idle ), 0.0f ) && !pObject->IsAnimationPlanned( TPS8( spin ) ) )
				{
					riSpins++;
					pObject->PlayAnimation( TPS8( spin ) );
				}
			}

			rfTimer = rRandom.GetFloatMinMax( m_fCowTimerMin, m_fCowTimerMax );
			continue;
		}

		if ( rfTip > 0.0f )
		{
			fTotalTip += rfTip;

			Enemy* pEnemy1 = TSTATICCAST( Enemy, FindClosestEnemy( &vUnk1, m_fCowSearchRadius ) );
			Enemy* pEnemy2 = TSTATICCAST( Enemy, FindClosestEnemy( &vUnk2, m_fCowSearchRadius ) );

			if ( pEnemy1 == pEnemy2 )
				pEnemy2 = TNULL;

			// Is any of the enemies near the cow busy?
			TBOOL bBusy;

			if ( pEnemy1 )
			{
				AAnimatableObject* pEnemyObject = CT_UNIT( pEnemy1, pUnit )->GetAnimatableObject();

				if ( !pEnemyObject->IsAnimPlaying( TPS8( yuk ), 0.0f ) && !CT_UNIT( pEnemy1, pUnit )->GetAnimatableObject()->IsAnimPlaying( TPS8( splat ), 0.0f ) )
					bBusy = TFALSE;
				else if ( !pEnemy2 )
					bBusy = TTRUE;
				else
					bBusy = CT_UNIT( pEnemy2, pUnit )->GetAnimatableObject()->IsAnimPlaying( TPS8( yuk ), 0.0f ) || CT_UNIT( pEnemy2, pUnit )->GetAnimatableObject()->IsAnimPlaying( TPS8( splat ), 0.0f );
			}
			else if ( !pEnemy2 )
			{
				rfTip -= a_fUnk1 * m_fTipRecoverSpeedFree;
				bBusy = TTRUE;
			}
			else
			{
				bBusy = CT_UNIT( pEnemy2, pUnit )->GetAnimatableObject()->IsAnimPlaying( TPS8( yuk ), 0.0f ) || CT_UNIT( pEnemy2, pUnit )->GetAnimatableObject()->IsAnimPlaying( TPS8( splat ), 0.0f );
			}

			if ( bBusy )
			{
				if ( !pObject->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) &&
				     !pObject->IsAnimationPlanned( TPS8( idle_lp ) ) &&
				     !pObject->IsAnimationPlanned( TPS8( getup ) ) &&
				     !pObject->IsAnimPlaying( TPS8( getup ), 0.0f ) )
				{
					pObject->PlayAnimation( TPS8( idle_lp ) );
				}
			}
			else
			{
				rfTip -= a_fUnk1 * m_fTipRecoverSpeed;
			}

			if ( rfTip < 0.0f )
				rfTip = 0.0f;

			rfUnk1 -= a_fUnk1;
			rfTimer = rRandom.GetFloatMinMax( m_fCowTimerMin, m_fCowTimerMax );
			continue;
		}

		Enemy* pEnemy1 = TSTATICCAST( Enemy, FindClosestEnemy( &vUnk1, m_fCowSearchRadius ) );
		Enemy* pEnemy2 = TSTATICCAST( Enemy, FindClosestEnemy( &vUnk2, m_fCowSearchRadius ) );
		TBOOL  bBusy   = TFALSE;

		if ( pEnemy1 && ( pEnemy1->eState == Enemy::ENEMYSTATE_PUSHING || pEnemy1->eState == Enemy::ENEMYSTATE_PUSH_END ) )
			bBusy = TTRUE;

		if ( pEnemy2 && ( pEnemy2->eState == Enemy::ENEMYSTATE_PUSHING || pEnemy2->eState == Enemy::ENEMYSTATE_PUSH_END ) )
			bBusy = TTRUE;

		rfTimer -= a_fUnk1;

		if ( !bBusy )
		{
			if ( !pObject->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) &&
			     !pObject->IsAnimationPlanned( TPS8( idle_lp ) ) &&
			     !pObject->IsAnimationPlanned( TPS8( getup ) ) &&
			     !pObject->IsAnimPlaying( TPS8( getup ), 0.0f ) )
			{
				pObject->PlayAnimation( TPS8( idle_lp ) );
			}

			if ( !CT_IsStageType23( this ) && rfTimer < 0.0f )
			{
				TVector4 vPosition;
				FUN_004a7560( &vPosition, pRecord, 2 );
				vPosition.y -= 0.5f;

				AParticleEffectManager::GetSingleton()->PlayEffect( m_iCowZzzzEffect, vPosition, TVector4::VEC_NEGY );
				rfTimer = rRandom.GetFloatMinMax( m_fCowTimerMin, m_fCowTimerMax );
			}
		}
	}

	if ( bSpinning )
	{
		m_oLives2Hud.SetWobble( 0.0f );
		return;
	}

	T2GUITextureSection** ppFrames1 = CT_PTR( T2GUITextureSection*, m_ppCowSpinSprites );
	T2GUITextureSection** ppFrames3 = CT_PTR( T2GUITextureSection*, m_ppCowShockSprites );
	T2GUITextureSection** ppFrames4 = CT_PTR( T2GUITextureSection*, m_ppCowSleepSprites );
	T2GUITextureSection** ppFrames5 = CT_PTR( T2GUITextureSection*, m_ppCowShockedSprites );

	if ( m_oLives2Hud.IsPlayingFrameAnimation( ppFrames1 ) )
	{
		m_oCowShockMarks.SetShown( TFALSE );

		if ( 0.0f == fTotalTip )
			m_oLives2Hud.SetFrameAnimations( TNULL, ppFrames4, 0, m_iNumCowSleepSprites, 5.0f, 5.0f );
		else
			m_oLives2Hud.SetFrameAnimations( TNULL, ppFrames3, 0, m_iNumCowFallSprites, 5.0f, 5.0f );
	}
	else if ( !m_oLives2Hud.IsPlayingFrameAnimation( ppFrames3 ) && !m_oLives2Hud.IsPlayingFrameAnimation( ppFrames4 ) )
	{
		if ( 0.0f == fTotalTip )
		{
			m_oLives2Hud.SetFrameAnimation( TNULL, 0, 1.0f );
			m_oCowShockMarks.SetShown( TFALSE );
		}
		else
		{
			m_oLives2Hud.SetFrameAnimation( ppFrames5, m_iNumCowShockedSprites, 0.0f );
			m_oCowShockMarks.SetShown( TTRUE );
		}
	}
}

// $Barnyard: FUNCTION 004ace80
void* ACowTippingMiniGameState::FindProjectileByObject( void* a_pUnk1 )
{
	for ( TINT i = 0; i < 2; i++ )
	{
		T2GenericSList& rList = CT_LIST( m_aProjectileLists[ i ].llHeld );

		for ( T2GenericSList::Node* pIt = rList.Begin(); pIt != rList.End(); pIt = pIt->Next() )
		{
			if ( TREINTERPRETCAST( Projectile*, pIt )->pObject == a_pUnk1 )
				return pIt;
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 004acee0
void* ACowTippingMiniGameState::FindProjectileByObjectEx( void* a_pUnk1, void* a_pUnk2 )
{
	for ( TINT i = 0; i < 2; i++ )
	{
		T2GenericSList& rList = CT_LIST( m_aProjectileLists[ i ].llHeld );

		for ( T2GenericSList::Node* pIt = rList.Begin(); pIt != rList.End(); pIt = pIt->Next() )
		{
			if ( TREINTERPRETCAST( Projectile*, pIt )->pObject == a_pUnk1 )
			{
				*TSTATICCAST( TINT, a_pUnk2 ) = i;
				return pIt;
			}
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 004ae5a0
void ACowTippingMiniGameState::DropKidTomato( TINT a_iUnk1 )
{
	Raccoon*           pNode   = TREINTERPRETCAST( Raccoon*, a_iUnk1 );
	AAnimatableObject* pParent = CT_UNIT( pNode, pUnit )->GetAnimatableObject();

	if ( !CT_IsStageType23( this ) )
		pParent = pParent->FindAttachedObject( m_iRaccoonHandBone );

	AAnimatableObject* pAttached = AAnimatableObjectManager::GetSingleton()->GetAttachedObject( pParent, m_iSlingshotBone, 0 );

	if ( pAttached )
	{
		TINT  iList       = a_iUnk1;
		void* pProjectile = FindProjectileByObjectEx( pAttached, &iList );
		AAnimatableObjectManager::GetSingleton()->DetachObject( pAttached, pParent );

		if ( pProjectile )
			FreeProjectile( pProjectile, iList );
	}
}

// $Barnyard: FUNCTION 004aecf0
void ACowTippingMiniGameState::RemoveKid( TINT a_iUnk1 )
{
	Raccoon* pNode = TREINTERPRETCAST( Raccoon*, a_iUnk1 );
	TUINT    uiId  = TUINT( pNode->iSpot );

	pNode->iState = PopUpUnit::POPUPSTATE_INACTIVE;
	DropKidTomato( a_iUnk1 );
	CT_UNIT( pNode, pUnit )->ChangeSteer( &TGetClass( AStationarySteer ) );
	pNode->iHits = 0;
	CT_LIST( m_llActiveRaccoons ).Transfer( TREINTERPRETCAST( T2GenericSList::Node*, pNode ), CT_LIST( m_llFreeRaccoons ) );

	TREINTERPRETCAST( TUINT8*, m_auiUsedRaccoonBits )
	[uiId >> 3] &= ~( 1 << ( uiId & 7 ) );

	CT_UNIT( pNode, pUnit )->GetAnimatableObject()->KillAllAnimations();
	CT_UNIT( pNode, pUnit )->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );
	CT_UNIT( pNode, pUnit )->RemoveFlags( AUnit::FLAGS_MILK_TARGET );
	CT_UNIT( pNode, pUnit )->GetAnimatableObject()->SetAnimControllersDisabled( TTRUE );

	if ( GetNumUnhit13268() == 0 )
		m_oRaccoonWarning.SetShown( TFALSE );

	if ( CT_IsStageType23( this ) )
	{
		CT_UNIT( pNode, pUnit )->SetVisible( TFALSE );
		m_oButtonSlamHud.StartFade( 0.1f, 1.0f, -1.0f );
	}
}

// $Barnyard: FUNCTION 004aef30
void ACowTippingMiniGameState::UpdateFadingUnits( TFLOAT a_fUnk1 )
{
	T2GenericSList&       rList = CT_LIST( m_llActiveGophers );
	T2GenericSList::Node* pIt   = rList.Begin();

	while ( pIt != rList.End() )
	{
		Gopher* pNode = TREINTERPRETCAST( Gopher*, pIt );
		pIt           = pIt->Next();

		AAnimatedUnit* pUnit = CT_UNIT( pNode, pUnit );

		// NOTE: the result is not used
		pUnit->GetSteer()->GetPosition();

		switch ( pNode->iState )
		{
			case PopUpUnit::POPUPSTATE_INACTIVE:
				break;
			case PopUpUnit::POPUPSTATE_LEAVING:
			{
				// Fading out
				TFLOAT fOpacity = CT_UNIT( pNode, pUnit )->GetAnimatableObject()->GetModelInstance()->GetOpacity();
				fOpacity -= a_fUnk1 * m_fFadeSpeed;

				if ( fOpacity >= 0.0f )
				{
					CT_UNIT( pNode, pUnit )->GetAnimatableObject()->GetModelInstance()->SetOpacity( fOpacity );
				}
				else
				{
					pNode->iState = PopUpUnit::POPUPSTATE_INACTIVE;
					RemoveFadedUnit( TREINTERPRETCAST( TINT, pNode ) );
					CT_UNIT( pNode, pUnit )->GetAnimatableObject()->GetModelInstance()->SetOpacity( 1.0f );
					m_fGopherTimer = m_fGopherDelay;
				}

				break;
			}
			case PopUpUnit::POPUPSTATE_HIT:
			{
				if ( !pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( yuk ), 0.0f ) &&
				     !pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( yuk ) ) &&
				     !pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( down ), 0.0f ) &&
				     !pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( down ) ) )
				{
					pNode->iState = PopUpUnit::POPUPSTATE_LEAVING;
				}

				break;
			}
			case PopUpUnit::POPUPSTATE_DRAWING:
			{
				AAnimatableObject* pObject = CT_UNIT( pNode, pUnit )->GetAnimatableObject();

				AToshiAnimationInterface& rAnimInterface = pObject->GetToshiAnimInterface();
				rAnimInterface.SetFlags( rAnimInterface.GetFlags() | AToshiAnimationInterface::FLAGS_ALWAYS_UPDATE );

				TSkeletonInstance* pSkeletonInstance = pObject->GetModelInstance()->GetSkeletonInstance();
				pSkeletonInstance->SetFlags( pSkeletonInstance->GetFlags() | TSkeletonInstance::FLAGS_UPDATE_STATE );

				if ( pObject->IsAnimPlaying( TPS8( fire ), 0.0f ) == TTRUE )
				{
					pNode->iState = PopUpUnit::POPUPSTATE_HIT;
					pUnit->GetAnimatableObject()->PlayAnimation( TPS8( down ) );

					AAnimatableObject* pAttached = AAnimatableObjectManager::GetSingleton()->GetAttachedObjectByBoneName( CT_UNIT( pNode, pUnit )->GetAnimatableObject(), TPS8( cb_rlocator ), 0 );

					if ( pAttached )
						ResetCrate( FindCrateByObject( TREINTERPRETCAST( TINT, pAttached ) ) );
				}

				break;
			}
			default:
				if ( pUnit->GetAnimatableObject()->IsAnimationFinished( TPS8( up ) ) )
				{
					CT_UNIT( pNode, pUnit )->GetAnimatableObject()->PlayAnimation( TPS8( draw ) );
					pNode->iState = PopUpUnit::POPUPSTATE_DRAWING;
				}

				break;
		}
	}
}

// $Barnyard: FUNCTION 004af7f0
void ACowTippingMiniGameState::Reload()
{
	TINT iMax = m_iMaxAmmo;

	if ( GetAmmoCount( m_iProjectileType ) != iMax && SetAmmoCount( 0, iMax ) )
	{
		if ( m_iProjectileType < 0 )
			SelectAmmoList();

		UpdateTomatoesAnim( 1000.0f );
	}
}

// $Barnyard: FUNCTION 004af840
void ACowTippingMiniGameState::UpdateReload()
{
	if ( m_iReloadState == RELOADSTATE_RELOADING )
	{
		AAnimatableObject* pObject = m_pLauncher;

		if ( pObject->IsAnimPlaying( TPS8( reload ), 0.0f ) && pObject->GetAnimationTime( TPS8( reload ) ) > 1.8f )
		{
			Reload();
		}
		else if ( pObject->IsAnimationFinished( TPS8( reload ) ) )
		{
			Reload();
			m_iReloadState = RELOADSTATE_NONE;
		}
	}
}

// $Barnyard: FUNCTION 004af920
void ACowTippingMiniGameState::LoadKidSlingshot( void** a_pUnk1 )
{
	Raccoon*           pNode   = TREINTERPRETCAST( Raccoon*, a_pUnk1 );
	AAnimatableObject* pParent = CT_UNIT( pNode, pUnit )->GetAnimatableObject();

	if ( !CT_IsStageType23( this ) )
		pParent = pParent->FindAttachedObject( m_iRaccoonHandBone );

	if ( !AAnimatableObjectManager::GetSingleton()->GetAttachedObject( pParent, m_iSlingshotBone, 0 ) )
	{
		// Puts a new tomato into the slingshot
		Projectile* pProjectile = TREINTERPRETCAST( Projectile*, CT_LIST( m_aProjectileLists[ 1 ].llFree ).PopFront() );
		CT_LIST( m_aProjectileLists[ 1 ].llHeld ).PushBack( TREINTERPRETCAST( T2GenericSList::Node*, pProjectile ) );

		TQuaternion        qRotation = TQuaternion::IDENTITY;
		AAnimatableObject* pObject   = pProjectile->pObject;

		pObject->GetModelInstance()->SetTranslation( m_vPlayerPos.AsVector3() );
		AAnimatableObjectManager::GetSingleton()->AttachObject( pObject, pParent, 0, m_iSlingshotBone, TVector4::VEC_ZERO, qRotation, 1.0f );
		pObject->SetSkeletonUpdating( TTRUE, TTRUE );
		pObject->SetVisible( TTRUE );
	}
}

// $Barnyard: FUNCTION 004afcf0
void ACowTippingMiniGameState::GetSlingshotBone( void* a_pUnk1, TFLOAT* a_pUnk2 )
{
	// Direction the slingshot points at and the position of its bone
	TVector4* pDirection = TREINTERPRETCAST( TVector4*, a_pUnk2 );
	*pDirection          = m_vAimDirection;
	pDirection->x        = -pDirection->x;
	pDirection->y        = -pDirection->y;
	pDirection->z        = -pDirection->z;

	TMatrix44 mBone;
	m_pLauncher->GetModelInstance()->GetBoneTransform( m_iLauncherGunEndBone, mBone );

	*TSTATICCAST( TVector4, a_pUnk1 ) = mBone.AsBasisVector4( BASISVECTOR_TRANSLATION );
}

// $Barnyard: FUNCTION 004b33e0
TINT ACowTippingMiniGameState::FindEnemyNearRay( TFLOAT* a_pUnk1, TFLOAT* a_pUnk2, TINT a_iUnk3, TFLOAT a_fUnk4 )
{
	// Finds the enemy (Enemy) that is the closest to the aiming ray
	const TVector4& rcOrigin = *TREINTERPRETCAST( TVector4*, a_pUnk1 );
	TFLOAT          fClosest = a_fUnk4;
	Enemy*          pFound   = TNULL;
	T2GenericSList& rList    = CT_LIST( m_llActiveEnemies );

	for ( T2GenericSList::Node* pIt = rList.Begin(); pIt != rList.End(); pIt = pIt->Next() )
	{
		Enemy* pNode = TREINTERPRETCAST( Enemy*, pIt );

		if ( pNode->eType == Enemy::ENEMYTYPE_COYOTE && pNode->iHits >= 1 )
			continue;

		TVector4 vPosition;
		GetUnitTargetPos( &vPosition, TREINTERPRETCAST( TINT, pNode ) );

		TVector4 vDirection;
		vDirection.x = rcOrigin.x - m_vCameraPos.x;
		vDirection.y = rcOrigin.y - CT_FLOAT( m_vCameraPos.y );
		vDirection.z = rcOrigin.z - CT_FLOAT( m_vCameraPos.z );
		vDirection.w = rcOrigin.w;
		vDirection.Normalise();

		TVector4 vOffset;
		vOffset.Multiply( vDirection, 1000.0f );

		TVector4 vRayEnd;
		vRayEnd.x = vOffset.x + rcOrigin.x;
		vRayEnd.y = vOffset.y + rcOrigin.y;
		vRayEnd.z = vOffset.z + rcOrigin.z;
		vRayEnd.w = rcOrigin.w;

		TVector4 vRayStart = rcOrigin;
		vOffset.Multiply( vDirection, 2.0f );
		vRayStart.x -= vOffset.x;
		vRayStart.y -= vOffset.y;
		vRayStart.z -= vOffset.z;

		// Where the enemy is going to be soon
		TVector4 vVelocity = CT_UNIT( pNode, pUnit )->GetSteer()->GetVelocity();
		vVelocity.y        = 0.0f;
		TFLOAT fSpeed      = vVelocity.NormaliseMag() * 2.0f;

		if ( fSpeed > 1.0f )
			vVelocity.Multiply( fSpeed );

		TFLOAT fScale = ( pNode->eType == Enemy::ENEMYTYPE_COYOTE ) ? 3.0f : 1.0f;
		vVelocity.y *= 0.2f;

		TVector4 vSegmentStart = vPosition;
		vOffset.Multiply( vVelocity, fScale );

		TVector4 vSegmentEnd;
		vSegmentEnd.x = vOffset.x + vPosition.x;
		vSegmentEnd.y = vOffset.y + vPosition.y;
		vSegmentEnd.z = vOffset.z + vPosition.z;
		vSegmentEnd.w = vPosition.w;

		TFLOAT fDistance = TMath::DistanceSegmentToSegmentSq( vSegmentStart.AsVector3(), vSegmentEnd.AsVector3(), vRayStart.AsVector3(), vRayEnd.AsVector3() );

		if ( fDistance < fClosest )
		{
			if ( IsUnitNotYukking( TREINTERPRETCAST( TINT, pNode ) ) )
			{
				fClosest = fDistance;
				*a_pUnk2 = fDistance;
				pFound   = pNode;
			}
			else if ( fClosest == a_fUnk4 )
			{
				*a_pUnk2 = fDistance;
				pFound   = pNode;
			}
		}
	}

	*a_pUnk2 += 0.3f;
	return TREINTERPRETCAST( TINT, pFound );
}

// $Barnyard: FUNCTION 004b37e0
TINT ACowTippingMiniGameState::FindKidNearRay( TINT a_iUnk1, TFLOAT* a_pUnk2, TINT a_iUnk3, TFLOAT a_fUnk4 )
{
	// Finds the slingshot kid (Raccoon) that is the closest to the aiming ray
	const TVector4& rcOrigin = *TREINTERPRETCAST( TVector4*, a_iUnk1 );
	TFLOAT          fClosest = a_fUnk4;
	Raccoon*        pFound   = TNULL;
	T2GenericSList& rList    = CT_LIST( m_llActiveRaccoons );

	for ( T2GenericSList::Node* pIt = rList.Begin(); pIt != rList.End(); pIt = pIt->Next() )
	{
		Raccoon* pNode = TREINTERPRETCAST( Raccoon*, pIt );

		TVector4 vPosition;
		GetUnit13268TargetPos( TREINTERPRETCAST( TINT, &vPosition ), TREINTERPRETCAST( void**, pNode ) );

		TVector4 vDirection;
		vDirection.x = rcOrigin.x - m_vCameraPos.x;
		vDirection.y = rcOrigin.y - CT_FLOAT( m_vCameraPos.y );
		vDirection.z = rcOrigin.z - CT_FLOAT( m_vCameraPos.z );
		vDirection.w = 1.0f;
		vDirection.Normalise();

		TVector4 vOffset;
		vOffset.Multiply( vDirection, 1000.0f );

		TVector4 vRayEnd;
		vRayEnd.x = vOffset.x + rcOrigin.x;
		vRayEnd.y = vOffset.y + rcOrigin.y;
		vRayEnd.z = vOffset.z + rcOrigin.z;
		vRayEnd.w = rcOrigin.w;

		TVector4 vRayStart = rcOrigin;
		vOffset.Multiply( vDirection, 2.0f );
		vRayStart.x -= vOffset.x;
		vRayStart.y -= vOffset.y;
		vRayStart.z -= vOffset.z;

		TFLOAT fT;
		TFLOAT fDistance = TMath::Sqrt( TMath::DistancePointToSegmentSq( vPosition, vRayStart, vRayEnd, fT ) );

		if ( fDistance < fClosest )
		{
			AAnimatedUnit* pUnit = CT_UNIT( pNode, pUnit );

			if ( CT_IsStageType23( this ) )
			{
				if ( !pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( yuk ), 0.0f ) &&
				     !pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( yuk ) ) &&
				     !pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( yukobject ), 0.0f ) &&
				     !pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( yukobject ) ) )
				{
					pFound   = pNode;
					fClosest = fDistance;
					*a_pUnk2 = fDistance;
				}
			}
			else if ( !pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( yuk ), 0.0f ) && !pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( yuk ) ) )
			{
				pFound   = pNode;
				fClosest = fDistance;
				*a_pUnk2 = fDistance;
			}
		}
	}

	return TREINTERPRETCAST( TINT, pFound );
}

// $Barnyard: FUNCTION 004b3ad0
void* ACowTippingMiniGameState::FindCrateNearRay( TINT a_iUnk1, TFLOAT* a_pUnk2, TINT a_iUnk3, TFLOAT a_fUnk4 )
{
	if ( CT_IsStageType23( this ) )
		return TNULL;

	const TVector4& rcOrigin = *TREINTERPRETCAST( TVector4*, a_iUnk1 );
	Crate*          pFound   = TNULL;

	Crate( *aRecords )[ NUM_CRATES_PER_GROUP ] = m_aCrates;

	for ( TINT i = 0; i < 6; i++ )
	{
		for ( TINT k = 4; k >= 0; k-- )
		{
			Crate* pRecord = &aRecords[ i ][ k ];
			TINT   iState  = pRecord->iState;

			if ( iState == Crate::CRATESTATE_ON_GROUND || iState == Crate::CRATESTATE_FALLING || iState == Crate::CRATESTATE_CARRIED )
			{
				TVector4 vPosition;
				GetCratePos( &vPosition, TREINTERPRETCAST( TINT, pRecord ) );

				TVector4 vDirection;
				vDirection.x = rcOrigin.x - m_vCameraPos.x;
				vDirection.y = rcOrigin.y - CT_FLOAT( m_vCameraPos.y );
				vDirection.z = rcOrigin.z - CT_FLOAT( m_vCameraPos.z );
				vDirection.w = 1.0f;
				vDirection.Normalise();

				TVector4 vOffset;
				vOffset.Multiply( vDirection, 1000.0f );

				TVector4 vRayEnd;
				vRayEnd.x = vOffset.x + rcOrigin.x;
				vRayEnd.y = vOffset.y + rcOrigin.y;
				vRayEnd.z = vOffset.z + rcOrigin.z;
				vRayEnd.w = rcOrigin.w;

				TVector4 vRayStart = rcOrigin;
				vOffset.Multiply( vDirection, 2.0f );
				vRayStart.x -= vOffset.x;
				vRayStart.y -= vOffset.y;
				vRayStart.z -= vOffset.z;

				TFLOAT fT;
				TFLOAT fDistance = TMath::Sqrt( TMath::DistancePointToSegmentSq( vPosition, vRayStart, vRayEnd, fT ) );

				if ( fDistance < a_fUnk4 )
				{
					a_fUnk4  = fDistance;
					*a_pUnk2 = fDistance;
					pFound   = pRecord;
				}
			}
		}
	}

	return pFound;
}

// $Barnyard: FUNCTION 004b3d70
void* ACowTippingMiniGameState::FindObjectNearRay( TINT a_iUnk1, TFLOAT* a_pUnk2, TINT a_iUnk3, TFLOAT a_fUnk4 )
{
	if ( CT_IsStageType23( this ) )
		return TNULL;

	const TVector4& rcOrigin = *TREINTERPRETCAST( TVector4*, a_iUnk1 );
	TFLOAT          fClosest = a_fUnk4;
	Coin*           pFound   = TNULL;
	Coin*           aRecords = m_aCoins;

	for ( TINT i = 7; i >= 0; i-- )
	{
		Coin*           pRecord        = &aRecords[ i ];
		AModelInstance* pModelInstance = pRecord->pObject->GetModelInstance();

		if ( pModelInstance && pModelInstance->IsUpdatingSkeleton() && pRecord->iState != Coin::COINSTATE_COLLECTED )
		{
			TVector4 vPosition;
			pRecord->pObject->GetModelInstance()->GetPosition( vPosition );

			TVector4 vDirection;
			vDirection.x = rcOrigin.x - m_vCameraPos.x;
			vDirection.y = rcOrigin.y - CT_FLOAT( m_vCameraPos.y );
			vDirection.z = rcOrigin.z - CT_FLOAT( m_vCameraPos.z );
			vDirection.w = 1.0f;
			vDirection.Normalise();

			TVector4 vOffset;
			vOffset.Multiply( vDirection, 1000.0f );

			TVector4 vRayEnd;
			vRayEnd.x = vOffset.x + rcOrigin.x;
			vRayEnd.y = vOffset.y + rcOrigin.y;
			vRayEnd.z = vOffset.z + rcOrigin.z;
			vRayEnd.w = rcOrigin.w;

			TVector4 vRayStart = rcOrigin;
			vOffset.Multiply( vDirection, 2.0f );
			vRayStart.x -= vOffset.x;
			vRayStart.y -= vOffset.y;
			vRayStart.z -= vOffset.z;

			TFLOAT fT;
			TFLOAT fDistance = TMath::Sqrt( TMath::DistancePointToSegmentSq( vPosition, vRayStart, vRayEnd, fT ) );

			if ( fDistance < fClosest )
			{
				fClosest = fDistance;
				*a_pUnk2 = fDistance;
				pFound   = pRecord;
			}
		}
	}

	return pFound;
}

// $Barnyard: FUNCTION 004b6170
void ACowTippingMiniGameState::UpdateAiming( TFLOAT a_fUnk1, TINT a_iUnk2 )
{
	// Aiming: places the target reticle and finds what the player aims at
	const CameraSettings& rCamera  = m_aCameraSettings[ m_aStages[ m_iStage ].iCameraSettings ];
	TVector4&             rvTarget = m_vAimTarget;

	m_pLockEnemy   = 0;
	m_pLockRaccoon = 0;

	TVector4 vTargetOffset;
	vTargetOffset.Multiply( TVector4::VEC_NEGY, m_fTargetHeight );

	TMath::Clip( m_fCameraStartPitch, rCamera.fMinStartPitch, rCamera.fMaxStartPitch );

	TVector4 vBonePosition;
	TVector4 vDirection;
	GetSlingshotBone( &vBonePosition, &vDirection.x );

	TVector4 vCameraPosition = m_vCameraPos;
	vCameraPosition.y        = m_vCameraLocatorPos.y;

	// The pitch input aims up and down: it replaces the height of the slingshot direction
	vDirection.y = CT_FLOAT( m_vCameraLocatorDir.y ) - ( rCamera.fUnk58 + m_fCameraStartPitch );
	vDirection.Normalise();

	// The reticle is 10 units in front of the slingshot
	TVector4 vOffset;
	vOffset.Multiply( vDirection, 10.0f );

	TVector4 vReticle;
	vReticle.x = vOffset.x + vCameraPosition.x;
	vReticle.y = vOffset.y + vCameraPosition.y;
	vReticle.z = vOffset.z + vCameraPosition.z;
	vReticle.w = vCameraPosition.w;
	vReticle.x = vTargetOffset.x + vReticle.x;
	vReticle.y = vTargetOffset.y + vReticle.y;
	vReticle.z = vTargetOffset.z + vReticle.z;

	TVector4 vScreen = vReticle;
	TVector4 vGUI    = vReticle;
	ARenderer::GetSingleton()->WorldToScreen( vScreen );
	ARenderer::GetSingleton()->WorldToGUI( vGUI );

	ATargetReticle* pReticle = &m_oTargetReticle;

	// NOTE: the result is not used
	TVector2 vReticlePosition;
	pReticle->GetPosition( vReticlePosition );
	pReticle->SetPosition( vGUI.x, vGUI.y );
	pReticle->Update( 0.0f );

	TFLOAT fRayLength = 1000.0f;
	TBOOL  bHasTarget = TFALSE;
	TBOOL  bCanShoot  = TTRUE;
	ClearUnitsFlag9();

	TVector2 vScreenPosition;
	vScreenPosition.x = vScreen.x;
	vScreenPosition.y = vScreen.y + m_fReticleOffsetY;
	m_pLockUnit       = 0;

	TVector4 vRayOrigin;
	TVector4 vRayDirection;
	ARenderer::GetSingleton()->GetScreenRay( ACameraManager::GetSingleton()->GetCurrentCamera(), vScreenPosition, vRayOrigin, vRayDirection );

	vOffset.Multiply( vRayDirection, 0.5f );
	vRayOrigin.x = vOffset.x + vRayOrigin.x;
	vRayOrigin.y = vOffset.y + vRayOrigin.y;
	vRayOrigin.z = vOffset.z + vRayOrigin.z;

	vOffset.Multiply( vRayDirection, 12.0f );
	TVector4& rvUnk_0112C = m_vReticleTarget;
	rvUnk_0112C.x         = vOffset.x + vRayOrigin.x;
	rvUnk_0112C.y         = vOffset.y + vRayOrigin.y;
	rvUnk_0112C.z         = vOffset.z + vRayOrigin.z;
	rvUnk_0112C.w         = vRayOrigin.w;

	TFLOAT fRadiusScale = 1.0f;

	if ( m_aPlayers[ m_iCurrentPlayer ].aPowerUps[ 2 ].fTime > 0.0f )
		fRadiusScale = m_fAimLockRadiusScale;

	ATerrainInterface::GetSingleton()->CollideRay( vRayOrigin, vRayDirection, fRayLength, 0, TTRUE );

	if ( !a_iUnk2 )
	{
		vOffset.Multiply( vRayDirection, fRayLength );
		rvTarget.x = vOffset.x + vRayOrigin.x;
		rvTarget.y = vOffset.y + vRayOrigin.y;
		rvTarget.z = vOffset.z + vRayOrigin.z;
		rvTarget.w = vRayOrigin.w;
	}

	TFLOAT fUnk          = m_fCameraStartPitch;
	m_fCameraTargetPitch = fUnk * rCamera.fTargetPitchScale;

	if ( !m_bSideStepping )
		m_fCameraPitchDelta = fUnk * rCamera.fUnk04;

	TBOOL bLocked = TFALSE;

	if ( !a_iUnk2 )
	{
		TFLOAT fDistance  = 1000.0f;
		void*  pLockKid   = TNULL;
		TINT   iLockUnit  = 0;
		void*  pLockEnemy = TNULL;

		// Kids that are drawing their slingshot are locked on directly
		void* pKid = FindHitUnk13268( TREINTERPRETCAST( const TVector4*, &vRayDirection.x ), TREINTERPRETCAST( const TVector4*, &rvTarget.x ), &fDistance, fRadiusScale * m_fAimLockRadius, fRadiusScale * m_fAimLockDistance, 0.0f );

		if ( pKid )
		{
			pLockKid  = pKid;
			iLockUnit = TSTATICCAST( TINT, pKid )[ 1 ];
		}
		else
		{
			pLockEnemy = FindHitEnemy( TREINTERPRETCAST( const TVector4*, &vRayDirection.x ), TREINTERPRETCAST( const TVector4*, &rvTarget.x ), &fDistance, fRadiusScale * m_fAimLockRadius, fDistance, 0.0f );

			if ( m_aPlayers[ m_iCurrentPlayer ].aPowerUps[ 2 ].fTime <= 0.0f )
			{
				if ( FindAimedCrate( &vRayDirection.x, &rvTarget.x, &fDistance, m_fAimLockRadius, m_fAimLockDistance, 0.0f ) )
				{
					bHasTarget = TTRUE;
				}
				else
				{
					TBOOL bCheckOther = TTRUE;

					if ( CT_IsStageType23( this ) )
					{
						AAnimatableObject* pObject        = m_pWildMikeBox;
						AModelInstance*    pModelInstance = pObject->GetModelInstance();

						if ( pModelInstance && pModelInstance->IsUpdatingSkeleton() && m_eRunnerState == RUNNERSTATE_PIPE_UP )
						{
							TVector4 vObjectPosition;
							pObject->GetModelInstance()->GetPosition( vObjectPosition );
							vObjectPosition.y -= 0.3f;

							if ( IsTargetInAim( &vRayDirection, &rvTarget.x, &vObjectPosition.x, CT_FLOAT_BITS( m_fShooterRadius ), &fDistance, m_fShooterRadius, TTRUE ) )
							{
								bHasTarget = TTRUE;

								// NOTE: the result is not used
								TVector4::Distance( vRayOrigin, vBonePosition );
								bCheckOther = TFALSE;
							}
						}
					}

					if ( bCheckOther && FindAimedCoin( &vRayDirection.x, &rvTarget.x, &fDistance, *TREINTERPRETCAST( void**, &m_fCoinHitDistance ), *TREINTERPRETCAST( void**, &m_fCoinNearRayRadius ), 0.0f ) )
						bHasTarget = TTRUE;
				}
			}

			if ( pLockEnemy )
			{
				Enemy* pEnemy = TSTATICCAST( Enemy, pLockEnemy );

				if ( !bHasTarget )
					iLockUnit = TREINTERPRETCAST( TINT, pEnemy->pUnit );

				// NOTE: retail reads the table through the singleton pointer (00782efc), it's this object
				TINT iMax = ( pEnemy->eType >= Enemy::ENEMYTYPE_COYOTE ) ? 1 : m_aiUnitMaxHits[ pEnemy->eType ];

				if ( iMax <= pEnemy->iHits )
					bCanShoot = TFALSE;
			}
		}

		if ( iLockUnit )
		{
			// Locked on a target: the reticle sticks to it
			TVector4 vOldTarget = rvTarget;
			bLocked             = TTRUE;

			TVector4 vLockPosition;
			TINT     iLockType;

			if ( pLockKid )
			{
				vLockPosition = *TREINTERPRETCAST( TVector4*, GetUnit13268TargetPos( TREINTERPRETCAST( TINT, &vLockPosition ), TSTATICCAST( void*, pLockKid ) ) );
				iLockType     = m_iRaccoonPelvisBone;
			}
			else
			{
				vLockPosition = *TSTATICCAST( TVector4, GetUnitTargetPos( &vLockPosition, TREINTERPRETCAST( TINT, pLockEnemy ) ) );
				iLockType     = ( TSTATICCAST( Enemy, pLockEnemy )->eType == Enemy::ENEMYTYPE_COYOTE ) ? m_iCoyotePelvisBone : m_iKidPelvisBone;
			}

			if ( m_aPlayers[ m_iCurrentPlayer ].aPowerUps[ 1 ].fTime <= 0.0f )
				vLockPosition.y += 0.3f;

			rvTarget        = vLockPosition;
			m_vLockPosition = vLockPosition;

			vOffset.Multiply( vRayDirection, 0.5f );
			m_vLockPosition.x += vOffset.x;
			m_vLockPosition.y += vOffset.y;
			m_vLockPosition.z += vOffset.z;

			rvTarget.x += vTargetOffset.x;
			rvTarget.y += vTargetOffset.y;
			rvTarget.z += vTargetOffset.z;

			// Moves the target towards the locked position
			TVector4 vDelta;
			vDelta.Substract( rvTarget, vOldTarget );
			vOffset.Multiply( vDelta, m_fAimLockFollowSpeed );
			rvTarget.Add( vOffset, vOldTarget );

			if ( m_aPlayers[ m_iCurrentPlayer ].aPowerUps[ 2 ].fTime > 0.0f )
			{
				m_pLockUnit       = TREINTERPRETCAST( AAnimatedUnit*, iLockUnit );
				m_iLockTargetBone = iLockType;
				m_fUnk_1330C      = 2.0f;
			}

			if ( pLockEnemy )
				m_pLockEnemy = TSTATICCAST( Enemy, pLockEnemy );

			if ( pLockKid )
				m_pLockRaccoon = TSTATICCAST( Raccoon, pLockKid );

			bHasTarget = TTRUE;
		}
		else
		{
			// Not locked, find the closest thing to the aiming ray
			TFLOAT   fClosest = 0.0f;
			TINT     iEnemy   = FindEnemyNearRay( &vRayOrigin.x, &fClosest, CT_FLOAT_BITS( m_fNearRayRadius ), m_fAimLockDistance );
			TVector4 vPosition;

			if ( iEnemy )
			{
				GetUnitTargetPos( &vPosition, iEnemy );
				fRayLength = TVector4::Distance( vRayOrigin, vPosition );
			}
			else
			{
				TINT iKid = FindKidNearRay( TREINTERPRETCAST( TINT, &vRayOrigin ), &fClosest, CT_FLOAT_BITS( m_fNearRayRadius ), m_fAimLockDistance );

				if ( iKid )
				{
					GetUnit13268TargetPos( TREINTERPRETCAST( TINT, &vPosition ), TREINTERPRETCAST( void**, iKid ) );
					fRayLength = TVector4::Distance( vRayOrigin, vPosition );
				}
				else
				{
					void* pRecord = FindCrateNearRay( TREINTERPRETCAST( TINT, &vRayOrigin ), &fClosest, CT_FLOAT_BITS( m_fNearRayRadius ), m_fAimLockDistance );

					if ( pRecord )
					{
						GetCratePos( &vPosition, TREINTERPRETCAST( TINT, pRecord ) );
						fRayLength = TVector4::Distance( vRayOrigin, vPosition );
					}
					else
					{
						Coin* pObjectRecord = TSTATICCAST( Coin, FindObjectNearRay( TREINTERPRETCAST( TINT, &vRayOrigin ), &fClosest, CT_FLOAT_BITS( m_fNearRayRadius ), m_fCoinNearRayRadius ) );

						if ( pObjectRecord )
						{
							pObjectRecord->pObject->GetModelInstance()->GetPosition( vPosition );
							fRayLength = TVector4::Distance( vRayOrigin, vPosition );
						}
					}
				}
			}

			vOffset.Multiply( vRayDirection, fRayLength );
			rvTarget.Add( vRayOrigin, vOffset );

			if ( m_iUnk_13400 != 0 )
			{
				m_fUnk_1330C -= a_fUnk1;

				if ( m_fUnk_1330C < 0.0f )
					m_iUnk_13400 = 0;
			}
		}
	}

	if ( m_aPlayers[ m_iCurrentPlayer ].aPowerUps[ 2 ].fTime > 0.0f && m_pLockUnit != 0 && !m_bUnk_13408 )
	{
		FlagAimableUnits( TREINTERPRETCAST( TINT, &vRayDirection ), TREINTERPRETCAST( TINT, &rvTarget ), TREINTERPRETCAST( TINT, &pReticle ), CT_FloatBits( fRadiusScale * m_fAimLockRadius ), CT_FloatBits( fRadiusScale * m_fAimLockDistance ) );
		pReticle->SetMode( ATargetReticle::MODE_TARGET );
		bHasTarget = TTRUE;
	}

	if ( m_fThrowCooldown <= 0.0f )
	{
		if ( !a_iUnk2 && m_aPlayers[ m_iCurrentPlayer ].aPowerUps[ 1 ].fTime > 0.0f && ( fRayLength < 1000.0f || bLocked ) )
		{
			TVector4 vStep;
			vStep.Multiply( vDirection, 0.1f );
			rvTarget.x += vStep.x;
			rvTarget.y += vStep.y;
			rvTarget.z += vStep.z;

			ATerrainInterface::GetSingleton()->SetToGroundHeightTop( rvTarget, 0 );
			rvTarget.y -= 0.2f;
		}

		if ( bHasTarget && bCanShoot )
		{
			if ( IsPowerUpActive( 0 ) && m_bUnk_13408 )
				pReticle->SetMode( ATargetReticle::MODE_POWERUP_TARGET );
			else
				pReticle->SetMode( ATargetReticle::MODE_TARGET );
		}
		else if ( m_aPlayers[ m_iCurrentPlayer ].aPowerUps[ 0 ].fTime > 0.0f && m_bUnk_13408 )
		{
			pReticle->SetMode( ATargetReticle::MODE_POWERUP_IDLE );
		}
		else
		{
			pReticle->SetMode( ATargetReticle::MODE_IDLE );
		}
	}
}

// $Barnyard: FUNCTION 004b87a0
void ACowTippingMiniGameState::UpdatePlayerTransform( TFLOAT a_fUnk1, TINT a_iUnk2 )
{
	// Moves the slingshot (the player) along its rail and points it at the target
	TVector4&             rvPosition   = m_vPlayerPos;
	TVector4&             rvVelocity   = m_vPlayerVelocity;
	TVector4&             rvRail       = m_vRailDirection;
	TVector4&             rvRailOrigin = m_vPlayerCentre;
	TVector4&             rvTarget     = m_vAimTarget;
	TVector4&             rvUnk_0112C  = m_vReticleTarget;
	const CameraSettings& rCamera      = m_aCameraSettings[ m_aStages[ m_iStage ].iCameraSettings ];

	TFLOAT fRailPosition = rvPosition.x - rvRailOrigin.x;

	if ( rCamera.fRailHalfWidth + rCamera.fRailCentre >= fRailPosition )
	{
		if ( rCamera.fRailCentre - rCamera.fRailHalfWidth <= fRailPosition )
		{
			m_fCameraYawDelta = 0.0f;
		}
		else
		{
			// Hit the start of the rail
			TVector4 vOffset;
			vOffset.Multiply( rvRail, ( rCamera.fRailCentre - rCamera.fRailHalfWidth ) / rvRail.x );
			rvPosition.Add( rvRailOrigin, vOffset );
			rvVelocity = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
		}
	}
	else
	{
		// Hit the end of the rail
		TVector4 vOffset;
		vOffset.Multiply( rvRail, ( rCamera.fRailHalfWidth + rCamera.fRailCentre ) / rvRail.x );
		rvPosition.x = vOffset.x + rvRailOrigin.x;
		rvPosition.y = vOffset.y + rvRailOrigin.y;
		rvPosition.z = vOffset.z + rvRailOrigin.z;
		rvPosition.w = rvRailOrigin.w;
		rvVelocity   = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	}

	m_fCameraTargetYaw = fRailPosition * rCamera.fRailYawScale;

	if ( !m_bSideStepping )
		m_fCameraYawDelta = ( rvPosition.x - rvRailOrigin.x ) * 0.05f * rCamera.fUnk04;

	AAnimatableObject* pObject = m_pLauncher;
	pObject->GetModelInstance()->SetTranslation( rvPosition.AsVector3() );

	// Direction from the camera to the slingshot (ignoring the height)
	const TVector4& rcCamera = m_vCameraPos;

	TVector4 vToCamera;
	vToCamera.x = rvPosition.x - rcCamera.x;
	vToCamera.y = 0.0f;
	vToCamera.z = rvPosition.z - rcCamera.z;
	vToCamera.w = rvPosition.w;

	TVector4 vOffset;
	vOffset.Multiply( m_vCameraDir, 0.5f );
	vToCamera.x = vOffset.x + vToCamera.x;
	vToCamera.y = vOffset.y + vToCamera.y;
	vToCamera.z = vOffset.z + vToCamera.z;
	vToCamera.Normalise();

	TVector4& rvUnk_010FC = m_vAimDirection;
	rvUnk_010FC.x         = -vToCamera.x;
	rvUnk_010FC.y         = -vToCamera.y;
	rvUnk_010FC.z         = -vToCamera.z;
	rvUnk_010FC.w         = vToCamera.w;

	TVector4 vOldTarget  = rvTarget;
	TVector4 vOldUnk112C = rvUnk_0112C;

	UpdateAiming( a_fUnk1, a_iUnk2 );

	// Smooths the movement of the target
	TFLOAT fProgress = a_fUnk1 * 10.0f;

	if ( m_bUnk_1340A )
		fProgress = 1.0f;

	rvTarget    = LerpClamped3( vOldTarget, rvTarget, fProgress );
	rvUnk_0112C = LerpClamped3( vOldUnk112C, rvUnk_0112C, fProgress );

	TVector4 vRailOrigin = rvRailOrigin;
	pObject->GetModelInstance()->SetTranslation( vRailOrigin.AsVector3() );

	TVector4 vBonePosition;
	TVector4 vDirection;
	GetSlingshotBone( &vBonePosition, &vDirection.x );

	TVector4 vAim;
	CalculateThrowVelocityToTarget2( this, &vAim, TREINTERPRETCAST( const TVector4*, &vBonePosition.x ) );
	vAim.Normalise();
	vAim.x = -vAim.x;
	vAim.y = -vAim.y;
	vAim.z = -vAim.z;
	m_pLauncher->GetModelInstance()->SetDirection( vAim );

	TMatrix44 mTransform;
	m_pLauncher->GetModelInstance()->GetLocalTransform( mTransform );

	const TVector4& vRight = mTransform.AsBasisVector4( BASISVECTOR_RIGHT );
	vAim.RotateAroundAxis( vRight, m_fAimTilt );

	// Shaking
	if ( m_fShakeTimeLeft2 <= 0.0f )
	{
		m_fRecoilX = 0;
		m_fRecoilY = 0.0f;
	}
	else
	{
		m_fShakeIntervalTimer2 -= a_fUnk1;
		m_fShakeTimeLeft2 -= a_fUnk1;

		if ( m_fShakeIntervalTimer2 < 0.0f )
		{
			TFLOAT fStrength = m_fShakeTimeLeft2 / m_fShakeTime2;

			if ( fStrength < 0.0f )
				fStrength = 0.0f;

			TRandom& rRandom = ARandom::GetSingleton()->m_oRandom;

			TFLOAT fRandom1 = rRandom.GetFloatMinMax( -m_fShakeAmplitude2, m_fShakeAmplitude2 );
			TFLOAT fRandom2 = rRandom.GetFloatMinMax( -m_fShakeAmplitude2, m_fShakeAmplitude2 );
			m_fRecoilX      = ( fRandom2 + fRandom1 ) * fStrength;

			TFLOAT fRandom3        = rRandom.GetFloatMinMax( m_fShakeAmplitude2 * -0.5f, m_fShakeAmplitude2 + m_fShakeAmplitude2 );
			TFLOAT fRandom4        = rRandom.GetFloatMinMax( m_fShakeAmplitude2 * -0.5f, m_fShakeAmplitude2 + m_fShakeAmplitude2 );
			m_fShakeIntervalTimer2 = m_fShakeInterval2;
			m_fRecoilY             = ( fRandom4 + fRandom3 ) * fStrength;
		}
	}

	vAim.AsVector3().RotateX( m_fRecoilX );
	vAim.AsVector3().RotateY( m_fRecoilY );
	m_pLauncher->GetModelInstance()->SetDirection( vAim );
}

// $Barnyard: FUNCTION 004ba9e0
void ACowTippingMiniGameState::KidStartShooting( TINT a_iUnk1 )
{
	Raccoon* pNode = TREINTERPRETCAST( Raccoon*, a_iUnk1 );

	if ( m_eTurnEnd != TURNEND_NONE )
	{
		m_fRaccoonShootTimer = m_fRaccoonShootDelay;
	}
	else if ( m_iReloadState != RELOADSTATE_NONE )
	{
		m_fRaccoonShootTimer = m_fRaccoonShootDelay;
	}
	else if ( pNode->iType != 0 )
	{
		// Taunts the player
		ASoundManager::GetSingleton()->PlayCue( soundbank::DAGF_DAG_TAUNT1 );
		CT_UNIT( pNode, pUnit )->GetAnimatableObject()->PlayAnimation( TPS8( tawnt ) );
		pNode->iState        = PopUpUnit::POPUPSTATE_TAUNTING;
		m_fRaccoonShootTimer = m_fRaccoonShootDelay;
	}
	else
	{
		// Draws the slingshot
		ShowShootHint();
		CT_UNIT( pNode, pUnit )->GetAnimatableObject()->PlayAnimation( TPS8( draw ) );

		if ( !CT_IsStageType23( this ) )
		{
			CT_PTR( AAnimatableObject, GetUnitHeldObject( a_iUnk1 ) )->PlayAnimation( TPS8( draw ) );
			LoadKidSlingshot( TREINTERPRETCAST( void**, pNode ) );
		}

		pNode->iState        = PopUpUnit::POPUPSTATE_DRAWING;
		m_fRaccoonShootTimer = m_fRaccoonShootDelay;
	}
}

// $Barnyard: FUNCTION 004bbd90
void ACowTippingMiniGameState::KidFireTomato( void* a_pUnk1 )
{
	// Fires the tomato of a slingshot kid at the player
	Raccoon* pNode = TSTATICCAST( Raccoon, a_pUnk1 );

	TVector4 vTarget;
	GetPlayerHandPosition( &vTarget );

	AAnimatableObject* pParent = CT_UNIT( pNode, pUnit )->GetAnimatableObject();

	if ( !CT_IsStageType23( this ) )
		pParent = pParent->FindAttachedObject( m_iRaccoonHandBone );

	AAnimatableObject* pAttached = AAnimatableObjectManager::GetSingleton()->GetAttachedObject( pParent, m_iSlingshotBone, 0 );
	AAnimatableObjectManager::GetSingleton()->DetachObject( pAttached, pParent );

	TVector4 vPosition;
	pAttached->GetModelInstance()->GetPosition( vPosition );

	Projectile* pProjectile = TSTATICCAST( Projectile, FindProjectileByObject( pAttached ) );
	pProjectile->vDirection = TVector4::VEC_NEGX;
	pProjectile->pTarget    = 0;
	pProjectile->pEnemy     = 0;

	TFLOAT fSpeed = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( m_fTomatoSpeedMin, m_fTomatoSpeedMax );

	TVector4 vVelocity;
	AProjectile::CalculateThrowVelocity( vTarget, vPosition, fSpeed, vVelocity );
	pProjectile->vVelocity = vVelocity;

	pProjectile->pObject->GetModelInstance()->SetTranslation( vPosition.AsVector3() );

	TREINTERPRETCAST( TUINT8*, pProjectile )
	[0x49]                 = 0;
	pProjectile->pThrower  = pNode->pUnit;
	pProjectile->iGrowType = 6;
}

// $Barnyard: FUNCTION 004be5f0
void ACowTippingMiniGameState::UpdateShooter( TFLOAT a_fUnk1 )
{
	if ( !CT_IsStageType23( this ) )
		return;

	// The unit that shoots milk at the slingshot kids
	AAnimatedUnit* pShooter = CT_PTR( AAnimatedUnit, m_pOtis );

	TVector4 vPosition;
	pShooter->GetAnimatableObject()->GetModelInstance()->GetPosition( vPosition );

	TVector4 vForward;
	pShooter->GetAnimatableObject()->GetModelInstance()->GetUpVector( vForward );
	vForward.x = -vForward.x;
	vForward.y = -vForward.y;
	vForward.z = -vForward.z;

	if ( m_bShooterTurning )
	{
		// Turning
		TQuaternion qFrom;
		qFrom.SetVectorDirection( m_vFarmerFacing.AsVector3(), TVector3::VEC_POSZ );

		TQuaternion qTo;
		qTo.SetVectorDirection( m_vFarmerToTarget.AsVector3(), TVector3::VEC_POSZ );

		m_fFarmerTurnProgress = a_fUnk1 + a_fUnk1 + m_fFarmerTurnProgress;

		if ( m_fFarmerTurnProgress > 1.0f )
		{
			m_bShooterTurning     = TFALSE;
			m_fFarmerTurnProgress = 1.0f;
		}

		TQuaternion qRotation;
		TQuaternion::Lerp( qRotation, qFrom, qTo, m_fFarmerTurnProgress );

		TVector4 vDirection;
		TQuaternion::RotateVector( vDirection.AsVector3(), qRotation, TVector3::VEC_POSZ );
		vDirection.Normalise();
		pShooter->GetAnimatableObject()->GetModelInstance()->SetDirection( vDirection );
	}

	if ( !pShooter->GetAnimatableObject()->IsAnimationPlanned( TPS8( distress_idle ) ) &&
	     !pShooter->GetAnimatableObject()->IsAnimPlaying( TPS8( distress_idle ), 0.0f ) )
	{
		TINT iEnemy = FindClosestEnemyXZ( TREINTERPRETCAST( TINT, &vPosition ), m_fKidTargetRadius );

		if ( iEnemy )
		{
			TVector4 vTarget;
			GetUnitTargetPos( &vTarget, iEnemy );

			TVector4 vToTarget;
			vToTarget.Substract( vTarget, vPosition );

			if ( !m_bShooterTurning )
			{
				TFLOAT fDistance = vToTarget.NormaliseMag();

				if ( !ATerrainInterface::GetSingleton()->CollideRay( vPosition, vToTarget, fDistance, 0, TFALSE ) )
					TurnFarmerTowards( TREINTERPRETCAST( TINT, &vTarget ) );
			}

			m_fKidShootTimer -= a_fUnk1;

			if ( m_fKidShootTimer < 0.0f )
			{
				vToTarget.Normalise();

				AAimedSquirtProjectileLauncher* pLauncher = TSTATICCAST( AAimedSquirtProjectileLauncher, TSTATICCAST( AUnitPlayer, pShooter )->GetLauncher( 1 ) );

				if ( TVector4::DotProduct3( vForward, vToTarget ) >= m_fLauncherFireMinDot && pLauncher->FireAt( vTarget, TFALSE ) )
				{
					m_fKidShootTimer = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( m_fKidShootDelayMin, m_fKidShootDelayMax );
					pShooter->GetAnimatableObject()->KillAllAnimations();
					pShooter->GetAnimatableObject()->PlayAnimation( TPS8( shoot ) );
				}
			}
		}
	}

	// Milk that hits the enemies
	T2GenericSList& rList = CT_LIST( m_llActiveEnemies );

	for ( T2GenericSList::Node* pIt = rList.Begin(); pIt != rList.End(); pIt = pIt->Next() )
	{
		TVector4 vEnemyPosition;
		GetUnitTargetPos( &vEnemyPosition, TREINTERPRETCAST( TINT, pIt ) );

		if ( ASimpleProjectileManager::GetSingleton()->FindClosestProjectile( vEnemyPosition, m_fKidProjectileRadius ) )
			OnUnitHit( pIt, 1, 0, 1 );
	}
}

// $Barnyard: FUNCTION 004befe0
void ACowTippingMiniGameState::UpdatePlayerControls( TFLOAT a_fUnk1 )
{
	// Controls of the slingshot (the player)
	TRandom& rRandom = ARandom::GetSingleton()->m_oRandom;

	m_fThrowCooldown -= a_fUnk1;
	TFLOAT fTimer = m_fThrowCooldown;

	if ( fTimer > 0.0f )
	{
		TFLOAT fUnk = m_fThrowDelay - m_fHitShakeTime;

		if ( fTimer >= fUnk )
		{
			ClearUnitsFlag9();

			m_fHitShakeIntervalTimer -= a_fUnk1;
			m_fShakeTime -= a_fUnk1;

			if ( m_fHitShakeIntervalTimer < 0.0f )
			{
				TFLOAT fStrength = m_fShakeTime / m_fHitShakeTime;

				if ( fStrength < 0.0f )
					fStrength = 0.0f;

				TFLOAT fRandom1 = rRandom.GetFloatMinMax( -m_fHitShakeAmplitude, m_fHitShakeAmplitude );
				TFLOAT fRandom2 = rRandom.GetFloatMinMax( -m_fHitShakeAmplitude, m_fHitShakeAmplitude );
				m_fCameraShakeX = ( fRandom2 + fRandom1 ) * fStrength;

				TFLOAT fRandom3          = rRandom.GetFloatMinMax( -m_fHitShakeAmplitude, m_fHitShakeAmplitude );
				TFLOAT fRandom4          = rRandom.GetFloatMinMax( -m_fHitShakeAmplitude, m_fHitShakeAmplitude );
				m_fHitShakeIntervalTimer = m_fHitShakeInterval;
				m_fCameraShakeY          = ( fRandom4 + fRandom3 ) * fStrength;
			}
		}
		else
		{
			m_pCameraTarget = 0;
			m_iReloadState  = RELOADSTATE_NONE;

			m_fUnk_132D0 -= ( ( m_fReticleSpeedMax - m_fReticleSpeedMin ) * ( fUnk - fTimer ) + m_fReticleSpeedMin ) * a_fUnk1;

			if ( m_fUnk_132D0 < -m_fUnk_00D44 )
				m_fUnk_132D0 = m_fUnk_132D0 + m_fUnk_00D40 + m_fUnk_00D44;

			m_oTargetReticle.SetActive( m_fUnk_132D0 >= 0.0f, TTRUE, 50.0f );
			m_fCameraShakeX = 0;
			m_fCameraShakeY = 0;
		}
	}
	else
	{
		if ( !m_oTargetReticle.IsActive() )
			m_oTargetReticle.SetActive( TTRUE, TTRUE, 50.0f );

		if ( m_iReloadState == RELOADSTATE_HIT )
		{
			m_iReloadState  = RELOADSTATE_NONE;
			m_pCameraTarget = 0;
		}

		m_fCameraShakeX = 0;
		m_fCameraShakeY = 0;
	}

	if ( m_pCameraTarget != 0 )
		return;

	TFLOAT&   rfTimerX   = m_fAccelTimerX;
	TFLOAT&   rfTimerY   = m_fAccelTimerY;
	TFLOAT&   rfLastX    = m_fLastInputX;
	TFLOAT&   rfLastY    = m_fLastInputY;
	TFLOAT&   rfSpeedY   = m_fSpeedY;
	TFLOAT&   rfInputX   = m_fInputX;
	TFLOAT&   rfInputY   = m_fMoveY;
	TVector4& rvPosition = m_vPlayerPos;
	TVector4& rvVelocity = m_vPlayerVelocity;
	TVector4& rvRail     = m_vRailDirection;

	rfTimerX -= a_fUnk1;
	rfTimerY -= a_fUnk1;

	TFLOAT fInputSq = rfInputX * rfInputX + rfInputY * rfInputY;

	if ( fInputSq <= 0.0f )
	{
		rfTimerX    = TMath::MAXFLOAT;
		rfLastX     = 0.0f;
		rfLastY     = 0.0f;
		m_bNoInputX = TTRUE;
		m_bNoInputY = TTRUE;
		rfTimerY    = TMath::MAXFLOAT;
	}
	else
	{
		TFLOAT fInputX = rfInputX;
		TFLOAT fInputY = rfInputY;

		if ( TMath::Abs( TMath::Sqrt( fInputSq ) ) < 0.1f )
		{
			fInputX *= 0.25f;
			fInputY *= 0.25f;
		}

		// Moves faster when far from the centre of the rail
		TFLOAT fSpeedScale = TMath::Pow( TMath::Abs( rvPosition.x - m_vPlayerCentre.x ) * m_fRailSpeedScale, m_fRailSpeedExponent );

		if ( fSpeedScale < 1.0f )
			fSpeedScale = 1.0f;

		TVector4 vMove;
		vMove.Multiply( rvRail, fInputX );
		vMove.Multiply( fInputX );
		vMove.Multiply( fInputX );
		vMove.Multiply( m_fMoveSpeed );
		vMove.Multiply( a_fUnk1 );
		vMove.Multiply( fSpeedScale );
		rvPosition.x = vMove.x + rvPosition.x;
		rvPosition.y = vMove.y + rvPosition.y;
		rvPosition.z = vMove.z + rvPosition.z;

		m_bUnk_13404        = TFALSE;
		m_fCameraStartPitch = m_fCameraStartPitch - fInputY * m_fPitchInputSpeed * fInputY * fInputY * a_fUnk1;

		m_bNoInputX = TMath::Abs( rfInputX ) < m_fInputDeadZone;

		if ( m_bNoInputX )
		{
			rfTimerX = TMath::MAXFLOAT;
			rfLastX  = 0.0f;
		}
		else if ( TMath::Sign( rfLastX ) != TMath::Sign( rfInputX ) || rfTimerX > m_fInputAccelTime )
		{
			rfLastX    = rfInputX;
			rvVelocity = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
			rfTimerX   = m_fInputAccelTime;
		}

		m_bNoInputY = TMath::Abs( rfInputY ) < m_fInputDeadZone;

		if ( m_bNoInputY )
		{
			rfTimerY = TMath::MAXFLOAT;
			rfLastY  = 0.0f;
		}
		else if ( TMath::Sign( rfLastY ) != TMath::Sign( rfInputY ) || rfTimerY > m_fInputAccelTime )
		{
			rfLastY  = rfInputY;
			rfSpeedY = 0.0f;
			rfTimerY = m_fInputAccelTime;
		}
	}

	const AimAcceleration& rAccel = m_aAimAcceleration[ m_aStages[ m_iStage ].iCameraSettings ];

	if ( m_bNoInputX || rfTimerX >= 0.0f )
	{
		rvVelocity = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	}
	else
	{
		TFLOAT fTime = -rfTimerX;

		if ( fTime > rAccel.fMaxTimeX )
			fTime = rAccel.fMaxTimeX;

		TFLOAT fSign = 1.0f;

		if ( rfInputX <= 0.0f )
			fSign = -1.0f;

		TVector4 vAccel;
		vAccel.Multiply( rvRail, fSign );
		vAccel.Multiply( rfInputX );
		vAccel.Multiply( rfInputX );
		vAccel.Multiply( rAccel.fAccelX );
		vAccel.Multiply( fTime );
		rvVelocity.x = vAccel.x + rvVelocity.x;
		rvVelocity.y = vAccel.y + rvVelocity.y;
		rvVelocity.z = vAccel.z + rvVelocity.z;

		TVector4 vMove;
		vMove.Multiply( rvVelocity, a_fUnk1 );
		rvPosition.x = vMove.x + rvPosition.x;
		rvPosition.y = vMove.y + rvPosition.y;
		rvPosition.z = vMove.z + rvPosition.z;
	}

	if ( m_bNoInputY || rfTimerY >= 0.0f )
	{
		rfSpeedY = 0.0f;
	}
	else
	{
		TFLOAT fTime = -rfTimerY;

		if ( fTime > rAccel.fMaxTimeY )
			fTime = rAccel.fMaxTimeY;

		TFLOAT fSign = ( rfInputY <= 0.0f ) ? -1.0f : 1.0f;

		rfSpeedY            = fTime * ( fSign * rAccel.fAccelY * rfInputY * rfInputY ) + rfSpeedY;
		m_fCameraStartPitch = m_fCameraStartPitch - rfSpeedY * a_fUnk1;
	}

	UpdatePlayerTransform( a_fUnk1, TFALSE );

	if ( m_bUnk_1340A )
	{
		m_fInputX = 0;
		m_fMoveY  = 0.0f;
		m_fMouseX = 0.0f;
		m_fMouseY = 0.0f;
	}
}

// $Barnyard: FUNCTION 004bfb70
void ACowTippingMiniGameState::UpdateKids( TFLOAT a_fUnk1 )
{
	// Updates the slingshot kids
	T2GenericSList&       rList = CT_LIST( m_llActiveRaccoons );
	T2GenericSList::Node* pIt   = rList.Begin();

	while ( pIt != rList.End() )
	{
		Raccoon* pNode = TREINTERPRETCAST( Raccoon*, pIt );
		pIt            = pIt->Next();

		// NOTE: the result is not used
		CT_UNIT( pNode, pUnit )->GetSteer()->GetPosition();

		AAnimatedUnit* pUnit       = CT_UNIT( pNode, pUnit );
		TFLOAT&        rfDrawTimer = pNode->fDrawTimer;
		TFLOAT&        rfDownTimer = pNode->fDownTimer;
		TBOOL          bGoDown     = TFALSE;

		switch ( pNode->iState )
		{
			case PopUpUnit::POPUPSTATE_LEAVING:
				RemoveKid( TREINTERPRETCAST( TINT, pNode ) );
				m_fRaccoonShootTimer = m_fRaccoonShootDelay;
				break;
			case PopUpUnit::POPUPSTATE_HIT:
			{
				const TPString8* pAnimation;

				if ( !CT_IsStageType23( this ) )
				{
					if ( pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( yuk ), 0.0f ) )
						break;

					pAnimation = &TPS8( yuk );
				}
				else
				{
					if ( pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( yuk ), 0.0f ) ||
					     pUnit->GetAnimatableObject()->IsAnimationPlanned( TPS8( yuk ) ) ||
					     pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( yukobject ), 0.0f ) )
						break;

					pAnimation = &TPS8( yukobject );
				}

				if ( !pUnit->GetAnimatableObject()->IsAnimationPlanned( *pAnimation ) )
					pNode->iState = PopUpUnit::POPUPSTATE_LEAVING;

				break;
			}
			case PopUpUnit::POPUPSTATE_GOING_DOWN:
				if ( pUnit->GetAnimatableObject()->IsAnimationFinished( TPS8( down ) ) )
					pNode->iState = PopUpUnit::POPUPSTATE_LEAVING;

				break;
			case PopUpUnit::POPUPSTATE_DRAWING:
				if ( pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( fire ), 0.0f ) == TTRUE )
				{
					pNode->iState = PopUpUnit::POPUPSTATE_ACTIVE;

					Level& rLevel     = m_aLevels[ m_aPlayers[ m_iCurrentPlayer ].iLevel ];
					pNode->fDrawTimer = CT_IsStageType23( this ) ? rLevel.fRaccoonReloadTimeLarge : rLevel.fRaccoonReloadTime;

					KidFireTomato( pNode );
					pNode->iShots++;
				}

				break;
			case PopUpUnit::POPUPSTATE_TAUNTING:
				if ( pUnit->GetAnimatableObject()->IsAnimationFinished( TPS8( tawnt ) ) )
					bGoDown = TTRUE;

				break;
			case PopUpUnit::POPUPSTATE_INACTIVE:
				break;
			default:
			{
				if ( pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( sl_idle_lp ), 0.0f ) != TTRUE &&
				     pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( tawnt ), 0.0f ) != TTRUE &&
				     pUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( idle_lp2 ), 0.0f ) != TTRUE )
					break;

				rfDownTimer -= a_fUnk1;

				if ( ( rfDownTimer < 0.0f || m_bRaccoonsShootNow == TTRUE ) && pNode->iShots > 0 )
				{
					bGoDown = TTRUE;
				}
				else if ( m_fThrowCooldown < 0.0f && ( m_fReticleTimer < 0.0f || m_bRaccoonsShootNow == TTRUE ) )
				{
					rfDrawTimer -= a_fUnk1;

					if ( rfDrawTimer < 0.0f || m_bRaccoonsShootNow == TTRUE )
						KidStartShooting( TREINTERPRETCAST( TINT, pNode ) );
				}
				else
				{
					Level& rLevel     = m_aLevels[ m_aPlayers[ m_iCurrentPlayer ].iLevel ];
					pNode->fDrawTimer = CT_IsStageType23( this ) ? rLevel.fRaccoonReloadTimeLarge : rLevel.fRaccoonReloadTime;
				}

				break;
			}
		}

		if ( bGoDown )
		{
			pNode->iState = PopUpUnit::POPUPSTATE_GOING_DOWN;
			pUnit->GetAnimatableObject()->PlayAnimation( TPS8( down ) );
			m_bRaccoonsShootNow = TFALSE;
		}
	}
}

// $Barnyard: FUNCTION 004c1a40
void ACowTippingMiniGameState::UpdatePlayer( TFLOAT a_fUnk1 )
{
	UpdateReload();
	UpdatePlayerControls( a_fUnk1 );
}
