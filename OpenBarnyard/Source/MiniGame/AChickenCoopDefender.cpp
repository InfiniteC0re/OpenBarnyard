#include "pch.h"
#include <Render/TCameraObject.h>
#include "AChickenCoopDefender.h"
#include "Helpers/APlayerProgress.h"
#include "AMiniGameManager.h"
#include "ALoadScreen.h"
#include "ARandom.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AModelRepos.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "Cameras/AGTAStyleCameraHelper.h"
#include "FX/AGameTimeFXManager.h"
#include "FX/AMilkManager.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AAnticGameOverState.h"
#include "GameInterface/ASSeqState.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AHighScoreManager.h"
#include "Helpers/AInstanceManager.h"
#include "HUD/AHudManager.h"
#include "Input/AInputMapManager.h"
#include "Input/AInputHandler.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Render/ARenderer.h"
#include "Sound/AMusicManager.h"
#include "Sound/APositionalSoundSet.h"
#include "Sound/ASoundManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Terrain/ATerrainLocator.h"
#include "Units/ASSeqController.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitPlayer.h"
#include "Units/AUnitTransform.h"
#include "Units/ASimplePlayerControlComponent.h"
#include "Units/AChickenCoopDefenderHen.h"
#include "Units/AChickenCoopDefenderEgg.h"
#include "Units/AChickenCoopDefenderThief.h"
#include "Units/Projectiles/AAimedSquirtProjectileLauncher.h"
#include "Units/Projectiles/AProjectileLauncher.h"
#include "Units/Projectiles/ASimpleProjectileManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "UnitMovement/Steering/APlayerSteer.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/Steering/AStationarySteer.h"
#include "Physics/ACollisionObject.h"
#include "Physics/AObjectHash.h"
#include "Physics/AObjectHashMain.h"
#include "Physics/ACollisionCommon.h"

#include <GUI/T2GUITextureSectionManager.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>
#include <Render/TRenderInterface.h>
#include <Render/TModel.h>
#include <Toshi/TStringManager.h>
#include <Toshi/T2String8.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

// Opcode defines PI
#undef PI

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00515c70, CreateObject 0051b550, CreateObjectInPlace 0051b5b0 (the constructor is inlined in both)
TDEFINE_CLASS( AChickenCoopDefender );

TPSTRING8_DECLARE( AHerdingMinigameState );
TPSTRING8_DECLARE( raccoon );
TPSTRING8_DECLARE( coyote );
TPSTRING8_DECLARE( Goal0 );
TPSTRING8_DECLARE( Goal1 );
TPSTRING8_DECLARE( Goal2 );
TPSTRING8_DECLARE( hanna );
TPSTRING8_DECLARE( sunglasses1 );
TPSTRING8_DECLARE( player_chickencoop );
TPSTRING8_DECLARE( chickencoop_raccoon );
TPSTRING8_DECLARE( chickencoop_coyote );
TPSTRING8_DECLARE( chickencoop_evilfreddy );
TPSTRING8_DECLARE( chickencoop_hen );
TPSTRING8_DECLARE( chickencoop_roast );
TPSTRING8_DECLARE( chickencoop_egg );
TPSTRING8_DECLARE( chickencoop_hanna );
TPSTRING8_DECLARE( PlayerStart0 );
TPSTRING8_DECLARE( cb_rlocator );
TPSTRING8_DECLARE( carry_overlay );
TPSTRING8_DECLARE( pickup );
TPSTRING8_DECLARE( pickup_raccoon );
TPSTRING8_DECLARE( picked_up );
TPSTRING8_DECLARE( throw );
TPSTRING8_DECLARE( throw_raccoon );
TPSTRING8_DECLARE( thrown );
TPSTRING8_DECLARE( yuk );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( defeat );
TPSTRING8_DECLARE( winner_start );
TPSTRING8_DECLARE( winner_loop );
TPSTRING8_DECLARE( loser_start );
TPSTRING8_DECLARE( loser_loop );

// Locale strings
static constexpr TINT LOCALE_PICK_UP             = 196; // "Pick up {0}"
static constexpr TINT LOCALE_RESULT_DEFAULT      = 677;
static constexpr TINT LOCALE_RACCOONS_MILK_HITS  = 684;
static constexpr TINT LOCALE_RACCOONS_THROWN     = 688;
static constexpr TINT LOCALE_RACCOONS_EGGS_LEFT  = 689;
static constexpr TINT LOCALE_RACCOONS_EGGS_SAVED = 690;
static constexpr TINT LOCALE_COYOTES_HENS_SAVED  = 691;
static constexpr TINT LOCALE_COYOTES_HENS_LEFT   = 692;
static constexpr TINT LOCALE_COYOTES_MILK_HITS   = 693;
static constexpr TINT LOCALE_FREDDY_MILK_HITS    = 694;
static constexpr TINT LOCALE_FREDDY_ROASTS_LEFT  = 695;
static constexpr TINT LOCALE_FREDDY_ROASTS_SAVED = 696;

// Where the player throws the raccoons (0x00773d78)
static const TVector4 s_aThrowTargets[ AChickenCoopDefender::NUM_THROW_TARGETS ] = {
	TVector4( 60.0f, -50.0f, -60.0f, 1.0f ),
	TVector4( 60.0f, -50.0f, 30.0f, 1.0f ),
	TVector4( 68.0f, -50.0f, 29.0f, 1.0f ),
	TVector4( 15.0f, -50.0f, 60.0f, 1.0f ),
	TVector4( 48.0f, -50.0f, 52.0f, 1.0f ),
	TVector4( -48.0f, -80.0f, 80.0f, 1.0f ),
	TVector4( -52.0f, -50.0f, -10.0f, 1.0f ),
	TVector4( -40.0f, -50.0f, -80.0f, 1.0f ),
};

static AToshiAnimationRef* GetOverlayAnim( AUnit* a_pUnit, const TPString8& a_rcName )
{
	AToshiAnimationInterface& rInterface = TSTATICCAST( AAnimatedUnit, a_pUnit )->GetAnimatableObject()->GetToshiAnimInterface();
	return AToshiAnimationInterface::GetAnimationRef( rInterface.GetOverlayAnimRefs(), a_rcName );
}

// Angle of a throw at a target at the distance a_fDistance (when the hot spot has no throw angle)
static TFLOAT GetThrowAngle( TFLOAT a_fDistance )
{
	if ( a_fDistance < 10.0f )
		return TMath::PI * 0.25f;
	else if ( a_fDistance < 20.0f )
		return TMath::PI * 0.16666667f;
	else if ( a_fDistance < 30.0f )
		return TMath::PI * 0.125f;
	else
		return TMath::PI * 0.083333336f;
}

// $Barnyard: FUNCTION 0051b1b0
AChickenCoopDefender::AChickenCoopDefender()
    : m_oGame( &m_fElapsedTime, &m_oCounter, &m_oLifeHud, &m_oMap )
    , m_pPlayer( TNULL )
    , m_pSunglasses( TNULL )
    , m_pNetwork( TNULL )
    , m_pUnitNetwork( TNULL )
    , m_fWaveTimer( 0.0f )
    , m_pArrowModel( TNULL )
    , m_pGlowObject( TNULL )
    , m_fOldNearClip( 1.0f )
    , m_bRenderReticles( TFALSE )
    , m_bGameOverStarted( TFALSE )
{
}

// $Barnyard: FUNCTION 0051b5f0
// $Barnyard: FUNCTION 0051b3c0
// $Barnyard: FUNCTION 0051b3d0
AChickenCoopDefender::~AChickenCoopDefender()
{
}

// NOTE: folded with AChasingChicksMiniGame::ProcessInput (0050e550)
TBOOL AChickenCoopDefender::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	return AGameState::ProcessInput( a_pInputEvent );
}

void AChickenCoopDefender::DisablePlayerControl()
{
	if ( m_pPlayer->GetActiveLauncher() != m_pPlayer->GetLauncher( 0 ) )
		m_pPlayer->SelectLauncher( TFALSE );

	m_pControl->SetMovementEnabled( TFALSE );
	m_pControl->SetLauncherInput( TFALSE );
	m_pControl->SetSecondaryLauncherInput( TFALSE );
}

void AChickenCoopDefender::EnablePlayerControl()
{
	m_pControl->SetMovementEnabled( TTRUE );
	m_pControl->SetLauncherInput( TTRUE );
	m_pControl->SetSecondaryLauncherInput( TTRUE );
}

// $Barnyard: FUNCTION 00516d20
TBOOL AChickenCoopDefender::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( a_eInputCommand != AInputCommand_Interact )
		return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	switch ( m_oGame.ePlayerState )
	{
		case PLAYERSTATE_FREE:
		{
			// Picks the milked raccoon up
			AUnit* pThief = m_oGame.pNearThief;

			if ( pThief && pThief->IsA( &TGetClass( AChickenCoopDefenderThief ) ) &&
			     TSTATICCAST( APlayerSteer, m_pPlayer->GetSteer() )->GetState() == APlayerSteer::STATE_WALKING &&
			     m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( pickup_raccoon ) ) )
			{
				TSTATICCAST( AAnimatedUnit, m_oGame.pNearThief )->GetAnimatableObject()->KillAllAnimations();
				TSTATICCAST( AAnimatedUnit, m_oGame.pNearThief )->GetAnimatableObject()->PlayAnimation( TPS8( picked_up ) );

				m_oGame.ePlayerState = PLAYERSTATE_PICKUP_THIEF;
				DisablePlayerControl();
			}

			return TTRUE;
		}
		case PLAYERSTATE_CARRY_HEN:
		{
			AChickenCoopDefenderHotSpot* pHotSpot = m_oGame.pHotSpot;

			if ( pHotSpot && !pHotSpot->bClosed )
			{
				// Throws the hen into the coop
				if ( m_pPlayer->GetAttachedUnit( TPS8( cb_rlocator ), -1 )->IsA( &TGetClass( AChickenCoopDefenderHen ) ) )
				{
					m_oGame.pHotSpot->SetClosed( TTRUE );

					// Another hot spot of the kind opens
					TINT iKind = m_oGame.pHotSpot->iKind;
					auto it    = m_oGame.aClosedHotSpots[ iKind ].Begin();

					for ( TINT iRand = ARandom::GetSingleton()->m_oRandom.GetInt( m_oGame.aClosedHotSpots[ iKind ].Size() ); iRand > 0; iRand-- )
						it++;

					it->SetClosed( TFALSE );

					DisablePlayerControl();
					m_oGame.ePlayerState = PLAYERSTATE_AIM_HEN;
				}

				return TTRUE;
			}

			// NOTE: retail goes on with the code of PLAYERSTATE_CARRY_EGG, which does nothing then
			return TTRUE;
		}
		case PLAYERSTATE_CARRY_EGG:
		{
			AChickenCoopDefenderHotSpot* pHotSpot = m_oGame.pHotSpot;

			if ( pHotSpot && !pHotSpot->bClosed )
			{
				// Throws the egg into the coop
				// NOTE: retail gets the egg and doesn't use it
				m_pPlayer->GetAttachedUnit( TPS8( cb_rlocator ), -1 );
				m_oGame.pHotSpot->SetClosed( TTRUE );

				// Another hot spot of the kind opens
				TINT iKind = m_oGame.pHotSpot->iKind;
				auto it    = m_oGame.aClosedHotSpots[ iKind ].Begin();

				for ( TINT iRand = ARandom::GetSingleton()->m_oRandom.GetInt( m_oGame.aClosedHotSpots[ iKind ].Size() ); iRand > 0; iRand-- )
					it++;

				it->SetClosed( TFALSE );

				DisablePlayerControl();
				m_oGame.ePlayerState = PLAYERSTATE_AIM_EGG;
			}

			return TTRUE;
		}
		default:
			return TTRUE;
	}
}

// $Barnyard: FUNCTION 00516400
void AChickenCoopDefender::OnRender( TFLOAT a_fDeltaTime )
{
	if ( g_oLoadScreen.IsLoadingScreen() )
		return;

	if ( m_bRenderReticles )
		m_oReticles.UpdateMiniGame( a_fDeltaTime );

	if ( m_oGame.pScoringNetwork && m_oGame.pHotSpots )
	{
		TBOOL bCarrying    = m_oGame.ePlayerState == PLAYERSTATE_CARRY_HEN || m_oGame.ePlayerState == PLAYERSTATE_CARRY_EGG;
		TINT  iNumHotSpots = m_oGame.pScoringNetwork->m_pINetwork->m_iNumNormalAreas;

		for ( TINT i = 0; i < iNumHotSpots; i++ )
		{
			AChickenCoopDefenderHotSpot& rHotSpot = m_oGame.pHotSpots[ i ];

			if ( ( bCarrying && !rHotSpot.bClosed ) || rHotSpot.eState != AChickenCoopDefenderHotSpot::STATE_IDLE )
				rHotSpot.Render( a_fDeltaTime );
		}
	}

	if ( AMilkManager::GetSingleton() )
		AMilkManager::GetSingleton()->Render();

	if ( AGameStateController::GetSingleton()->GetCurrentState()->GetClass() != &TGetClass( ASSeqState ) )
		m_oMilkBottles.Render();
}

