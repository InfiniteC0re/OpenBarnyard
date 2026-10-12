#include "pch.h"
#include <Render/TCameraObject.h>
#include "AChasingChicksMiniGame.h"
#include "AMiniGameManager.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/ACharacterAnimController.h"
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
#include "HUD/AHudManager.h"
#include "Input/AInputMapManager.h"
#include "Input/AInputHandler.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Render/ARenderer.h"
#include "ALoadScreen.h"
#include "ARandom.h"
#include "Sound/ASoundManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/ASSeqController.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitPlayer.h"
#include "Units/AUnitTransform.h"
#include "Units/ASimplePlayerControlComponent.h"
#include "Units/AChasingChicksAI.h"
#include "Units/AChasingChicksChick.h"
#include "Units/AChasingChicksHanna.h"
#include "Units/Projectiles/AAimedSquirtProjectileLauncher.h"
#include "Units/Projectiles/AProjectileLauncher.h"
#include "Units/Projectiles/ASimpleProjectileManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "UnitMovement/Steering/APlayerSteer.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/Steering/AThrowItemSteer.h"
#include "Physics/ACollisionCommon.h"

#include <GUI/T2GUITextureSectionManager.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>
#include <Render/TRenderInterface.h>
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

// NOTE: GetClass 0050e540, CreateObject 00512440, CreateObjectInPlace 005124a0 (the constructor is inlined in both)
TDEFINE_CLASS( AChasingChicksMiniGame );

TPSTRING8_DECLARE( AHerdingMinigameState );
TPSTRING8_DECLARE( Goal0 );
TPSTRING8_DECLARE( sunglasses1 );
TPSTRING8_DECLARE( player_chicks );
TPSTRING8_DECLARE( chasingchicks_ai );
TPSTRING8_DECLARE( chasingchicks_chick );
TPSTRING8_DECLARE( chasingchicks_hanna );
TPSTRING8_DECLARE( spawn );
TPSTRING8_DECLARE( hanna );
TPSTRING8_DECLARE( PlayerStart0 );
TPSTRING8_DECLARE( PlayerStart1 );
TPSTRING8_DECLARE( PlayerStart2 );
TPSTRING8_DECLARE( PlayerStart3 );
TPSTRING8_DECLARE( carry_overlay );
TPSTRING8_DECLARE( pickup );
TPSTRING8_DECLARE( throw );
TPSTRING8_DECLARE( throw2 );
TPSTRING8_DECLARE( yuk );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( distress_idle );
TPSTRING8_DECLARE( cb_rlocator );
TPSTRING8_DECLARE( winner_start );
TPSTRING8_DECLARE( winner_loop );
TPSTRING8_DECLARE( loser_start );
TPSTRING8_DECLARE( loser_loop );

// Locale strings
static constexpr TINT LOCALE_THROW_CHICK = 196; // "{0}" prompt over the pen (same string as "Pick up {0}")

// Names of the result rows (one chick / several chicks) per kind of pen
static constexpr TINT LOCALE_ROW_SINGULAR = 1022; // 1022 - 1025
static constexpr TINT LOCALE_ROW_PLURAL   = 1026; // 1026 - 1029

// Hanna's distress sound when a chick is thrown far
static constexpr ASoundId SOUND_HANNA_DISTRESS = 347;

// Where the players start (0x00773d68)
static const TPString8* s_apPlayerStarts[ AChasingChicksGame::MAX_PLAYERS ] = {
	&TPS8( PlayerStart0 ),
	&TPS8( PlayerStart1 ),
	&TPS8( PlayerStart2 ),
	&TPS8( PlayerStart3 ),
};

// NOTE: engine TVector4 method of the original binary (006c7540), see AGTAStyleCameraHelper.cpp
static void NormaliseXZ( TVector4& a_rVector )
{
	a_rVector.y = 0.0f;

	TFLOAT fMagSq = a_rVector.x * a_rVector.x + a_rVector.z * a_rVector.z;

	if ( fMagSq == 0.0f )
	{
		a_rVector.x = 0.0f;
		a_rVector.z = 1.0f;
		return;
	}

	TFLOAT fMag = TMath::Sqrt( fMagSq );
	a_rVector.x = 1.0f / fMag * a_rVector.x;
	a_rVector.z = 1.0f / fMag * a_rVector.z;
}

static AToshiAnimationRef* GetOverlayAnim( AUnit* a_pUnit, const TPString8& a_rcName )
{
	AToshiAnimationInterface& rInterface = TSTATICCAST( AAnimatedUnit, a_pUnit )->GetAnimatableObject()->GetToshiAnimInterface();
	return AToshiAnimationInterface::GetAnimationRef( rInterface.GetOverlayAnimRefs(), a_rcName );
}

//-----------------------------------------------------------------------------
// AChasingChicksPlayer
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0050ed70
AChasingChicksPlayer::AChasingChicksPlayer()
    : pSunglasses( TNULL )
    , fUnk8( 0.0f ) // NOTE: retail copies two floats of .bss (0x007d3018 / 0x007d301c), zero
    , fUnkC( 0.0f )
    , uiFlags( 0 )
    , eState( STATE_FREE )
    , iScore( 0 )
    , pChick( TNULL )
    , pControl( TNULL )
    , fStunTime( 0.0f )
    , pHotSpot( TNULL )
{
	for ( TINT i = 0; i < TARRAYSIZE( aNumChicksOfKind ); i++ )
		aNumChicksOfKind[ i ] = 0;
}

// $Barnyard: FUNCTION 0050edb0
AChasingChicksPlayer::~AChasingChicksPlayer()
{
	// NOTE: retail releases oUnit twice (the second time it's already TNULL)
	oUnit = TNULL;
}

//-----------------------------------------------------------------------------
// AChasingChicksGame
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0050fe50
AChasingChicksGame::AChasingChicksGame( TFLOAT* a_pTimeLimit, TFLOAT* a_pElapsedTime, AGUI2MiniMap* a_pMap, AMilkBottlePickups* a_pMilkBottles )
    : iNumFreeChicks( 0 )
    , iNumPlayers( 0 )
    , iNumChicks( 0 )
    , iLeader( 0 )
    , iRunnerUp( 0 )
    , pTimeLimit( a_pTimeLimit )
    , pElapsedTime( a_pElapsedTime )
    , vCentre( TVector4::VEC_ZERO )
    , pScoringNetwork( TNULL )
    , pAIData( TNULL )
    , iNumSpawns( 0 )
    , pSpawns( TNULL )
    , pHotSpots( TNULL )
    , pMap( a_pMap )
    , pMapIcons( TNULL )
    , pMilkBottles( a_pMilkBottles )
    , ePhase( 0 )
    , iUnk300( 0 )
    , pHanna( TNULL )
{
}

// $Barnyard: FUNCTION 0050ee60
AChasingChicksGame::~AChasingChicksGame()
{
}

// $Barnyard: FUNCTION 0050ecc0
void AChasingChicksGame::PushChick( const TRef<AUnitProxy>& a_rChick )
{
	aFreeChicks[ iNumFreeChicks ] = a_rChick;
	iNumFreeChicks++;
}

// $Barnyard: FUNCTION 0050ed00
void AChasingChicksGame::PopChick()
{
	iNumFreeChicks--;
	aFreeChicks[ iNumFreeChicks ] = TNULL;
}

//-----------------------------------------------------------------------------
// AChasingChicksMiniGame
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005116f0
AChasingChicksMiniGame::AChasingChicksMiniGame()
    : m_oGame( &m_fTimeLimit, &m_fElapsedTime, &m_oMap, &m_oMilkBottles )
    , m_pPlayer( TNULL )
    , m_pNetwork( TNULL )
    , m_bTimeUp( TFALSE )
    , m_fEndTimer( 0.0f )
    , m_iUnked8( 0 )
    , m_pGameTimeFXManager( TNULL )
    , m_fOldNearClip( 1.0f )
    , m_bRenderReticles( TFALSE )
{
}

// $Barnyard: FUNCTION 00511900
// $Barnyard: FUNCTION 005124e0
// $Barnyard: FUNCTION 005118f0
AChasingChicksMiniGame::~AChasingChicksMiniGame()
{
}

// $Barnyard: FUNCTION 0050e550
TBOOL AChasingChicksMiniGame::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	return AGameState::ProcessInput( a_pInputEvent );
}

