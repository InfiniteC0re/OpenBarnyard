#include "pch.h"
#include <Render/TCameraObject.h>
#include "APrecisionSquirt.h"
#include "AMiniGameManager.h"
#include "AMiniGame.h"
#include "ALoadScreen.h"
#include "ARandom.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AAlienAnimController.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACameraHelper.h"
#include "FX/AGameTimeFXManager.h"
#include "FX/AMilkManager.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AAnticGameOverState.h"
#include "GameInterface/ASSeqState.h"
#include "GUI/AGUI2.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AHighScoreManager.h"
#include "Input/AInputMapManager.h"
#include "Input/AInputHandler.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Render/ARenderer.h"
#include "Sound/AMusicManager.h"
#include "Sound/AMultiplexStream.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/am_f.h"
#include "SoundBank/am_m.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/ASSeqController.h"
#include "Units/ASSeq.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitPlayer.h"
#include "Units/AUnitTransform.h"
#include "Units/ACharacterMovementComponent.h"
#include "Units/ASimplePlayerControlComponent.h"
#include "Units/APrecisionCoyote.h"
#include "Units/APrecisionRacoon.h"
#include "Units/APrecisionScarecrow.h"
#include "Units/APrecisionTargetItem.h"
#include "Units/Projectiles/AAimedSquirtProjectileLauncher.h"
#include "Units/Projectiles/AProjectileLauncher.h"
#include "Units/Projectiles/ALaunchAmmo.h"
#include "Units/Projectiles/AMilkSquirtProjectileLauncher.h"
#include "Units/Projectiles/ASimpleProjectileManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/Steering/AAlienSteer.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/Steering/AStationarySteer.h"
#include "UnitMovement/Steering/AThrowItemSteer.h"
#include "Physics/ACollisionObject.h"

#include <GUI/T2GUITextureSectionManager.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>
#include <Toshi/TStringManager.h>
#include <Math/TMath.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

// Opcode defines PI
#undef PI

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00502f20, CreateObject 005077f0, CreateObjectInPlace 00507850 (the constructor is inlined in both)
TDEFINE_CLASS( APrecisionSquirt );

TPSTRING8_DECLARE( NormalBarnyard );
TPSTRING8_DECLARE( none );
TPSTRING8_DECLARE( precision1 );
TPSTRING8_DECLARE( precision2 );
TPSTRING8_DECLARE( precision3 );
TPSTRING8_DECLARE( precision4 );
TPSTRING8_DECLARE( precision5 );
TPSTRING8_DECLARE( precision6 );
TPSTRING8_DECLARE( precision7 );
TPSTRING8_DECLARE( precision8 );
TPSTRING8_DECLARE( precision9 );
TPSTRING8_DECLARE( precision10 );
TPSTRING8_DECLARE( spawn );
TPSTRING8_DECLARE( respawn );
TPSTRING8_DECLARE( target );
TPSTRING8_DECLARE( target2 );
TPSTRING8_DECLARE( target3 );
TPSTRING8_DECLARE( coyote );
TPSTRING8_DECLARE( racoon );
TPSTRING8_DECLARE( cow1 );
TPSTRING8_DECLARE( cow2 );
TPSTRING8_DECLARE( cow3 );
TPSTRING8_DECLARE( cow4 );
TPSTRING8_DECLARE( scarecrow );
TPSTRING8_DECLARE( pyramid );
TPSTRING8_DECLARE( sharp_coyote );
TPSTRING8_DECLARE( sharp_racoon );
TPSTRING8_DECLARE( sharp_jersy );
TPSTRING8_DECLARE( precision_pumpkin );
TPSTRING8_DECLARE( precision_scarecrow );
TPSTRING8_DECLARE( precisiontarget );
TPSTRING8_DECLARE( barnbuck );
TPSTRING8_DECLARE( sunglasses1 );
TPSTRING8_DECLARE( player_ps );
TPSTRING8_DECLARE( defeat );
TPSTRING8_DECLARE( loser_start );
TPSTRING8_DECLARE( loser_loop );
TPSTRING8_DECLARE( winner_start );
TPSTRING8_DECLARE( winner_loop );
TPSTRING8_DECLARE( winner2_start );
TPSTRING8_DECLARE( winner2_loop );
TPSTRING8_DECLARE( pickup_l );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( default );
TPSTRING8_DECLARE( cb_rlocator );
TPSTRING8_DECLARE( cb_llocator );

// Locale strings
static constexpr TINT LOCALE_ROUND         = 555; // "Round {0}"
static constexpr TINT LOCALE_NEXT_ROUND    = 994;
static constexpr TINT LOCALE_TARGETS       = 625;
static constexpr TINT LOCALE_WOODS_TARGETS = 624;
static constexpr TINT LOCALE_COMBO_BONUS   = 636;
static constexpr TINT LOCALE_TIME_BONUS    = 606;

// Section the player starts in (0x00773d34)
static TINT s_iTerrainSection = -1;

// The next of the m_awszFloatTexts (0x007834d8)
static TINT s_iFloatText = -1;

// Cheers of the player after a round (0x007834dc)
static TINT s_iNumCheers = -1;

//-----------------------------------------------------------------------------
// Coin
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005004d0
APrecisionSquirt::Coin::Coin( AAnimatableObject* a_pObject, const TVector4& a_rcPosition )
{
	pObject   = a_pObject;
	fTime     = 0.0f;
	vPosition = a_rcPosition;
	bFading   = TFALSE;
	bActive   = TTRUE;

	// Random start of the spin
	fTime = ARandom::GetSingleton()->m_oRandom.GetFloat();
}

// $Barnyard: FUNCTION 00500550
void APrecisionSquirt::Coin::Update( TFLOAT a_fDeltaTime )
{
	if ( !bActive )
		return;

	fTime += a_fDeltaTime;

	AModelInstanceRef pModelInstance = pObject->GetModelInstance();

	// NOTE: retail tests the skeleton-updating bit (0x1), not the visible bit
	if ( !pModelInstance || !pModelInstance->IsUpdatingSkeleton() )
		return;

	// Spinning
	TMatrix44 oTransform;
	oTransform.Identity();
	TMatrix44::TransformVector( oTransform.AsBasisVector4( 3 ), oTransform, vPosition );
	oTransform.Rotate( 0.0f, fTime * 3.0f, 0.0f );
	oTransform.Rotate( TMath::HALF_PI, 0.0f, 0.0f );
	pObject->GetModelInstance()->SetTransform( oTransform );

	if ( bFading )
	{
		TFLOAT fAngle = TMath::PI * 2.0f * fTime;
		TMath::NormaliseAngle( fAngle );
		pObject->SetOpacity( ( TMath::Cos( fAngle ) + 1.0f ) * 0.5f, TTRUE );
	}
	else
	{
		pObject->GetModelInstance()->GetSceneObject()->DisableOpacity();
	}
}

// $Barnyard: FUNCTION 00500490
void APrecisionSquirt::Coin::SetActive( TBOOL a_bActive )
{
	bActive = a_bActive;

	if ( !a_bActive )
		pObject->SetOpacity( 0.0f, TTRUE );
}

// $Barnyard: FUNCTION 005004b0
void APrecisionSquirt::Coin::SetSkeletonUpdating( TBOOL a_bUpdating )
{
	pObject->SetSkeletonUpdating( a_bUpdating, TTRUE );
}

// $Barnyard: FUNCTION 00502ff0
void APrecisionSquirt::PumpkinList::Add( const Pumpkin& a_rcPumpkin )
{
	aPumpkins[ iCount++ ] = a_rcPumpkin;
}

//-----------------------------------------------------------------------------
// APrecisionSquirt
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00507340
APrecisionSquirt::APrecisionSquirt()
{
	m_iNumSpawnPoints          = 0;
	m_iNumCoyotes              = 0;
	m_iNumRacoons              = 0;
	m_iNumScarecrows           = 0;
	m_iNumKnockedOver          = 0;
	m_oPumpkins.iCount         = 0;
	m_iNumCows                 = 0;
	m_iNumTargets              = 0;
	m_oTargetPositions.iCount  = 0;
	m_oTargetPositions2.iCount = 0;
	m_oTargetPositions3.iCount = 0;
	m_pNearestPumpkin          = TNULL;

	m_oMapViewport.iX      = 0;
	m_oMapViewport.iY      = 0;
	m_oMapViewport.iWidth  = 0;
	m_oMapViewport.iHeight = 0;

	m_pNetwork    = TNULL;
	m_pPlayer     = TNULL;
	m_pSunglasses = TNULL;
	m_pControl    = TNULL;
	m_strNetwork  = TPS8( none );

	m_vPyramid            = TVector4::VEC_ZERO;
	m_iNumRounds          = 1;
	m_iNumPreviousTargets = 0;
	m_iRound              = 1;
	m_iNumCoinsLeft       = 0;
	m_iNumCoins           = 0;
	m_iNumCoinsCollected  = 0;
	m_ePhase              = PHASE_PLAYING;
	m_vSpawnPosition      = TVector4::VEC_ZERO;
	m_fOldNear            = 1.0f;
	m_pGameData           = TNULL;

	m_pInputMap = AInputMapManager::GetSingleton()->FindMap( TPS8( NormalBarnyard ) );

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail never calls Create on the round text, it's on the root element without a font until
	// the first SetText (which shows it)
	m_oRoundText.Hide();
#endif
}

// $Barnyard: FUNCTION 00507650
// $Barnyard: FUNCTION 00507890
// $Barnyard: FUNCTION 00507640
APrecisionSquirt::~APrecisionSquirt()
{
}

// $Barnyard: FUNCTION 00502f90
TINT APrecisionSquirt::GetTerrainSection()
{
	return s_iTerrainSection;
}

// $Barnyard: FUNCTION 00502f30
APrecisionSquirt::RESULT_ APrecisionSquirt::GetRoundResult()
{
	if ( m_iLives <= 0 )
		return RESULT_NO_LIVES;

	if ( m_fElapsedTime < m_fTimeLimit )
		return ( m_iNumTargetsLeft != 0 ) ? RESULT_PLAYING : RESULT_WON;

	return RESULT_TIME_UP;
}

// $Barnyard: FUNCTION 00502f70
TBOOL APrecisionSquirt::IsHoldingPumpkin()
{
	return m_pPlayer->GetAttachedUnit( TPS8( cb_rlocator ), -1 ) != TNULL;
}

// $Barnyard: FUNCTION 00502fa0
void APrecisionSquirt::OnPreStart()
{
	if ( m_iVariant < 6 )
	{
		// Hidden during the intro cut-scene
		m_pPlayer->SetVisible( TFALSE );
		m_pPlayer->SetMovement( TFALSE );
		m_pPlayer->SetAIEnabled( TFALSE );
		m_pPlayer->GetAnimatableObject()->SetAnimControllersDisabled( TTRUE );
	}
}

// $Barnyard: FUNCTION 005047e0
void APrecisionSquirt::OnPause()
{
	if ( m_iVariant < 6 )
	{
		m_pPlayer->SetVisible( TTRUE );
		m_pPlayer->SetMovement( TTRUE );
		m_pPlayer->SetAIEnabled( TTRUE );
		m_pPlayer->GetAnimatableObject()->SetAnimControllersDisabled( TFALSE );
	}

	ATerrainInterface::GetSingleton()->SetGetCurrentVISGroupCallback( ATerrainInterface::GetSectionPlayerIsIn );
	AGameState::OnPause();
}