// $Barnyard: FUNCTION 00515eb0
void AChickenCoopDefender::OnPostRender( TFLOAT a_fDeltaTime )
{
	if ( g_oLoadScreen.IsLoadingScreen() )
		return;

	if ( m_oGame.pScoringNetwork && m_oGame.pHotSpots )
	{
		TBOOL bCarrying    = m_oGame.ePlayerState == PLAYERSTATE_CARRY_HEN || m_oGame.ePlayerState == PLAYERSTATE_CARRY_EGG;
		TINT  iNumHotSpots = m_oGame.pScoringNetwork->m_pINetwork->m_iNumNormalAreas;

		for ( TINT i = 0; i < iNumHotSpots; i++ )
		{
			AChickenCoopDefenderHotSpot& rHotSpot = m_oGame.pHotSpots[ i ];

			if ( ( bCarrying && !rHotSpot.bClosed ) || rHotSpot.eState != AChickenCoopDefenderHotSpot::STATE_IDLE )
				rHotSpot.PostRender( a_fDeltaTime );
		}
	}
}

// $Barnyard: FUNCTION 005163d0
ASoundId AChickenCoopDefender::GetSound()
{
	switch ( AMiniGameManager::GetSingleton()->GetMiniGameVariant() )
	{
		case VARIANT_RACCOONS:
			return 23;
		case VARIANT_COYOTES:
			return 31;
		case VARIANT_FREDDYS_DREAM:
			return 34;
		default:
			return 33;
	}
}

// $Barnyard: FUNCTION 005178d0
void AChickenCoopDefender::SetThievesVisible( TBOOL a_bVisible )
{
	for ( TINT i = 0; i < m_oGame.oRaccoons.Size(); i++ )
		TSTATICCAST( AAnimatedUnit, m_oGame.oRaccoons[ i ]->GetUnit() )->SetVisible( a_bVisible );

	for ( TINT i = 0; i < m_oGame.oCoyotes.Size(); i++ )
		TSTATICCAST( AAnimatedUnit, m_oGame.oCoyotes[ i ]->GetUnit() )->SetVisible( a_bVisible );

	for ( TINT i = 0; i < m_oGame.oFreddies.Size(); i++ )
		TSTATICCAST( AAnimatedUnit, m_oGame.oFreddies[ i ]->GetUnit() )->SetVisible( a_bVisible );
}

// $Barnyard: FUNCTION 00517b20
void AChickenCoopDefender::OnPause()
{
	SetThievesVisible( TTRUE );

	m_pPlayer->SetVisible( TTRUE );
	m_pPlayer->SetMovement( TTRUE );
	m_pPlayer->SetAIEnabled( TTRUE );
	m_pPlayer->GetAnimatableObject()->SetAnimControllersDisabled( TFALSE );

	AGameState::OnPause();
}

// $Barnyard: FUNCTION 0051ab60
void AChickenCoopDefender::OnInsertion()
{
	m_pInputMap = AInputMapManager::GetSingleton()->FindMap( TPS8( AHerdingMinigameState ) );
	APositionalSoundSet::GetSingleton()->Load( "Data/Sound/Defender.trb" );

	TINT iVariant = AMiniGameManager::GetSingleton()->GetMiniGameVariant();

	if ( iVariant == VARIANT_RACCOONS )
		ASoundManager::GetSingleton()->LoadSoundBank( TPS8( raccoon ), TFALSE, TTRUE );
	else if ( iVariant == VARIANT_COYOTES )
		ASoundManager::GetSingleton()->LoadSoundBank( TPS8( coyote ), TFALSE, TTRUE );

	m_fOldNearClip = ARenderer::GetSingleton()->SetNear( 0.1f );

	AGameLoader::Terrain_Set( AGameLoader::Terrain_ChasingChicks2, TFALSE, TTRUE, 0, 0, 0, 0 );
	AGameLoader::Terrain_Load();

	m_oGame.pScoringNetwork = AInteriorNetworkManager::GetSingleton()->LoadNetwork( TPS8D( "chasingchicks_scoring" ) );

	TPString8 strNetwork = TPS8D( "chickencoop" );
	AInteriorNetworkManager::GetSingleton()->LoadNetwork( strNetwork );
	m_pNetwork = AInteriorNetworkManager::GetSingleton()->SetDefaultNetwork( strNetwork );

	if ( !m_oMilkBottles.LoadFromLocators( ATerrainInterface::GetSingleton()->GetLocatorList() ) )
		m_oMilkBottles.LoadFromNetwork( m_pNetwork );

	m_pUnitNetwork = m_pNetwork;

	// The coop and where the hens and eggs are thrown to
	m_pNetwork->FindGoal( TPS8( Goal0 ) )->GetPosition( AChickenCoopDefenderGame::ms_aGoals[ 0 ] );
	m_pNetwork->FindGoal( TPS8( Goal1 ) )->GetPosition( AChickenCoopDefenderGame::ms_aGoals[ 1 ] );
	m_pNetwork->FindGoal( TPS8( Goal2 ) )->GetPosition( AChickenCoopDefenderGame::ms_aGoals[ 2 ] );
	AChickenCoopDefenderGame::ms_aGoals[ 0 ].y -= 0.5f;
	AChickenCoopDefenderGame::ms_aGoals[ 1 ].y -= 0.5f;
	AChickenCoopDefenderGame::ms_aGoals[ 2 ].y -= 0.5f;

	// The minimap (the one of Chasing Chicks)
	m_oMap.Reset();

	AGUI2MiniMap::Viewport oViewport;
	oViewport.iX      = 0;
	oViewport.iY      = 0;
	oViewport.iWidth  = TINT( m_GUIElement.GetWidth() );
	oViewport.iHeight = TINT( m_GUIElement.GetHeight() );

	m_oMap.Create(
	    "Map_Chasing_Chicks",
	    0.3f,
	    0.3f,
	    oViewport,
	    TVector4( 25.433218f, -10.1f, 0.346945f, 1.0f ),
	    TVector2( 309.0f, 228.0f ),
	    TVector4( -5.546019f, -10.1f, -4.653247f, 1.0f ),
	    TVector2( 161.0f, 249.0f ),
	    TTRUE,
	    TNULL,
	    TTRUE,
	    64
	);
	m_oMap.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_BOTTOMLEFT );
	m_GUIElement.AddChildTail( m_oMap );
	m_oMap.SetZoom( 0.5f );

	LoadData();
	CreatePlayer( &m_oMap );
	CreateThieves();
	CreateHensAndEggs();
	CreateHanna();
	CreateModels();

	ASimpleProjectileManager::GetSingleton()->CreatePool( 10, ASimpleProjectileManager::TYPE_WITH_UNIT );
	ASimpleProjectileManager::GetSingleton()->CreatePool( 10, ASimpleProjectileManager::TYPE_CHICKEN_COOP_DEFENDER_THROW );
	ASimpleProjectileManager::GetSingleton()->CreatePool( 50, ASimpleProjectileManager::TYPE_CHICKEN_COOP_DEFENDER_MILK );

	if ( AMiniGameManager::GetSingleton()->GetMiniGameVariant() == VARIANT_FREDDYS_DREAM )
		AMilkManager::Load( "Data/MilkParticles/FreddysDreamMilk.trb" );
	else
		AMilkManager::Load( "Data/MilkParticles/SimMilk.trb" );

	m_fTimeLimit = m_oGame.pGameData->fTimeLimit;

	AHudManager::GetSingleton()->AddElement( &m_oInteractionHud, TFALSE, TTRUE, TFALSE );
	m_oInteractionHud.SetForceHide( TTRUE );

	// NOTE: retail gets the dimensions of the clock and doesn't use them
	TINT iWidth, iHeight;
	m_oTimeLeftHudElement.GetDimensions( iWidth, iHeight );

	m_oMap.SetColours( 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF );
	m_oMap.Show();

	// The hens/eggs left
	m_oCounter.Create( m_oGame.pGameData->szCounterIcon );
	m_oCounter.SetBacking( "Score_Wood_Thin" );
	m_oCounter.SetLayout( AGUI2TargetCounter::LAYOUT_ICON_RIGHT );
	m_oCounter.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );
	m_GUIElement.AddChildTail( m_oCounter );

	CreateHotSpots();

	m_oLifeHud.Create( &m_GUIElement, TNULL, 8, "Score_Wood_Thin", T2GUIATTACHMENT_BOTTOMRIGHT, T2GUIATTACHMENT_BOTTOMRIGHT, 1.0f, TFALSE );

	if ( m_oGame.pGameData->iLives <= -1 )
		m_oLifeHud.SetVisible( TFALSE );

	// It's night (Freddy's nightmare in his dream)
	AGameTimeFXManager* pFXManager = AGameTimeFXManager::GetSingleton();

	if ( AMiniGameManager::GetSingleton()->GetMiniGameVariant() == VARIANT_FREDDYS_DREAM )
		pFXManager->SetForcedFX( pFXManager->FindFX( "Nightmare" ) );
	else
		pFXManager->SetForcedFX( pFXManager->GetWeatherSettings( AWeatherManager::WEATHERTYPE_NORMAL, AGameTimeManager::DAYPHASE_NIGHT ) );

	m_bRenderBottles = TTRUE;

	iVariant = AMiniGameManager::GetSingleton()->GetMiniGameVariant();

	if ( iVariant == VARIANT_RACCOONS || iVariant == VARIANT_COYOTES )
		ASSeqController::GetSingleton()->LoadScript( "SSCCDef_ChickCoopDefender.tso", TVector4::VEC_ZERO );
	else if ( iVariant == VARIANT_FREDDYS_DREAM )
		ASSeqController::GetSingleton()->LoadScript( "SSEF_Main.tso", TVector4::VEC_ZERO );

	AMiniGameState::OnInsertion();
}

// $Barnyard: FUNCTION 0051b090
void AChickenCoopDefender::OnRemoval()
{
	ARenderer::GetSingleton()->SetNear( m_fOldNearClip );
	m_fOldNearClip = 1.0f;

	TRenderInterface::GetSingleton()->BeginEndSceneHAL();
	APositionalSoundSet::GetSingleton()->Unload();

	TINT iVariant = AMiniGameManager::GetSingleton()->GetMiniGameVariant();

	if ( iVariant == VARIANT_RACCOONS )
		ASoundManager::GetSingleton()->UnloadSoundBank( TPS8( raccoon ), TFALSE );
	else if ( iVariant == VARIANT_COYOTES )
		ASoundManager::GetSingleton()->UnloadSoundBank( TPS8( coyote ), TFALSE );

	m_oMilkBottles.Clear();
	ASimpleProjectileManager::GetSingleton()->DestroyAllPools();

	if ( AMilkManager::GetSingleton() )
		delete AMilkManager::GetSingleton();

	// NOTE: AChasingChicksMiniGame::ResetCamera (0050e870) is called by retail
	ACameraManager* pCameraManager = ACameraManager::GetSingleton();
	pCameraManager->ResetHelperTargets();
	pCameraManager->DetachSplitscreenCameraHelpers();
	pCameraManager->DetachCameraHelpers();

	DestroyHotSpots();

	if ( AGameLoader::IsStoryMode() )
		AMusicManager::GetSingleton()->SetEnabled( TTRUE, TTRUE );

	DestroyPlayer();
	DestroyThieves();
	DestroyHensAndEggs();

	if ( m_oGame.pHanna )
	{
		AUnitManager::GetSingleton()->DestroyUnit( m_oGame.pHanna );
		m_oGame.pHanna = TNULL;
	}

	DestroyModels();
	m_oInteractionHud.Detach();

	AGameTimeFXManager::GetSingleton()->ClearForcedFX();
	AMiniGameState::OnRemoval();
}