// $Barnyard: FUNCTION 0050e640
TBOOL AChasingChicksMiniGame::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( FindChildState( &TGetClass( AAnticGameOverState ) ) )
		return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	if ( a_eInputCommand == AInputCommand_Interact && a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
		m_oGame.aPlayers[ 0 ].uiFlags |= AChasingChicksPlayer::FLAGS_INTERACT;

	return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $Barnyard: FUNCTION 0050e720
void AChasingChicksMiniGame::OnRender( TFLOAT a_fDeltaTime )
{
	if ( g_oLoadScreen.IsLoadingScreen() )
		return;

	if ( m_oGame.pScoringNetwork && m_oGame.pHotSpots )
	{
		TINT  iNumHotSpots = m_oGame.pScoringNetwork->m_pINetwork->m_iNumNormalAreas;
		TBOOL bThrowing    = m_oGame.aPlayers[ 0 ].eState == AChasingChicksPlayer::STATE_CARRY;

		for ( TINT i = 0; i < iNumHotSpots; i++ )
		{
			AChasingChicksHotSpot& rHotSpot = m_oGame.pHotSpots[ i ];

			if ( ( bThrowing && !rHotSpot.bClosed ) || rHotSpot.eState != AChasingChicksHotSpot::STATE_IDLE )
				rHotSpot.Render( a_fDeltaTime );
		}
	}

	if ( m_bRenderReticles )
		m_oReticles.UpdateMiniGame( a_fDeltaTime );

	if ( AMilkManager::GetSingleton() )
		AMilkManager::GetSingleton()->Render();

	if ( AGameStateController::GetSingleton()->GetCurrentState()->GetClass() != &TGetClass( ASSeqState ) )
		m_oMilkBottles.Render();
}

// $Barnyard: FUNCTION 0050e7f0
void AChasingChicksMiniGame::OnPostRender( TFLOAT a_fDeltaTime )
{
	if ( g_oLoadScreen.IsLoadingScreen() )
		return;

	if ( m_oGame.pScoringNetwork && m_oGame.pHotSpots )
	{
		TINT  iNumHotSpots = m_oGame.pScoringNetwork->m_pINetwork->m_iNumNormalAreas;
		TBOOL bThrowing    = m_oGame.aPlayers[ 0 ].eState == AChasingChicksPlayer::STATE_CARRY;

		for ( TINT i = 0; i < iNumHotSpots; i++ )
		{
			AChasingChicksHotSpot& rHotSpot = m_oGame.pHotSpots[ i ];

			if ( ( bThrowing && !rHotSpot.bClosed ) || rHotSpot.eState != AChasingChicksHotSpot::STATE_IDLE )
				rHotSpot.PostRender( a_fDeltaTime );
		}
	}
}

// $Barnyard: FUNCTION 0050e9f0
void AChasingChicksMiniGame::OnPause()
{
	for ( TINT i = 0; i < m_oGame.iNumChicks; i++ )
	{
		AUnit* pChick = m_oGame.aFreeChicks[ i ] ? m_oGame.aFreeChicks[ i ]->GetUnit() : TNULL;

		if ( pChick )
			TSTATICCAST( AAnimatedUnit, pChick )->SetVisible( TTRUE );
	}

	for ( TINT i = 0; i < m_oGame.iNumPlayers; i++ )
	{
		if ( m_oGame.aPlayers[ i ].oUnit )
			TSTATICCAST( AAnimatedUnit, m_oGame.aPlayers[ i ].oUnit->GetUnit() )->SetVisible( TTRUE );
	}

	m_pPlayer->GetAnimatableObject()->SetAnimControllersDisabled( TFALSE );
	m_oGame.pHanna->SetVisible( TTRUE );

	AGameState::OnPause();
}

// $Barnyard: FUNCTION 00511160
void AChasingChicksMiniGame::OnInsertion()
{
	m_pInputMap    = AInputMapManager::GetSingleton()->FindMap( TPS8( AHerdingMinigameState ) );
	m_fOldNearClip = ARenderer::GetSingleton()->SetNear( 0.1f );

	AGameLoader::Terrain_Set( AGameLoader::Terrain_ChasingChicks2, TFALSE, TTRUE, 0, 0, 0, 0 );
	AGameLoader::Terrain_Load();

	m_oGame.pScoringNetwork = AInteriorNetworkManager::GetSingleton()->LoadNetwork( TPS8D( "chasingchicks_scoring" ) );

	TPString8 strNetwork = TPS8D( "chasingchicks" );
	AInteriorNetworkManager::GetSingleton()->LoadNetwork( strNetwork );
	m_pNetwork = AInteriorNetworkManager::GetSingleton()->SetDefaultNetwork( strNetwork );

	if ( !m_oMilkBottles.LoadFromLocators( ATerrainInterface::GetSingleton()->GetLocatorList() ) )
		m_oMilkBottles.LoadFromNetwork( m_pNetwork );

	LoadData();

	// The centre of the farmyard
	AInteriorGoal* pGoal = m_oGame.pScoringNetwork->FindGoalInArea( TPS8( Goal0 ), -1 );
	pGoal->GetPosition( m_oGame.vCentre );
	m_oGame.vCentre.y -= 1.0f;
	m_oGame.vCentreDir = pGoal->m_vForward;

	// The minimap
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
	    50
	);
	m_oMap.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_BOTTOMLEFT );
	m_GUIElement.AddChildTail( m_oMap );
	m_oMap.SetZoom( 0.5f );

	m_oGame.iNumPlayers = 0;

	CreatePlayers();
	SetupCamera();
	CreateChicks();
	CreateHanna();

	ASimpleProjectileManager::GetSingleton()->CreatePool( 10, ASimpleProjectileManager::TYPE_WITH_UNIT );
	ASimpleProjectileManager::GetSingleton()->CreatePool( 10, ASimpleProjectileManager::TYPE_CHASING_CHICKS_THROW );
	ASimpleProjectileManager::GetSingleton()->CreatePool( 250, ASimpleProjectileManager::TYPE_CHASING_CHICKS_MILK );
	AMilkManager::Load( "Data/MilkParticles/CCMilk.trb" );

	AChasingChicksGame::ms_pCurrent = &m_oGame;
	m_fTimeLimit                    = m_oGame.pGameData->fTimeLimit;

	AHudManager::GetSingleton()->AddElement( &m_oInteractionHud, TFALSE, TTRUE, TFALSE );
	m_oInteractionHud.SetForceHide( TTRUE );

	// The light goes from the dusk to the night during the game
	m_pGameTimeFXManager = AGameTimeFXManager::GetSingleton();
	AGameTimeFX* pDusk   = m_pGameTimeFXManager->GetWeatherSettings( AWeatherManager::WEATHERTYPE_NORMAL, AGameTimeManager::DAYPHASE_DUSK );
	m_oFXTo              = *pDusk;
	m_oFXFrom            = *pDusk;
	m_oFXFrom.Interpolate( *pDusk, *m_pGameTimeFXManager->GetWeatherSettings( AWeatherManager::WEATHERTYPE_NORMAL, AGameTimeManager::DAYPHASE_NIGHT ), m_oGame.pGameData->fFXStart );
	m_oFXTo.Interpolate( *m_pGameTimeFXManager->GetWeatherSettings( AWeatherManager::WEATHERTYPE_NORMAL, AGameTimeManager::DAYPHASE_DUSK ), *m_pGameTimeFXManager->GetWeatherSettings( AWeatherManager::WEATHERTYPE_NORMAL, AGameTimeManager::DAYPHASE_NIGHT ), m_oGame.pGameData->fFXEnd );
	m_pGameTimeFXManager->SetForcedFX( &m_oFXFrom );

	CreateHotSpots();

	m_oMap.SetColours( 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF );

	// The milk ammo
	m_oAmmoBar.Create( 32.5f, 1.625f, 0.3f );
	m_oAmmoBar.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oAmmoBacking.AddChildTail( m_oAmmoBar );
	m_oAmmoBacking.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Score_Wood_Thin" ) );
	m_oAmmoBacking.SetWidth( m_oAmmoBar.GetWidth() + 52.0f );
	m_oAmmoBacking.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oAmmoRoot.SetDimensions( m_oAmmoBacking.GetWidth(), m_oAmmoBar.GetHeight() );
	m_oAmmoRoot.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oAmmoRoot.AddChildTail( m_oAmmoBacking );
	m_aPortraits[ 0 ].AddChildTail( m_oAmmoRoot );

	ASSeqController::GetSingleton()->LoadScript( "SSCC_ChasingChicks.tso", TVector4::VEC_ZERO );
	AMiniGameState::OnInsertion();
}

// $Barnyard: FUNCTION 0050ffb0
void AChasingChicksMiniGame::OnRemoval()
{
	ARenderer::GetSingleton()->SetNear( m_fOldNearClip );
	m_fOldNearClip = 1.0f;

	TRenderInterface::GetSingleton()->BeginEndSceneHAL();
	ASimpleProjectileManager::GetSingleton()->DestroyAllPools();

	if ( AMilkManager::GetSingleton() )
		delete AMilkManager::GetSingleton();

	AChasingChicksGame::ms_pCurrent = TNULL;
	m_oMilkBottles.Clear();

	ResetCamera();
	DestroyHotSpots();
	DestroyPlayers();
	DestroyChicks();

	if ( m_oGame.pHanna )
	{
		AUnitManager::GetSingleton()->DestroyUnit( m_oGame.pHanna );
		m_oGame.pHanna = TNULL;
	}

	m_oAmmoRoot.RemoveSelf();
	m_oAmmoBar.RemoveSelf();
	m_oAmmoBacking.RemoveSelf();
	m_oInteractionHud.Detach();

	AGameTimeFXManager::GetSingleton()->ClearForcedFX();
	AMiniGameState::OnRemoval();
}

// $Barnyard: FUNCTION 0050ebc0
void AChasingChicksMiniGame::Pause()
{
	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "SSCC_ChasingChicks", "SSCC_ChasingChicks", "SSCC_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
	);
}

// $Barnyard: FUNCTION 0050eaa0
void AChasingChicksMiniGame::PlayLoseSequence()
{
	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "SSCC_ChasingChicksDefeat", "SSCC_ChasingChicksDefeat", "SSCC_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
	);
}

// $Barnyard: FUNCTION 0050eb30
void AChasingChicksMiniGame::PlayWinSequence()
{
	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "SSCC_ChasingChickswinner", "SSCC_ChasingChickswinner", "SSCC_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
	);
}

// $Barnyard: FUNCTION 0050ec50
void AChasingChicksMiniGame::OnPreStart()
{
	for ( TINT i = 0; i < AChasingChicksGame::NUM_CHICKS; i++ )
	{
		AUnit* pChick = m_oGame.aFreeChicks[ i ] ? m_oGame.aFreeChicks[ i ]->GetUnit() : TNULL;

		if ( pChick )
			TSTATICCAST( AAnimatedUnit, pChick )->SetVisible( TFALSE );
	}

	m_pPlayer->SetVisible( TFALSE );
	m_pPlayer->GetAnimatableObject()->SetAnimControllersDisabled( TTRUE );
	m_oGame.pHanna->SetVisible( TFALSE );
}

// $Barnyard: FUNCTION 0050e570
TBOOL AChasingChicksMiniGame::IsGameOver()
{
	if ( m_bTimeUp )
		return TTRUE;

	if ( m_fElapsedTime <= m_fTimeLimit )
		return TFALSE;

	m_bTimeUp = TTRUE;
	return TTRUE;
}