// $Barnyard: FUNCTION 00503070
// $BarnyardWiiJP: FUNCTION 8024e4fc
// NOTE: on the Wii, GetRoundResult (802510bc) has no RESULT_NO_LIVES (the lives of the variants 5+ are gone
// with the reworked Wii Remote controls), so only RESULT_TIME_UP ends the game there. The PC result is kept.
TBOOL APrecisionSquirt::IsGameOver()
{
	if ( m_ePhase == PHASE_GAME_OVER )
		return TTRUE;

	if ( m_ePhase == PHASE_ROUND_WON )
		return TFALSE;

	switch ( GetRoundResult() )
	{
		case RESULT_WON:
		{
#ifdef BARNYARD_LATEST
			// The latest version puts the normal (third person) launcher back, so the camera returns
			// behind the player who cheers (winner2_start/winner_start), and the player shouts a cheer
			if ( m_pPlayer )
			{
				m_pPlayer->SelectLauncher( TFALSE );
				// NOTE: the Wii also resets a Wii Remote aiming mode of the AAlienSteer here (+0x67c),
				// which the PC steer doesn't have
			}

			ASoundId eCheer = ( APlayerProfileManager::GetSingleton()->GetMainGender() != APlayerProfile::GENDER_MALE ) ? soundbank::AM_F_CHEER : soundbank::AM_M_CHEER;

			if ( ASoundManager::GetSingleton()->FindPlayingCue( eCheer ) == -1 )
				ASoundManager::GetSingleton()->PlayCue( eCheer );
#endif

			if ( m_iRound == m_iNumRounds )
				break;

			// The next round, the time left is a bonus
			TFLOAT fTimeLeft = m_fTimeLimit - m_fElapsedTime;
			m_iRound++;
			m_iTimeBonus += TINT( TINT64( fTimeLeft ) );
			m_ePhase = PHASE_ROUND_WON;

			if ( m_pPlayer )
			{
				AAlienSteer* pSteer = TSTATICCAST( AAlienSteer, m_pPlayer->GetSteer() );

				if ( pSteer->IsStrafingOrRolling() )
					pSteer->SetStrafing( TFALSE );

				m_pPlayer->GetActiveLauncher()->Deactivate();
				m_pControl->SetLauncherInput( TFALSE );
				m_pControl->SetSecondaryLauncherInput( TFALSE );
				m_pControl->SetCanSprint( TFALSE );
				m_pControl->SetMovementEnabled( TFALSE );
				m_pControl->SetCameraInput( TFALSE );
				m_pPlayer->GetSteer()->SetTilt( 0.0f, 0.0f, TFALSE );

				ALaunchAmmo* pAmmo = m_pPlayer->GetActiveLauncher()->GetAmmo();
				pAmmo->SetAmmo( pAmmo->GetMaxAmmo() );
			}

			// "Round N" (the one that's over)
			m_fRoundEndTimer = 0.0f;
			TStringManager::String16Format( m_wszRoundText, TARRAYSIZE( m_wszRoundText ), ALocaleManager::Interface()->GetString( LOCALE_ROUND ), m_iRound - 1 );
			m_oRoundText.m_fUnk1     = 0.0f;
			m_oRoundText.m_fUnk4     = 1.0f;
			m_oRoundText.m_fUnk3     = 1.0f;
			m_oRoundText.m_fUnk2     = 1.2f;
			m_oRoundText.m_fFadeTime = 3.0f;
			// NOTE: retail also passes ( 0.5f, 0.5f ) to a function that does nothing (00628f40)
			m_oRoundText.SetText( m_wszRoundText, 0xFF00FF00 );

			m_oReticles.SetActive( TFALSE );
			m_oTimeLeftHudElement.SetForceHide( TTRUE );
			m_GUIElement.Hide();
			return TFALSE;
		}
		case RESULT_TIME_UP:
		case RESULT_NO_LIVES:
			break;
		default:
			return TFALSE;
	}

	m_ePhase  = PHASE_GAME_OVER;
	m_eResult = GetRoundResult();
	return TTRUE;
}

// $Barnyard: FUNCTION 00503270
void APrecisionSquirt::OnActivate()
{
	if ( m_pPlayer )
	{
		if ( !m_oStartHudElement.IsForceHidden() )
			goto LABEL_DONE;

		if ( m_ePhase == PHASE_PLAYING )
		{
			m_pPlayer->SetMovement( TTRUE );
			m_pPlayer->GetActiveLauncher()->Activate();
			m_pControl->SetLauncherInput( TTRUE );
			m_pControl->SetSecondaryLauncherInput( TTRUE );
			m_pControl->SetMovementEnabled( TTRUE );
			m_pControl->SetCanSprint( TTRUE );
			TSTATICCAST( AAlienAnimController, m_pPlayer->GetAnimatableObject()->FindAnimController( &TGetClass( AAlienAnimController ) ) )->SetAnimEnabled( TTRUE );
		}
	}

	if ( m_oStartHudElement.IsForceHidden() && !IsGameOver() )
	{
		m_oTimeLeftHudElement.SetForceHide( TFALSE );
		m_GUIElement.Show();
		m_oFloats.Show();
		m_oTimeLeftHudElement.GetTimer().SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
		m_oReticles.SetActive( TTRUE );
	}

LABEL_DONE:
	if ( m_iVariant < 5 )
		m_oLifeHud.SetVisible( TFALSE );

	AMultiplexStream::GetSingleton()->Pause( TFALSE );
	AMiniGameState::OnActivate();
}

// $Barnyard: FUNCTION 00503370
void APrecisionSquirt::OnDeactivate()
{
	if ( m_pPlayer )
	{
		m_pPlayer->GetActiveLauncher()->Deactivate();
		m_pControl->SetLauncherInput( TFALSE );
		m_pControl->SetSecondaryLauncherInput( TFALSE );
		m_pControl->SetMovementEnabled( TFALSE );
		m_pControl->SetCanSprint( TFALSE );
		TSTATICCAST( AAlienAnimController, m_pPlayer->GetAnimatableObject()->FindAnimController( &TGetClass( AAlienAnimController ) ) )->SetAnimEnabled( TFALSE );
	}

	m_oTimeLeftHudElement.GetTimer().SetVisible( TFALSE );
	m_oReticles.SetActive( TFALSE );
	m_oFloats.Hide();
	AMultiplexStream::GetSingleton()->Pause( TTRUE );
	AMiniGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 00503410
TBOOL APrecisionSquirt::OnUpdate( TFLOAT a_fDeltaTime )
{
	AMultiplexStream* pMusic = AMultiplexStream::GetSingleton();

	if ( pMusic->GetState() != AMultiplexStream::STATE_PLAYING )
	{
		pMusic->Play( 1.0f );
		pMusic->SetVolume( 1.0f, 0, TTRUE );
		pMusic->SetVolume( 1.0f, 1, TTRUE );
		pMusic->SetVolume( 0.0f, 2, TTRUE );
		pMusic->SetVolume( 0.0f, 3, TTRUE );
	}

	if ( m_ePhase != PHASE_ROUND_WON )
		return AMiniGameState::OnUpdate( a_fDeltaTime );

	// The clock stops between the rounds
	m_fElapsedTime -= a_fDeltaTime;
	TMath::Clip( m_fElapsedTime, 0.0f, 99999.992f );
	AMiniGameState::OnUpdate( a_fDeltaTime );
	return TTRUE;
}

// $Barnyard: FUNCTION 005034c0
void APrecisionSquirt::UpdateTargets( TFLOAT a_fDeltaTime )
{
	m_fComboTimer -= a_fDeltaTime;

	if ( m_fComboTimer <= 0.0f )
		m_iCombo = 0;

	TINT iNumTargets = m_pGameData->aRounds[ m_iRound - 1 ].iNumTargets;

	for ( TINT i = iNumTargets; i != 0; i-- )
	{
		APrecisionTargetItem* pTarget = m_apTargets[ i - 1 ];

		if ( pTarget->IsHit() )
			m_oMap.SetIconVisible( pTarget->m_iMapIcon, TFALSE );

		if ( !pTarget->ConsumeHit() )
			continue;

		m_iNumTargetsLeft--;
		m_fComboTimer = 3.0f;
		m_oMap.SetIconVisible( pTarget->m_iMapIcon, TFALSE );

		// Targets hit by the same squirt (or the next one) build up a combo
		TINT iStreamId   = pTarget->GetStreamId();
		TINT iStreamDiff = ( iStreamId > m_iLastStreamId ) ? iStreamId - m_iLastStreamId : m_iLastStreamId - iStreamId;

		if ( iStreamId > m_iLastStreamId )
			m_iLastStreamId = iStreamId;

		if ( iStreamDiff >= 2 )
			m_iCombo = 0;

		m_iCombo++;
		TMath::Clip( m_iCombo, 1, 5 );

		TINT iPoints = 2 * m_iCombo;
		m_iScore += iPoints;
		m_oPortrait.SetScore( m_iScore );

		s_iFloatText = ( s_iFloatText + 1 >= NUM_FLOAT_TEXTS ) ? 0 : s_iFloatText + 1;
		TStringManager::String16Format( m_awszFloatTexts[ s_iFloatText ], 8, L"%d", iPoints );

		// NOTE: retail passes one more argument (TTRUE)
		m_oFloats.AddTimedFloat( m_awszFloatTexts[ s_iFloatText ], 0xFFFFC800, *pTarget->GetSteer()->GetPosition(), 1.0f, -2.6f, 1.0f );
	}
}

// $Barnyard: FUNCTION 00503660
void APrecisionSquirt::OnRender( TFLOAT a_fDeltaTime )
{
	if ( m_ePhase == PHASE_PLAYING )
		m_oReticles.UpdateSim( a_fDeltaTime );

	if ( AMilkManager::GetSingleton() )
		AMilkManager::GetSingleton()->Render();

	if ( AGameStateController::GetSingleton()->GetCurrentState()->GetClass() != &TGetClass( ASSeqState ) )
		m_oMilkBottles.Render();
}

// $Barnyard: FUNCTION 005036c0
void APrecisionSquirt::UpdateResultAnimation()
{
	AAnimatableObject* pObject = m_pPlayer->GetAnimatableObject();

	switch ( m_eResult )
	{
		case RESULT_WON:
			if ( !pObject->IsAnimPlaying( TPS8( winner_start ), 0.0f ) && !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( winner_loop ), 0.0f ) )
			{
				m_pPlayer->GetAnimatableObject()->KillAllAnimations();
				m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( winner_start ) );
			}

			break;
		case RESULT_TIME_UP:
			if ( !pObject->IsAnimPlaying( TPS8( loser_start ), 0.0f ) && !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( loser_loop ), 0.0f ) )
			{
				m_pPlayer->GetAnimatableObject()->KillAllAnimations();
				m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( loser_start ) );
			}

			break;
		case RESULT_NO_LIVES:
			if ( !pObject->IsAnimPlaying( TPS8( defeat ), 0.0f ) )
			{
				m_pPlayer->GetAnimatableObject()->KillAllAnimations();
				m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( defeat ) );
			}

			break;
		default:
			break;
	}
}