// $Barnyard: FUNCTION 00517000
void AChickenCoopDefender::OnActivate()
{
	SetupCamera();
	m_HUDParams.SetFlags( AHudElement::MASK_MINIGAME | AHudElement::MASK_INTERACTION | AHudElement::MASK_UNK8 );
	AMiniGameState::OnActivate();
}

// $Barnyard: FUNCTION 00515c90
void AChickenCoopDefender::OnDeactivate()
{
	m_HUDParams.SetFlags( 0 );
	AMiniGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 00516c00
void AChickenCoopDefender::Pause()
{
	m_bRenderBottles = TFALSE;

	TINT iVariant = AMiniGameManager::GetSingleton()->GetMiniGameVariant();

	if ( iVariant < VARIANT_RACCOONS )
		return;

	if ( iVariant <= VARIANT_COYOTES )
	{
		AGameStateController::GetSingleton()->PushState(
		    new ASSeqState( "SSCCDef_ChickCoopDefender", "SSCCDef_ChickCoopDefender", "SSCCDef_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
		);
	}
	else if ( iVariant == VARIANT_FREDDYS_DREAM )
	{
		AGameStateController::GetSingleton()->PushState(
		    new ASSeqState( "SSEF_Main", "SSEF_Main", "SSEF_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
		);
	}
}

// $Barnyard: FUNCTION 00516a80
void AChickenCoopDefender::PlayLoseSequence()
{
	TINT iVariant = AMiniGameManager::GetSingleton()->GetMiniGameVariant();

	if ( iVariant == VARIANT_RACCOONS || iVariant == VARIANT_COYOTES )
	{
		AMiniGameState::PlayLoseSequence();
	}
	else if ( iVariant == VARIANT_FREDDYS_DREAM )
	{
		AGameStateController::GetSingleton()->PushState(
		    new ASSeqState( "SSEF_Defeat", "SSEF_Defeat", "SSEF_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
		);
	}
}

// $Barnyard: FUNCTION 00516b40
void AChickenCoopDefender::PlayWinSequence()
{
	TINT iVariant = AMiniGameManager::GetSingleton()->GetMiniGameVariant();

	if ( iVariant == VARIANT_RACCOONS || iVariant == VARIANT_COYOTES )
	{
		AMiniGameState::PlayWinSequence();
	}
	else if ( iVariant == VARIANT_FREDDYS_DREAM )
	{
		AGameStateController::GetSingleton()->PushState(
		    new ASSeqState( "SSEF_Victory", "SSEF_Victory", "SSEF_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
		);
	}
}

// $Barnyard: FUNCTION 00517b70
void AChickenCoopDefender::OnPreStart()
{
	SetThievesVisible( TFALSE );

	m_pPlayer->SetVisible( TFALSE );
	m_pPlayer->SetMovement( TFALSE );
	m_pPlayer->SetAIEnabled( TFALSE );
	m_pPlayer->GetAnimatableObject()->SetAnimControllersDisabled( TTRUE );
}

// $Barnyard: FUNCTION 00515cb0
TBOOL AChickenCoopDefender::IsGameOver()
{
	if ( m_fElapsedTime <= m_fTimeLimit && m_oGame.iNumLeft > 0 && m_oGame.iLives != 0 )
		return TFALSE;

	m_oGame.ePhase = PHASE_ENDED;
	return TTRUE;
}

// NOTE: folded with AChasingChicksMiniGame::UpdateAlways (00515e90)
void AChickenCoopDefender::UpdateAlways( TFLOAT a_fDeltaTime )
{
	if ( AMilkManager::GetSingleton() )
		AMilkManager::GetSingleton()->Update( a_fDeltaTime );
}

void AChickenCoopDefender::SpawnThieves( TINT a_iCount, AChickenCoopDefenderGame::UnitList10& a_rFree, AChickenCoopDefenderGame::UnitList10& a_rSpawned )
{
	for ( TINT i = 0; i < a_iCount; i++ )
	{
		if ( a_rFree.Size() == 0 )
			break;

		AChickenCoopDefenderThief* pThief = TSTATICCAST( AChickenCoopDefenderThief, a_rFree.Back().Value()->GetUnit() );

		if ( pThief->GetState() == AChickenCoopDefenderThief::STATE_GONE )
		{
			pThief->SetState( AChickenCoopDefenderThief::STATE_SPAWN );

			m_oGame.oThieves.PushBack( pThief->GetSelfProxy() );
			a_rSpawned.PushBack( pThief->GetSelfProxy() );
			a_rFree.PopBack();
			m_oGame.iNumThieves++;
		}
	}
}

// $Barnyard: FUNCTION 00518b70
void AChickenCoopDefender::UpdateGame( TFLOAT a_fDeltaTime )
{
	m_oGame.fTime += a_fDeltaTime;
	m_bRenderReticles = TTRUE;
	m_oMilkBottles.Update( a_fDeltaTime, TTRUE );

	if ( m_oGame.ePhase == PHASE_FOLLOW )
	{
		// The clock stops while the camera follows a throw
		m_fElapsedTime -= a_fDeltaTime;
	}
	else if ( m_oGame.ePhase == PHASE_ENDED )
	{
		return;
	}

#ifdef BARNYARD_COMMUNITY_PATCH
	DevAutoPlay( a_fDeltaTime );
#endif

	// The icons of the open hot spots
	TINT iIcon = 0;

	for ( TINT i = 0; i < AChickenCoopDefenderGame::NUM_KINDS; i++ )
	{
		for ( auto it = m_oGame.aOpenHotSpots[ i ].Begin(); it != m_oGame.aOpenHotSpots[ i ].End(); it++ )
			it->Update( a_fDeltaTime, m_oGame.pMapIcons[ iIcon++ ] );
	}

	// The next wave of thieves
	m_fWaveTimer -= a_fDeltaTime;

	if ( m_fWaveTimer < 0.0f || m_oGame.iNumThieves == 0 )
	{
		const AChickenCoopDefenderWaveData& rcWave = m_oGame.pGameData->pWaves[ m_oGame.iWave ];

		SpawnThieves( rcWave.iNumRaccoons, m_oGame.oFreeRaccoons, m_oGame.oRaccoons );
		SpawnThieves( rcWave.iNumCoyotes, m_oGame.oFreeCoyotes, m_oGame.oCoyotes );
		SpawnThieves( rcWave.iNumFreddies, m_oGame.oFreeFreddies, m_oGame.oFreddies );

		if ( ++m_oGame.iWave >= m_oGame.pGameData->iNumWaves )
			m_oGame.iWave = 0;

		m_fWaveTimer = m_oGame.pGameData->fWaveInterval;
	}

	AAnimatableObject* pPlayerObject = m_pPlayer->GetAnimatableObject();

	switch ( m_oGame.ePlayerState )
	{
		case PLAYERSTATE_FREE:
			if ( GetOverlayAnim( m_pPlayer, TPS8( carry_overlay ) ) )
				pPlayerObject->GetToshiAnimInterface().StopAnim( TPS8( carry_overlay ) );

			// Bitten by a coyote while throwing
			if ( m_oGame.bPlayerHit && TSTATICCAST( APlayerSteer, m_pPlayer->GetSteer() )->GetState() == APlayerSteer::STATE_WALKING )
			{
				m_oGame.ePlayerState = PLAYERSTATE_HIT;
				m_pPlayer->GetAnimatableObject()->KillAllAnimations();
				m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( yuk ) );

				ASimplePlayerControlComponent* pControl = TSTATICCAST( ASimplePlayerControlComponent, m_pPlayer->FindComponent( &TGetClass( ASimplePlayerControlComponent ) ) );

				if ( m_pPlayer->GetActiveLauncher() != m_pPlayer->GetLauncher( 0 ) )
					m_pPlayer->SelectLauncher( TFALSE );

				pControl->SetMovementEnabled( TFALSE );
				pControl->SetLauncherInput( TFALSE );
				pControl->SetSecondaryLauncherInput( TFALSE );
				m_oGame.bPlayerHit = TFALSE;
			}

			break;
		case PLAYERSTATE_PICKUP_THIEF:
		{
			AChickenCoopDefenderThief* pThief = TSTATICCAST( AChickenCoopDefenderThief, m_oGame.pNearThief );

			if ( m_pPlayer->GetAnimatableObject()->IsAnimationFinished( TPS8( pickup_raccoon ) ) )
			{
				if ( m_pPlayer->GetAttachedUnit( TPS8( cb_rlocator ), -1 ) == m_oGame.pNearThief )
				{
					// Turns to the throw target, then throws the raccoon
					ACommonSteer* pSteer = TSTATICCAST( ACommonSteer, m_pPlayer->GetSteer() );

					if ( TVector4::DotProduct3( *pSteer->GetDirection(), m_oGame.vThrowDirection ) <= 0.95f )
					{
						pSteer->StopAndFace( m_oGame.vThrowDirection );
					}
					else if ( pThief->GetState() == AChickenCoopDefenderThief::STATE_PICKED_UP || ( pThief->SetState( AChickenCoopDefenderThief::STATE_PICKED_UP ), pThief->GetState() == AChickenCoopDefenderThief::STATE_PICKED_UP ) )
					{
						if ( !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( throw_raccoon ), 0.0f ) )
						{
							m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( throw_raccoon ) );
							pThief->GetAnimatableObject()->PlayAnimation( TPS8( thrown ) );
						}
					}
				}
				else
				{
					m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( pickup_raccoon ) );
				}
			}

			break;
		}
		case PLAYERSTATE_THROW_THIEF:
			if ( m_pPlayer->GetAnimatableObject()->IsAnimationFinished( TPS8( throw_raccoon ) ) )
			{
				EnablePlayerControl();
				m_oGame.ePlayerState = PLAYERSTATE_FREE;
			}

			break;
		case PLAYERSTATE_HIT:
			if ( m_pPlayer->GetAnimatableObject()->IsAnimationFinished( TPS8( yuk ) ) )
			{
				m_oGame.ePlayerState = PLAYERSTATE_FREE;
				EnablePlayerControl();
			}

			break;
		case PLAYERSTATE_PICKUP_ITEM:
			if ( m_pPlayer->GetAnimatableObject()->IsAnimationFinished( TPS8( pickup ) ) )
			{
				AUnit* pItem = m_pPlayer->GetAttachedUnit( TPS8( cb_rlocator ), -1 );

				if ( !pItem )
				{
					m_oGame.ePlayerState = PLAYERSTATE_FREE;
				}
				else if ( pItem->IsA( &TGetClass( AChickenCoopDefenderEgg ) ) )
				{
					m_oGame.ePlayerState = PLAYERSTATE_CARRY_EGG;
				}
				else if ( pItem->IsA( &TGetClass( AChickenCoopDefenderHen ) ) )
				{
					m_oGame.ePlayerState = PLAYERSTATE_CARRY_HEN;
				}

				EnablePlayerControl();
			}

			break;
		case PLAYERSTATE_CARRY_EGG:
		case PLAYERSTATE_CARRY_HEN:
			if ( m_pPlayer->GetSteer()->GetMovementType() < 4 && !GetOverlayAnim( m_pPlayer, TPS8( carry_overlay ) ) )
				pPlayerObject->GetToshiAnimInterface().PlayAnim( TPS8( carry_overlay ), 1.0f, TFALSE );

			break;
		case PLAYERSTATE_THROW_HEN:
			if ( m_pPlayer->GetAnimatableObject()->IsAnimationFinished( TPS8( throw ) ) && m_oGame.ePhase != PHASE_FOLLOW )
			{
				if ( !m_oGame.pItem && !m_pPlayer->GetAttachedUnit( TPS8( cb_rlocator ), -1 ) )
				{
					m_oGame.ePlayerState = PLAYERSTATE_FREE;
					EnablePlayerControl();
				}
				else
				{
					m_oGame.ePlayerState = PLAYERSTATE_CARRY_HEN;
				}
			}

			break;
		case PLAYERSTATE_THROW_EGG:
			if ( m_pPlayer->GetAnimatableObject()->IsAnimationFinished( TPS8( throw ) ) && m_oGame.ePhase != PHASE_FOLLOW )
			{
				if ( m_oGame.pItem || m_pPlayer->GetAttachedUnit( TPS8( cb_rlocator ), -1 ) )
				{
					m_oGame.ePlayerState = PLAYERSTATE_CARRY_EGG;
				}
				else
				{
					m_oGame.ePlayerState = PLAYERSTATE_FREE;
					EnablePlayerControl();
				}
			}

			break;
		case PLAYERSTATE_AIM_EGG:
		case PLAYERSTATE_AIM_HEN:
		{
			// Turns to the coop, then throws
			ACommonSteer* pSteer = TSTATICCAST( ACommonSteer, m_pPlayer->GetSteer() );

			TVector4 vToCoop;
			vToCoop.Substract( AChickenCoopDefenderGame::ms_aGoals[ 1 ], *pSteer->GetPosition() );
			vToCoop.w = AChickenCoopDefenderGame::ms_aGoals[ 1 ].w;
			AChickenCoopDefenderNormaliseXZ( vToCoop );
			pSteer->StopAndFace( vToCoop );

			if ( pSteer->IsFacingDesired( 0.99f ) && m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( throw ) ) )
			{
				ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 )->ResetPosition( TFALSE );

				if ( GetOverlayAnim( m_pPlayer, TPS8( carry_overlay ) ) )
					pPlayerObject->GetToshiAnimInterface().StopAnim( TPS8( carry_overlay ) );

				m_oGame.ePlayerState = ( m_oGame.ePlayerState == PLAYERSTATE_AIM_EGG ) ? PLAYERSTATE_THROW_EGG : PLAYERSTATE_THROW_HEN;
				TSTATICCAST( AAnimatedUnit, m_oGame.pHanna )->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );
			}

			break;
		}
		default:
			break;
	}

	if ( m_oGame.ePlayerState != PLAYERSTATE_HIT && m_oGame.fHitCooldown > 0.0f )
		m_oGame.fHitCooldown -= a_fDeltaTime;

	UpdateInteraction();

	// The score
	TINT iVariant = AMiniGameManager::GetSingleton()->GetMiniGameVariant();

	if ( iVariant == VARIANT_RACCOONS )
		m_oPortrait.SetScore( m_oGame.iNumRaccoonHits + m_oGame.iNumHits + m_oGame.iBonus );
	else if ( iVariant > VARIANT_RACCOONS && iVariant <= VARIANT_FREDDYS_DREAM )
		m_oPortrait.SetScore( m_oGame.iNumHits + m_oGame.iBonus );

	// Hanna looks at the player
	TVector4 vToPlayer;
	vToPlayer.Substract( *m_pPlayer->GetSteer()->GetPosition(), *m_oGame.pHanna->GetSteer()->GetPosition() );
	AChickenCoopDefenderNormaliseXZ( vToPlayer );
	m_oGame.pHanna->GetSteer()->SetDesiredDirection( vToPlayer );

	ASteer* pPlayerSteer = m_pPlayer->GetSteer();
	m_oMap.SetCentre( *pPlayerSteer->GetPosition(), *pPlayerSteer->GetDirection() );

	// NOTE: retail seems to pass a leftover stack value here
	AProjectileLauncher* pLauncher = m_pPlayer->GetActiveLauncher();
	pLauncher->Update( a_fDeltaTime );
	m_oAmmoBar.SetAmmo( TINT( pLauncher->GetAmmo()->GetAmmo() ), pLauncher->GetAmmo()->GetMaxAmmo() );
}