// $Barnyard: FUNCTION 00515e90
// NOTE: folded with the UpdateAlways of Chicken Coop Defender
void AChasingChicksMiniGame::UpdateAlways( TFLOAT a_fDeltaTime )
{
	if ( AMilkManager::GetSingleton() )
		AMilkManager::GetSingleton()->Update( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00511a60
void AChasingChicksMiniGame::UpdateGame( TFLOAT a_fDeltaTime )
{
	m_bRenderReticles = TTRUE;
	m_oMilkBottles.Update( a_fDeltaTime, TTRUE );

	if ( m_oGame.ePhase == PHASE_FOLLOW_CHICK )
	{
		m_fElapsedTime -= a_fDeltaTime;
		return;
	}

	if ( m_oGame.ePhase == PHASE_ENDING )
	{
		// Wait for the throws to finish
		for ( TINT i = 0; i < m_oGame.iNumPlayers; i++ )
		{
			AChasingChicksPlayer& rPlayer = m_oGame.aPlayers[ i ];
			AAnimatedUnit*        pUnit   = TSTATICCAST( AAnimatedUnit, rPlayer.oUnit->GetUnit() );

			if ( pUnit->GetAnimatableObject()->IsAnimationFinished( TPS8( throw ) ) &&
			     pUnit->GetAnimatableObject()->IsAnimationFinished( TPS8( throw2 ) ) )
				rPlayer.eState = AChasingChicksPlayer::STATE_FREE;
		}

		m_fElapsedTime -= a_fDeltaTime;
		return;
	}

	if ( m_oGame.ePhase == PHASE_ENDED )
	{
		m_fElapsedTime -= a_fDeltaTime;
		return;
	}

	// Update the open pens and their icons on the minimap
	const AChasingChicksGameData* pGameData = m_oGame.pGameData;
	TINT                          iNumOpen  = pGameData->aChickKinds[ 0 ].iNumOpen + pGameData->aChickKinds[ 1 ].iNumOpen + pGameData->aChickKinds[ 2 ].iNumOpen + pGameData->aChickKinds[ 3 ].iNumOpen;

	if ( iNumOpen > m_oGame.pScoringNetwork->m_pINetwork->m_iNumNormalAreas )
		iNumOpen = m_oGame.pScoringNetwork->m_pINetwork->m_iNumNormalAreas;

	TINT iIcon = 0;

	for ( TINT i = 0; iIcon < iNumOpen; )
	{
		T2_FOREACH( m_oGame.aOpenPens[ i ], it )
		{
			if ( iIcon >= iNumOpen )
				break;

			it->Update( a_fDeltaTime, m_oGame.pMapIcons[ iIcon ] );
			iIcon++;
		}

		if ( ++i >= AChasingChicksGame::MAX_PLAYERS )
		{
			// Hide the icons left
			for ( ; iIcon < iNumOpen; iIcon++ )
				m_oMap.SetIconVisible( m_oGame.pMapIcons[ iIcon ], TFALSE );

			break;
		}
	}

	// The light goes down with the time
	AGameTimeFX oFX( m_oFXFrom );
	TFLOAT      fProgress = m_fElapsedTime / m_oGame.pGameData->fTimeLimit;
	oFX.Interpolate( m_oFXFrom, m_oFXTo, ( fProgress >= 1.0f ) ? 1.0f : fProgress );
	m_pGameTimeFXManager->SetForcedFX( &oFX );

	for ( TINT i = 0; i < m_oGame.iNumPlayers; i++ )
	{
		AChasingChicksPlayer& rPlayer = m_oGame.aPlayers[ i ];
		AUnitPlayer*          pUnit   = TSTATICCAST( AUnitPlayer, rPlayer.oUnit->GetUnit() );
		ASteer*               pSteer  = pUnit->GetSteer();

		if ( rPlayer.uiFlags & AChasingChicksPlayer::FLAGS_AI )
			pUnit->SetFlags( pUnit->GetFlags() | AUnit::FLAGS_MILK_TARGET );
		else
			pUnit->SetFlags( pUnit->GetFlags() & ~AUnit::FLAGS_MILK_TARGET );

		switch ( rPlayer.eState )
		{
			case AChasingChicksPlayer::STATE_FREE:
			{
				if ( GetOverlayAnim( pUnit, TPS8( carry_overlay ) ) )
					pUnit->GetAnimatableObject()->GetToshiAnimInterface().StopAnim( TPS8( carry_overlay ) );

				// Catch a chick
				TFLOAT fRadius = pSteer->GetRadius() + 1.0f;
				rPlayer.pChick = AChasingChicksGame::FindChickNear( *pSteer->GetPosition(), fRadius );

				if ( rPlayer.pChick )
					PlayPlayerAnim( &rPlayer, TPS8( pickup ) );

				break;
			}
			case AChasingChicksPlayer::STATE_PICKUP:
				if ( pUnit->GetAnimatableObject()->IsAnimationFinished( TPS8( pickup ) ) )
				{
					rPlayer.eState = AChasingChicksPlayer::STATE_FREE;
					rPlayer.pControl->SetMovementEnabled( TTRUE );
				}

				break;
			case AChasingChicksPlayer::STATE_CARRY:
			{
				if ( pSteer->GetMovementType() < 4 && !GetOverlayAnim( pUnit, TPS8( carry_overlay ) ) )
					pUnit->GetAnimatableObject()->GetToshiAnimInterface().PlayAnim( TPS8( carry_overlay ), 1.0f, TFALSE );

				TINT iArea = m_oGame.pScoringNetwork->FindContainingArea( *pSteer->GetPosition() );

				if ( iArea <= -1 )
					break;

				// Is the player in a pen?
				AChasingChicksHotSpot* pHotSpot = &m_oGame.pHotSpots[ iArea ];
				AInteriorArea*         pArea    = &m_oGame.pScoringNetwork->m_pINetwork->m_pAreas[ iArea ];
				TBOOL                  bInside  = !pHotSpot->bClosed;

				if ( bInside )
				{
					TVector4 vAreaPos;
					pArea->GetPosition( vAreaPos );
					bInside = TVector4::DistanceXZ( vAreaPos, *pSteer->GetPosition() ) < pHotSpot->fRadius;
				}

				if ( !bInside )
				{
					if ( i == 0 )
						m_oInteractionHud.SetForceHide( TTRUE );

					if ( rPlayer.pHotSpot && rPlayer.pHotSpot->eState == AChasingChicksHotSpot::STATE_INSIDE )
					{
						rPlayer.pHotSpot->SetState( AChasingChicksHotSpot::STATE_IDLE );
						rPlayer.pHotSpot = TNULL;
					}

					rPlayer.uiFlags &= ~AChasingChicksPlayer::FLAGS_INTERACT;
					break;
				}

				if ( rPlayer.pHotSpot && rPlayer.pHotSpot != pHotSpot && rPlayer.pHotSpot->eState == AChasingChicksHotSpot::STATE_INSIDE )
					rPlayer.pHotSpot->SetState( AChasingChicksHotSpot::STATE_IDLE );

				rPlayer.pHotSpot = pHotSpot;
				pHotSpot->SetState( AChasingChicksHotSpot::STATE_INSIDE );

				if ( !( rPlayer.uiFlags & AChasingChicksPlayer::FLAGS_INTERACT ) )
				{
					if ( i == 0 )
					{
						// "Throw <chick>"
						ALocale::FormatTags( m_wszInteraction, TARRAYSIZE( m_wszInteraction ), LOCALE_THROW_CHICK, PPropertyValue( const_cast<TWCHAR*>( rPlayer.pChick->GetScreenName() ), TFALSE ) );
						ShowInteraction( TVector4::VEC_ZERO, m_wszInteraction );
					}

					rPlayer.uiFlags &= ~AChasingChicksPlayer::FLAGS_INTERACT;
					break;
				}

				// Throw the chick into the pen
				rPlayer.uiFlags |= AChasingChicksPlayer::FLAGS_THROWING;

				if ( pUnit->GetActiveLauncher() != pUnit->GetLauncher( 0 ) )
					pUnit->SelectLauncher( TFALSE );

				rPlayer.pControl->SetMovementEnabled( TFALSE );
				rPlayer.pControl->SetLauncherInput( TFALSE );
				rPlayer.pControl->SetSecondaryLauncherInput( TFALSE );
				rPlayer.eState = AChasingChicksPlayer::STATE_AIM;

				TVector4 vToCentre;
				vToCentre.Substract( m_oGame.vCentre, *pSteer->GetPosition() );
				vToCentre.w = m_oGame.vCentre.w;
				NormaliseXZ( vToCentre );
				TSTATICCAST( ACommonSteer, pSteer )->StopAndFace( vToCentre );

				// Close the pen and open another one of the same kind
				rPlayer.pHotSpot->SetState( AChasingChicksHotSpot::STATE_SCORED );
				rPlayer.pHotSpot->SetClosed( TTRUE );

				T2DList<AChasingChicksHotSpot>& rClosed = m_oGame.aClosedPens[ rPlayer.pHotSpot->iKind ];
				auto                            itPen   = rClosed.Begin();

				for ( TINT iRand = ARandom::GetSingleton()->m_oRandom.GetInt( rClosed.Size() ); iRand > 0; iRand-- )
					itPen++;

				itPen->SetClosed( TFALSE );
				rPlayer.uiFlags &= ~AChasingChicksPlayer::FLAGS_INTERACT;
				break;
			}
			case AChasingChicksPlayer::STATE_AIM:
			{
				TVector4 vToCentre;
				vToCentre.Substract( m_oGame.vCentre, *pSteer->GetPosition() );
				vToCentre.y  = 0.0f;
				vToCentre.w  = m_oGame.vCentre.w;
				TFLOAT fDist = vToCentre.NormaliseMag();
				TSTATICCAST( ACommonSteer, pSteer )->StopAndFace( vToCentre );

				if ( !pSteer->IsFacingDesired( 0.98f ) )
					break;

				if ( i == 0 )
				{
					AGTAStyleCameraHelper* pHelper = TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 ) );
					pHelper->ResetPosition( TFALSE );
				}

				if ( fDist <= 10.0f )
				{
					// A short throw
					if ( PlayPlayerAnim( &rPlayer, TPS8( throw ) ) && GetOverlayAnim( pUnit, TPS8( carry_overlay ) ) )
						pUnit->GetAnimatableObject()->GetToshiAnimInterface().StopAnim( TPS8( carry_overlay ) );
				}
				else if ( PlayPlayerAnim( &rPlayer, TPS8( throw2 ) ) )
				{
					// A long throw, Hanna is worried
					m_oGame.pHanna->GetAnimatableObject()->PlayAnimation( TPS8( distress_idle ) );
					ASoundManager::GetSingleton()->PlayCueAtPosition( SOUND_HANNA_DISTRESS, *m_oGame.pHanna->GetSteer()->GetPosition() );

					if ( GetOverlayAnim( pUnit, TPS8( carry_overlay ) ) )
						pUnit->GetAnimatableObject()->GetToshiAnimInterface().StopAnim( TPS8( carry_overlay ) );
				}

				break;
			}
			case AChasingChicksPlayer::STATE_THROW:
				if ( !pUnit->GetAnimatableObject()->IsAnimationFinished( TPS8( throw ) ) )
					break;

				if ( !pUnit->GetAnimatableObject()->IsAnimationFinished( TPS8( throw2 ) ) || m_oGame.ePhase == PHASE_FOLLOW_CHICK )
					break;

				rPlayer.pControl->SetMovementEnabled( TTRUE );
				rPlayer.pControl->SetLauncherInput( TTRUE );
				rPlayer.pControl->SetSecondaryLauncherInput( TTRUE );

				if ( pUnit->GetAttachedUnit( TPS8( cb_rlocator ), -1 ) )
				{
					// Still holds a chick
					rPlayer.eState = AChasingChicksPlayer::STATE_CARRY;
				}
				else
				{
					rPlayer.eState = AChasingChicksPlayer::STATE_FREE;

					if ( rPlayer.uiFlags & AChasingChicksPlayer::FLAGS_HIT )
					{
						// Hit during the throw
						OnPlayerHit( rPlayer.oUnit );
					}
					else
					{
						goto UPDATE_LAUNCHER;
					}
				}

				break;
			case AChasingChicksPlayer::STATE_STUNNED:
				rPlayer.fStunTime -= a_fDeltaTime;

				if ( rPlayer.fStunTime >= 0.0f )
					break;

				pUnit->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );
				rPlayer.pControl->SetMovementEnabled( TTRUE );
				rPlayer.pControl->SetLauncherInput( TTRUE );
				rPlayer.pControl->SetSecondaryLauncherInput( TTRUE );
				rPlayer.eState = AChasingChicksPlayer::STATE_FREE;
				pUnit->GetAnimatableObject()->SetAnimControllersDisabled( TFALSE );

				if ( pUnit->GetClass() == &TGetClass( AChasingChicksAI ) )
					rPlayer.uiFlags |= AChasingChicksPlayer::FLAGS_AI;

				break;
		}

		// Hit while jumping or throwing: stun as soon as possible
		if ( rPlayer.uiFlags & AChasingChicksPlayer::FLAGS_HIT )
		{
			APlayerSteer::STATE eSteerState = TSTATICCAST( APlayerSteer, pSteer )->GetState();

			if ( ( eSteerState < APlayerSteer::STATE_JUMPING || eSteerState > APlayerSteer::STATE_FALLING ) && rPlayer.eState != AChasingChicksPlayer::STATE_THROW )
				HitPlayer( pUnit, i );
		}

	UPDATE_LAUNCHER:
		pUnit->GetActiveLauncher()->Update( a_fDeltaTime );

		if ( i == 0 )
		{
			ALaunchAmmo* pAmmo = pUnit->GetActiveLauncher()->GetAmmo();
			m_oAmmoBar.SetAmmo( TINT( pAmmo->GetAmmo() ), pAmmo->GetMaxAmmo() );
		}
	}

	if ( m_oGame.aPlayers[ 0 ].eState != AChasingChicksPlayer::STATE_CARRY )
		m_oInteractionHud.SetForceHide( TTRUE );

	// The scores and the leaders
	for ( TINT i = 0; i < m_oGame.iNumPlayers; i++ )
	{
		m_aPortraits[ i ].SetScore( m_oGame.aPlayers[ i ].iScore );

		if ( m_oGame.aPlayers[ i ].iScore > m_oGame.aPlayers[ m_oGame.iLeader ].iScore )
		{
			m_oGame.iRunnerUp = m_oGame.iLeader;
			m_oGame.iLeader   = i;
		}
	}

	ASteer* pSteer = m_pPlayer->GetSteer();
	m_oMap.SetCentre( *pSteer->GetPosition(), *pSteer->GetDirection() );