// $Barnyard: FUNCTION 00503800
void APrecisionSquirt::CreateFromGoal( AInteriorGoal* a_pGoal )
{
	TVector4 vOffset( 0.0f, -1.0f, 0.0f, 1.0f );

	// One unit above the goal
	TVector4 vPosition;
	a_pGoal->GetPosition( vPosition );
	vPosition.y -= 1.0f;

	const TPString8& rcName = *a_pGoal->m_strName;

	if ( rcName == TPS8( spawn ) )
		return;

	if ( rcName == TPS8( respawn ) )
	{
		ASpawnPoint oSpawnPoint;
		TINT        iIndex       = m_iNumSpawnPoints;
		m_aSpawnPoints[ iIndex ] = oSpawnPoint;
		vPosition.y -= -1.0f;
		m_iNumSpawnPoints++;
		m_aSpawnPoints[ iIndex ].SetPosition( vPosition );
		m_aSpawnPoints[ iIndex ].SetArea( a_pGoal->m_iAreaIndex );
		return;
	}

	if ( rcName == TPS8( target ) )
	{
		m_oTargetPositions.Add( vPosition );
		return;
	}

	if ( rcName == TPS8( target2 ) )
	{
		m_oTargetPositions2.Add( vPosition );
		return;
	}

	if ( rcName == TPS8( target3 ) )
	{
		m_oTargetPositions3.Add( vPosition );
		return;
	}

	if ( rcName == TPS8( coyote ) )
	{
		TINT           iIndex = m_iNumCoyotes;
		AUnitTransform oTransform( vPosition, TVector4::VEC_NEGZ, 0, -1, -1, TTRUE );

		APrecisionCoyote* pCoyote = TSTATICCAST( APrecisionCoyote, AUnitManager::GetSingleton()->SpawnUnit( TPS8( sharp_coyote ), &oTransform ) );
		pCoyote->ChangeSteer( &TGetClass( AInteriorSteer ) );
		m_apCoyotes[ m_iNumCoyotes++ ] = pCoyote;

		m_apCoyotes[ iIndex ]->m_vSpawnPosition = vPosition;
		m_apCoyotes[ iIndex ]->m_iSpawnArea     = a_pGoal->m_iAreaIndex;
		m_apCoyoteSteers[ iIndex ]              = TSTATICCAST( AInteriorSteer, pCoyote->GetSteer() );
		m_apCoyoteSteers[ iIndex ]->ConnectToNetwork( m_pNetwork );
		return;
	}

	if ( rcName == TPS8( racoon ) )
	{
		TINT           iIndex = m_iNumRacoons;
		AUnitTransform oTransform( vPosition, TVector4::VEC_NEGZ, 0, -1, -1, TTRUE );

		APrecisionRacoon* pRacoon = TSTATICCAST( APrecisionRacoon, AUnitManager::GetSingleton()->SpawnUnit( TPS8( sharp_racoon ), &oTransform ) );
		pRacoon->ChangeSteer( &TGetClass( AInteriorSteer ) );

		// NOTE: retail checks the limit after spawning the raccoon (it's left in the world)
		if ( m_iNumRacoons >= MAX_RACOONS )
			return;

		m_apRacoons[ m_iNumRacoons++ ] = pRacoon;

		m_apRacoons[ iIndex ]->m_vSpawnPosition = vPosition;
		m_apRacoons[ iIndex ]->m_iSpawnArea     = a_pGoal->m_iAreaIndex;
		m_apRacoonSteers[ iIndex ]              = TSTATICCAST( AInteriorSteer, pRacoon->GetSteer() );
		m_apRacoonSteers[ iIndex ]->ConnectToNetwork( m_pNetwork );
		return;
	}

	if ( rcName == TPS8( cow1 ) || rcName == TPS8( cow2 ) || rcName == TPS8( cow3 ) || rcName == TPS8( cow4 ) )
	{
		TINT iCowIndex = 3;

		if ( rcName == TPS8( cow1 ) )
			iCowIndex = 0;
		else if ( rcName == TPS8( cow2 ) )
			iCowIndex = 1;
		else if ( rcName == TPS8( cow3 ) )
			iCowIndex = 2;

		// Another goal of a cow that's already there
		for ( TINT i = m_iNumCows; i != 0; i-- )
		{
			if ( m_apCows[ i - 1 ]->m_iCowIndex == iCowIndex )
			{
				m_apCows[ i - 1 ]->m_oWaypoints.Add( vPosition );
				return;
			}
		}

		TINT           iIndex = m_iNumCows;
		AUnitTransform oTransform( vPosition, TVector4::VEC_NEGZ, 0, iCowIndex, -1, TTRUE );

		APrecisionJersyCow* pCow = TSTATICCAST( APrecisionJersyCow, AUnitManager::GetSingleton()->SpawnUnit( TPS8( sharp_jersy ), &oTransform ) );
		pCow->ChangeSteer( &TGetClass( AInteriorSteer ) );
		pCow->m_oWaypoints.Add( vPosition );
		pCow->m_iCowIndex        = iCowIndex;
		m_apCows[ m_iNumCows++ ] = pCow;

		m_apCowSteers[ iIndex ] = TSTATICCAST( AInteriorSteer, pCow->GetSteer() );
		m_apCowSteers[ iIndex ]->ConnectToNetwork( m_pNetwork );
		return;
	}

	if ( rcName == TPS8( scarecrow ) )
	{
		TVector4 vScarecrowPosition;
		vScarecrowPosition.Substract( vPosition, vOffset );

		// The pumpkin on its head
		AUnitTransform oPumpkinTransform( vScarecrowPosition, TVector4::VEC_NEGZ, 0, -1, -1, TTRUE );
		AUnit*         pPumpkinUnit = AUnitManager::GetSingleton()->SpawnUnit( TPS8( precision_pumpkin ), &oPumpkinTransform );
		pPumpkinUnit->ChangeSteer( &TGetClass( AThrowItemSteer ) );

		// NOTE: retail copies the rest from the leftovers of the position on the stack
		Pumpkin oPumpkin;
		oPumpkin.pUnit    = pPumpkinUnit;
		oPumpkin.iMapIcon = -1;
		oPumpkin.bUnk8    = TFALSE;
		m_oPumpkins.Add( oPumpkin );

		// The knocked over scarecrow
		TINT           iKnockedOver = m_iNumKnockedOver;
		AUnitTransform oKnockedOverTransform( vScarecrowPosition, TVector4::VEC_NEGZ, 0, -1, -1, TTRUE );
		oKnockedOverTransform.m_iModelIndex = 1;

		APrecisionScarecrow* pKnockedOver = TSTATICCAST( APrecisionScarecrow, AUnitManager::GetSingleton()->SpawnUnit( TPS8( precision_scarecrow ), &oKnockedOverTransform ) );
		pKnockedOver->ChangeSteer( &TGetClass( AStationarySteer ) );
		m_apKnockedOver[ m_iNumKnockedOver++ ]            = pKnockedOver;
		m_apKnockedOver[ iKnockedOver ]->m_vSpawnPosition = vScarecrowPosition;

		// The standing one
		TINT           iScarecrow = m_iNumScarecrows;
		AUnitTransform oScarecrowTransform( vScarecrowPosition, TVector4::VEC_NEGZ, 0, -1, -1, TTRUE );
		oScarecrowTransform.m_iModelIndex = 0;

		APrecisionScarecrow* pScarecrow = TSTATICCAST( APrecisionScarecrow, AUnitManager::GetSingleton()->SpawnUnit( TPS8( precision_scarecrow ), &oScarecrowTransform ) );
		pScarecrow->ChangeSteer( &TGetClass( AStationarySteer ) );
		m_apScarecrows[ m_iNumScarecrows++ ]           = pScarecrow;
		m_apScarecrows[ iScarecrow ]->m_vSpawnPosition = vScarecrowPosition;
		return;
	}

	if ( rcName == TPS8( pyramid ) )
		m_vPyramid.Substract( vPosition, vOffset );
}

// $Barnyard: FUNCTION 00503e50
void APrecisionSquirt::CreateTargets()
{
	for ( TINT i = MAX_TARGETS; i != 0; i-- )
	{
		AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_POSZ, 0, -1, -1, TTRUE );

		APrecisionTargetItem* pTarget = TSTATICCAST( APrecisionTargetItem, AUnitManager::GetSingleton()->SpawnUnit( TPS8( precisiontarget ), &oTransform ) );
		pTarget->ChangeSteer( &TGetClass( AThrowItemSteer ) );
		pTarget->SetPosition( TVector4::VEC_ZERO );
		pTarget->Park( TFALSE );
		m_apTargets[ m_iNumTargets++ ] = pTarget;
	}
}

// $Barnyard: FUNCTION 00503ee0
void APrecisionSquirt::PlaceTargets( TINT a_iNumTargets )
{
	for ( TINT i = m_iNumTargets; i != 0; i-- )
		m_apTargets[ i - 1 ]->Park( TFALSE );

	// NOTE: retail loops forever if the network has no target goals at all
	// The round's list first, then the next ones. Once a list is used up, its next
	// targets are stacked one unit higher.
	TINT iList        = m_iRound;
	TINT iIndex       = 0;
	TINT iNumStacked1 = 0;
	TINT iNumStacked2 = 0;
	TINT iNumStacked3 = 0;

	for ( TINT i = a_iNumTargets; i != 0; i-- )
	{
		TVector4 vPosition;

		while ( TTRUE )
		{
			if ( iList == 1 )
			{
				if ( iIndex < m_oTargetPositions.iCount )
				{
					vPosition = m_oTargetPositions.aPositions[ iIndex ];

					for ( TINT j = iNumStacked1; j != 0; j-- )
						vPosition.y -= 1.0f;

					break;
				}

				iIndex = 0;
				iList  = 2;
				iNumStacked1++;
			}
			else if ( iList == 2 )
			{
				if ( iIndex < m_oTargetPositions2.iCount )
				{
					vPosition = m_oTargetPositions2.aPositions[ iIndex ];

					for ( TINT j = iNumStacked2; j != 0; j-- )
						vPosition.y -= 1.0f;

					break;
				}

				iIndex = 0;
				iList  = 3;
				iNumStacked2++;
			}
			else if ( iList == 3 )
			{
				if ( iIndex < m_oTargetPositions3.iCount )
				{
					vPosition = m_oTargetPositions3.aPositions[ iIndex ];

					for ( TINT j = iNumStacked3; j != 0; j-- )
						vPosition.y -= 1.0f;

					break;
				}

				iIndex = 0;
				iList  = 1;
				iNumStacked3++;
			}
		}

		m_apTargets[ i - 1 ]->SetPosition( vPosition );
		iIndex++;
		m_apTargets[ i - 1 ]->Park( TTRUE );
	}
}