// $Barnyard: FUNCTION 005164e0
void AChickenCoopDefender::UpdateGameOver( TFLOAT a_fDeltaTime )
{
	TBOOL bWon = m_oGame.iNumLeft > 0 && m_oGame.iLives != 0;

	if ( !m_bGameOverStarted )
	{
		m_oGame.ePhase = PHASE_ENDED;

		if ( m_pPlayer->GetActiveLauncher() != m_pPlayer->GetLauncher( 0 ) )
			m_pPlayer->SelectLauncher( TFALSE );

		m_pControl->SetLauncherInput( TFALSE );
		m_pControl->SetSecondaryLauncherInput( TFALSE );
		m_pControl->SetCanSprint( TFALSE );
		m_pControl->SetMovementEnabled( TFALSE );
		m_pControl->SetCameraInput( TFALSE );
		m_pPlayer->GetSteer()->SetTilt( 0.0f, 0.0f, TFALSE );

		// Won: the thieves run away
		if ( bWon )
		{
			for ( TINT i = 0; i < m_oGame.iNumThieves; i++ )
				TSTATICCAST( AChickenCoopDefenderThief, m_oGame.oThieves[ i ]->GetUnit() )->SetState( AChickenCoopDefenderThief::STATE_FLEE );
		}

		m_oReticles.SetActive( TFALSE );
		m_fWaveTimer       = 5.0f;
		m_bGameOverStarted = TTRUE;
	}

	if ( m_oGame.iLives == 0 )
	{
		if ( !m_oGame.bDefeated && TSTATICCAST( APlayerSteer, m_pPlayer->GetSteer() )->GetState() == APlayerSteer::STATE_WALKING &&
		     !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( defeat ), 0.0f ) )
		{
			m_oGame.bDefeated = TTRUE;
			m_pPlayer->GetAnimatableObject()->KillAllAnimations();
			m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( defeat ) );
		}
	}
	else if ( !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( winner_start ), 0.0f ) && !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( winner_loop ), 0.0f ) && !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( loser_start ), 0.0f ) && !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( loser_loop ), 0.0f ) )
	{
		// Turns to the camera, then cheers or sulks
		ACommonSteer* pSteer = TSTATICCAST( ACommonSteer, m_pPlayer->GetSteer() );

		TVector4 vToCamera;
		vToCamera.Substract( ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation(), *pSteer->GetPosition() );
		AChickenCoopDefenderNormaliseXZ( vToCamera );

		if ( TVector4::DotProduct3( *m_pPlayer->GetSteer()->GetDirection(), vToCamera ) > 0.98f )
		{
			if ( m_oGame.iNumLeft > 0 && m_oGame.iLives != 0 )
				m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( winner_start ) );
			else
				m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( loser_start ) );
		}
		else
		{
			pSteer->StopAndFace( vToCamera );
		}
	}

	if ( ( m_fElapsedTime < m_fTimeLimit || m_oTimeLeftHudElement.GetTimer().IsTickComplete() ) && m_fWaveTimer < 0.0f )
		AGameStateController::GetSingleton()->PushState( new AAnticGameOverState( GetResults(), TFALSE ) );

	m_fWaveTimer -= a_fDeltaTime;
}