#ifdef BARNYARD_COMMUNITY_PATCH
	DevAutoPlay( a_fDeltaTime );
#endif
}

// $Barnyard: FUNCTION 0050f3a0
void AChasingChicksMiniGame::UpdateGameOver( TFLOAT a_fDeltaTime )
{
	for ( TINT i = 0; i < m_oGame.iNumPlayers; i++ )
		m_aPortraits[ i ].SetScore( m_oGame.aPlayers[ i ].iScore );

	UpdateEnd();

	if ( m_oGame.ePhase != PHASE_ENDED )
		return;

	AAnimatableObject* pObject = m_pPlayer->GetAnimatableObject();

	if ( !pObject->IsAnimPlaying( TPS8( winner_start ), 0.0f ) &&
	     !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( winner_loop ), 0.0f ) &&
	     !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( loser_start ), 0.0f ) &&
	     !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( loser_loop ), 0.0f ) )
	{
		// Turn to the camera, then cheer or sulk
		ASteer*  pSteer = m_pPlayer->GetSteer();
		TVector4 vToCamera;
		vToCamera.Substract( ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation(), *pSteer->GetPosition() );
		NormaliseXZ( vToCamera );

		if ( TVector4::DotProduct3( *m_pPlayer->GetSteer()->GetDirection(), vToCamera ) <= 0.98f )
			TSTATICCAST( ACommonSteer, m_pPlayer->GetSteer() )->StopAndFace( vToCamera );
		else if ( m_oResults.iResult != 0 )
			m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( loser_start ) );
		else
			m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( winner_start ) );
	}

	if ( ( m_fElapsedTime < m_fTimeLimit || m_oTimeLeftHudElement.GetTimer().IsTickComplete() ) && m_fEndTimer < 0.0f )
		AGameStateController::GetSingleton()->PushState( new AAnticGameOverState( &m_oResults, TFALSE ) );

	m_fEndTimer -= a_fDeltaTime;
}