// $Barnyard: FUNCTION 005040c0
void APrecisionSquirt::SetupRoundHud()
{
	m_oTargetCounter.SetCount( m_iNumTargetsLeft, TFALSE );
	m_oLifeHud.SetLives( 0 );
	m_oPortrait.SetScore( m_iScore );
	m_oFloats.Reset();
	m_oTimeLeftHudElement.GetTimer().SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );

	switch ( m_iVariant )
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			m_oTargetCounter.SetIcon( "tin_can" );
			m_oTargetCounter.SetLayout( AGUI2TargetCounter::LAYOUT_ICON_RIGHT );
			m_oPortrait.Show();
			break;
		case 7:
		case 8:
		case 9:
			m_oTargetCounter.SetIcon( "Scarecrow" );
			m_oTargetCounter.SetLayout( AGUI2TargetCounter::LAYOUT_ICON_RIGHT );
			m_oPortrait.Hide();
			break;
		default:
			break;
	}

	// The map of the variant
	m_oMap.Reset();
	// NOTE: retail also calls a function that does nothing (005acd80) with the map

	switch ( m_iVariant )
	{
		case 0:
		case 3:
			m_oMap.Create( "Map_Chasing_Chicks", 0.3f, 0.3f, m_oMapViewport, TVector4( 25.433218f, -10.1f, 0.34694499f, 0.0f ), TVector2( 309.0f, 228.0f ), TVector4( -5.5460191f, -10.1f, -4.6532469f, 0.0f ), TVector2( 161.0f, 249.0f ), TTRUE, TNULL, TTRUE, 50 );
			m_oMap.SetZoom( 0.25f );
			break;
		case 1:
			m_oMap.Create( "Map_Bens_Hill", 0.3f, 0.3f, m_oMapViewport, TVector4( -27.317625f, -10.1f, 87.359245f, 0.0f ), TVector2( 324.0f, 125.0f ), TVector4( -41.167221f, -10.1f, 53.5173f, 0.0f ), TVector2( 220.0f, 428.0f ), TTRUE, TNULL, TTRUE, 50 );
			m_oMap.SetZoom( 0.25f );
			break;
		case 2:
			m_oMap.Create( "Map_Beady_Farm", 0.3f, 0.3f, m_oMapViewport, TVector4( 223.74985f, -10.1f, 96.735275f, 0.0f ), TVector2( 123.0f, 339.0f ), TVector4( 256.55115f, -10.1f, 128.92354f, 0.0f ), TVector2( 287.0f, 174.0f ), TTRUE, TNULL, TTRUE, 50 );
			m_oMap.SetZoom( 0.25f );
			break;
		case 4:
			m_oMap.Create( "Map_Golf_Course", 0.3f, 0.3f, m_oMapViewport, TVector4( 239.40077f, -5.0f, -47.342949f, 0.0f ), TVector2( 216.0f, 226.0f ), TVector4( 224.22229f, -5.0f, -103.54791f, 0.0f ), TVector2( 159.0f, 438.0f ), TTRUE, TNULL, TTRUE, 50 );
			m_oMap.SetZoom( 0.25f );
			break;
		case 5:
			m_oMap.Create( "Map_Junkyard", 0.3f, 0.3f, m_oMapViewport, TVector4( -263.68103f, -5.0f, -71.306435f, 0.0f ), TVector2( 272.0f, 414.0f ), TVector4( -340.18753f, -5.0f, -50.132076f, 0.0f ), TVector2( 307.0f, 86.0f ), TTRUE, TNULL, TTRUE, 50 );
			m_oMap.SetZoom( 0.25f );
			break;
		case 6:
		case 7:
		case 8:
		case 9:
		{
			TVector4    vBottomRight;
			TVector4    vTopLeft;
			TQuaternion oRotation;
			ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( "map_bottom_right", vBottomRight, oRotation );
			ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( "map_top_left", vTopLeft, oRotation );

			m_oMap.Create( "Map_Woods", 0.3f, 0.3f, m_oMapViewport, vBottomRight, TVector2( 122.0f, 380.0f ), vTopLeft, TVector2( 376.0f, 70.0f ), TTRUE, TNULL, TTRUE, 50 );
			m_oMap.SetZoom( 0.51f );
			break;
		}
		default:
			break;
	}

	m_oMap.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_BOTTOMLEFT );
	m_oMap.Show();
	m_GUIElement.AddChildTail( m_oMap );

	ASteer* pPlayerSteer = m_pPlayer->GetSteer();
	m_oMap.AddIcon( *pPlayerSteer->GetPosition(), *pPlayerSteer->GetDirection(), AGUI2MiniMap::ICONTYPE_PLAYER, AGUI2MiniMap::ICON_PLAYER, 0.15f );

	for ( TINT i = m_iNumRacoons; i != 0; i-- )
		m_apRacoons[ i - 1 ]->m_iMapIcon = m_oMap.AddIcon( *m_apRacoons[ i - 1 ]->GetSteer()->GetPosition(), TVector4::VEC_POSZ, AGUI2MiniMap::ICONTYPE_EDGE, AGUI2MiniMap::ICON_ENEMY, 0.15f );

	for ( TINT i = m_iNumCoyotes; i != 0; i-- )
		m_apCoyotes[ i - 1 ]->m_iMapIcon = m_oMap.AddIcon( *m_apCoyotes[ i - 1 ]->GetSteer()->GetPosition(), TVector4::VEC_POSZ, AGUI2MiniMap::ICONTYPE_EDGE, AGUI2MiniMap::ICON_ENEMY, 0.15f );

	for ( TINT i = m_oPumpkins.iCount; i != 0; i-- )
		m_oPumpkins.aPumpkins[ i - 1 ].iMapIcon = m_oMap.AddIcon( *m_oPumpkins.aPumpkins[ i - 1 ].pUnit->GetSteer()->GetPosition(), TVector4::VEC_POSZ, AGUI2MiniMap::ICONTYPE_EDGE, AGUI2MiniMap::ICON_APPLES, 0.15f );

	for ( TINT i = m_pGameData->aRounds[ m_iRound - 1 ].iNumTargets; i != 0; i-- )
		m_apTargets[ i - 1 ]->m_iMapIcon = m_oMap.AddIcon( *m_apTargets[ i - 1 ]->GetSteer()->GetPosition(), TVector4::VEC_POSZ, AGUI2MiniMap::ICONTYPE_EDGE, AGUI2MiniMap::ICON_ITEM, 0.114f );

	for ( TINT i = m_iNumCows; i != 0; i-- )
		m_apCows[ i - 1 ]->m_iMapIcon = m_oMap.AddIcon( *m_apCows[ i - 1 ]->GetSteer()->GetPosition(), TVector4::VEC_POSZ, AGUI2MiniMap::ICONTYPE_EDGE, AGUI2MiniMap::ICON_ENEMY, 0.15f );

	m_oMap.SetColours( 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF );

	pPlayerSteer = m_pPlayer->GetSteer();
	m_oMap.SetCentre( *pPlayerSteer->GetPosition(), *pPlayerSteer->GetDirection() );
}

// $Barnyard: FUNCTION 00504850
void APrecisionSquirt::UpdateGameOver( TFLOAT a_fDeltaTime )
{
	m_fElapsedTime -= a_fDeltaTime;

	if ( m_fRoundEndTimer <= 0.0f )
	{
		if ( m_pPlayer )
		{
			m_pPlayer->GetActiveLauncher()->Deactivate();

			ASimplePlayerControlComponent* pControl = TSTATICCAST( ASimplePlayerControlComponent, m_pPlayer->FindComponent( &TGetClass( AControlComponent ) ) );
			pControl->SetLauncherInput( TFALSE );
			pControl->SetSecondaryLauncherInput( TFALSE );
			pControl->SetMovementEnabled( TFALSE );
			pControl->SetCanSprint( TFALSE );
			TSTATICCAST( AAlienAnimController, m_pPlayer->GetAnimatableObject()->FindAnimController( &TGetClass( AAlienAnimController ) ) )->SetAnimEnabled( TFALSE );
		}

		m_fRoundEndTimer = 0.1f;
		GetResults();
		m_oTimeLeftHudElement.SetForceHide( TTRUE );
		m_GUIElement.Hide();
	}

	UpdateResultAnimation();
	m_fRoundEndTimer += a_fDeltaTime;

	// The results, 6 seconds later
	if ( m_fRoundEndTimer >= 6.0f && !m_bGameOverPushed )
	{
		m_bGameOverPushed = TTRUE;
		AGameStateController::GetSingleton()->PushState( new AAnticGameOverState( &m_oResults, TFALSE ) );
	}
}

// $Barnyard: FUNCTION 005049b0
void APrecisionSquirt::OnStartHudFinished()
{
	AMiniGameState::OnStartHudFinished();

	TSTATICCAST( ACharacterMovementComponent, m_pPlayer->FindComponent( &TGetClass( ACharacterMovementComponent ) ) )->SetRunningSpeed( ACharacterMovementComponent::APPLY_IF_MOVING, 0.0f );
	m_pPlayer->GetAnimatableObject()->KillAllAnimations();
	m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );

	if ( ASteer* pSteer = m_pPlayer->GetSteer() )
		TSTATICCAST( APlayerSteer, pSteer )->SetState( APlayerSteer::STATE_WALKING );

	m_oLifeHud.SetLives( 0 );
	m_oLifeHud.AddLives( m_iLives );
	m_oFloats.Show();
	m_oTimeLeftHudElement.SetForceHide( TFALSE );
	m_GUIElement.Show();
	m_oTimeLeftHudElement.GetTimer().SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );

	if ( m_iVariant < 5 )
		m_oLifeHud.SetVisible( TFALSE );
	else
		m_oLifeHud.SetVisible( TTRUE );

	m_oReticles.SetActive( TTRUE );
	m_ePhase = PHASE_PLAYING;
	m_pPlayer->SetMovement( TTRUE );

	// Full milk
	ALaunchAmmo* pAmmo = m_pPlayer->GetActiveLauncher()->GetAmmo();
	pAmmo->SetAmmo( TMath::Min( pAmmo->GetMaxAmmo(), m_pPlayer->GetActiveLauncher()->GetAmmo()->GetMaxAmmo() ) );
	m_pPlayer->GetActiveLauncher()->Activate();

	m_oTargetIndicator.SetVisible( TFALSE );

	m_pControl->SetLauncherInput( TTRUE );
	m_pControl->SetSecondaryLauncherInput( TTRUE );
	m_pControl->SetMovementEnabled( TTRUE );
	m_pControl->SetCanSprint( TTRUE );
	TSTATICCAST( AAlienAnimController, m_pPlayer->GetAnimatableObject()->FindAnimController( &TGetClass( AAlienAnimController ) ) )->SetAnimEnabled( TTRUE );

	if ( m_iVariant == 3 )
		m_oPickUpHudElement.SetForceHide( TFALSE );

	for ( TINT i = 0; i < m_iNumTargets; i++ )
	{
		if ( m_apTargets[ i ] )
			m_apTargets[ i ]->SetActive( TTRUE );
	}

	T2_FOREACH( m_llCoins, it )
	{
		it->SetActive( TTRUE );
	}
}

// $Barnyard: FUNCTION 00504bb0
void APrecisionSquirt::UpdateCoins( TFLOAT a_fDeltaTime )
{
	for ( auto it = m_llCoins.Begin(); it != m_llCoins.End(); )
	{
		Coin* pCoin = it;
		it++;

		if ( TMath::SphereVsSphere( pCoin->vPosition, 1.0f, *m_pPlayer->GetSteer()->GetPosition(), 1.0f ) )
		{
			// Picked up
			StartCollectEffect( pCoin->pObject, 1, 30.0f );
			pCoin->Remove();
			m_llCollectedCoins.PushBack( pCoin );
			m_iNumCoinsCollected++;
			m_iNumCoinsLeft--;
		}
		else
		{
			pCoin->Update( a_fDeltaTime );
		}
	}
}