// $Barnyard: FUNCTION 00517600
AMiniGameState::S1* AChickenCoopDefender::GetResults()
{
	PlayerInfo& rInfo = m_oResults.oPlayerList[ 0 ];

	if ( m_oGame.ePhase == PHASE_ENDED && m_oGame.iNumLeft > 0 && m_oGame.iLives != 0 )
	{
		m_oResults.iResult = 0;
		rInfo.iUnk2        = m_oGame.iNumLeft;
	}
	else
	{
		m_oResults.iResult = -1;
		rInfo.iUnk2        = 0;
	}

	m_oResults.bFlag1         = TFALSE;
	rInfo.bUnk1               = TTRUE;
	m_oResults.iMiniGameIndex = AMiniGameManager::GetSingleton()->GetNextMiniGameIndex();
	m_oResults.bWon           = m_oResults.iResult == 0;

	while ( rInfo.vecRows.Size() != 0 )
	{
		rInfo.vecRows[ rInfo.vecRows.Size() - 1 ] = ResultRow();
		rInfo.vecRows.PopBack();
	}

	ALocaleManager* pLocale = ALocaleManager::Interface();

	switch ( AMiniGameManager::GetSingleton()->GetMiniGameVariant() )
	{
		case VARIANT_RACCOONS:
			rInfo.vecRows.PushBack( ResultRow{ pLocale->GetString( LOCALE_RACCOONS_MILK_HITS ), m_oGame.iNumRaccoonHits } );
			rInfo.vecRows.PushBack( ResultRow{ pLocale->GetString( LOCALE_RACCOONS_THROWN ), m_oGame.iNumHits } );
			rInfo.vecRows.PushBack( ResultRow{ pLocale->GetString( LOCALE_RACCOONS_EGGS_LEFT ), 20 * m_oGame.iNumLeft } );
			rInfo.vecRows.PushBack( ResultRow{ pLocale->GetString( LOCALE_RACCOONS_EGGS_SAVED ), m_oGame.iBonus } );
			break;
		case VARIANT_COYOTES:
			rInfo.vecRows.PushBack( ResultRow{ pLocale->GetString( LOCALE_COYOTES_MILK_HITS ), m_oGame.iNumHits } );
			rInfo.vecRows.PushBack( ResultRow{ pLocale->GetString( LOCALE_COYOTES_HENS_LEFT ), 20 * m_oGame.iNumLeft } );
			rInfo.vecRows.PushBack( ResultRow{ pLocale->GetString( LOCALE_COYOTES_HENS_SAVED ), m_oGame.iBonus } );
			break;
		case VARIANT_FREDDYS_DREAM:
			rInfo.vecRows.PushBack( ResultRow{ pLocale->GetString( LOCALE_FREDDY_MILK_HITS ), m_oGame.iNumHits } );
			rInfo.vecRows.PushBack( ResultRow{ pLocale->GetString( LOCALE_FREDDY_ROASTS_LEFT ), 20 * m_oGame.iNumLeft } );
			rInfo.vecRows.PushBack( ResultRow{ pLocale->GetString( LOCALE_FREDDY_ROASTS_SAVED ), m_oGame.iBonus } );
			break;
		default:
			rInfo.vecRows.PushBack( ResultRow{ pLocale->GetString( LOCALE_RESULT_DEFAULT ), rInfo.iUnk2 } );
			break;
	}

	m_oResults.ComputeTotal( 0 );

	if ( m_bSubmitHighScore )
	{
		m_bSubmitHighScore = TFALSE;
		TINT iVariant      = AMiniGameManager::GetSingleton()->GetMiniGameVariant();

		if ( iVariant == VARIANT_RACCOONS || iVariant == VARIANT_COYOTES )
			AHighScoreManager::AddScore( HIGHSCORE_TABLE, TFLOAT( rInfo.iTotal ), m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
		else if ( iVariant == VARIANT_FREDDYS_DREAM )
			AHighScoreManager::AddScore( HIGHSCORE_TABLE + 1, TFLOAT( rInfo.iTotal ), m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
	}

	return &m_oResults;
}

// $Barnyard: FUNCTION 00515cf0
void AChickenCoopDefender::OnStartHudFinished()
{
	AMiniGameState::OnStartHudFinished();

	m_pControl->SetLauncherInput( TTRUE );
	m_pControl->SetSecondaryLauncherInput( TTRUE );
	m_pControl->SetMovementEnabled( TTRUE );
	m_pControl->SetCanSprint( TTRUE );

	m_oInteractionHud.SetForceHide( TFALSE );

	m_oTimeLeftHudElement.GetTimer().SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oTimeLeftHudElement.SetForceHide( TFALSE );

	// The milk ammo, under the portrait
	m_oAmmoBar.Create( 32.5f, 1.625f, 0.3f );
	m_oAmmoBar.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oAmmoBacking.AddChildTail( m_oAmmoBar );
	m_oAmmoBacking.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Score_Wood_Thin" ) );
	m_oAmmoBacking.SetWidth( m_oAmmoBar.GetWidth() + 52.0f );
	m_oAmmoBacking.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oAmmoRoot.SetDimensions( m_oAmmoBacking.GetWidth(), m_oAmmoBar.GetHeight() );
	m_oAmmoRoot.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oAmmoRoot.AddChildTail( m_oAmmoBacking );
	m_oPortrait.AddChildTail( m_oAmmoRoot );

	m_oGame.iNumRaccoonHits = 0;
	m_oGame.iNumHits        = 0;
	m_oGame.iBonus          = 0;
	m_oGame.iNumLeft        = m_oGame.pGameData->iNumToDefend;
	m_oGame.iLives          = m_oGame.pGameData->iLives;
	m_oGame.bDefeated       = TFALSE;
	m_oGame.fHitCooldown    = -0.1f;

	m_oCounter.SetCount( m_oGame.iNumLeft, TFALSE );

	if ( m_oGame.pGameData->iLives > -1 )
		m_oLifeHud.SetLives( m_oGame.iLives );

	m_bGameOverStarted = TFALSE;
	TSTATICCAST( AAnimatedUnit, m_oGame.pHanna )->SetVisible( TTRUE );
	m_oGame.ePhase = PHASE_PLAYING;
}

// $Barnyard: FUNCTION 005160b0
TINT AChickenCoopDefender::FindThrowTarget( const TVector4& a_rcFrom, const TVector4& a_rcDirection )
{
	TINT   iBest    = -1;
	TFLOAT fBestDot = -2.0f;

	for ( TINT i = 0; i < NUM_THROW_TARGETS; i++ )
	{
		const TVector4& rcTarget = s_aThrowTargets[ i ];

		TVector4 vToTarget;
		vToTarget.Substract( rcTarget, a_rcFrom );
		vToTarget.w = rcTarget.w;

		TFLOAT fLength   = vToTarget.NormaliseMag();
		TFLOAT fDistance = fLength;

		if ( ATerrainInterface::GetSingleton()->CollideRay( a_rcFrom, vToTarget, fDistance, TNULL, TCollisionCommon::TOSHICGROUPFLAG( 0x80000 ), TFALSE ) )
			continue;

		// The collision objects around the middle of the throw
		TFLOAT   fRadius = fLength * 0.5f;
		TVector4 vHalf;
		vHalf.Multiply( vToTarget, fRadius );

		TVector4 vCentre( vHalf.x + a_rcFrom.x, vHalf.y + a_rcFrom.y, vHalf.z + a_rcFrom.z, a_rcFrom.w );

		TINT                      iNumObjects;
		AObjectHash::QueryResult* pObjects = AObjectHashMain::GetSingleton()->FindObjects( vCentre, fRadius, &iNumObjects, TFALSE, 20 );

		ARayData oRay( a_rcFrom, rcTarget );
		TBOOL    bBlocked = TFALSE;

		for ( TUINT j = 0; j < TUINT( iNumObjects ); j++ )
		{
			ACollisionObject* pObject = TSTATICCAST( ACollisionObject, pObjects[ j ].pObject );

			// Not the units
			if ( !pObject->GetOwnerUnit() )
			{
				ACollisionHit oHit;
				oHit.vNormal = TVector4::VEC_ZERO;

				if ( pObject->CollideRay( oRay, oHit ) )
				{
					bBlocked = TTRUE;
					break;
				}
			}
		}

		if ( !bBlocked )
		{
			// The one closest to the direction
			TFLOAT fDot = vToTarget.z * a_rcDirection.z + vToTarget.x * a_rcDirection.x;

			if ( fDot > fBestDot )
			{
				fBestDot = fDot;
				iBest    = i;
			}
		}
	}

	return iBest;
}

// $Barnyard: FUNCTION 005181f0
void AChickenCoopDefender::OnUnitAction( AAnimationBreakpointUnitAction::Event* a_pEvent )
{
	TRef<AUnitProxy> pEventUnit = a_pEvent->pUnitProxy;

	if ( m_pPlayer != pEventUnit->GetUnit() )
	{
		// The animation of a thief
		for ( TINT i = 0; i < m_oGame.oThieves.Size(); i++ )
		{
			if ( m_oGame.oThieves[ i ] )
			{
				AUnit*           pThief = m_oGame.oThieves[ i ]->GetUnit();
				TRef<AUnitProxy> pProxy = a_pEvent->pUnitProxy;

				if ( pThief == pProxy->GetUnit() )
				{
					TSTATICCAST( AChickenCoopDefenderThief, pThief )->OnUnitAction( a_pEvent );
					return;
				}
			}
		}

		return;
	}

	if ( a_pEvent->eAction == AAnimationBreakpointUnitAction::ACTION_PICKUP )
	{
		if ( m_oGame.ePlayerState == PLAYERSTATE_PICKUP_THIEF )
		{
			// Holds the raccoon
			AUnit* pThief = m_oGame.pNearThief;

			if ( !pThief || !pThief->IsA( &TGetClass( AChickenCoopDefenderThief ) ) )
				return;

			m_pPlayer->AttachUnitToBone( pThief, TPS8( cb_rlocator ), TPString8() );

			// Where to throw it
			ACommonSteer* pSteer = TSTATICCAST( ACommonSteer, m_pPlayer->GetSteer() );
			TVector4      vFrom  = *pSteer->GetPosition();
			vFrom.y -= 2.0f;

			TVector4 vCameraDir  = ACameraManager::GetSingleton()->GetCurrentCamera()->GetOrientation();
			m_oGame.iThrowTarget = FindThrowTarget( vFrom, vCameraDir );

			if ( m_oGame.iThrowTarget == -1 )
			{
				pSteer->SetSpeed( 0.0f );
				TSTATICCAST( APlayerSteer, pSteer )->TurnTowards( *pSteer->GetDirection() );
			}
			else
			{
				TVector4 vDirection = s_aThrowTargets[ m_oGame.iThrowTarget ];
				vDirection.y        = 0.0f;
				vDirection.x -= vFrom.x;
				vDirection.z -= vFrom.z;
				vDirection.Normalise();

				pSteer->SetSpeed( 0.0f );
				m_oGame.vThrowDirection = vDirection;
				pSteer->StopAndFace( vDirection );
			}

			TSTATICCAST( AChickenCoopDefenderThief, pThief )->SetState( AChickenCoopDefenderThief::STATE_PICKED_UP );
		}
		else if ( m_oGame.ePlayerState == PLAYERSTATE_PICKUP_ITEM )
		{
			// Holds the hen/egg
			AUnit* pItem = m_oGame.pItem;

			if ( !pItem )
				return;

			if ( pItem->IsA( &TGetClass( AChickenCoopDefenderEgg ) ) )
			{
				if ( m_oGame.oLooseEggs.Find( pItem->GetSelfProxy() ) == m_oGame.oLooseEggs.End() )
					return;

				TSTATICCAST( AChickenCoopDefenderEgg, pItem )->SetMapIconVisible( TFALSE );
				AChickenCoopDefenderGame::MoveUnit( m_oGame.oLooseEggs, m_oGame.oCarriedEggs, pItem->GetSelfProxy() );
			}
			else if ( pItem->IsA( &TGetClass( AChickenCoopDefenderHen ) ) )
			{
				if ( m_oGame.oLooseHens.Find( pItem->GetSelfProxy() ) == m_oGame.oLooseHens.End() )
					return;

				TSTATICCAST( AChickenCoopDefenderHen, pItem )->SetState( AChickenCoopDefenderHen::STATE_HELD );
				AChickenCoopDefenderGame::MoveUnit( m_oGame.oLooseHens, m_oGame.oCarriedHens, pItem->GetSelfProxy() );
			}
			else
			{
				return;
			}

			m_pPlayer->AttachUnit( m_oGame.pItem, TPS8( cb_rlocator ) );
			m_oGame.pHotSpot = TNULL;
		}
	}
	else if ( a_pEvent->eAction == AAnimationBreakpointUnitAction::ACTION_HD_THROW )
	{
		switch ( m_oGame.ePlayerState )
		{
			case PLAYERSTATE_PICKUP_THIEF:
			{
				// Throws the raccoon away
				AUnit* pThief = m_oGame.pNearThief;

				if ( !pThief || !pThief->IsA( &TGetClass( AChickenCoopDefenderThief ) ) )
					return;

				m_pPlayer->DetachUnit( pThief, TTRUE, TTRUE );

				TMatrix44         mBone;
				AModelInstanceRef pModelInstance = m_pPlayer->GetAnimatableObject()->GetModelInstance();
				pModelInstance->GetBoneTransform( pModelInstance->GetBoneIndex( TPS8( cb_rlocator ) ), mBone );
				m_oGame.vThrowFrom = mBone.GetTranslation();

				TVector4 vTarget;

				if ( m_oGame.iThrowTarget >= 0 )
				{
					vTarget = s_aThrowTargets[ m_oGame.iThrowTarget ];
				}
				else
				{
					// Straight down
					m_pPlayer->GetSteer()->CopyPosition( vTarget );
					vTarget.y -= 100.0f;
				}

				TVector4 vVelocity;
				vVelocity.Substract( vTarget, m_oGame.vThrowFrom );
				vVelocity.w = 1.0f;

				TVector4 vDirection = vVelocity;
				vDirection.y        = 0.0f;
				vDirection.Normalise();

				AChickenCoopDefenderThief* pThrownThief = TSTATICCAST( AChickenCoopDefenderThief, m_oGame.pNearThief );
				pThrownThief->SetState( AChickenCoopDefenderThief::STATE_THROWN );
				pThrownThief->GetSteer()->SetDirection( vDirection );

				TFLOAT fSpeed = vVelocity.NormaliseMag() * 2.0f;
				vVelocity.Multiply( TMath::Min( 100.0f, fSpeed ) );
				pThrownThief->SetThrowVelocity( vVelocity );

				m_oGame.pNearThief   = TNULL;
				m_oGame.ePlayerState = PLAYERSTATE_THROW_THIEF;
				m_oGame.iNumHits++;
				break;
			}
			case PLAYERSTATE_THROW_HEN:
			{
				// Throws the hen into the coop
				AUnit* pItem = m_oGame.pItem;

				if ( !pItem || !pItem->IsA( &TGetClass( AChickenCoopDefenderHen ) ) )
					return;

				TFLOAT                          fAngle    = m_oGame.pHotSpot->fThrowAngle;
				AAimedSquirtProjectileLauncher* pLauncher = TSTATICCAST( AAimedSquirtProjectileLauncher, m_pPlayer->GetLauncher( 1 ) );

				if ( fAngle <= 0.0f || fAngle >= TMath::HALF_PI )
					fAngle = GetThrowAngle( TVector4::DistanceXZ( AChickenCoopDefenderGame::ms_aGoals[ 1 ], *m_pPlayer->GetSteer()->GetPosition() ) );

				pLauncher->FireAt( AChickenCoopDefenderGame::ms_aGoals[ 1 ], fAngle, TTRUE );

				m_oGame.pHotSpot = TNULL;
				m_oGame.pItem    = TNULL;
				break;
			}
			case PLAYERSTATE_THROW_EGG:
			{
				// Throws the egg into the coop
				AUnit* pItem = m_oGame.pItem;

				if ( !pItem || !pItem->IsA( &TGetClass( AChickenCoopDefenderEgg ) ) )
					return;

				m_pPlayer->DetachUnit( m_oGame.pItem, TTRUE, TTRUE );

				AChickenCoopDefenderEgg* pEgg = TSTATICCAST( AChickenCoopDefenderEgg, m_oGame.pItem );

				// NOTE: retail gets the position of Hanna and computes a velocity it doesn't use
				m_oGame.pHanna->GetSteer()->GetPosition();
				TVector4 vUnused;
				AAimedSquirtProjectileLauncher::GetBallisticVelocity( TMath::PI * 0.25f, *pEgg->GetSteer()->GetPosition(), AChickenCoopDefenderGame::ms_aGoals[ 1 ], vUnused );

				TFLOAT fAngle = m_oGame.pHotSpot->fThrowAngle;

				if ( fAngle <= 0.0f || fAngle >= TMath::HALF_PI )
					fAngle = GetThrowAngle( TVector4::DistanceXZ( AChickenCoopDefenderGame::ms_aGoals[ 1 ], *m_pPlayer->GetSteer()->GetPosition() ) );

				TVector4 vVelocity;
				AAimedSquirtProjectileLauncher::GetBallisticVelocity( fAngle, *pEgg->GetSteer()->GetPosition(), AChickenCoopDefenderGame::ms_aGoals[ 1 ], vVelocity );
				pEgg->Throw( vVelocity );
				pEgg->SetValue( m_oGame.pHotSpot->iValue );

				if ( m_oGame.pHotSpot->fThrowAngle > 0.0f && m_oGame.pHotSpot->fThrowAngle < TMath::HALF_PI )
				{
					// The camera of the player follows the egg into the coop
					AGTAStyleCameraHelper* pHelper = TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 ) );
					pHelper->SetTarget( pEgg->GetSelfProxy() );
					pHelper->SetDefaultMaxPitch( 70.0f );
					pHelper->SetMaxPitch( 70.0f );
					pHelper->SetHeightOffset( 0.5f );
					pHelper->SetTargetPitch( 0.0f );
					pHelper->SetDefaultPitch( 0.0f );
					pHelper->SetCollisionRadius( 0.43f * 0.01f );
					pHelper->SetSmallCollisionRadius( 0.2f * 0.01f );

					m_oGame.ePhase = PHASE_FOLLOW;
				}

				m_oGame.pHotSpot = TNULL;
				m_oGame.pItem    = TNULL;
				break;
			}
			default:
				break;
		}
	}
}