// $Barnyard: FUNCTION 0050f600
AMiniGameState::S1* AChasingChicksMiniGame::GetResults()
{
	PlayerInfo& rInfo = m_oResults.oPlayerList[ 0 ];
	rInfo.iUnk2       = m_bTimeUp ? m_oGame.aPlayers[ 0 ].iScore : 0;
	rInfo.bUnk1       = TTRUE;

	// The winner: the player with a score better than everybody else's
	m_oResults.iResult = ( rInfo.iUnk2 > 0 ) - 1;

	for ( TINT i = 1; i < m_oGame.iNumPlayers; i++ )
	{
		PlayerInfo& rOther = m_oResults.oPlayerList[ i ];
		rOther.iUnk2       = m_oGame.aPlayers[ i ].iScore;
		rOther.bUnk1       = TTRUE;

		if ( rOther.iUnk2 > 0 && ( m_oResults.iResult < 0 || rOther.iUnk2 > m_oResults.oPlayerList[ m_oResults.iResult ].iUnk2 ) )
			m_oResults.iResult = i;
	}

	m_oResults.iMiniGameIndex = AMiniGameManager::GetSingleton()->GetNextMiniGameIndex();
	m_oResults.bWon           = m_oResults.iResult == 0 && rInfo.iUnk2 > 0;

	// "%d <kind of chick>" rows
	const AChasingChicksPlayer& rPlayer = m_oGame.aPlayers[ 0 ];

	for ( TINT i = 0; i < NUM_ROWS; i++ )
	{
		TINT iLocale = ( rPlayer.aNumChicksOfKind[ i ] == 1 ) ? LOCALE_ROW_SINGULAR + i : LOCALE_ROW_PLURAL + i;
		m_aRowNames[ i ].Format( L"%d %s", rPlayer.aNumChicksOfKind[ i ], ALocaleManager::Interface()->GetString( iLocale ) );
	}

	while ( rInfo.vecRows.Size() != 0 )
	{
		rInfo.vecRows[ rInfo.vecRows.Size() - 1 ] = ResultRow();
		rInfo.vecRows.PopBack();
	}

	// NOTE: the first kind goes last
	const AChasingChicksGameData* pGameData = m_oGame.pGameData;
	rInfo.vecRows.PushBack( ResultRow{ m_aRowNames[ 1 ], rPlayer.aNumChicksOfKind[ 1 ] * pGameData->aChickKinds[ 1 ].iValue } );
	rInfo.vecRows.PushBack( ResultRow{ m_aRowNames[ 2 ], rPlayer.aNumChicksOfKind[ 2 ] * pGameData->aChickKinds[ 2 ].iValue } );
	rInfo.vecRows.PushBack( ResultRow{ m_aRowNames[ 3 ], rPlayer.aNumChicksOfKind[ 3 ] * pGameData->aChickKinds[ 3 ].iValue } );
	rInfo.vecRows.PushBack( ResultRow{ m_aRowNames[ 0 ], rPlayer.aNumChicksOfKind[ 0 ] * pGameData->aChickKinds[ 0 ].iValue } );
	m_oResults.ComputeTotal( 0 );

	if ( m_bSubmitHighScore )
	{
		m_bSubmitHighScore = TFALSE;
		TINT iVariant      = AMiniGameManager::GetSingleton()->GetMiniGameVariant();

		if ( iVariant == 0 )
			AHighScoreManager::AddScore( 25, TFLOAT( rInfo.iTotal ), m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
		else if ( iVariant == 1 )
			AHighScoreManager::AddScore( 26, TFLOAT( rInfo.iTotal ), m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
		else if ( iVariant == 2 )
			AHighScoreManager::AddScore( 27, TFLOAT( rInfo.iTotal ), m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
	}

	return &m_oResults;
}

// $Barnyard: FUNCTION 0050e6a0
void AChasingChicksMiniGame::OnStartHudFinished()
{
	// NOTE: retail doesn't call AMiniGameState::OnStartHudFinished
	m_HUDParams.SetFlags( AHudElement::MASK_MINIGAME | AHudElement::MASK_INTERACTION | AHudElement::MASK_UNK8 );
	m_oTimeLeftHudElement.SetForceHide( TFALSE );
	m_oTimeLeftHudElement.GetTimer().SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oStartHudElement.SetForceHide( TTRUE );

	for ( TINT i = 0; i < m_oGame.iNumPlayers; i++ )
	{
		ASimplePlayerControlComponent* pControl = m_oGame.aPlayers[ i ].pControl;
		pControl->SetMovementEnabled( TTRUE );
		pControl->SetLauncherInput( TTRUE );
		pControl->SetCanJump( TTRUE );
		pControl->SetCanSprint( TTRUE );
		pControl->SetSecondaryLauncherInput( TTRUE );
	}

	m_bTimeUp = TFALSE;
}

// $Barnyard: FUNCTION 0050ef10
void AChasingChicksMiniGame::OnUnitAction( AAnimationBreakpointUnitAction::Event* a_pEvent )
{
	for ( TINT i = 0; i < m_oGame.iNumPlayers; i++ )
	{
		AChasingChicksPlayer& rPlayer = m_oGame.aPlayers[ i ];

		TRef<AUnitProxy> pProxy = a_pEvent->pUnitProxy;

		if ( rPlayer.oUnit != pProxy )
			continue;

		AUnitPlayer* pUnit = TSTATICCAST( AUnitPlayer, rPlayer.oUnit->GetUnit() );

		if ( a_pEvent->eAction == AAnimationBreakpointUnitAction::ACTION_PICKUP )
		{
			if ( rPlayer.eState != AChasingChicksPlayer::STATE_PICKUP )
				continue;

			AChasingChicksChick* pChick = rPlayer.pChick;

			if ( pChick && ( pChick->GetState() == AChasingChicksChick::STATE_FREE || pChick->GetState() == AChasingChicksChick::STATE_FLEEING || pChick->GetState() == AChasingChicksChick::STATE_JUMPING ) && pChick->PickUp( i ) )
			{
				// Hold the chick
				pUnit->AttachUnit( pChick, TPS8( cb_rlocator ) );
				pUnit->GetAnimatableObject();

				AAnimController* pController = pUnit->GetAnimatableObject()->FindAnimController( &TGetClass( ACharacterAnimController ) );

				if ( pController )
					pController->OnUpdate( 0.0f );

				rPlayer.eState = AChasingChicksPlayer::STATE_CARRY;
			}
			else
			{
				rPlayer.eState = AChasingChicksPlayer::STATE_FREE;
			}

			rPlayer.pControl->SetMovementEnabled( TTRUE );
		}
		else if ( a_pEvent->eAction == AAnimationBreakpointUnitAction::ACTION_HD_THROW && rPlayer.eState == AChasingChicksPlayer::STATE_THROW )
		{
			if ( rPlayer.uiFlags & AChasingChicksPlayer::FLAGS_THROWING )
			{
				// Throw the chick at the coop ("Goal0" of the scoring network), higher when it's far
				AProjectileLauncher* pLauncher = pUnit->GetLauncher( 1 );
				TFLOAT               fAngle    = rPlayer.pHotSpot->fThrowAngle;

				if ( fAngle <= 0.0f || fAngle >= TMath::HALF_PI )
				{
					TFLOAT fDist = TVector4::DistanceXZ( m_oGame.vCentre, *pUnit->GetSteer()->GetPosition() );

					if ( fDist < 10.0f )
						fAngle = TMath::PI * 0.25f;
					else if ( fDist < 20.0f )
						fAngle = TMath::PI * ( 1.0f / 6.0f );
					else if ( fDist < 30.0f )
						fAngle = TMath::PI * 0.125f;
					else
						fAngle = TMath::PI * ( 1.0f / 12.0f );
				}

				TSTATICCAST( AAimedSquirtProjectileLauncher, pLauncher )->FireAt( m_oGame.vCentre, fAngle, TTRUE );
				rPlayer.pHotSpot = TNULL;
			}

			rPlayer.uiFlags &= ~AChasingChicksPlayer::FLAGS_THROWING;
		}
	}
}

// $Barnyard: FUNCTION 00511660
void AChasingChicksMiniGame::OnPlayerHit( TRef<AUnitProxy> a_pUnit )
{
	for ( TINT i = 0; i < m_oGame.iNumPlayers; i++ )
	{
		if ( m_oGame.aPlayers[ i ].oUnit == a_pUnit && a_pUnit->GetUnit() )
			HitPlayer( TSTATICCAST( AUnitPlayer, a_pUnit->GetUnit() ), i );
	}
}

// $Barnyard: FUNCTION 00510e60
void AChasingChicksMiniGame::HitPlayer( AUnitPlayer* a_pUnit, TINT a_iPlayer )
{
	AChasingChicksPlayer& rPlayer = m_oGame.aPlayers[ a_iPlayer ];

	if ( rPlayer.eState == AChasingChicksPlayer::STATE_STUNNED )
		return;

	if ( a_iPlayer == 0 && rPlayer.eState >= AChasingChicksPlayer::STATE_AIM && rPlayer.eState <= AChasingChicksPlayer::STATE_THROW )
	{
		// Throwing, later
		rPlayer.uiFlags |= AChasingChicksPlayer::FLAGS_HIT;
		return;
	}

	APlayerSteer::STATE eSteerState = TSTATICCAST( APlayerSteer, a_pUnit->GetSteer() )->GetState();

	if ( eSteerState < APlayerSteer::STATE_JUMPING || eSteerState > APlayerSteer::STATE_FALLING )
	{
		TBOOL bStunned = PlayPlayerAnim( &rPlayer, TPS8( yuk ) );

		if ( rPlayer.eState == AChasingChicksPlayer::STATE_STUNNED )
		{
			rPlayer.fStunTime = ( a_iPlayer != 0 ) ? m_oGame.pGameData->fAIStunTime : m_oGame.pGameData->fPlayerStunTime;
			rPlayer.uiFlags &= ~AChasingChicksPlayer::FLAGS_HIT;
		}

		if ( !bStunned )
			return;
	}
	else
	{
		// Jumping, later
		rPlayer.uiFlags |= AChasingChicksPlayer::FLAGS_HIT;
	}

	if ( !rPlayer.pChick )
		return;

	// Drop the chick
	if ( GetOverlayAnim( a_pUnit, TPS8( carry_overlay ) ) )
		a_pUnit->GetAnimatableObject()->GetToshiAnimInterface().StopAnim( TPS8( carry_overlay ) );

	AChasingChicksChick* pChick = rPlayer.pChick;

	if ( a_pUnit->GetAttachedUnit( TPS8( cb_rlocator ), -1 ) != pChick )
		return;

	a_pUnit->DetachUnit( pChick, TTRUE, TTRUE );
	a_pUnit->GetAnimatableObject();

	AAnimController* pController = a_pUnit->GetAnimatableObject()->FindAnimController( &TGetClass( ACharacterAnimController ) );

	if ( pController )
		pController->OnUpdate( 0.0f );

	if ( rPlayer.pChick->GetSteer()->GetClass() != &TGetClass( AThrowItemSteer ) )
		rPlayer.pChick->ChangeSteer( &TGetClass( AThrowItemSteer ) );

	// It falls from the hand of the player
	AThrowItemSteer* pSteer = TSTATICCAST( AThrowItemSteer, rPlayer.pChick->GetSteer() );

	TMatrix44         oBone;
	AModelInstanceRef pModelInstance = a_pUnit->GetAnimatableObject()->GetModelInstance();
	TINT              iBone          = pModelInstance->GetBoneIndex( TPS8( cb_rlocator ) );
	a_pUnit->GetAnimatableObject()->GetModelInstance()->GetBoneTransform( iBone, oBone );

	pSteer->SetPosition( oBone.GetTranslation() );

	TVector4 vDirection;
	a_pUnit->GetSteer()->CopyDirection( vDirection );
	pSteer->SetDirection( vDirection );

	// The velocity reuses the direction vector (w is kept)
	TVector4& vVelocity = vDirection;
	vVelocity.x         = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( -2.0f, 2.0f );
	vVelocity.z         = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( -2.0f, 2.0f );
	vVelocity.y         = -5.0f;
	pSteer->SetVelocity( vVelocity );

	pChick->SetState( AChasingChicksChick::STATE_JUMPING );
	rPlayer.pChick = TNULL;

	if ( rPlayer.eState == AChasingChicksPlayer::STATE_PICKUP || rPlayer.eState == AChasingChicksPlayer::STATE_THROW )
	{
		rPlayer.eState = AChasingChicksPlayer::STATE_FREE;

		if ( a_pUnit->GetActiveLauncher() != a_pUnit->GetLauncher( 0 ) )
			a_pUnit->SelectLauncher( TFALSE );

		rPlayer.pControl->SetMovementEnabled( TTRUE );
		rPlayer.pControl->SetLauncherInput( TTRUE );
		rPlayer.pControl->SetSecondaryLauncherInput( TTRUE );
	}
	else if ( rPlayer.eState == AChasingChicksPlayer::STATE_CARRY )
	{
		rPlayer.eState = AChasingChicksPlayer::STATE_FREE;
	}
}

// $Barnyard: FUNCTION 0050fc50
TBOOL AChasingChicksMiniGame::PlayPlayerAnim( AChasingChicksPlayer* a_pPlayer, const TPString8& a_rcAnim )
{
	AUnitPlayer*       pUnit   = TSTATICCAST( AUnitPlayer, a_pPlayer->oUnit->GetUnit() );
	AAnimatableObject* pObject = pUnit->GetAnimatableObject();
	ASteer*            pSteer  = pUnit->GetSteer();

	if ( a_rcAnim == TPS8( yuk ) )
	{
		if ( !pSteer->GetClass()->IsA( &TGetClass( APlayerSteer ) ) ||
		     ( TSTATICCAST( APlayerSteer, pSteer )->GetState() >= APlayerSteer::STATE_JUMPING && TSTATICCAST( APlayerSteer, pSteer )->GetState() <= APlayerSteer::STATE_FALLING ) ||
		     pObject->IsAnimPlaying( TPS8( yuk ), 0.0f ) ||
		     ( pObject->KillAllAnimations(), pObject->PlayAnimation( TPS8( yuk ) ) ) )
		{
			// Stunned
			if ( pUnit->GetActiveLauncher() != pUnit->GetLauncher( 0 ) )
				pUnit->SelectLauncher( TFALSE );

			a_pPlayer->pControl->SetMovementEnabled( TFALSE );
			a_pPlayer->pControl->SetLauncherInput( TFALSE );
			a_pPlayer->pControl->SetSecondaryLauncherInput( TFALSE );
			a_pPlayer->eState = AChasingChicksPlayer::STATE_STUNNED;
			a_pPlayer->uiFlags &= ~AChasingChicksPlayer::FLAGS_AI;
			pUnit->SetFlags( pUnit->GetFlags() & ~AUnit::FLAGS_MILK_TARGET );
			pObject->SetAnimControllersDisabled( TTRUE );

			if ( pUnit->GetActiveLauncher() && pUnit->GetActiveLauncher() != pUnit->GetPrimaryLauncher() )
				pUnit->SelectLauncher( TFALSE );

			return TTRUE;
		}

		return TFALSE;
	}

	switch ( a_pPlayer->eState )
	{
		case AChasingChicksPlayer::STATE_FREE:
			if ( pObject->IsAnimPlaying( TPS8( throw ), 0.0f ) || pObject->IsAnimPlaying( TPS8( throw2 ), 0.0f ) )
				return TFALSE;

			break;
		case AChasingChicksPlayer::STATE_CARRY:
			if ( pObject->IsAnimPlaying( TPS8( pickup ), 0.0f ) )
				return TFALSE;

			break;
		case AChasingChicksPlayer::STATE_AIM:
			if ( a_rcAnim != TPS8( throw ) && a_rcAnim != TPS8( throw2 ) )
				return TFALSE;

			break;
		default:
			return TFALSE;
	}

	if ( pSteer->GetClass()->IsA( &TGetClass( APlayerSteer ) ) )
	{
		APlayerSteer::STATE eSteerState = TSTATICCAST( APlayerSteer, pSteer )->GetState();

		if ( eSteerState >= APlayerSteer::STATE_JUMPING && eSteerState <= APlayerSteer::STATE_FALLING )
			return TFALSE;
	}

	if ( !pObject->PlayAnimation( a_rcAnim ) )
		return TFALSE;

	if ( a_rcAnim == TPS8( pickup ) )
	{
		a_pPlayer->pControl->SetMovementEnabled( TFALSE );
		a_pPlayer->eState = AChasingChicksPlayer::STATE_PICKUP;
	}
	else if ( a_rcAnim == TPS8( throw ) || a_rcAnim == TPS8( throw2 ) )
	{
		a_pPlayer->pControl->SetMovementEnabled( TFALSE );
		a_pPlayer->eState = AChasingChicksPlayer::STATE_THROW;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0050f120
void AChasingChicksMiniGame::UpdateEnd()
{
	if ( m_oGame.ePhase == PHASE_ENDING )
	{
		// Wait for the chicks in the air...
		for ( TINT i = 0; i < m_oGame.iNumChicks; i++ )
		{
			AChasingChicksChick* pChick = TSTATICCAST( AChasingChicksChick, m_oGame.aFreeChicks[ i ]->GetUnit() );

			if ( pChick->GetState() == AChasingChicksChick::STATE_THROWN || pChick->GetState() == AChasingChicksChick::STATE_POCKETED )
				return;
		}

		// ... and the throws
		for ( TINT i = 0; i < m_oGame.iNumPlayers; i++ )
		{
			if ( m_oGame.aPlayers[ i ].eState == AChasingChicksPlayer::STATE_THROW )
				return;
		}

		GetResults();

		// The rivals cheer or sulk
		for ( TINT i = 1; i < m_oGame.iNumPlayers; i++ )
		{
			AAnimatedUnit* pUnit = TSTATICCAST( AAnimatedUnit, m_oGame.aPlayers[ i ].oUnit->GetUnit() );
			pUnit->GetAnimatableObject()->KillAllAnimations();
			pUnit->GetAnimatableObject()->PlayAnimation( ( i == m_oResults.iResult ) ? TPS8( winner_start ) : TPS8( loser_start ) );
		}

		m_fEndTimer    = 5.0f;
		m_oGame.ePhase = PHASE_ENDED;
	}
	else if ( m_oGame.ePhase != PHASE_ENDED )
	{
		// The time is up
		m_oInteractionHud.SetForceHide( TTRUE );

		const AChasingChicksGameData* pGameData = m_oGame.pGameData;
		TINT                          iNumOpen  = pGameData->aChickKinds[ 0 ].iNumOpen + pGameData->aChickKinds[ 1 ].iNumOpen + pGameData->aChickKinds[ 2 ].iNumOpen + pGameData->aChickKinds[ 3 ].iNumOpen;

		if ( iNumOpen > m_oGame.pScoringNetwork->m_pINetwork->m_iNumNormalAreas )
			iNumOpen = m_oGame.pScoringNetwork->m_pINetwork->m_iNumNormalAreas;

		// Close all the pens
		for ( TINT i = 0; i < AChasingChicksGame::MAX_PLAYERS; i++ )
		{
			for ( auto it = m_oGame.aOpenPens[ i ].Begin(); it != m_oGame.aOpenPens[ i ].End(); )
			{
				auto itNext = it.Next();
				it->SetClosed( TTRUE );
				it = itNext;
			}
		}

		for ( TINT i = 0; i < iNumOpen; i++ )
			m_oMap.SetIconVisible( m_oGame.pMapIcons[ i ], TFALSE );

		// Nobody moves anymore
		for ( TINT i = 0; i < m_oGame.iNumPlayers; i++ )
		{
			AChasingChicksPlayer& rPlayer = m_oGame.aPlayers[ i ];
			AUnitPlayer*          pUnit   = TSTATICCAST( AUnitPlayer, rPlayer.oUnit->GetUnit() );

			pUnit->GetActiveLauncher()->Deactivate();
			rPlayer.pControl->SetLauncherInput( TFALSE );

			if ( pUnit->GetActiveLauncher() != pUnit->GetLauncher( 0 ) )
				pUnit->SelectLauncher( TFALSE );

			rPlayer.pControl->SetSecondaryLauncherInput( TFALSE );
			rPlayer.pControl->SetCanSprint( TFALSE );
			rPlayer.pControl->SetMovementEnabled( TFALSE );
			rPlayer.pControl->SetCameraInput( TFALSE );
			pUnit->GetSteer()->SetTilt( 0.0f, 0.0f, TFALSE );
		}

		m_oGame.ePhase = PHASE_ENDING;
	}
}

// $Barnyard: FUNCTION 0050e5a0
void AChasingChicksMiniGame::ShowInteraction( const TVector4& a_rcPosition, const TWCHAR* a_wszText )
{
	m_oInteractionHud.SetText( a_wszText, 2, TINT( m_pInputMap ), AInputCommand_Interact, 0, -1, -1.0f );
	m_oInteractionHud.SetForceHide( TFALSE );
}

// $Barnyard: FUNCTION 0050e970
void AChasingChicksMiniGame::LoadData()
{
	TTRB& rData = m_oGame.oData;
	rData.Load( "Data/MinigameData/ChasingChicks/Data.trb", 0 );

	m_oGame.pGameData = rData.CastSymbol<AChasingChicksGameData>( "GameData" );
	TINT iNumVariants = *rData.CastSymbol<TINT>( "NumGameVariants" );

	m_oGame.pGameData += AMiniGameManager::GetSingleton()->GetMiniGameVariant() % iNumVariants;
	m_oGame.pAIData      = m_oGame.pGameData->pAIData;
	m_oGame.pHotSpotData = rData.CastSymbol<AChasingChicksHotSpotData>( "HotSpotData" );
}

// $Barnyard: FUNCTION 005100b0
void AChasingChicksMiniGame::CreatePlayers()
{
	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();

	if ( pPlayerManager->GetCurrentPlayerSet() != pPlayerManager->GetPlayerSet( 1 ) )
		pPlayerManager->SetCurrentPlayerSet( 1 );

	TInputDeviceController* pController = AInputHandler::GetSingleton()->GetMainController();
	pPlayerManager->GetPlayerSet( 1 )->Reset();
	pPlayerManager->GetPlayerSet( 1 );
	pPlayerManager->GetCurrentPlayerSet()->AddHumanPlayer( pController, -1 );

	APlayerProfileManager* pProfileManager = APlayerProfileManager::GetSingleton();
	APlayerSimProfile*     pProfile        = pProfileManager->GetMainProfile();
	pProfile->SetStaminaWasteSpeed( 0.0f );

	AAnimatableObjectManager* pObjectManager = AAnimatableObjectManager::GetSingleton();
	AAnimatableObjectType*    pSunglasses    = pObjectManager->FindType( TPS8( sunglasses1 ) );

	// The player
	if ( !m_pPlayer )
	{
		TINT iGender = pProfileManager->GetMainGender();

		if ( iGender == APlayerProfile::GENDER_NONE )
			iGender = APlayerProfile::GENDER_MALE;

		AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_POSZ, 0, iGender, m_oGame.iNumPlayers, TTRUE );
		m_pPlayer = TSTATICCAST( AUnitPlayer, AUnitManager::GetSingleton()->SpawnUnit( TPS8( player_chicks ), &oTransform ) );
		m_pPlayer->ApplyMaterial( pProfileManager->GetMainCostume(), TTRUE );

		m_aPortraits[ 0 ].Create( pProfile->GetHideTextureName( -1, APlayerProfile::GENDER_NONE ), AGUI2Portrait::FLAGS_LEFT | AGUI2Portrait::FLAGS_UNK5, &m_GUIElement );
		m_aPortraits[ 0 ].SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
		m_GUIElement.AddChildTail( m_aPortraits[ 0 ] );

		ASteer* pSteer = m_pPlayer->GetSteer();
		m_oMap.AddIcon( *pSteer->GetPosition(), *pSteer->GetDirection(), AGUI2MiniMap::ICONTYPE_PLAYER, AGUI2MiniMap::ICON_PLAYER, 0.13f );
		m_oMap.SetCentre( *m_pPlayer->GetSteer()->GetPosition(), *m_pPlayer->GetSteer()->GetDirection() );

		// The player wears sunglasses
		if ( pSunglasses )
		{
			AAnimatableObject* pObject        = pObjectManager->CreateAnimatableObject( pSunglasses, m_pPlayer, 0 );
			m_oGame.aPlayers[ 0 ].pSunglasses = pObject;

			if ( pObject )
			{
				pObject->SetSkeletonUpdating( TTRUE, TTRUE );
				TINT iBone = -1;

				if ( iGender == APlayerProfile::GENDER_FEMALE )
					iBone = pObject->GetModelInstance()->GetSkeleton()->GetBoneID( "americanf_loc", 0 );

				TINT iHeadBone = m_pPlayer->GetModelInstance()->GetSkeleton()->GetBoneID( "c_head", 0 );
				pObjectManager->AttachObject( pObject, m_pPlayer->GetAnimatableObject(), iBone, iHeadBone, TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
			}
		}
	}

	m_oGame.aPlayers[ 0 ].oUnit = m_pPlayer->GetSelfProxy();
	m_oGame.iNumPlayers++;

	TINT iVariant = AMiniGameManager::GetSingleton()->GetMiniGameVariant();

	if ( iVariant == 0 )
	{
		// One rival: Otis
		AUnitTransform    oTransform( TVector4::VEC_ZERO, TVector4::VEC_POSZ, 0, 5, m_oGame.iNumPlayers, TTRUE );
		AChasingChicksAI* pAI = TSTATICCAST( AChasingChicksAI, AUnitManager::GetSingleton()->SpawnUnit( TPS8( chasingchicks_ai ), &oTransform ) );
		pAI->SetGame( &m_oGame, &m_oGame.aPlayers[ m_oGame.iNumPlayers ] );
		pAI->SetNetwork( m_pNetwork );
		pAI->SetFlags( pAI->GetFlags() | AUnit::FLAGS_MILK_TARGET );
		m_oGame.aPlayers[ 1 ].uiFlags |= AChasingChicksPlayer::FLAGS_AI;
		m_oGame.aPlayers[ m_oGame.iNumPlayers++ ].oUnit = pAI->GetSelfProxy();

		pPlayerManager->GetCurrentPlayerSet()->AddAIPlayer();
		pPlayerManager->GetCurrentPlayerSet()->FinishUpAddingAIs();
		pPlayerManager->GetCurrentPlayerSet()->SetPlayerUnit( pAI, 1 );

		m_aPortraits[ 1 ].Create( "head_otis", AGUI2Portrait::FLAGS_RIGHT | AGUI2Portrait::FLAGS_UNK5, &m_GUIElement );
		m_aPortraits[ 1 ].SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );
		m_GUIElement.AddChildTail( m_aPortraits[ 1 ] );

		if ( pSunglasses )
		{
			AAnimatableObject* pObject        = pObjectManager->CreateAnimatableObject( pSunglasses, pAI, 0 );
			m_oGame.aPlayers[ 1 ].pSunglasses = pObject;

			if ( pObject )
			{
				pObject->SetSkeletonUpdating( TTRUE, TTRUE );
				TINT iBone     = pObject->GetModelInstance()->GetSkeleton()->GetBoneID( "otis_loc", 0 );
				TINT iHeadBone = pAI->GetModelInstance()->GetSkeleton()->GetBoneID( "c_head", 0 );
				pObjectManager->AttachObject( pObject, pAI->GetAnimatableObject(), iBone, iHeadBone, TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
			}
		}
	}
	else if ( iVariant == 1 )
	{
		// Three rivals: Bud, Eddy and Igg
		for ( TINT i = 0; i < 3; i++ )
			pPlayerManager->GetCurrentPlayerSet()->AddAIPlayer();

		pPlayerManager->GetCurrentPlayerSet()->FinishUpAddingAIs();

		for ( TINT i = 0; i < 3; i++ )
		{
			AUnitTransform    oTransform( TVector4::VEC_ZERO, TVector4::VEC_POSZ, 0, 2 + i, m_oGame.iNumPlayers, TTRUE );
			AChasingChicksAI* pAI = TSTATICCAST( AChasingChicksAI, AUnitManager::GetSingleton()->SpawnUnit( TPS8( chasingchicks_ai ), &oTransform ) );
			pAI->SetGame( &m_oGame, &m_oGame.aPlayers[ m_oGame.iNumPlayers ] );
			pAI->SetNetwork( m_pNetwork );
			pAI->SetFlags( pAI->GetFlags() | AUnit::FLAGS_MILK_TARGET );
			m_oGame.aPlayers[ 1 + i ].uiFlags |= AChasingChicksPlayer::FLAGS_AI;
			m_oGame.aPlayers[ m_oGame.iNumPlayers++ ].oUnit = pAI->GetSelfProxy();

			pPlayerManager->GetCurrentPlayerSet()->SetPlayerUnit( pAI, i + 1 );
		}

		if ( pSunglasses )
		{
			static const TCHAR* s_aszBones[ 3 ] = { "bud_loc", "eddie_loc", "igg_loc" };

			for ( TINT i = 0; i < 3; i++ )
			{
				AAnimatedUnit*     pAI                = TSTATICCAST( AAnimatedUnit, m_oGame.aPlayers[ 1 + i ].oUnit->GetUnit() );
				AAnimatableObject* pObject            = pObjectManager->CreateAnimatableObject( pSunglasses, pAI, 0 );
				m_oGame.aPlayers[ 1 + i ].pSunglasses = pObject;

				if ( pObject )
				{
					pObject->SetSkeletonUpdating( TTRUE, TTRUE );
					TINT iBone     = pObject->GetModelInstance()->GetSkeleton()->GetBoneID( s_aszBones[ i ], 0 );
					TINT iHeadBone = pAI->GetModelInstance()->GetSkeleton()->GetBoneID( "c_head", 0 );
					pObjectManager->AttachObject( pObject, pAI->GetAnimatableObject(), iBone, iHeadBone, TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
				}
			}
		}

		m_aPortraits[ 1 ].Create( "head_bud", AGUI2Portrait::FLAGS_RIGHT | AGUI2Portrait::FLAGS_UNK5, &m_GUIElement );
		m_GUIElement.AddChildTail( m_aPortraits[ 1 ] );
		m_aPortraits[ 1 ].SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );

		m_aPortraits[ 2 ].Create( "head_eddy", AGUI2Portrait::FLAGS_LEFT | AGUI2Portrait::FLAGS_UNK5, &m_GUIElement );
		m_aPortraits[ 2 ].SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_BOTTOMLEFT );
		m_GUIElement.AddChildTail( m_aPortraits[ 2 ] );

		// The minimap goes above Eddy
		m_oMap.RemoveSelf();
		m_oMap.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_BOTTOMLEFT );
		m_aPortraits[ 2 ].AddChildTail( m_oMap );

		m_aPortraits[ 3 ].Create( "head_igg", AGUI2Portrait::FLAGS_RIGHT | AGUI2Portrait::FLAGS_UNK5, &m_GUIElement );
		m_aPortraits[ 3 ].SetAttachment( T2GUIATTACHMENT_BOTTOMRIGHT, T2GUIATTACHMENT_BOTTOMRIGHT );
		m_GUIElement.AddChildTail( m_aPortraits[ 3 ] );
	}

	// Set up the players
	for ( TINT i = 0; i < m_oGame.iNumPlayers; i++ )
	{
		AUnitPlayer* pUnit = TSTATICCAST( AUnitPlayer, m_oGame.aPlayers[ i ].oUnit->GetUnit() );

		// Milk, and the chicks they throw
		AProjectileLauncher* pLauncher = pUnit->GetActiveLauncher();
		pLauncher->GetAmmo()->SetProjectileType( ASimpleProjectileManager::TYPE_CHASING_CHICKS_MILK );
		pUnit->GetActiveLauncher()->SetThrowAmmo( &m_oAmmo );
		pUnit->GetActiveLauncher()->GetThrowAmmo()->SetProjectileType( ASimpleProjectileManager::TYPE_CHASING_CHICKS_THROW );

		pUnit->SetNoQuery( TFALSE );
		pUnit->SetHashed( TTRUE );
		pUnit->SetVisible( TFALSE );
		pUnit->GetAnimatableObject()->GetModelInstance()->SetShadowMode( AModelInstance::SHADOWMODE_DYNAMIC );

		AProjectileLauncher* pThrowLauncher = pUnit->GetLauncher( 1 );
		pThrowLauncher->SetThrowAmmo( &m_oAmmo );
		pThrowLauncher->GetThrowAmmo()->SetProjectileType( ASimpleProjectileManager::TYPE_CHASING_CHICKS_THROW );

		TSTATICCAST( APlayerSteer, pUnit->GetSteer() )->PlaceAtNamedGoal( *s_apPlayerStarts[ i ], TTRUE, TTRUE );

		ASimplePlayerControlComponent* pControl = TSTATICCAST( ASimplePlayerControlComponent, pUnit->FindComponent( &TGetClass( AControlComponent ) ) );
		m_oGame.aPlayers[ i ].pControl          = pControl;
		pControl->SetLauncherInput( TFALSE );
		pControl->SetCanJump( TFALSE );
		pControl->SetCanSprint( TFALSE );
		pControl->SetUnk32( TFALSE );
		pControl->SetMovementEnabled( TFALSE );
		pControl->SetSecondaryLauncherInput( TFALSE );

		m_aPortraits[ i ].SetScore( 0 );
	}

	m_oReticles.Create( m_pPlayer, 50.0f, &m_GUIElement );
}

// $Barnyard: FUNCTION 0050f960
void AChasingChicksMiniGame::DestroyPlayers()
{
	for ( TINT i = 0; i < m_oGame.iNumPlayers; i++ )
	{
		AChasingChicksPlayer& rPlayer = m_oGame.aPlayers[ i ];

		if ( !rPlayer.oUnit )
			continue;

		if ( rPlayer.pSunglasses )
		{
			AAnimatableObjectManager::GetSingleton()->DetachObject( rPlayer.pSunglasses, TSTATICCAST( AAnimatedUnit, rPlayer.oUnit->GetUnit() )->GetAnimatableObject() );
			AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( rPlayer.pSunglasses );
			rPlayer.pSunglasses = TNULL;
		}

		AUnitManager::GetSingleton()->DestroyUnit( rPlayer.oUnit->GetUnit() );
		rPlayer.oUnit = TNULL;
	}

	m_pPlayer = TNULL;
	APlayerProfileManager::GetSingleton()->GetMainProfile()->SetStaminaWasteSpeed( 0.8f );
}

// $Barnyard: FUNCTION 00510af0
void AChasingChicksMiniGame::CreateHanna()
{
	AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_POSX, 0, 0, -1, TTRUE );
	m_oGame.pHanna = TSTATICCAST( AChasingChicksHanna, AUnitManager::GetSingleton()->SpawnUnit( TPS8( chasingchicks_hanna ), &oTransform ) );
	m_oGame.pHanna->ChangeSteer( &TGetClass( AInteriorSteer ) );

	AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, m_oGame.pHanna->GetSteer() );
	pSteer->SetNetwork( m_pNetwork, 0 );
	pSteer->Command_SnapToGoal( TPS8( hanna ) );
	pSteer->Command_CancelCurrentCommand();
}

// $Barnyard: FUNCTION 00510b90
void AChasingChicksMiniGame::CreateChicks()
{
	m_oGame.iNumChicks = 0;

	for ( TINT i = 0; i < AChasingChicksGame::NUM_CHICKS; i++ )
	{
		AUnitTransform       oTransform( TVector4::VEC_ZERO, TVector4::VEC_POSX, 0, 0, -1, TTRUE );
		AChasingChicksChick* pChick = TSTATICCAST( AChasingChicksChick, AUnitManager::GetSingleton()->SpawnUnit( TPS8( chasingchicks_chick ), &oTransform ) );
		pChick->ChangeSteer( &TGetClass( AInteriorSteer ) );

		AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, pChick->GetSteer() );
		pSteer->SetFlags( pSteer->GetFlags() | AInteriorSteer::FLAGS_KINEMATICS_NO_AVOID );
		pSteer->EnableKinematicsObstructionStop( TFALSE );
		pSteer->SetNetwork( m_pNetwork, 0 );
		pSteer->Command_SnapToGoal( TPS8( spawn ) );
		pSteer->Command_CancelCurrentCommand();

		pChick->m_pNetwork = m_pNetwork;
		pChick->m_pGame    = &m_oGame;
		pChick->SetMovement( TTRUE );
		pChick->SetState( AChasingChicksChick::STATE_FREE );
		pChick->GetAnimatableObject()->GetModelInstance()->SetShadowMode( AModelInstance::SHADOWMODE_DYNAMIC );

		m_oGame.PushChick( pChick->GetSelfProxy() );
		m_oGame.iNumChicks++;
	}

	// Where the chicks respawn: the "spawn" goals
	AInteriorNetwork::INetwork* pINetwork = m_pNetwork->m_pINetwork;
	m_oGame.iNumSpawns                    = 0;

	for ( TINT i = 0; i < pINetwork->m_iNumGoals; i++ )
		m_oGame.iNumSpawns += ( *pINetwork->m_pGoals[ i ].m_strName == TPS8( spawn ) );

	m_oGame.pSpawns = TSTATICCAST( TVector4, TMalloc( sizeof( TVector4 ) * m_oGame.iNumSpawns ) );

	for ( TINT i = 0, iSpawn = 0; i < m_pNetwork->m_pINetwork->m_iNumGoals; i++ )
	{
		AInteriorGoal& rGoal = m_pNetwork->m_pINetwork->m_pGoals[ i ];

		if ( *rGoal.m_strName == TPS8( spawn ) )
		{
			rGoal.GetPosition( m_oGame.pSpawns[ iSpawn ] );
			m_oGame.pSpawns[ iSpawn ].y -= 0.5f;
			iSpawn++;
		}
	}
}