// $Barnyard: FUNCTION 00504c90
AMiniGameState::S1* APrecisionSquirt::GetResults()
{
	TINT iNumHit = m_iNumPreviousTargets + m_pGameData->aRounds[ m_iRound - 1 ].iNumTargets - m_iNumTargetsLeft;

	PlayerInfo& rInfo = m_oResults.oPlayerList[ 0 ];
	rInfo.bUnk1       = TTRUE;

	if ( m_iLives <= 0 || m_fElapsedTime >= m_fTimeLimit || m_iNumTargetsLeft != 0 )
		m_oResults.iResult = -1;
	else
		m_oResults.iResult = 0;

	m_oResults.iMiniGameIndex = AMiniGameManager::GetSingleton()->GetNextMiniGameIndex();
	m_oResults.bWon           = m_oResults.iResult != -1;

	while ( rInfo.vecRows.Size() != 0 )
	{
		rInfo.vecRows[ rInfo.vecRows.Size() - 1 ] = ResultRow();
		rInfo.vecRows.PopBack();
	}

	// The time left
	if ( m_bSubmitHighScore && m_fTimeLimit >= m_fElapsedTime )
		m_iTimeBonus += TINT( TINT64( m_fTimeLimit - m_fElapsedTime ) );

	TINT iComboBonus = m_iScore - iNumHit;

	if ( m_iVariant != 6 )
	{
		if ( m_iVariant > 6 && m_iVariant <= 9 )
		{
			rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( LOCALE_WOODS_TARGETS ), iNumHit } );
			rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( LOCALE_TIME_BONUS ), m_iTimeBonus } );
		}
		else
		{
			rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( LOCALE_TARGETS ), iNumHit } );
			rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( LOCALE_COMBO_BONUS ), iComboBonus } );
			rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( LOCALE_TIME_BONUS ), m_iTimeBonus } );
		}
	}

	// The coins collected in the charity variant
	TINT iNumCollected = m_iNumCoins - m_iNumCoinsLeft;
	APlayerProfileManager::GetSingleton()->GetMainProfile()->SetMiniGamePickups( TMath::Max( iNumCollected, 0 ) );

	m_oResults.ComputeTotal( 0 );
	rInfo.iUnk2 = rInfo.iTotal;

	if ( !m_bSubmitHighScore )
		return &m_oResults;

	AHighScoreManager::GetSingleton()->SetChartEnabled( TFALSE );
	m_bSubmitHighScore = TFALSE;

	TFLOAT fScore = TFLOAT( rInfo.iTotal );
	TMath::Clip( fScore, 0.0f, 999999.0f );

	switch ( m_iVariant )
	{
		case 0:
		case 3:
			AHighScoreManager::AddScore( 12, fScore, m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
			break;
		case 1:
			AHighScoreManager::AddScore( 13, fScore, m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
			break;
		case 2:
			AHighScoreManager::AddScore( 14, fScore, m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
			break;
		case 4:
			AHighScoreManager::AddScore( 16, fScore, m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
			break;
		case 5:
			AHighScoreManager::AddScore( 17, fScore, m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
			break;
		case 7:
			AHighScoreManager::AddScore( 19, fScore, m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
			break;
		case 8:
			AHighScoreManager::AddScore( 20, fScore, m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
			break;
		case 9:
			AHighScoreManager::AddScore( 21, fScore, m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
			break;
		default:
			break;
	}

	return &m_oResults;
}

// $Barnyard: FUNCTION 00505040
void APrecisionSquirt::OnRemoval()
{
	m_oWindmills.OnDestroy();
	ARenderer::GetSingleton()->SetNear( m_fOldNear );
	m_fOldNear = 1.0f;
	m_oFloats.Destroy();
	m_oLifeHud.Destroy();

	for ( TINT i = 0; i < m_iNumTargets; i++ )
		AUnitManager::GetSingleton()->DestroyUnit( m_apTargets[ i ] );

	AAnimatableObjectManager* pObjectManager = AAnimatableObjectManager::GetSingleton();

	while ( !m_llCoins.IsEmpty() )
	{
		Coin* pCoin = m_llCoins.Head();
		pObjectManager->DestroyAnimatableObject( pCoin->pObject );
		pCoin->Remove();
		delete pCoin;
	}

	while ( !m_llCollectedCoins.IsEmpty() )
	{
		Coin* pCoin = m_llCollectedCoins.Head();
		pObjectManager->DestroyAnimatableObject( pCoin->pObject );
		pCoin->Remove();
		delete pCoin;
	}

	if ( m_pSunglasses )
	{
		if ( m_pPlayer )
			pObjectManager->DetachObject( m_pSunglasses, m_pPlayer->GetAnimatableObject() );

		pObjectManager->DestroyAnimatableObject( m_pSunglasses );
		m_pSunglasses = TNULL;
	}

	// Sunglasses picked up in the world
	for ( AAnimatableObject* pObject = AAnimatableObjectManager::GetAttachedObjectOfType( m_pPlayer->GetAnimatableObject(), TPS8( cb_llocator ), TPS8( sunglasses1 ), 0 ); pObject; pObject = AAnimatableObjectManager::GetAttachedObjectOfType( m_pPlayer->GetAnimatableObject(), TPS8( cb_llocator ), TPS8( sunglasses1 ), 0 ) )
		AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( pObject );

	ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 )->ResetTarget();
	AUnitManager::GetSingleton()->DestroyUnit( m_pPlayer );
	m_pPlayer = TNULL;
	APlayerProfileManager::GetSingleton()->GetMainProfile()->SetStaminaWasteSpeed( 0.8f );

	for ( TINT i = m_iNumCoyotes; i != 0; i-- )
		AUnitManager::GetSingleton()->DestroyUnit( m_apCoyotes[ i - 1 ] );

	for ( TINT i = 0; i < m_iNumCoyotes; i++ )
		m_apCoyotes[ i ] = TNULL;

	m_iNumCoyotes = 0;

	for ( TINT i = m_iNumRacoons; i != 0; i-- )
		AUnitManager::GetSingleton()->DestroyUnit( m_apRacoons[ i - 1 ] );

	for ( TINT i = 0; i < m_iNumRacoons; i++ )
		m_apRacoons[ i ] = TNULL;

	m_iNumRacoons = 0;

	for ( TINT i = m_iNumScarecrows; i != 0; i-- )
		AUnitManager::GetSingleton()->DestroyUnit( m_apScarecrows[ i - 1 ] );

	for ( TINT i = 0; i < m_iNumScarecrows; i++ )
		m_apScarecrows[ i ] = TNULL;

	m_iNumScarecrows = 0;

	for ( TINT i = m_iNumKnockedOver; i != 0; i-- )
		AUnitManager::GetSingleton()->DestroyUnit( m_apKnockedOver[ i - 1 ] );

	for ( TINT i = 0; i < m_iNumKnockedOver; i++ )
		m_apKnockedOver[ i ] = TNULL;

	m_iNumKnockedOver = 0;

	for ( TINT i = m_oPumpkins.iCount; i != 0; i-- )
		AUnitManager::GetSingleton()->DestroyUnit( m_oPumpkins.aPumpkins[ i - 1 ].pUnit );

	for ( TINT i = 0; i < m_oPumpkins.iCount; i++ )
	{
		m_oPumpkins.aPumpkins[ i ].pUnit    = TNULL;
		m_oPumpkins.aPumpkins[ i ].iMapIcon = 0;
		m_oPumpkins.aPumpkins[ i ].bUnk8    = TFALSE;
	}

	m_oPumpkins.iCount = 0;

	// NOTE: retail doesn't clear the cows
	for ( TINT i = m_iNumCows; i != 0; i-- )
		AUnitManager::GetSingleton()->DestroyUnit( m_apCows[ i - 1 ] );

	m_oMilkBottles.Clear();
	m_oPortrait.Destroy();
	m_oAmmoBar.RemoveSelf();
	m_oAmmoBacking.RemoveSelf();
	m_oTargetIndicator.SetVisible( TFALSE );
	ASimpleProjectileManager::GetSingleton()->DestroyAllPools();

	if ( AMilkManager::GetSingleton() )
		delete AMilkManager::GetSingleton();

	ACameraManager::GetSingleton()->DetachCameraHelpers();

	if ( m_iVariant == 3 )
		DestroyCharityActors();

	s_iTerrainSection = -1;
	AMusicManager::GetSingleton()->SetEnabled( TTRUE, TTRUE );
	AMultiplexStream::GetSingleton()->Close( TTRUE );
	AMiniGameState::OnRemoval();
}

// $Barnyard: FUNCTION 00505480
TBOOL APrecisionSquirt::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	// NOTE: retail calls a shared function returning TFALSE (006528c0)
	if ( FindChildState( &TGetClass( AAnticGameOverState ) ) )
		return TFALSE;

	if ( m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( pickup_l ), 0.0f ) )
		return TTRUE;

	if ( a_eInputCommand == AInputCommand_Interact )
	{
		if ( m_pPlayer->GetActiveLauncher()->GetClass()->IsA( &TGetClass( AAimedSquirtProjectileLauncher ) ) )
			return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

		if ( IsHoldingPumpkin() )
		{
			// Drop the pumpkin in front of the player
			AUnit* pPumpkin = m_pPlayer->GetAttachedUnit( TPS8( cb_rlocator ), -1 );
			m_pPlayer->DetachUnit( pPumpkin, TTRUE, TFALSE );

			TVector4 vDirection = *m_pPlayer->GetSteer()->GetDirection();
			TVector4 vVelocity;
			vVelocity.Multiply( vDirection, 1.6f );
			pPumpkin->GetSteer()->SetVelocity( vVelocity );
			TSTATICCAST( AVisibleUnit, pPumpkin )->GetCollisionObject()->Activate();

			AAnimatableObject* pObject = m_pPlayer->GetAnimatableObject();
			pObject->GetToshiAnimInterface().SetAnimationSet( pObject->GetObjectType()->FindAnimationSetOrDefault( TPS8( default ) ) );

			if ( !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) )
				m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );
		}
		else if ( m_pNearestPumpkin )
		{
			TBOOL bDefaultSet = m_pPlayer->GetAnimatableObject()->GetToshiAnimInterface().GetAnimationSet()->GetName() == TPS8( default );

			if ( bDefaultSet )
			{
				// Pick it up (attached by OnUnitAction)
				TVector4 vToPumpkin;
				vToPumpkin.Substract( *m_pNearestPumpkin->GetSteer()->GetPosition(), *m_pPlayer->GetSteer()->GetPosition() );
				vToPumpkin.Normalise();
				TSTATICCAST( APlayerSteer, m_pPlayer->GetSteer() )->TurnTowards( vToPumpkin );
				m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( pickup_l ) );
			}
		}

		return TTRUE;
	}

	if ( a_eInputCommand == AInputCommand_Shoot )
	{
		if ( IsHoldingPumpkin() )
			return TTRUE;

		if ( m_ePhase != PHASE_PLAYING )
			return TTRUE;

		return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
	}

	if ( a_eInputCommand == AInputCommand_Switch && IsHoldingPumpkin() && !m_pPlayer->GetActiveLauncher()->GetClass()->IsA( &TGetClass( AAimedSquirtProjectileLauncher ) ) )
		return TTRUE;

	return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $Barnyard: FUNCTION 00505840
void APrecisionSquirt::StartRound( TBOOL a_bRestart )
{
	m_bGameOverPushed = TFALSE;
	m_iCombo          = 0;
	m_iLastStreamId   = 0;
	m_fComboTimer     = 0.0f;

	if ( a_bRestart )
		m_iRound = 1;
	else
		// NOTE: IsGameOver has already moved on to the next round, so retail counts the targets of the new round
		m_iNumPreviousTargets += m_pGameData->aRounds[ m_iRound - 1 ].iNumTargets;

	if ( m_iRound == 1 )
	{
		m_fElapsedTime        = 0.0f;
		m_iScore              = 0;
		m_iNumPreviousTargets = 0;
		m_iTimeBonus          = 0;
		AMilkSquirtProjectileLauncher::ResetLastStreamId();
	}

	m_iNumRacoonsAtStart    = m_iNumRacoons;
	m_iNumScarecrowsAtStart = m_iNumScarecrows;
	m_iNumTargetsLeft       = 0;
	m_iLives                = 4;
	m_iNumCoinsCollected    = 0;
	m_eResult               = RESULT_PLAYING;
	m_fTimeLimit            = m_pGameData->aRounds[ m_iRound - 1 ].fTimeLimit;

	PlaceTargets( m_pGameData->aRounds[ m_iRound - 1 ].iNumTargets );
	m_iNumTargetsLeft = m_pGameData->aRounds[ m_iRound - 1 ].iNumTargets;

	// Sunglasses picked up in the world
	for ( AAnimatableObject* pObject = AAnimatableObjectManager::GetAttachedObjectOfType( m_pPlayer->GetAnimatableObject(), TPS8( cb_llocator ), TPS8( sunglasses1 ), 0 ); pObject; pObject = AAnimatableObjectManager::GetAttachedObjectOfType( m_pPlayer->GetAnimatableObject(), TPS8( cb_llocator ), TPS8( sunglasses1 ), 0 ) )
		AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( pObject );

	// Drop what the player holds
	if ( AUnit* pHeld = m_pPlayer->GetAttachedUnit( TPS8( cb_rlocator ), -1 ) )
		m_pPlayer->DetachUnit( pHeld, TTRUE, TFALSE );

	m_pPlayer->GetSteer()->SetTilt( 0.0f, 0.0f, TFALSE );

	if ( m_pSunglasses )
		m_pSunglasses->SetSkeletonUpdating( TTRUE, TTRUE );

	if ( a_bRestart )
	{
		// Back to the start
		AInteriorGoal* pSpawn = m_pNetwork->FindGoal( TPS8( spawn ) );
		ATerrainInterface::GetSingleton()->ForceSection( pSpawn->m_iGroupId, TTRUE );
		TSTATICCAST( APlayerSteer, m_pPlayer->GetSteer() )->PlaceAtNamedGoal( TPS8( spawn ), TTRUE, TTRUE );
	}

	m_pPlayer->SetMovement( TTRUE );

	AAnimatableObject* pPlayerObject = m_pPlayer->GetAnimatableObject();
	pPlayerObject->GetToshiAnimInterface().SetAnimationSet( pPlayerObject->GetObjectType()->FindAnimationSetOrDefault( TPS8( default ) ) );

	if ( !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) )
		m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );

	ACameraManager* pCameraManager = ACameraManager::GetSingleton();

	if ( pCameraManager->GetCurrentCameraHelper() != pCameraManager->GetSplitscreenCameraHelper( 1 ) )
	{
		pCameraManager->SetSplitscreenCameraHelper( 1 );
		ACameraHelper* pHelper = pCameraManager->GetSplitscreenCameraHelper( 1 );
		pHelper->SetTargetUnit( m_pPlayer );
		pHelper->ResetPosition( TTRUE );
	}

	m_pControl->SetCameraInput( TTRUE );

	for ( TINT i = m_iNumTargetsLeft; i != 0; i-- )
	{
		m_apTargets[ i - 1 ]->SetPlayer( m_pPlayer );
		m_apTargets[ i - 1 ]->SetHit( TFALSE, -1 );
	}

	for ( TINT i = m_iNumCoyotes; i != 0; i-- )
	{
		m_apCoyotes[ i - 1 ]->SetNetwork( m_pNetwork );
		m_apCoyotes[ i - 1 ]->SetPlayer( m_pPlayer );
		m_apCoyotes[ i - 1 ]->SetMilked( TFALSE );
	}

	if ( m_iVariant == 3 )
	{
		// All the coins are back
		m_iNumCoinsLeft = m_iNumCoins;

		while ( !m_llCollectedCoins.IsEmpty() )
		{
			Coin* pCoin = m_llCollectedCoins.Head();
			pCoin->Remove();
			m_llCoins.PushFront( pCoin );
		}

		T2_FOREACH( m_llCoins, it )
		{
			it->SetSkeletonUpdating( TTRUE );
		}

		APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();
		pProfile->SetMiniGamePickups( 0 );
		pProfile->AddMiniGamePickups( 0 );
	}

	for ( TINT i = m_iNumRacoons; i != 0; i-- )
	{
		m_apRacoons[ i - 1 ]->SetNetwork( m_pNetwork );
		m_apRacoons[ i - 1 ]->SetPlayer( m_pPlayer );
		m_apRacoons[ i - 1 ]->SetMilked( TFALSE );
	}

	for ( TINT i = m_iNumScarecrows; i != 0; i-- )
	{
		APrecisionScarecrow* pScarecrow = m_apScarecrows[ i - 1 ];
		Pumpkin&             rPumpkin   = m_oPumpkins.aPumpkins[ i - 1 ];

		m_apKnockedOver[ i - 1 ]->SetPlayer( m_pPlayer );
		rPumpkin.bUnk8 = TFALSE;
		rPumpkin.pUnit->ChangeSteer( &TGetClass( AThrowItemSteer ) );

		if ( TSTATICCAST( AVisibleUnit, rPumpkin.pUnit )->GetCollisionObject()->IsCollisionObjectLinked() )
			pScarecrow->SetPlayer( m_pPlayer );

		pScarecrow->SetPumpkin( rPumpkin.pUnit );
		pScarecrow->SetKnockedOver( m_apKnockedOver[ i - 1 ] );
		pScarecrow->SetHit( TFALSE );
	}

	m_pNearestPumpkin = TNULL;

	for ( TINT i = m_iNumCows; i != 0; i-- )
		m_apCows[ i - 1 ]->SetPlayer( m_pPlayer );

	m_oMilkBottles.SetAllCollected( TFALSE );
	m_fRoundEndTimer = 0.0f;
	SetupRoundHud();
}