// $Barnyard: FUNCTION 00516330
void AChickenCoopDefender::LoadData()
{
	TTRB& rData = m_oGame.oData;
	rData.Load( "Data/MinigameData/ChickenCoopDefender/Data.trb", 0 );

	TINT iNumVariants = 1;

	m_oGame.pThiefData = rData.CastSymbol<AChickenCoopDefenderThiefData>( "ThiefData" );
	m_oGame.pGameData  = rData.CastSymbol<AChickenCoopDefenderGameData>( "GameData" );

	if ( rData.GetSymbolAddress( "NumGameVariants" ) )
		iNumVariants = *rData.CastSymbol<TINT>( "NumGameVariants" );

	m_oGame.pGameData += AMiniGameManager::GetSingleton()->GetMiniGameVariant() % iNumVariants;
	m_oGame.pHotSpotData = rData.CastSymbol<AChickenCoopDefenderHotSpotData>( "HotSpotData" );

#ifdef BARNYARD_LATEST
	// Latest version (Wii 802668a4): a shorter game, 2 more lives and a fifth more to defend on Easy
	if ( APlayerProgress::GetSingleton()->IsEasyDifficulty() )
	{
		// NOTE: the data is in the TRB loaded above, which the game owns
		AChickenCoopDefenderGameData* pGameData = const_cast<AChickenCoopDefenderGameData*>( m_oGame.pGameData );
		pGameData->fTimeLimit *= 0.9f;

		if ( pGameData->iLives != -1 )
			pGameData->iLives += 2;

		pGameData->iNumToDefend += TINT( 0.2f * pGameData->iNumToDefend );
	}
#endif
}

// $Barnyard: FUNCTION 005199f0
void AChickenCoopDefender::CreatePlayer( AGUI2MiniMap* a_pMap )
{
	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();

	if ( pPlayerManager->GetCurrentPlayerSet() != pPlayerManager->GetPlayerSet( 1 ) )
		pPlayerManager->SetCurrentPlayerSet( 1 );

	TInputDeviceController* pController = AInputHandler::GetSingleton()->GetMainController();
	pPlayerManager->GetPlayerSet( 1 )->Reset();
	pPlayerManager->GetPlayerSet( 1 )->AddHumanPlayer( pController, -1 );

	if ( !m_pPlayer )
	{
		APlayerProfileManager* pProfileManager = APlayerProfileManager::GetSingleton();
		APlayerSimProfile*     pProfile        = pProfileManager->GetMainProfile();

		TINT iGender = pProfileManager->GetMainGender();

		if ( iGender == APlayerProfile::GENDER_NONE )
			iGender = APlayerProfile::GENDER_MALE;

		AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_NEGZ, 0, iGender, -1, TTRUE );
		pProfile->SetStaminaWasteSpeed( 0.0f );

		m_pPlayer = TSTATICCAST( AUnitPlayer, AUnitManager::GetSingleton()->SpawnUnit( TPS8( player_chickencoop ), &oTransform ) );
		m_pPlayer->ApplyMaterial( pProfileManager->GetMainCostume(), TTRUE );
		pPlayerManager->GetCurrentPlayerSet()->SetPlayerUnit( m_pPlayer, 0 );

		// The player wears sunglasses
		AAnimatableObjectManager* pObjectManager = AAnimatableObjectManager::GetSingleton();
		AAnimatableObjectType*    pType          = pObjectManager->FindType( TPS8( sunglasses1 ) );

		if ( pType )
		{
			m_pSunglasses = pObjectManager->CreateAnimatableObject( pType, m_pPlayer, 0 );
			TINT iBone    = -1;

			if ( pProfile->GetGender() == APlayerProfile::GENDER_FEMALE )
				iBone = m_pSunglasses->GetModelInstance()->GetSkeleton()->GetBoneID( "americanf_loc", 0 );

			TINT iHeadBone = m_pPlayer->GetModelInstance()->GetSkeleton()->GetBoneID( "c_head", 0 );
			pObjectManager->AttachObject( m_pSunglasses, m_pPlayer->GetAnimatableObject(), iBone, iHeadBone, TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
		}

		m_oPortrait.Create( pProfile->GetHideTextureName( -1, APlayerProfile::GENDER_NONE ), AGUI2Portrait::FLAGS_LEFT | AGUI2Portrait::FLAGS_UNK5, &m_GUIElement );
		m_oPortrait.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
		m_GUIElement.AddChildTail( m_oPortrait );
	}

	m_pPlayer->SetHashed( TTRUE );
	m_pPlayer->SetNoQuery( TFALSE );
	m_pPlayer->GetAnimatableObject()->GetModelInstance()->SetShadowMode( AModelInstance::SHADOWMODE_DYNAMIC );

	// Milk, and the hens and raccoons the player throws
	m_pPlayer->GetActiveLauncher()->GetAmmo()->SetProjectileType( ASimpleProjectileManager::TYPE_CHICKEN_COOP_DEFENDER_MILK );
	m_oGame.oPlayer = m_pPlayer->GetSelfProxy();

	m_pPlayer->GetActiveLauncher()->SetThrowAmmo( &m_oAmmo );
	m_pPlayer->GetActiveLauncher()->GetThrowAmmo()->SetProjectileType( ASimpleProjectileManager::TYPE_CHICKEN_COOP_DEFENDER_THROW );

	AProjectileLauncher* pThrowLauncher = m_pPlayer->GetLauncher( 1 );
	pThrowLauncher->SetThrowAmmo( &m_oAmmo );
	pThrowLauncher->GetThrowAmmo()->SetProjectileType( ASimpleProjectileManager::TYPE_CHICKEN_COOP_DEFENDER_THROW );

	m_oReticles.Create( m_pPlayer, 50.0f, &m_GUIElement );

	TSTATICCAST( APlayerSteer, m_pPlayer->GetSteer() )->PlaceAtNamedGoal( TPS8( PlayerStart0 ), TTRUE, TTRUE );

	m_pControl = TSTATICCAST( ASimplePlayerControlComponent, m_pPlayer->FindComponent( &TGetClass( AControlComponent ) ) );
	m_pControl->SetCanSprint( TFALSE );
	m_pControl->SetUnk32( TFALSE );
	m_pControl->SetLauncherInput( TFALSE );
	m_pControl->SetSecondaryLauncherInput( TFALSE );
	m_pControl->SetMovementEnabled( TFALSE );

	m_pGlowObject = TNULL;

	ASteer* pSteer = m_pPlayer->GetSteer();
	a_pMap->AddIcon( *pSteer->GetPosition(), *pSteer->GetDirection(), AGUI2MiniMap::ICONTYPE_PLAYER, AGUI2MiniMap::ICON_PLAYER, 0.13f );
	a_pMap->SetCentre( *m_pPlayer->GetSteer()->GetPosition(), *m_pPlayer->GetSteer()->GetDirection() );
}

// $Barnyard: FUNCTION 005168d0
void AChickenCoopDefender::DestroyPlayer()
{
	if ( m_pGlowObject )
	{
		AGlowViewport::GetSingleton()->RemoveGlowObject( m_pGlowObject );
		m_pGlowObject = TNULL;
	}

	if ( m_pSunglasses )
	{
		if ( m_pPlayer )
			AAnimatableObjectManager::GetSingleton()->DetachObject( m_pSunglasses, m_pPlayer->GetAnimatableObject() );

		AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pSunglasses );
		m_pSunglasses = TNULL;
	}

	if ( m_pPlayer )
	{
		AUnitManager::GetSingleton()->DestroyUnit( m_pPlayer );
		m_pPlayer       = TNULL;
		m_oGame.oPlayer = TNULL;
	}

	APlayerProfileManager::GetSingleton()->GetMainProfile()->SetStaminaWasteSpeed( 0.8f );
}

// $Barnyard: FUNCTION 00516810
void AChickenCoopDefender::SetupCamera()
{
	ACameraManager*        pCameraManager = ACameraManager::GetSingleton();
	AGTAStyleCameraHelper* pHelper;

	if ( pCameraManager->GetCurrentCameraHelper() == pCameraManager->GetSplitscreenCameraHelper( 1 ) )
	{
		pHelper = TSTATICCAST( AGTAStyleCameraHelper, pCameraManager->GetSplitscreenCameraHelper( 1 ) );
	}
	else
	{
		pCameraManager->SetSplitscreenCameraHelper( 1 );
		pHelper = TSTATICCAST( AGTAStyleCameraHelper, pCameraManager->GetSplitscreenCameraHelper( 1 ) );
		pHelper->SetTargetUnit( m_pPlayer );
		pHelper->ResetPosition( TTRUE );
	}

	pHelper->SetDefaultPitch( 11.0f );
	pHelper->SetTargetPitch( 11.0f );
	pHelper->SetDefaultDistance( 5.0f );
	pHelper->SetMaxPitch( 22.0f );
	pHelper->SetDefaultMaxPitch( 22.0f );
	pHelper->SetCollisionRadius( 0.43f );
	pHelper->SetSmallCollisionRadius( 0.2f );
	pHelper->SetHeightOffset( 1.7f );
}

// $Barnyard: FUNCTION 00519e30
void AChickenCoopDefender::CreateThieves()
{
	// The raccoons are visible from further away than the other units
	TFLOAT fFar       = ARenderer::GetSingleton()->GetFar();
	TFLOAT fFadeStart = fFar * 0.5f;
	TFLOAT fFadeScale = 1.0f / ( fFar - fFadeStart );

	for ( TINT i = AChickenCoopDefenderGame::NUM_KIND_THIEVES; i != 0; i-- )
	{
		AChickenCoopDefenderThief* pThief = TSTATICCAST( AChickenCoopDefenderThief, AUnitManager::GetSingleton()->SpawnUnit( TPS8( chickencoop_raccoon ), TVector4::VEC_ZERO, TVector4::VEC_POSX ) );

		TSceneObject* pSceneObject         = pThief->GetAnimatableObject()->GetModelInstance()->GetSceneObject();
		pSceneObject->GetFadeDistances().x = fFadeStart;
		pSceneObject->GetFadeDistances().y = fFar;
		pSceneObject->GetFadeParams().x    = fFadeScale;

		pThief->SetGame( &m_oGame );
		pThief->SetNetwork( m_pNetwork );
		pThief->SetState( AChickenCoopDefenderThief::STATE_GONE );
		m_oGame.oFreeRaccoons.PushBack( pThief->GetSelfProxy() );
	}

	for ( TINT i = AChickenCoopDefenderGame::NUM_KIND_THIEVES; i != 0; i-- )
	{
		AChickenCoopDefenderThief* pThief = TSTATICCAST( AChickenCoopDefenderThief, AUnitManager::GetSingleton()->SpawnUnit( TPS8( chickencoop_coyote ), TVector4::VEC_ZERO, TVector4::VEC_POSX ) );
		pThief->SetGame( &m_oGame );
		pThief->SetNetwork( m_pNetwork );
		pThief->SetState( AChickenCoopDefenderThief::STATE_GONE );
		m_oGame.oFreeCoyotes.PushBack( pThief->GetSelfProxy() );
	}

	for ( TINT i = AChickenCoopDefenderGame::NUM_KIND_THIEVES; i != 0; i-- )
	{
		AChickenCoopDefenderThief* pThief = TSTATICCAST( AChickenCoopDefenderThief, AUnitManager::GetSingleton()->SpawnUnit( TPS8( chickencoop_evilfreddy ), TVector4::VEC_ZERO, TVector4::VEC_POSX ) );
		pThief->SetGame( &m_oGame );
		pThief->SetNetwork( m_pNetwork );
		pThief->SetState( AChickenCoopDefenderThief::STATE_GONE );
		m_oGame.oFreeFreddies.PushBack( pThief->GetSelfProxy() );
	}
}

// $Barnyard: FUNCTION 0051a1a0
void AChickenCoopDefender::DestroyThieves()
{
	AChickenCoopDefenderGame::UnitList10* apLists[] = {
		&m_oGame.oFreeRaccoons,
		&m_oGame.oRaccoons,
		&m_oGame.oFreeCoyotes,
		&m_oGame.oCoyotes,
		&m_oGame.oFreeFreddies,
		&m_oGame.oFreddies,
	};

	for ( TINT i = 0; i < TARRAYSIZE( apLists ); i++ )
	{
		AChickenCoopDefenderGame::UnitList10& rList = *apLists[ i ];

		for ( TINT j = 0; j < rList.Size(); j++ )
		{
			AUnitManager::GetSingleton()->DestroyUnit( rList[ j ]->GetUnit() );
			rList[ j ] = TNULL;
		}

		rList.Clear();
	}
}