// $Barnyard: FUNCTION 0050fa20
void AChasingChicksMiniGame::DestroyChicks()
{
	while ( m_oGame.iNumFreeChicks != 0 )
	{
		TRef<AUnitProxy>&    rProxy = m_oGame.aFreeChicks[ m_oGame.iNumFreeChicks - 1 ];
		AChasingChicksChick* pChick = rProxy ? TSTATICCAST( AChasingChicksChick, rProxy->GetUnit() ) : TNULL;
		m_oGame.PopChick();

		if ( pChick )
		{
			pChick->KillTrail();
			AUnitManager::GetSingleton()->DestroyUnit( pChick );
		}
	}

	if ( m_oGame.pSpawns )
	{
		TFree( m_oGame.pSpawns );
		m_oGame.pSpawns = TNULL;
	}
}

// $Barnyard: FUNCTION 0050fa90
void AChasingChicksMiniGame::CreateHotSpots()
{
	TINT iNumHotSpots = m_oGame.pScoringNetwork->m_pINetwork->m_iNumNormalAreas;
	m_oGame.pHotSpots = new AChasingChicksHotSpot[ iNumHotSpots ];

	for ( TINT i = 0; i < iNumHotSpots; i++ )
		m_oGame.pHotSpots[ i ].Create( &m_oGame, i );

	// An icon on the minimap per open pen
	const AChasingChicksGameData* pGameData = m_oGame.pGameData;
	TINT                          iNumOpen  = pGameData->aChickKinds[ 0 ].iNumOpen + pGameData->aChickKinds[ 1 ].iNumOpen + pGameData->aChickKinds[ 2 ].iNumOpen + pGameData->aChickKinds[ 3 ].iNumOpen;

	if ( iNumOpen > iNumHotSpots )
		iNumOpen = iNumHotSpots;

	m_oGame.pMapIcons = TSTATICCAST( TINT, TMalloc( sizeof( TINT ) * iNumOpen ) );

	for ( TINT i = 0; i < iNumOpen; i++ )
		m_oGame.pMapIcons[ i ] = m_oMap.AddIcon( TVector4::VEC_ZERO, TVector4::VEC_POSX, AGUI2MiniMap::ICONTYPE_EDGE, AGUI2MiniMap::ICON_PING_RING, 0.1f );

	// Open random pens of every kind
	for ( TINT i = 0; i < AChasingChicksGame::MAX_PLAYERS; i++ )
	{
		for ( TINT iNumOpenNow = m_oGame.aOpenPens[ i ].Size(); !m_oGame.aClosedPens[ i ].IsEmpty(); iNumOpenNow++ )
		{
			if ( iNumOpenNow >= pGameData->aChickKinds[ i ].iNumOpen )
				break;

			auto itPen = m_oGame.aClosedPens[ i ].Begin();

			for ( TINT iRand = ARandom::GetSingleton()->m_oRandom.GetInt( m_oGame.aClosedPens[ i ].Size() ); iRand > 0; iRand-- )
				itPen++;

			itPen->SetClosed( TFALSE );
		}
	}
}