// $Barnyard: FUNCTION 00505d80
void APrecisionSquirt::OnUnitAction( AAnimationBreakpointUnitAction::Event* a_pEvent )
{
	TRef<AUnitProxy> pUnitProxy = a_pEvent->pUnitProxy;
	TBOOL            bOther     = pUnitProxy->GetUnit() != m_pPlayer->GetSelfProxy()->GetUnit();

	// The player has picked up the pumpkin
	if ( !bOther && a_pEvent->eAction == AAnimationBreakpointUnitAction::ACTION_PICKUP && m_pNearestPumpkin )
	{
		if ( !m_pPlayer->GetAttachedUnit( TPS8( cb_rlocator ), -1 ) )
		{
			m_pPlayer->AttachUnit( m_pNearestPumpkin, TPS8( cb_rlocator ) );
			TSTATICCAST( AVisibleUnit, m_pNearestPumpkin )->GetCollisionObject()->UnlinkCollisionObject();
		}
	}
}

// $Barnyard: FUNCTION 00505e90
void APrecisionSquirt::UpdateMap( TFLOAT a_fDeltaTime )
{
	ASteer* pPlayerSteer = m_pPlayer->GetSteer();
	m_oMap.SetCentre( *pPlayerSteer->GetPosition(), *pPlayerSteer->GetDirection() );

	for ( TINT i = m_iNumRacoons; i != 0; i-- )
	{
		ASteer* pSteer = m_apRacoons[ i - 1 ]->GetSteer();
		m_oMap.SetIconTransform( m_apRacoons[ i - 1 ]->m_iMapIcon, *pSteer->GetPosition(), *pSteer->GetDirection() );
	}

	for ( TINT i = m_iNumCoyotes; i != 0; i-- )
	{
		ASteer* pSteer = m_apCoyotes[ i - 1 ]->GetSteer();
		m_oMap.SetIconTransform( m_apCoyotes[ i - 1 ]->m_iMapIcon, *pSteer->GetPosition(), *pSteer->GetDirection() );
	}

	for ( TINT i = m_oPumpkins.iCount; i != 0; i-- )
	{
		AAnimatedUnit* pPumpkin = TSTATICCAST( AAnimatedUnit, m_oPumpkins.aPumpkins[ i - 1 ].pUnit );

		TVector4 vPosition;
		pPumpkin->GetAnimatableObject()->GetModelInstance()->GetPosition( vPosition );

		TVector4 vUp;
		pPumpkin->GetAnimatableObject()->GetModelInstance()->GetUpVector( vUp );

		m_oMap.SetIconTransform( m_oPumpkins.aPumpkins[ i - 1 ].iMapIcon, vPosition, vUp );
	}

	for ( TINT i = m_pGameData->aRounds[ m_iRound - 1 ].iNumTargets; i != 0; i-- )
	{
		APrecisionTargetItem* pTarget = m_apTargets[ i - 1 ];

		TVector4 vDirection;
		vDirection.Substract( *pTarget->GetSteer()->GetPosition(), *m_pPlayer->GetSteer()->GetPosition() );
		vDirection.Normalise();

		m_oMap.SetIconTransform( pTarget->m_iMapIcon, *pTarget->GetSteer()->GetPosition(), vDirection );
	}

	for ( TINT i = m_iNumCows; i != 0; i-- )
	{
		ASteer* pSteer = m_apCows[ i - 1 ]->GetSteer();
		m_oMap.SetIconTransform( m_apCows[ i - 1 ]->m_iMapIcon, *pSteer->GetPosition(), *pSteer->GetDirection() );
	}
}