// $Barnyard: FUNCTION 0051a460
void AChickenCoopDefender::CreateHensAndEggs()
{
	for ( TINT i = AChickenCoopDefenderGame::NUM_HENS; i != 0; i-- )
	{
		const TPString8& rcType = ( AMiniGameManager::GetSingleton()->GetMiniGameVariant() == VARIANT_FREDDYS_DREAM ) ? TPS8( chickencoop_roast ) : TPS8( chickencoop_hen );

		AChickenCoopDefenderHen* pHen = TSTATICCAST( AChickenCoopDefenderHen, AUnitManager::GetSingleton()->SpawnUnit( rcType, TVector4::VEC_ZERO, TVector4::VEC_POSX ) );
		pHen->SetGame( &m_oGame );
		pHen->SetNetwork( m_pUnitNetwork );
		pHen->SetVisible( TFALSE );
		pHen->SetMovement( TFALSE );
		pHen->SetNoQuery( TTRUE );
		pHen->SetHashed( TFALSE );
		pHen->GetAnimatableObject()->GetModelInstance()->SetShadowMode( AModelInstance::SHADOWMODE_DYNAMIC );

		m_oGame.oHenPool.PushBack( pHen->GetSelfProxy() );
	}

	for ( TINT i = AChickenCoopDefenderGame::NUM_EGGS; i != 0; i-- )
	{
		AChickenCoopDefenderEgg* pEgg = TSTATICCAST( AChickenCoopDefenderEgg, AUnitManager::GetSingleton()->SpawnUnit( TPS8( chickencoop_egg ), TVector4::VEC_ZERO, TVector4::VEC_POSX ) );
		pEgg->ChangeSteer( &TGetClass( AStationarySteer ) );
		pEgg->SetGame( &m_oGame );
		pEgg->SetNoQuery( TTRUE );
		pEgg->SetHashed( TFALSE );
		pEgg->SetMovement( TFALSE );
		pEgg->SetVisible( TFALSE );
		pEgg->GetAnimatableObject()->GetModelInstance()->SetShadowMode( AModelInstance::SHADOWMODE_DYNAMIC );

		m_oGame.oEggPool.PushBack( pEgg->GetSelfProxy() );
	}
}

// $Barnyard: FUNCTION 0051a740
void AChickenCoopDefender::DestroyHensAndEggs()
{
	AChickenCoopDefenderGame::UnitList15* apLists[] = {
		&m_oGame.oHenPool,
		&m_oGame.oCarriedHens,
		&m_oGame.oLooseHens,
		&m_oGame.oEggPool,
		&m_oGame.oCarriedEggs,
		&m_oGame.oLooseEggs,
	};

	for ( TINT i = 0; i < TARRAYSIZE( apLists ); i++ )
	{
		AChickenCoopDefenderGame::UnitList15& rList = *apLists[ i ];

		while ( rList.Size() != 0 )
		{
			AUnitManager::GetSingleton()->DestroyUnit( rList.Back().Value()->GetUnit() );
			rList.PopBack();
		}
	}
}

// $Barnyard: FUNCTION 0051a390
void AChickenCoopDefender::CreateHanna()
{
	AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_POSX, 0, 0, -1, TTRUE );
	m_oGame.pHanna = AUnitManager::GetSingleton()->SpawnUnit( TPS8( chickencoop_hanna ), &oTransform );
	m_oGame.pHanna->ChangeSteer( &TGetClass( AInteriorSteer ) );

	AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, m_oGame.pHanna->GetSteer() );
	pSteer->SetNetwork( m_pNetwork, 0 );
	pSteer->Command_SnapToGoal( TPS8( hanna ) );
	pSteer->Command_CancelCurrentCommand();

	TSTATICCAST( AAnimatedUnit, m_oGame.pHanna )->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );
	TSTATICCAST( AAnimatedUnit, m_oGame.pHanna )->SetVisible( TFALSE );
}

// $Barnyard: FUNCTION 0051a870
void AChickenCoopDefender::CreateModels()
{
	if ( !m_pArrowModel )
		m_pArrowModel = AModelRepos::GetSingleton()->InstantiateNewModel( TPString8( "data\\models\\chickencoop_arrow" ), TNULL );

	// The fence around the coop
	ATerrainInterface*   pTerrain = ATerrainInterface::GetSingleton();
	TPString8            strFence( "data\\models\\fncp01" );
	TPString8            strFence3( "data\\models\\fncp03" );
	TPString8            strBroken( "data\\models\\fncpbroken" );
	ATerrainLocatorList* pLocators = pTerrain->GetLocatorList();
	TINT                 iCount    = pLocators->GetNumLocators();

	for ( TINT i = 0; i < iCount; i++ )
	{
		const TCHAR*     szName = pLocators->GetLocatorName( i );
		const TPString8* pModelName;

		if ( TStringManager::String8FindString( szName, "fncp01" ) == szName )
		{
			if ( TStringManager::String8Compare( szName, "fncp01_59" ) != 0 && TStringManager::String8Compare( szName, "fncp01_67" ) != 0 )
				pModelName = &strFence;
			else
				pModelName = &strBroken;
		}
		else if ( TStringManager::String8FindString( szName, "fncp03" ) == szName )
		{
			if ( TStringManager::String8Compare( szName, "fncp03_18" ) != 0 )
				pModelName = &strFence3;
			else
				pModelName = &strBroken;
		}
		else
		{
			continue;
		}

		AModelInstance* pModelInstance = AModelRepos::GetSingleton()->InstantiateNewModel( *pModelName, TNULL );

		if ( pModelInstance )
		{
			TMatrix44 oTransform;
			pTerrain->GetLocatorTransform( szName, oTransform );
			pModelInstance->SetSkeletonUpdating( TTRUE );
			pModelInstance->SetTransform( oTransform );

			FenceModel* pNode = new FenceModel;
			pNode->pInstance  = pModelInstance;
			m_oFenceModels.PushFront( pNode );
		}
	}

	// Visible from as far as the static instances
	TFLOAT fLOD0Distance, fBlendEnd, fBlendStart;
	AInstanceManager::GetSingleton()->GetVisibilityDistances( fLOD0Distance, fBlendEnd, fBlendStart );

	if ( AModel* pModel = AModelRepos::GetSingleton()->GetModel( strFence ) )
		pModel->GetManagedModel()->GetModel()->m_aLODDistances[ 0 ] = fLOD0Distance;

	// NOTE: retail sets the distance of "fncp01" twice, "fncp03" was probably meant
	if ( AModel* pModel = AModelRepos::GetSingleton()->GetModel( strFence ) )
		pModel->GetManagedModel()->GetModel()->m_aLODDistances[ 0 ] = fLOD0Distance;
}

// $Barnyard: FUNCTION 005169b0
void AChickenCoopDefender::DestroyModels()
{
	if ( m_pArrowModel )
	{
		AModelRepos::GetSingleton()->DestroyModelInstance( m_pArrowModel );
		m_pArrowModel = TNULL;
	}

	while ( m_oFenceModels.IsLinked() )
	{
		FenceModel* pNode = m_oFenceModels.PopFront();
		AModelRepos::GetSingleton()->DestroyModelInstance( pNode->pInstance );
		delete pNode;
	}
}

// $Barnyard: FUNCTION 00517960
// $BarnyardWiiJP: FUNCTION 8026c280
void AChickenCoopDefender::CreateHotSpots()
{
	TINT iNumHotSpots = m_oGame.pScoringNetwork->m_pINetwork->m_iNumNormalAreas;
	m_oGame.pHotSpots = new AChickenCoopDefenderHotSpot[ iNumHotSpots ];

	for ( TINT i = 0; i < iNumHotSpots; i++ )
		m_oGame.pHotSpots[ i ].Create( &m_oGame, i );

	// An icon on the minimap per open hot spot
	const AChickenCoopDefenderGameData* pGameData = m_oGame.pGameData;
	TINT                                iNumOpen  = pGameData->aHotSpotKinds[ 0 ].iNumOpen + pGameData->aHotSpotKinds[ 1 ].iNumOpen + pGameData->aHotSpotKinds[ 2 ].iNumOpen + pGameData->aHotSpotKinds[ 3 ].iNumOpen;

	if ( iNumOpen > iNumHotSpots )
		iNumOpen = iNumHotSpots;

	m_oGame.pMapIcons = TSTATICCAST( TINT, TMalloc( sizeof( TINT ) * iNumOpen ) );

	for ( TINT i = 0; i < iNumOpen; i++ )
		m_oGame.pMapIcons[ i ] = m_oMap.AddIcon( TVector4::VEC_ZERO, TVector4::VEC_POSX, AGUI2MiniMap::ICONTYPE_EDGE, AGUI2MiniMap::ICON_PING_RING,
#ifdef BARNYARD_LATEST
		                                         0.13f // Latest version: bigger hotspot icons
#else
		                                         0.1f
#endif
		);

	// Open random hot spots of every kind
	for ( TINT i = 0; i < AChickenCoopDefenderGame::NUM_KINDS; i++ )
	{
		for ( TINT iNumOpenNow = m_oGame.aOpenHotSpots[ i ].Size(); !m_oGame.aClosedHotSpots[ i ].IsEmpty(); iNumOpenNow++ )
		{
			if ( iNumOpenNow >= pGameData->aHotSpotKinds[ i ].iNumOpen )
				break;

			auto it = m_oGame.aClosedHotSpots[ i ].Begin();

			for ( TINT iRand = ARandom::GetSingleton()->m_oRandom.GetInt( m_oGame.aClosedHotSpots[ i ].Size() ); iRand > 0; iRand-- )
				it++;

			it->SetClosed( TFALSE );
		}
	}
}

// $Barnyard: FUNCTION 00516a10
void AChickenCoopDefender::DestroyHotSpots()
{
	for ( TINT i = 0; i < AChickenCoopDefenderGame::NUM_KINDS; i++ )
	{
		m_oGame.aOpenHotSpots[ i ].Clear();
		m_oGame.aClosedHotSpots[ i ].Clear();
	}

	if ( m_oGame.pHotSpots )
	{
		delete[] m_oGame.pHotSpots;
		m_oGame.pHotSpots = TNULL;
	}

	if ( m_oGame.pMapIcons )
	{
		TFree( m_oGame.pMapIcons );
		m_oGame.pMapIcons = TNULL;
	}
}

// $Barnyard: FUNCTION 00515f40
void AChickenCoopDefender::ShowInteraction( const TVector4& a_rcPosition, const TWCHAR* a_wszText )
{
	// A spinning arrow bobs over what the player can use
	TVector3 vTranslation( a_rcPosition.x, a_rcPosition.y - ( TMath::Sin( TMath::TWO_PI * m_fElapsedTime ) * 0.25f + 0.5f ), a_rcPosition.z );
	TVector3 vEuler( TMath::PI * m_fElapsedTime, 0.0f, TMath::HALF_PI );

	m_pArrowModel->SetSkeletonUpdating( TTRUE );
	m_pArrowModel->GetTransform().SetTranslate( vTranslation );
	m_pArrowModel->GetTransform().SetEuler( vEuler );
	m_pArrowModel->GetTransform().SetScale( 0.5f, 0.5f, 0.5f );
	m_pArrowModel->SetOpacity( 0.5f );

	m_oInteractionHud.SetText( a_wszText, 2, TINT( m_pInputMap ), AInputCommand_Interact, 0, -1, -1.0f );
	m_oInteractionHud.SetForceHide( TFALSE );
}