// $Barnyard: FUNCTION 0050e900
void AChasingChicksMiniGame::DestroyHotSpots()
{
	for ( TINT i = 0; i < AChasingChicksGame::MAX_PLAYERS; i++ )
	{
		m_oGame.aOpenPens[ i ].Clear();
		m_oGame.aClosedPens[ i ].Clear();
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

// $Barnyard: FUNCTION 0050f910
void AChasingChicksMiniGame::SetupCamera()
{
	ACameraManager* pCameraManager = ACameraManager::GetSingleton();
	pCameraManager->SetSplitscreenCameraHelper( 1 );

	AGTAStyleCameraHelper* pHelper = TSTATICCAST( AGTAStyleCameraHelper, pCameraManager->GetSplitscreenCameraHelper( 1 ) );
	pHelper->SetTarget( m_oGame.aPlayers[ 0 ].oUnit );
	pHelper->ResetPosition( TTRUE );
}

// $Barnyard: FUNCTION 0050e870
void AChasingChicksMiniGame::ResetCamera()
{
	ACameraManager* pCameraManager = ACameraManager::GetSingleton();
	pCameraManager->ResetHelperTargets();
	pCameraManager->DetachSplitscreenCameraHelpers();
	pCameraManager->DetachCameraHelpers();

	// Back to the defaults
	AGTAStyleCameraHelper* pHelper = TSTATICCAST( AGTAStyleCameraHelper, pCameraManager->GetSplitscreenCameraHelper( 1 ) );
	pHelper->SetDefaultPitch( 11.0f );
	pHelper->SetTargetPitch( 11.0f );
	pHelper->SetDefaultDistance( 5.0f );
	pHelper->SetMaxPitch( 22.0f );
	pHelper->SetDefaultMaxPitch( 22.0f );
	pHelper->SetCollisionRadius( 0.43f );
	pHelper->SetSmallCollisionRadius( 0.2f );
	pHelper->SetHeightOffset( 1.7f );
}

#ifdef BARNYARD_COMMUNITY_PATCH

//-----------------------------------------------------------------------------
// -devautoplay: the player walks to the nearest free chick, picks it up, walks
// into the nearest open pen and throws it in.
//-----------------------------------------------------------------------------
void AChasingChicksMiniGame::DevAutoPlay( TFLOAT a_fDeltaTime )
{
	if ( !T2String8::FindString( GetCommandLineA(), "-devautoplay" ) )
		return;

	static TFLOAT s_fTimer = 0.0f;
	s_fTimer += a_fDeltaTime;

	if ( s_fTimer < 0.75f )
		return;

	s_fTimer = 0.0f;

	AChasingChicksPlayer& rPlayer = m_oGame.aPlayers[ 0 ];
	APlayerSteer*         pSteer  = TSTATICCAST( APlayerSteer, m_pPlayer->GetSteer() );

	if ( rPlayer.eState == AChasingChicksPlayer::STATE_FREE )
	{
		// Next to the nearest free chick (UpdateGame picks it up)
		AChasingChicksChick* pTarget   = TNULL;
		TFLOAT               fDistance = 0.0f;

		for ( TINT i = 0; i < m_oGame.iNumChicks; i++ )
		{
			AChasingChicksChick* pChick = TSTATICCAST( AChasingChicksChick, m_oGame.aFreeChicks[ i ]->GetUnit() );

			if ( pChick->GetState() != AChasingChicksChick::STATE_FREE && pChick->GetState() != AChasingChicksChick::STATE_FLEEING )
				continue;

			TFLOAT fChickDistance = TVector4::DistanceSq( *pChick->GetSteer()->GetPosition(), *pSteer->GetPosition() );

			if ( !pTarget || fChickDistance < fDistance )
			{
				pTarget   = pChick;
				fDistance = fChickDistance;
			}
		}

		if ( pTarget )
		{
			TVector4 vPosition = *pTarget->GetSteer()->GetPosition();
			vPosition.x += 0.5f;
			pSteer->SetPosition( vPosition );
			TINFO( "[ccauto] go to a chick (%d)\n", pTarget->GetState() );
		}
	}
	else if ( rPlayer.eState == AChasingChicksPlayer::STATE_CARRY )
	{
		if ( rPlayer.pHotSpot && rPlayer.pHotSpot->eState == AChasingChicksHotSpot::STATE_INSIDE )
		{
			// In an open pen: throw
			TINFO( "[ccauto] throw (kind %d, score %d)\n", rPlayer.pHotSpot->iKind, rPlayer.iScore );
			rPlayer.uiFlags |= AChasingChicksPlayer::FLAGS_INTERACT;
			return;
		}

		// Into the nearest open pen
		AChasingChicksHotSpot* pPen = m_oGame.FindClosestPen( *pSteer->GetPosition(), 1000.0f );

		if ( pPen )
		{
			pSteer->SetPosition( pPen->vPosition );
			TINFO( "[ccauto] go to a pen (kind %d)\n", pPen->iKind );
		}
	}
}

#endif