// $Barnyard: FUNCTION 00506170
void APrecisionSquirt::Pause()
{
	for ( TINT i = 0; i < m_iNumTargets; i++ )
	{
		if ( m_apTargets[ i ] )
			m_apTargets[ i ]->SetActive( TFALSE );
	}

	T2_FOREACH( m_llCoins, it )
	{
		it->SetActive( TFALSE );
	}

	m_GUIElement.Hide();
	m_oTimeLeftHudElement.SetForceHide( TTRUE );

	// Load the section of the start (the instruction screen may have been in another place)
	TPString8 strPicture;
	g_oLoadScreen.SetPicture( AMiniGameManager::GetSingleton()->GetLoadingScreenName( strPicture, AMiniGameManager::GetSingleton()->GetNextMiniGameIndex() ), TFALSE );

	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
	pTerrain->m_bIsLoaded       = TFALSE;
	pTerrain->SetGetCurrentVISGroupCallback( GetTerrainSection );
	pTerrain->Update();
	pTerrain->m_bIsLoaded = TTRUE;

	if ( !pTerrain->IsLoaded() && !g_oLoadScreen.IsLoadingScreen() )
		g_oLoadScreen.SetLoadingState( TTRUE, TTRUE );

	AGameLoader::Terrain_Load();
	g_oLoadScreen.Reset();
	g_oLoadScreen.SetLoadingState( TFALSE, TTRUE );

	if ( m_iVariant < 6 )
	{
		AGameStateController::GetSingleton()->PushState(
		    new ASSeqState( "SSSS_Intro", "SSSS_Intro", "SSSS_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
		);
	}
}

// $Barnyard: FUNCTION 00506330
void APrecisionSquirt::CreateCoins()
{
	m_oCoinSplineTRB.Load( "Data/MiniGameData/SharpSquirter/CharityCoinSpline.trb", 0 );
	m_oCoinSpline.Load( m_oCoinSplineTRB, "sharpsquirt" );

	AAnimatableObjectManager* pObjectManager = AAnimatableObjectManager::GetSingleton();

	for ( TINT i = 0; i < NUM_COINS; i++ )
	{
		TVector4 vPosition;
		m_oCoinSpline.GetPosition( TFLOAT( i ) * ( 1.0f / NUM_COINS ), vPosition );
		vPosition.y -= 0.5f;

		AAnimatableObjectType* pType = pObjectManager->FindType( TPS8( barnbuck ) );

		if ( !pType )
			continue;

		AAnimatableObject* pObject = pObjectManager->CreateAnimatableObject( pType, TNULL, 0 );

		if ( !pObject )
			continue;

		pObject->GetModelInstance()->SetTranslation( vPosition.AsVector3() );
		pObject->SetSkeletonUpdating( TTRUE, TTRUE );

		Coin* pCoin = new Coin( pObject, vPosition );

		if ( pCoin )
		{
			pCoin->Update( 0.0f );
			m_llCoins.PushBack( pCoin );
			m_iNumCoinsLeft++;
			m_iNumCoins++;
		}
		else
		{
			pObjectManager->DestroyAnimatableObject( pObject );
		}
	}

	m_oCoinSplineTRB.Close();
}

// $Barnyard: FUNCTION 005064d0
void APrecisionSquirt::OnInsertion()
{
	m_pPlayer  = TNULL;
	m_fOldNear = ARenderer::GetSingleton()->SetNear( 0.1f );

	// The variant ("precision1".."precision10")
	m_iVariant = AMiniGameManager::GetSingleton()->GetMiniGameVariant();

	AMiniGame oMiniGame( *AMiniGameManager::GetSingleton()->GetNextMiniGame() );

	if ( oMiniGame.GetUnk18() == 2 )
		m_iVariant += 6;
	else if ( oMiniGame.GetUnk18() == 3 )
		m_iVariant += 7;

	TMath::Clip( m_iVariant, 0, 9 );

	AGameTimeFXManager* pFXManager = AGameTimeFXManager::GetSingleton();

	switch ( m_iVariant )
	{
		case 0: m_strNetwork = TPS8( precision1 ); break;
		case 1: m_strNetwork = TPS8( precision2 ); break;
		case 2: m_strNetwork = TPS8( precision3 ); break;
		case 3: m_strNetwork = TPS8( precision4 ); break;
		case 4: m_strNetwork = TPS8( precision5 ); break;
		case 5: m_strNetwork = TPS8( precision6 ); break;
		case 6: m_strNetwork = TPS8( precision7 ); break;
		case 7: m_strNetwork = TPS8( precision8 ); break;
		case 8: m_strNetwork = TPS8( precision9 ); break;
		case 9: m_strNetwork = TPS8( precision10 ); break;
		default: break;
	}

	m_pNetwork = AInteriorNetworkManager::GetSingleton()->SetDefaultNetwork( m_strNetwork );

	AInteriorGoal* pSpawn = m_pNetwork->FindGoal( TPS8( spawn ) );
	s_iTerrainSection     = pSpawn->m_iGroupId;
	ACameraManager::GetSingleton()->DetachCameraHelpers();

	// The terrain
	switch ( m_iVariant )
	{
		case 0:
		case 1:
		case 2:
			Terrain_SetMain( TTRUE, pSpawn->m_iGroupId, TFALSE );
			m_iNumRounds = 3;
			m_oWindmills.CreateFarmObjects();
			break;
		case 3:
			Terrain_SetMain( TTRUE, pSpawn->m_iGroupId, TFALSE );
			m_iNumRounds = 1;
			m_oWindmills.CreateFarmObjects();
			break;
		case 4:
			Terrain_SetMain( TTRUE, pSpawn->m_iGroupId, TFALSE );
			pFXManager->SetForcedFX( pFXManager->GetWeatherSettings( AWeatherManager::WEATHERTYPE_NORMAL, AGameTimeManager::DAYPHASE_MORNING ) );
			m_iNumRounds = 3;
			m_oWindmills.CreateFarmObjects();
			break;
		case 5:
			Terrain_SetMain( TTRUE, pSpawn->m_iGroupId, TFALSE );
			pFXManager->SetForcedFX( pFXManager->FindFX( "JunkNight" ) );
			m_iNumRounds = 3;
			m_oWindmills.CreateFarmObjects();
			break;
		case 7:
		case 8:
		case 9:
			AGameLoader::Terrain_Set( AGameLoader::Terrain_EnvWoods, TFALSE, TTRUE, 0, 0, 0, 0 );
			pFXManager->SetForcedFX( pFXManager->FindFX( "Woods" ) );
			m_iNumRounds = 1;
			break;
		default:
			// NOTE: retail doesn't set a terrain for variant 6
			break;
	}

	AGameLoader::Terrain_Load();

	// The data of the variant
	m_oData.Load( "Data/MinigameData/SharpSquirter/Data.trb", 0 );

	TINT iNumVariants = 1;
	m_pGameData       = m_oData.CastSymbol<APrecisionSquirtGameData>( "GameData" );

	if ( m_oData.GetSymbolAddress( "NumGameVariants" ) )
		iNumVariants = *m_oData.CastSymbol<TINT>( "NumGameVariants" );

	m_pGameData += TMath::Min( iNumVariants - 1, m_iVariant );

	// The player
	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();

	if ( pPlayerManager->GetCurrentPlayerSet() != pPlayerManager->GetPlayerSet( 1 ) )
		pPlayerManager->SetCurrentPlayerSet( 1 );

	TInputDeviceController* pController = AInputHandler::GetSingleton()->GetMainController();
	pPlayerManager->GetPlayerSet( 1 )->Reset();
	pPlayerManager->GetPlayerSet( 1 )->AddHumanPlayer( pController, -1 );

	pSpawn->GetPosition( m_vSpawnPosition );
	// NOTE: retail turns the height of the spawn goal into a height above the ground
	m_vSpawnPosition.y = m_vSpawnPosition.y - ATerrainInterface::GetSingleton()->GetGroundHeightTop( m_vSpawnPosition.x, m_vSpawnPosition.z, TCollisionCommon::TOSHICGROUPFLAG( 0 ) );

	APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();
	TINT               iCostume = pProfile->GetCostume();

	AUnitTransform oTransform( m_vSpawnPosition, TVector4::VEC_NEGZ, 0, pProfile->GetGender(), -1, TTRUE );
	AUnitPlayer*   pPlayer = TSTATICCAST( AUnitPlayer, AUnitManager::GetSingleton()->SpawnUnit( TPS8( player_ps ), &oTransform ) );
	pPlayer->ChangeSteer( &TGetClass( AAlienSteer ) );
	pProfile->SetStaminaWasteSpeed( 0.0f );
	m_pPlayer = pPlayer;

	// The milk of the minigame
	pPlayer->GetActiveLauncher()->GetAmmo()->SetProjectileType( ASimpleProjectileManager::TYPE_MILK_PRECISION );
	pPlayer->ApplyMaterial( iCostume, TTRUE );

	m_pControl = TSTATICCAST( ASimplePlayerControlComponent, m_pPlayer->FindComponent( &TGetClass( ASimplePlayerControlComponent ) ) );
	m_pControl->SetUnk32( TFALSE );

	// The player wears sunglasses
	AAnimatableObjectManager* pObjectManager = AAnimatableObjectManager::GetSingleton();

	if ( AAnimatableObjectType* pType = pObjectManager->FindType( TPS8( sunglasses1 ) ) )
	{
		m_pSunglasses = pObjectManager->CreateAnimatableObject( pType, m_pPlayer, 0 );
		TINT8 iBone   = -1;

		if ( pProfile->GetGender() == APlayerProfile::GENDER_FEMALE )
			iBone = m_pSunglasses->GetModelInstance()->GetSkeleton()->GetBoneID( "americanf_loc", 0 );

		TINT iHeadBone = m_pPlayer->GetModelInstance()->GetSkeleton()->GetBoneID( "c_head", 0 );
		pObjectManager->AttachObject( m_pSunglasses, m_pPlayer->GetAnimatableObject(), iBone, iHeadBone, TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
		m_pSunglasses->SetSkeletonUpdating( TTRUE, TTRUE );
	}

	// Everet and Daisy watch the charity variant
	if ( m_iVariant == 3 )
	{
		CreateCharityActors( TTRUE );

		TVector4 vEveretPosition;
		vEveretPosition.Multiply( TVector4::VEC_POSX, 2.0f );
		vEveretPosition.Add( m_vSpawnPosition );
		vEveretPosition.w = m_vSpawnPosition.w;
		vEveretPosition.y = ATerrainInterface::GetSingleton()->GetGroundHeight( vEveretPosition, TCollisionCommon::TOSHICGROUPFLAG( 0 ) );

		TVector4 vEveretDirection;
		vEveretDirection.Substract( m_vSpawnPosition, vEveretPosition );
		vEveretDirection.y = 0.0f;
		vEveretDirection.Normalise();

		TVector4 vDaisyPosition;
		vDaisyPosition.Multiply( TVector4::VEC_POSX, 2.0f );
		vDaisyPosition.Add( m_vSpawnPosition );
		vDaisyPosition.Add( TVector4::VEC_NEGZ );
		vDaisyPosition.w = m_vSpawnPosition.w;
		vDaisyPosition.y = ATerrainInterface::GetSingleton()->GetGroundHeight( vDaisyPosition, TCollisionCommon::TOSHICGROUPFLAG( 0 ) );

		TVector4 vDaisyDirection;
		vDaisyDirection.Substract( m_vSpawnPosition, vDaisyPosition );
		vDaisyDirection.y = 0.0f;
		vDaisyDirection.Normalise();

		PlaceCharityActors( vEveretPosition, vDaisyPosition, vEveretDirection, vDaisyDirection );
		SetCharityActorsSkeletonUpdating( TTRUE );
	}

	ATerrainInterface::GetSingleton()->ForceSection( m_pNetwork->FindGoal( TPS8( spawn ) )->m_iGroupId, TTRUE );

	// Everything at the goals of the network
	for ( TINT i = 0; i < m_pNetwork->m_pINetwork->m_iNumGoals; i++ )
		CreateFromGoal( &m_pNetwork->m_pINetwork->m_pGoals[ i ] );

	CreateTargets();

	if ( !m_oMilkBottles.LoadFromLocators( ATerrainInterface::GetSingleton()->GetLocatorList() ) )
		m_oMilkBottles.LoadFromNetwork( m_pNetwork );

	if ( m_iVariant == 3 )
		CreateCoins();

	// The camera
	ACameraManager* pCameraManager = ACameraManager::GetSingleton();
	pCameraManager->SetSplitscreenCameraHelper( 1 );
	ACameraHelper* pHelper = pCameraManager->GetSplitscreenCameraHelper( 1 );
	pHelper->SetTargetUnit( m_pPlayer );
	pHelper->ResetPosition( TTRUE );

	// The HUD
	TFLOAT fWidth;
	TFLOAT fHeight;
	m_GUIElement.GetDimensions( fWidth, fHeight );
	m_oMapViewport.iX      = 0;
	m_oMapViewport.iY      = 0;
	m_oMapViewport.iHeight = TINT( fHeight );
	m_oMapViewport.iWidth  = TINT( fWidth );

	m_oFloats.Create( TNULL, "rekord26" );
	m_oFloats.Hide();

	m_oAmmoBar.Create( 32.5f, 1.625f, 0.3f );
	m_oAmmoBar.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oAmmoBacking.AddChildTail( m_oAmmoBar );
	m_oAmmoBacking.SetTextureSection( T2GUITextureSectionManager::GetTextureSection( "Score_Wood_Thin" ) );
	m_oAmmoBacking.SetWidth( m_oAmmoBar.GetWidth() + 26.0f );
	m_oAmmoBacking.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_GUIElement.AddChildTail( m_oAmmoBacking );

	m_oPortrait.Create( pProfile->GetHideTextureName( -1, APlayerProfile::GENDER_NONE ), 0x21, TNULL );
	m_oPortrait.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oAmmoBacking.AddChildTail( m_oPortrait );

	m_oTargetCounter.Create( "Sharp_Squirt" );
	m_oTargetCounter.SetBacking( "Score_Wood_Thin" );
	m_oTargetCounter.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );

	// Below the coin counter
	if ( m_iVariant == 3 )
		m_oTargetCounter.SetTransform( 0.0f, m_oPickUpHudElement.m_oBacking.GetHeight() * 1.1f );

	m_oTargetCounter.Show();
	m_GUIElement.AddChildTail( m_oTargetCounter );

	m_oTargetIndicator.Create( &m_GUIElement );
	m_oLifeHud.Create( &m_GUIElement, TNULL, 8, "Score_Wood_Thin", T2GUIATTACHMENT_BOTTOMRIGHT, T2GUIATTACHMENT_BOTTOMRIGHT, 1.0f, TFALSE );
	m_HUDParams.SetFlags( AHudElement::MASK_MINIGAME | AHudElement::MASK_UNK4 | AHudElement::MASK_INTERACTION | AHudElement::MASK_UNK8 | AHudElement::MASK_UNK9 | AHudElement::MASK_MINIMAP | AHudElement::MASK_UNK13 | AHudElement::MASK_UNK14 | AHudElement::MASK_UNK26 );

	m_oReticles.Create( m_pPlayer, 50.0f, &m_GUIElement );
	ASimpleProjectileManager::GetSingleton()->CreatePool( 25, ASimpleProjectileManager::TYPE_MILK_PRECISION );
	AMilkManager::Load( "Data/MilkParticles/SimMilk.trb" );

	m_iRound              = 1;
	m_iNumPreviousTargets = 0;
	StartRound( TTRUE );

	m_ePhase = PHASE_PRESTART;
	m_oTimeLeftHudElement.SetForceHide( TTRUE );

	// The intro cut-scene, around the player
	if ( m_iVariant < 6 )
	{
		ASSeqController::GetSingleton()->LoadScript( "SSSS_Intro.tso", TVector4::VEC_ZERO );
		ASSeq* pScript = ASSeqController::GetSingleton()->FindScript( "SSSS_Intro", TNULL );

		const TVector4* pPosition = m_pPlayer->GetSteer()->GetPosition();

		TMatrix44 oTransform = TMatrix44::IDENTITY;
		oTransform.m_f41     = pPosition->x;
		oTransform.m_f42     = pPosition->z;
		oTransform.m_f43     = -pPosition->y;
		oTransform.m_f44     = pPosition->w;

		if ( m_iVariant == 2 )
			oTransform.RotateZ( TMath::PI * 0.5f );

		pScript->SetTransform( oTransform );
	}

	AMusicManager::GetSingleton()->SetEnabled( TFALSE, TTRUE );
	AMultiplexStream::GetSingleton()->Open( "SharpSquirt" );
	AMiniGameState::OnInsertion();
}

// $Barnyard: FUNCTION 00507050
void APrecisionSquirt::UpdateGame( TFLOAT a_fDeltaTime )
{
	UpdateTargets( a_fDeltaTime );
	UpdateCoins( a_fDeltaTime );
	m_oWindmills.OnUpdate( a_fDeltaTime );
	m_oRoundText.Update( a_fDeltaTime );

#ifdef BARNYARD_LATEST
	// The latest version keeps the player still while the round or the game is over
	if ( ( m_ePhase == PHASE_ROUND_WON || m_ePhase == PHASE_GAME_OVER ) && m_pControl )
	{
		m_pControl->SetMovementEnabled( TFALSE );
		m_pControl->SetLauncherInput( TFALSE );
		m_pControl->SetSecondaryLauncherInput( TFALSE );
	}
#endif

	// Play again from the results
	if ( m_ePhase == PHASE_GAME_OVER && m_bGameOverPushed && !FindChildState( &TGetClass( AAnticGameOverState ) ) )
	{
		StartRound( TTRUE );
		OnStartHudFinished();
	}

	if ( m_ePhase == PHASE_ROUND_WON )
	{
		m_fElapsedTime -= a_fDeltaTime;

		if ( m_pPlayer && s_iNumCheers < 2 )
		{
			m_pPlayer->GetSteer()->SetTilt( 0.0f, 0.0f, TFALSE );

			AAnimatableObject* pObject = m_pPlayer->GetAnimatableObject();

			if ( !pObject->IsAnimPlaying( TPS8( winner2_start ), 0.0f ) &&
			     !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( winner2_loop ), 0.0f ) &&
			     !m_pPlayer->GetAnimatableObject()->IsAnimationPlanned( TPS8( winner2_start ) ) &&
			     !m_pPlayer->GetAnimatableObject()->IsAnimationPlanned( TPS8( winner2_loop ) ) )
			{
				s_iNumCheers++;
				m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( winner2_start ) );
			}
		}

		m_fRoundEndTimer += a_fDeltaTime;

		if ( m_fRoundEndTimer > 5.0f )
		{
			// The next round
			s_iNumCheers = 0;

			PPropertyValue oRound( m_iRound );
			ALocale::FormatTags( m_wszRoundText, TARRAYSIZE( m_wszRoundText ), ALocaleManager::Interface()->GetString( LOCALE_NEXT_ROUND ), oRound );

			m_oRoundText.m_fUnk1     = 0.0f;
			m_oRoundText.m_fUnk4     = 1.0f;
			m_oRoundText.m_fUnk3     = 1.0f;
			m_oRoundText.m_fUnk2     = 1.2f;
			m_oRoundText.m_fFadeTime = 3.0f;
			// NOTE: retail also passes ( 0.5f, 0.5f ) to a function that does nothing (00628f40)
			m_oRoundText.SetText( m_wszRoundText, 0xFF00FF00 );

			StartRound( TFALSE );
			m_fElapsedTime = 0.0f;
			OnStartHudFinished();
		}
	}

	AProjectileLauncher* pLauncher = m_pPlayer->GetActiveLauncher();