// $Barnyard: FUNCTION 00517020
void AChickenCoopDefender::UpdateInteraction()
{
	m_pArrowModel->SetSkeletonUpdating( TFALSE );
	m_oInteractionHud.SetText( TNULL, 2, TINT( m_pInputMap ), AInputCommand_Interact, 0, -1, -1.0f );
	m_oInteractionHud.SetForceHide( TTRUE );

	if ( m_pPlayer->GetActiveLauncher()->GetClass()->IsA( &TGetClass( AAimedSquirtProjectileLauncher ) ) )
		return;

	ASteer* pPlayerSteer = m_pPlayer->GetSteer();

	if ( m_oGame.ePlayerState == PLAYERSTATE_FREE )
	{
		// A milked raccoon to pick up
		TFLOAT          fRadius   = pPlayerSteer->GetRadius() + 0.75f;
		const TVector4* pPosition = pPlayerSteer->GetPosition();
		m_oGame.pNearThief        = TNULL;

		for ( TINT i = 0; i < m_oGame.iNumThieves; i++ )
		{
			if ( !m_oGame.oThieves[ i ] )
				continue;

			AChickenCoopDefenderThief* pThief = TSTATICCAST( AChickenCoopDefenderThief, m_oGame.oThieves[ i ]->GetUnit() );

			if ( pThief->GetKind() == AChickenCoopDefenderThief::KIND_RACCOON && pThief->GetState() == AChickenCoopDefenderThief::STATE_MILKED )
			{
				TVector4 vToThief;
				vToThief.Substract( *pThief->GetSteer()->GetPosition(), *pPosition );

				TFLOAT fDistance = vToThief.Magnitude() - pThief->GetSteer()->GetRadius();

				if ( fDistance < fRadius )
				{
					fRadius            = fDistance;
					m_oGame.pNearThief = pThief;
				}
			}
		}

		if ( m_oGame.pNearThief )
		{
			TVector4 vArrow = *m_oGame.pNearThief->GetSteer()->GetPosition();
			vArrow.y -= 0.5f;

			ALocale::FormatTags( m_wszInteraction, TARRAYSIZE( m_wszInteraction ), LOCALE_PICK_UP, PPropertyValue( const_cast<TWCHAR*>( m_oGame.pNearThief->GetScreenName() ), TFALSE ) );
			ShowInteraction( vArrow, m_wszInteraction );
		}

		// A loose hen/egg to pick up
		fRadius       = pPlayerSteer->GetRadius() + 0.75f;
		m_oGame.pItem = TNULL;

		for ( TINT i = 0; i < m_oGame.oLooseEggs.Size(); i++ )
		{
			if ( !m_oGame.oLooseEggs[ i ] )
				continue;

			AUnit* pEgg = m_oGame.oLooseEggs[ i ]->GetUnit();

			TVector4 vToEgg;
			vToEgg.Substract( *pEgg->GetSteer()->GetPosition(), *pPosition );

			TFLOAT fDistance = vToEgg.Magnitude();

			if ( fDistance < fRadius )
			{
				fRadius       = fDistance;
				m_oGame.pItem = pEgg;
			}
		}

		for ( TINT i = 0; i < m_oGame.oLooseHens.Size(); i++ )
		{
			if ( !m_oGame.oLooseHens[ i ] )
				continue;

			AUnit* pHen = m_oGame.oLooseHens[ i ]->GetUnit();

			TVector4 vToHen;
			vToHen.Substract( *pHen->GetSteer()->GetPosition(), *pPosition );

			TFLOAT fDistance = vToHen.Magnitude() - pHen->GetSteer()->GetRadius();

			if ( fDistance < fRadius )
			{
				fRadius       = fDistance;
				m_oGame.pItem = pHen;
			}
		}

		// Picks it up right away
		if ( m_oGame.pItem && ( m_oGame.pItem->IsA( &TGetClass( AChickenCoopDefenderEgg ) ) || m_oGame.pItem->IsA( &TGetClass( AChickenCoopDefenderHen ) ) ) &&
		     m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( pickup ) ) )
		{
			m_oGame.ePlayerState = PLAYERSTATE_PICKUP_ITEM;
			DisablePlayerControl();
		}
	}
	else if ( m_oGame.ePlayerState == PLAYERSTATE_CARRY_EGG || m_oGame.ePlayerState == PLAYERSTATE_CARRY_HEN )
	{
		// A hot spot to throw from
		TINT iArea = m_oGame.pScoringNetwork->FindContainingArea( *pPlayerSteer->GetPosition() );

		if ( iArea > -1 )
		{
			AChickenCoopDefenderHotSpot* pHotSpot = &m_oGame.pHotSpots[ iArea ];

			if ( !pHotSpot->bClosed )
			{
				TVector4 vAreaPosition;
				m_oGame.pScoringNetwork->m_pINetwork->m_pAreas[ iArea ].GetPosition( vAreaPosition );

				if ( TVector4::DistanceXZ( vAreaPosition, *pPlayerSteer->GetPosition() ) >= pHotSpot->fRadius )
				{
					m_oInteractionHud.SetText( TNULL, 2, TINT( m_pInputMap ), AInputCommand_Interact, 0, -1, -1.0f );

					if ( m_oGame.pHotSpot )
					{
						m_oGame.pHotSpot->SetState( AChickenCoopDefenderHotSpot::STATE_IDLE );
						m_oGame.pHotSpot = TNULL;
					}

					m_oInteractionHud.SetForceHide( TTRUE );
				}
				else
				{
					if ( m_oGame.pHotSpot && m_oGame.pHotSpot != pHotSpot && m_oGame.pHotSpot->eState == AChickenCoopDefenderHotSpot::STATE_INSIDE )
						m_oGame.pHotSpot->SetState( AChickenCoopDefenderHotSpot::STATE_IDLE );

					m_oGame.pHotSpot = pHotSpot;
					pHotSpot->SetState( AChickenCoopDefenderHotSpot::STATE_INSIDE );

					ALocale::FormatTags( m_wszInteraction, TARRAYSIZE( m_wszInteraction ), LOCALE_PICK_UP, PPropertyValue( const_cast<TWCHAR*>( m_oGame.pItem->GetScreenName() ), TFALSE ) );
					ShowInteraction( TVector4::VEC_ZERO, m_wszInteraction );
				}
			}
		}

		TSTATICCAST( AAnimatedUnit, m_oGame.pHanna )->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );
	}
}

#ifdef BARNYARD_COMMUNITY_PATCH

// Development: "-devautoplay" runs. The player squirts the thieves, picks up the
// loose hens/eggs and throws them back into the coop from an open hot spot, and throws the milked
// raccoons away. He is moved next to his target instead of walking there.
void AChickenCoopDefender::DevAutoPlay( TFLOAT a_fDeltaTime )
{
	static TINT s_iEnabled = -1;

	if ( s_iEnabled < 0 )
		s_iEnabled = T2String8::FindString( GetCommandLineA(), "-devautoplay" ) != TNULL;

	if ( !s_iEnabled )
		return;

	static TFLOAT s_fLogTimer = 0.0f;
	s_fLogTimer += a_fDeltaTime;

	if ( s_fLogTimer >= 5.0f )
	{
		s_fLogTimer = 0.0f;
		TINFO( "[coopauto] time %.1f/%.1f left %d lives %d thieves %d loose hens %d eggs %d hits %d/%d bonus %d state %d\n", m_fElapsedTime, m_fTimeLimit, m_oGame.iNumLeft, m_oGame.iLives, m_oGame.iNumThieves, m_oGame.oLooseHens.Size(), m_oGame.oLooseEggs.Size(), m_oGame.iNumRaccoonHits, m_oGame.iNumHits, m_oGame.iBonus, m_oGame.ePlayerState );
	}

	static TFLOAT s_fTimer = 0.0f;
	s_fTimer += a_fDeltaTime;

	if ( s_fTimer < 0.75f )
		return;

	APlayerSteer* pSteer = TSTATICCAST( APlayerSteer, m_pPlayer->GetSteer() );

	if ( m_oGame.ePlayerState == PLAYERSTATE_CARRY_EGG || m_oGame.ePlayerState == PLAYERSTATE_CARRY_HEN )
	{
		s_fTimer = 0.0f;

		// In an open hot spot: throw
		if ( m_oGame.pHotSpot && !m_oGame.pHotSpot->bClosed )
		{
			TINFO( "[coopauto] throw the %s into the coop\n", m_oGame.ePlayerState == PLAYERSTATE_CARRY_HEN ? "hen" : "egg" );

			TInputInterface::InputEvent oEvent( TNULL, 0, TInputInterface::EVENT_TYPE_GONE_DOWN );
			TBOOL                       bStopEvents = TFALSE;
			ProcessCommand( AInputCommand_Interact, &oEvent, bStopEvents );
			return;
		}

		// Go to an open hot spot
		for ( TINT i = 0; i < AChickenCoopDefenderGame::NUM_KINDS; i++ )
		{
			if ( !m_oGame.aOpenHotSpots[ i ].IsEmpty() )
			{
				pSteer->SetPosition( m_oGame.aOpenHotSpots[ i ].Begin()->vPosition );
				TINFO( "[coopauto] go to a hot spot of kind %d\n", i );
				return;
			}
		}

		return;
	}

	if ( m_oGame.ePlayerState != PLAYERSTATE_FREE )
		return;

	s_fTimer = 0.0f;

	// A milked raccoon next to the player (found by UpdateInteraction)
	if ( m_oGame.pNearThief )
	{
		TINFO( "[coopauto] pick up the raccoon\n" );

		TInputInterface::InputEvent oEvent( TNULL, 0, TInputInterface::EVENT_TYPE_GONE_DOWN );
		TBOOL                       bStopEvents = TFALSE;
		ProcessCommand( AInputCommand_Interact, &oEvent, bStopEvents );
		return;
	}

	// A loose hen/egg to pick up (UpdateInteraction picks it up)
	AChickenCoopDefenderGame::UnitList15* pLoose = m_oGame.oLooseHens.Size() ? &m_oGame.oLooseHens : &m_oGame.oLooseEggs;

	if ( pLoose->Size() && ( *pLoose )[ 0 ] )
	{
		TVector4 vPosition = *( *pLoose )[ 0 ]->GetUnit()->GetSteer()->GetPosition();
		vPosition.x += 0.5f;
		pSteer->SetPosition( vPosition );
		TINFO( "[coopauto] go to a loose %s\n", pLoose == &m_oGame.oLooseHens ? "hen" : "egg" );
		return;
	}

	// A milked raccoon to pick up
	for ( TINT i = 0; i < m_oGame.iNumThieves; i++ )
	{
		AChickenCoopDefenderThief* pThief = TSTATICCAST( AChickenCoopDefenderThief, m_oGame.oThieves[ i ]->GetUnit() );

		if ( pThief->GetKind() == AChickenCoopDefenderThief::KIND_RACCOON && pThief->GetState() == AChickenCoopDefenderThief::STATE_MILKED )
		{
			TVector4 vPosition = *pThief->GetSteer()->GetPosition();
			vPosition.x += 1.0f;
			pSteer->SetPosition( vPosition );
			TINFO( "[coopauto] go to the milked raccoon\n" );
			return;
		}
	}

	// Squirt the nearest thief, the ones carrying something first
	AChickenCoopDefenderThief* pTarget   = TNULL;
	TFLOAT                     fDistance = 0.0f;

	for ( TINT i = 0; i < m_oGame.iNumThieves; i++ )
	{
		AChickenCoopDefenderThief*        pThief = TSTATICCAST( AChickenCoopDefenderThief, m_oGame.oThieves[ i ]->GetUnit() );
		AChickenCoopDefenderThief::STATE_ eState = pThief->GetState();

		if ( eState == AChickenCoopDefenderThief::STATE_IN_COOP || eState == AChickenCoopDefenderThief::STATE_GONE || eState == AChickenCoopDefenderThief::STATE_MILKED ||
		     eState == AChickenCoopDefenderThief::STATE_MILKED_DROP || eState == AChickenCoopDefenderThief::STATE_PICKED_UP || eState == AChickenCoopDefenderThief::STATE_THROWN ||
		     eState == AChickenCoopDefenderThief::STATE_FLEE )
			continue;

		TFLOAT fThiefDistance = TVector4::DistanceSq( *pThief->GetSteer()->GetPosition(), *pSteer->GetPosition() );

		if ( pThief->GetCarried() )
			fThiefDistance *= 0.01f;

		if ( !pTarget || fThiefDistance < fDistance )
		{
			pTarget   = pThief;
			fDistance = fThiefDistance;
		}
	}

	if ( !pTarget )
		return;

	const TVector4& rcThiefPos = *pTarget->GetSteer()->GetPosition();
	TVector4        vToThief( rcThiefPos.x - pSteer->GetPosition()->x, 0.0f, rcThiefPos.z - pSteer->GetPosition()->z, 1.0f );
	AChickenCoopDefenderNormaliseXZ( vToThief );

	if ( TVector4::DistanceSq( rcThiefPos, *pSteer->GetPosition() ) > 16.0f )
	{
		TVector4 vPosition( rcThiefPos.x - vToThief.x * 3.0f, rcThiefPos.y, rcThiefPos.z - vToThief.z * 3.0f, 1.0f );
		pSteer->SetPosition( vPosition );
	}

	pSteer->SetDirection( vToThief );
	m_pPlayer->GetActiveLauncher()->GetAmmo()->SetAmmo( m_pPlayer->GetActiveLauncher()->GetAmmo()->GetMaxAmmo() );
	m_pPlayer->GetActiveLauncher()->Fire();
	TINFO( "[coopauto] squirt a thief (kind %d state %d)\n", pTarget->GetKind(), pTarget->GetState() );
}

#endif