#ifdef BARNYARD_LATEST
	// NOTE: the Wii (80250274) selects the aimed launcher only while its aiming input is held and the round
	// is being played (Wii Remote controls). Only the end of the round is recovered: no launcher is updated
	if ( m_ePhase != PHASE_ROUND_WON && m_ePhase != PHASE_GAME_OVER )
#endif
		pLauncher->Update( a_fDeltaTime );

	if ( AMilkManager::GetSingleton() )
		AMilkManager::GetSingleton()->Update( a_fDeltaTime );

	m_oRoundText.Update( a_fDeltaTime );

	if ( m_ePhase != PHASE_PLAYING )
		return;

#ifdef BARNYARD_COMMUNITY_PATCH
	DevAutoPlay( a_fDeltaTime );
#endif

	m_oMilkBottles.Update( a_fDeltaTime, TTRUE );
	m_oAmmoBar.SetAmmo( TINT( pLauncher->GetAmmo()->GetAmmo() ), pLauncher->GetAmmo()->GetMaxAmmo() );
	m_oTargetCounter.SetCount( m_iNumTargetsLeft, TFALSE );
	UpdateMap( a_fDeltaTime );
}

#ifdef BARNYARD_COMMUNITY_PATCH

// Development: "-devautoplay" runs. Every half a second the player squirts the
// nearest target that's still standing (with the aimed launcher if it's the active one).
void APrecisionSquirt::DevAutoPlay( TFLOAT a_fDeltaTime )
{
	if ( !T2String8::FindString( GetCommandLineA(), "-devautoplay" ) )
		return;

	static TFLOAT s_fTimer = 0.0f;
	s_fTimer += a_fDeltaTime;

	if ( s_fTimer < 0.5f )
		return;

	s_fTimer = 0.0f;

	ASteer*               pSteer    = m_pPlayer->GetSteer();
	APrecisionTargetItem* pTarget   = TNULL;
	TFLOAT                fDistance = 0.0f;

	for ( TINT i = 0; i < m_pGameData->aRounds[ m_iRound - 1 ].iNumTargets; i++ )
	{
		APrecisionTargetItem* pItem = m_apTargets[ i ];

		if ( !pItem->IsActive() || pItem->IsHit() || !pItem->IsVisible() )
			continue;

		TFLOAT fItemDistance = TVector4::DistanceSq( *pItem->GetSteer()->GetPosition(), *pSteer->GetPosition() );

		if ( !pTarget || fItemDistance < fDistance )
		{
			pTarget   = pItem;
			fDistance = fItemDistance;
		}
	}

	if ( !pTarget )
		return;

	const TVector4& rcTargetPos = *pTarget->GetSteer()->GetPosition();
	TVector4        vToTarget( rcTargetPos.x - pSteer->GetPosition()->x, 0.0f, rcTargetPos.z - pSteer->GetPosition()->z, 1.0f );
	vToTarget.Normalise();

	// Not too far away
	if ( TVector4::DistanceSqXZ( rcTargetPos, *pSteer->GetPosition() ) > 8.5f * 8.5f || TMath::Abs( rcTargetPos.y - pSteer->GetPosition()->y ) > 4.0f )
	{
		TVector4 vPosition( rcTargetPos.x - vToTarget.x * 8.0f, ATerrainInterface::GetSingleton()->GetGroundHeightTop( rcTargetPos.x - vToTarget.x * 8.0f, rcTargetPos.z - vToTarget.z * 8.0f, TCollisionCommon::TOSHICGROUPFLAG( 0 ) ), rcTargetPos.z - vToTarget.z * 8.0f, 1.0f );
		pSteer->SetPosition( vPosition );
	}

	pSteer->SetDirection( vToTarget );

	// The aimed squirt (the secondary launcher of the player)
	if ( !m_pPlayer->GetActiveLauncher()->GetClass()->IsA( &TGetClass( AAimedSquirtProjectileLauncher ) ) && m_pPlayer->GetLauncher( 1 ) &&
	     m_pPlayer->GetLauncher( 1 )->GetClass()->IsA( &TGetClass( AAimedSquirtProjectileLauncher ) ) )
	{
		// NOTE: only the active launcher gets the milk of the minigame (type 7) in OnInsertion, the pool of
		// the aimed launcher's own type isn't created
		m_pPlayer->GetLauncher( 1 )->GetAmmo()->SetProjectileType( ASimpleProjectileManager::TYPE_MILK_PRECISION );
		m_pPlayer->SetActiveLauncher( 1, TFALSE );
	}

	AProjectileLauncher* pLauncher = m_pPlayer->GetActiveLauncher();
	pLauncher->GetAmmo()->SetAmmo( pLauncher->GetAmmo()->GetMaxAmmo() );

	if ( pLauncher->GetClass()->IsA( &TGetClass( AAimedSquirtProjectileLauncher ) ) )
	{
		// A bit above the can, the milk drops a little
		TVector4 vAimPos = rcTargetPos;
		vAimPos.y -= 0.3f;
		TSTATICCAST( AAimedSquirtProjectileLauncher, pLauncher )->SetNPCControlled( TTRUE );
		TSTATICCAST( AAimedSquirtProjectileLauncher, pLauncher )->FireAt( vAimPos, TFALSE );
	}
	else
		pLauncher->Fire();

	TINFO( "[squirtauto] squirt at a target %.1f away (%d left, score %d, launcher %s)\n", TMath::Sqrt( fDistance ), m_iNumTargetsLeft, m_iScore, pLauncher->GetClass()->GetName() );
}

#endif
