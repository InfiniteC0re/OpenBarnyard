#include "pch.h"
#include <Render/TCameraObject.h>
#include "ADartMiniGame.h"
#include "AMiniGameManager.h"
#include "ARandom.h"
#include "Animation/ADartThrowAnimController.h"
#include "Animation/AAnimatableObject.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "FX/AGameTimeFX.h"
#include "FX/AGameTimeFXManager.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AAnticGameOverState.h"
#include "GameInterface/ASSeqState.h"
#include "GUI/AFadeManager.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AHighScoreManager.h"
#include "Helpers/AWeatherManager.h"
#include "HUD/AHudManager.h"
#include "Input/AInputMapManager.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Render/ARenderer.h"
#include "Render/ASunFlareEffect.h"
#include "Sound/ASoundManager.h"
#include "Tasks/ARootTask.h"
#include "ALoadScreen.h"
#include "Units/AAnimatedUnit.h"
#include "Units/AInteractionComponent.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitTransform.h"
#include "UnitMovement/Steering/AStationarySteer.h"

#include <GUI/T2GUIFontManager.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>
#include <Toshi/TStringManager.h>
#include <Math/TMathInline.h>
#include "SoundBank/darts.h"
#include "SoundBank/music.h"
#include "SoundBank/ui.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

// Opcode defines PI
#undef PI

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00522020, CreateObject 005258a0, CreateObjectInPlace 00525900 (the constructor is inlined in both)
TDEFINE_CLASS( ADartMiniGame );

TPSTRING8_DECLARE( ADartMiniGame );
TPSTRING8_DECLARE( dart );
TPSTRING8_DECLARE( dart_loc );
TPSTRING8_DECLARE( player_darts );

// Colours of the players (yellow and blue)
static constexpr TFLOAT YELLOW_R = 1.0f;
static constexpr TFLOAT YELLOW_G = 0.3019608f;
static constexpr TFLOAT YELLOW_B = 0.027450981f;
static constexpr TFLOAT BLUE_R   = 0.39215687f;
static constexpr TFLOAT BLUE_G   = 0.3019608f;
static constexpr TFLOAT BLUE_B   = 0.94901961f;

// Clock fade of the inactive player and how fast the time bonus fades (00773e44..00773e50)
static constexpr TFLOAT INACTIVE_ALPHA   = 0.5f;
static constexpr TFLOAT FADE_DURATION    = 0.5f;
static constexpr TFLOAT BONUS_FADE_SPEED = 0.3f;

// The AI considers a target hit if it is within this of the aim (00773e5c)
static constexpr TFLOAT AI_TOLERANCE = 0.1f;

// The camera (00773e54, 00773e58); the position is 0xc1a00000 = -20.0f (00783844, 00783858, 00783868)
static constexpr TFLOAT CAMERA_FOV      = 45.0f;
static TBOOL            s_bSetupCamera  = TTRUE;
static const TVector4   CAMERA_POSITION = TVector4( -20.0f, 0.0f, 0.0f, 1.0f );

static TUINT32 MakeColour( const TFLOAT* a_pColour )
{
	return ( ( ( TUINT32( a_pColour[ 0 ] * 255.0f ) | 0xFFFFFF00 ) << 8 | TUINT32( a_pColour[ 1 ] * 255.0f ) ) << 8 ) | TUINT32( a_pColour[ 2 ] * 255.0f );
}

// Creates the portrait of the main player with the hide of their profile
// $Barnyard: FUNCTION 00648080
static void CreatePortraitFromProfile( AGUI2Portrait* a_pPortrait, AGUI2Portrait::FLAGS a_eFlags, T2GUIElement* a_pParent )
{
	a_pPortrait->Create( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetHideTextureName( -1, APlayerProfile::GENDER_NONE ), a_eFlags, a_pParent );
}

// Shows the name of the main player on the portrait
// $Barnyard: FUNCTION 00647300
static void SetPortraitNameFromProfile( AGUI2Portrait* a_pPortrait )
{
	a_pPortrait->SetName( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetName().GetString( 0 ) );
}

// $Barnyard: FUNCTION 00525540
ADartMiniGame::ADartMiniGame()
    : m_fFlightTime( 0.0f )
    , m_fRaiseTimer( 0.0f )
    , m_fResultTimer( 0.0f )
    , m_fAimTimerY( 0.0f )
    , m_fAimTimerX( 0.0f )
    , m_bFireX( TFALSE )
    , m_bFireY( TFALSE )
    , m_bGameOver( TFALSE )
    , m_bGameOverPushed( TFALSE )
    , m_bRoundFlag( TFALSE )
    , m_bFading( TFALSE )
    , m_bFadeIn( TFALSE )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: not initialised by the original
	m_bCogsSpin = TFALSE;
#endif

	m_pInputMap = AInputMapManager::GetSingleton()->FindMap( TPS8( ADartMiniGame ) );
}

// $Barnyard: FUNCTION 00525940
// $Barnyard: FUNCTION 00525740
// $Barnyard: FUNCTION 00525730
ADartMiniGame::~ADartMiniGame()
{
}

// $Barnyard: FUNCTION 00522670
TBOOL ADartMiniGame::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	TINT iSlot = APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetPlayerSlotIndexByDevice( a_pInputEvent->GetSource() );

	if ( iSlot != m_iCurrentPlayer || m_bGameOver || !m_oStartHudElement.IsForceHidden() || a_eInputCommand != AInputCommand_Interact )
		return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	ADart::STATE eState = GetCurrentDart().m_eState;

	if ( eState == ADart::STATE_AIM_X )
	{
		if ( m_fAimTimerX > 0.1f )
			m_bFireX = TTRUE;
	}
	else
	{
		if ( eState != ADart::STATE_AIM_Y )
			return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

		if ( m_fAimTimerY > 0.1f )
		{
			m_bFireY = TTRUE;
			return TTRUE;
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00522030
ASoundId ADartMiniGame::GetSound()
{
	return soundbank::MUSIC_POOL;
}

// $Barnyard: FUNCTION 00522f10
void ADartMiniGame::OnPause()
{
	ARootTask::GetSingleton()->SetRenderWorld( TFALSE );
	g_oLoadScreen.StartLoading( 100, TTRUE );
	AGameLoader::Terrain_Set( AGameLoader::Terrain_EnvDartsLarge, TFALSE, TTRUE, 0, 0, 0, 0 );
	g_oLoadScreen.Reset();
	g_oLoadScreen.SetLoadingState( TFALSE, TTRUE );

	SetupCamera();
	SetVisible( TTRUE );

	AGameStateController::GetSingleton()->SetFlags( AGameStateController::FLAGS_FADING_IN );
}

// $Barnyard: FUNCTION 00524e40
void ADartMiniGame::OnInsertion()
{
	m_bShowPlayerFloats = TFALSE;

	ASkyDome* pSkyDome = ARenderer::GetSingleton()->GetSkyDome();
	pSkyDome->SetFlags( pSkyDome->GetFlags() & ~ASkyDome::FLAGS_RENDER );
	pSkyDome->SetFlags( ( pSkyDome->GetFlags() & ~( ASkyDome::FLAGS_FROZEN | ASkyDome::FLAGS_ROTATE ) ) | ASkyDome::FLAGS_FROZEN );

	ASunFlareEffect::GetSingleton()->SetEnabled( TFALSE );

	AGameLoader::Terrain_Set( AGameLoader::Terrain_EnvDartsLarge, TFALSE, TTRUE, 0, 0, 0, 0 );

	ACameraManager::GetSingleton()->DetachCameraHelpers();
	ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().SetTranslation( TVector4::VEC_POSZ );

	AUnitTransform oDartTransform( TVector4::VEC_ZERO, TVector4::VEC_NEGZ, 0, -1, -1, TTRUE );

	m_oBoard.Create();

	m_eSavedDayPhase = AGameTimeManager::GetSingleton()->GetCurrentDayPhase();

	AGameTimeFX oFX( *AGameTimeFXManager::GetSingleton()->FindFX( "DARTS" ) );
	AGameTimeFXManager::GetSingleton()->SetForcedFX( &oFX );

	AWeatherManager::GetSingleton()->SetCurrentWeather( AWeatherManager::WEATHERTYPE_NORMAL );

	// The darts of the players
	for ( TINT i = 0; i < NUM_PLAYERS; i++ )
	{
		Player& rPlayer = m_aPlayers[ i ];

		for ( TINT j = 0; j < NUM_DARTS; j++ )
		{
			ADart& rDart = rPlayer.aDarts[ j ];

			oDartTransform.m_iModelIndex = i;
			rDart.m_pUnit                = AUnitManager::GetSingleton()->SpawnUnit( TPS8( dart ), &oDartTransform );
			TSTATICCAST( AVisibleUnit, rDart.m_pUnit )->SetVisible( TFALSE );
			rDart.m_iScore = 0;
			rDart.m_eState = ADart::STATE_IDLE;
			rDart.SetSpinning( TFALSE );
			rDart.m_iRing     = ADartThrow::RING_NONE;
			rDart.m_bOnTarget = TFALSE;
			rDart.m_fScale    = 1.0f;
		}

		rPlayer.aRoundScores[ 0 ] = 0;
		rPlayer.aRoundScores[ 1 ] = 0;
		rPlayer.aRoundScores[ 2 ] = 0;
	}

	m_oAimDart.m_pUnit = AUnitManager::GetSingleton()->SpawnUnit( TPS8( dart ), &oDartTransform );
	TSTATICCAST( AVisibleUnit, m_oAimDart.m_pUnit )->SetVisible( TFALSE );
	m_oAimDart.SetSpinning( TFALSE );

	m_aPlayers[ 0 ].eGender = APlayerProfileManager::GetSingleton()->GetMainGender();
	m_iCurrentPlayer        = 0;

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );
	m_oUnusedText.SetFont( pFont );
	m_oUnusedText.SetDimensionsDirty();
	m_oUnusedText.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oUnusedText.SetTransform( 10.0f, 10.0f, 0.0f );
	m_oUnusedText.SetTextAlign( T2GUIFont::TextAlign( 1 ) );
	m_GUIElement.AddChildTail( m_oUnusedText );

	// HUD
	m_aCountHuds[ 0 ].SetParent( &m_GUIElement );
	m_aCountHuds[ 1 ].SetParent( &m_GUIElement );
	m_oFireHud.SetParent( &m_GUIElement );
	m_oInteractionHud.m_pParent = &m_GUIElement;

	m_aCountHuds[ 0 ].SetOffTexture( "Dart_Off" );
	m_aCountHuds[ 1 ].SetOffTexture( "Dart_Off" );
	m_aCountHuds[ 0 ].SetOnTexture( "Dart_On" );
	m_aCountHuds[ 1 ].SetOnTexture( "Dart_On" );

	AHudManager* pHudManager = AHudManager::GetSingleton();
	pHudManager->AddElement( &m_oInteractionHud, TFALSE, TTRUE, TFALSE );
	pHudManager->AddElement( &m_aCountHuds[ 0 ], TFALSE, TTRUE, TFALSE );
	pHudManager->AddElement( &m_aCountHuds[ 1 ], TFALSE, TTRUE, TFALSE );
	pHudManager->AddElement( &m_oFireHud, TFALSE, TTRUE, TFALSE );

	m_aCountHuds[ 0 ].SetColour( YELLOW_R, YELLOW_G, YELLOW_B );
	m_aCountHuds[ 1 ].SetColour( BLUE_R, BLUE_G, BLUE_B );
	m_aCountHuds[ 1 ].FlipU();

	m_fSavedFOV = ACameraManager::GetSingleton()->GetCurrentCamera()->GetFOV();

	// The players
	for ( TINT i = 0; i < NUM_PLAYERS; i++ )
	{
		AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_POSZ, 0, i, -1, TTRUE );

		// The skin: the male or female player cow, the second player is Root
		if ( i == 0 )
			oTransform.m_iModelIndex = APlayerProfileManager::GetSingleton()->GetMainGender() != APlayerProfile::GENDER_MALE;
		else if ( i == 1 )
			oTransform.m_iModelIndex = 2;

		AUnit* pUnit = AUnitManager::GetSingleton()->SpawnUnit( TPS8( player_darts ), &oTransform );
		TSTATICCAST( AInteractionComponent, pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) )->SetInteractionsDisabled( TTRUE );
		TSTATICCAST( AVisibleUnit, pUnit )->SetVisible( TFALSE );
		pUnit->ChangeSteer( &TGetClass( AStationarySteer ) );

		m_aPlayers[ i ].strDartBone = TPS8( dart_loc );
		m_aPlayers[ i ].pUnit       = pUnit;
		m_aPlayers[ i ].bComputer   = TFALSE;
	}

	m_aPlayers[ 0 ].pCountHud     = &m_aCountHuds[ 0 ];
	m_aPlayers[ 1 ].bComputer     = TTRUE;
	m_aPlayers[ 0 ].iTeam         = ADartThrow::TEAM_YELLOW;
	m_aPlayers[ 0 ].pPortrait     = &m_aPortraits[ 0 ];
	m_aPlayers[ 0 ].afColour[ 0 ] = 1.0f;
	m_aPlayers[ 0 ].afColour[ 1 ] = 0.84705883f;
	m_aPlayers[ 0 ].afColour[ 2 ] = 0.0f;
	m_aPlayers[ 1 ].afColour[ 0 ] = BLUE_R;
	m_aPlayers[ 1 ].afColour[ 1 ] = BLUE_G;
	m_aPlayers[ 1 ].afColour[ 2 ] = BLUE_B;
	m_aPlayers[ 1 ].pCountHud     = &m_aCountHuds[ 1 ];
	m_aPlayers[ 1 ].pPortrait     = &m_aPortraits[ 1 ];
	m_aPlayers[ 1 ].iTeam         = ADartThrow::TEAM_BLUE;

	// Portraits
	CreatePortraitFromProfile( &m_aPortraits[ 0 ], AGUI2Portrait::FLAGS_LEFT | AGUI2Portrait::FLAGS_STOPWATCH | AGUI2Portrait::FLAGS_UNK5, &m_GUIElement );
	m_aPortraits[ 0 ].SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_GUIElement.AddChildTail( m_aPortraits[ 0 ] );

	m_aPortraits[ 1 ].Create( "Root_Mug", AGUI2Portrait::FLAGS_RIGHT | AGUI2Portrait::FLAGS_STOPWATCH | AGUI2Portrait::FLAGS_UNK5, &m_GUIElement );
	m_aPortraits[ 1 ].SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );
	m_GUIElement.AddChildTail( m_aPortraits[ 1 ] );

	TFLOAT fCountY = m_aPortraits[ 0 ].GetHeight() * 1.7f;
	m_aCountHuds[ 0 ].SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT, fCountY );
	m_aCountHuds[ 1 ].SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT, fCountY );
	m_aCountHuds[ 1 ].SetScale( 66.0f );
	m_aCountHuds[ 0 ].SetScale( 66.0f );

	// Time bonus texts
	m_aBonusTexts[ 0 ].SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_aBonusTexts[ 1 ].SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );
	m_aBonusTexts[ 0 ].SetTransform( m_aPortraits[ 0 ].GetWidth() * 0.2f, m_aPortraits[ 0 ].GetHeight() * 1.3f, 0.0f );
	m_aBonusTexts[ 1 ].SetTransform( m_aPortraits[ 0 ].GetWidth() * -0.2f, m_aPortraits[ 0 ].GetHeight() * 1.3f, 0.0f );
	m_aBonusTexts[ 0 ].SetFont( pFont );
	m_aBonusTexts[ 0 ].SetDimensionsDirty();
	m_aBonusTexts[ 1 ].SetFont( pFont );
	m_aBonusTexts[ 1 ].SetDimensionsDirty();
	m_aPlayers[ 1 ].pBonusText = &m_aBonusTexts[ 1 ];
	m_aPlayers[ 0 ].pBonusText = &m_aBonusTexts[ 0 ];
	m_GUIElement.AddChildTail( m_aBonusTexts[ 0 ] );
	m_GUIElement.AddChildTail( m_aBonusTexts[ 1 ] );

	m_aPortraits[ 0 ].SetNameColour( YELLOW_R, YELLOW_G, YELLOW_B );
	SetPortraitNameFromProfile( &m_aPortraits[ 0 ] );
	m_aPortraits[ 1 ].SetNameColour( BLUE_R, BLUE_G, BLUE_B );
	m_aPortraits[ 1 ].SetName( ALocaleManager::Interface()->GetString( 1071 ) );

	m_oFireHud.SetBullseyeVisible( TFALSE );

	AMiniGameState::OnInsertion();

	m_fSavedNear = ARenderer::GetSingleton()->GetNear();
	ARenderer::GetSingleton()->SetNear( 0.1f );

	SetupCamera();

	m_oDataTRB.Load( "Data/MiniGameData/Darts/dartsdata.trb" );
	m_pData  = m_oDataTRB.CastSymbol<ADartsRoundData>( "dartsdata" );
	m_iRound = 0;

	pHudManager->AddElement( &m_oRoundHud, TFALSE, TTRUE, TFALSE );
	m_oRoundHud.SetForceHide( TTRUE );
	m_oRoundHud.SetParent( &m_GUIElement );

	SetupRound( m_iRound );

	// No start HUD, the game starts as soon as the intro is over
	m_oStartHudElement.SetForceHide( TTRUE );

	AGameLoader::Terrain_Set( AGameLoader::Terrain_EnvSSDarts, TFALSE, TTRUE, 0, 0, 0, 0 );
}

// $Barnyard: FUNCTION 00522770
void ADartMiniGame::OnRemoval()
{
	AGameTimeManager::GetSingleton()->ChangePhaseOfDay( m_eSavedDayPhase, TTRUE, TTRUE );

	m_oRoundHud.Detach();
	m_oFireHud.Detach();
	m_oDataTRB.Close();
	m_oBoard.Destroy();
	m_aCountHuds[ 0 ].Detach();
	m_aCountHuds[ 1 ].Detach();
	m_oInteractionHud.Detach();
	m_aPortraits[ 0 ].Destroy();
	m_aPortraits[ 1 ].Destroy();

	for ( TINT i = 0; i < NUM_PLAYERS; i++ )
	{
		AUnitManager::GetSingleton()->DestroyUnit( m_aPlayers[ i ].pUnit );

		for ( TINT j = 0; j < NUM_DARTS; j++ )
			AUnitManager::GetSingleton()->DestroyUnit( m_aPlayers[ i ].aDarts[ j ].m_pUnit );
	}

	ARenderer::GetSingleton()->SetNear( m_fSavedNear );
	ACameraManager::GetSingleton()->GetCurrentCamera()->SetFOV( m_fSavedFOV );

	ASkyDome* pSkyDome = ARenderer::GetSingleton()->GetSkyDome();
	pSkyDome->SetFlags( pSkyDome->GetFlags() | ASkyDome::FLAGS_RENDER );
	pSkyDome->SetFlags( ( pSkyDome->GetFlags() & ~( ASkyDome::FLAGS_FROZEN | ASkyDome::FLAGS_ROTATE ) ) | ASkyDome::FLAGS_ROTATE );

	ASunFlareEffect::GetSingleton()->SetEnabled( TTRUE );

	AMiniGameState::OnRemoval();
}

// $Barnyard: FUNCTION 00522040
void ADartMiniGame::OnActivate()
{
	AMiniGameState::OnActivate();
}

// $Barnyard: FUNCTION 00522050
void ADartMiniGame::OnDeactivate()
{
	AMiniGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 00522e80
void ADartMiniGame::Pause()
{
	PlayIntro();
}

// NOTE: inlined into Pause (00522e80) in the original
void ADartMiniGame::PlayIntro()
{
	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "ssdart_intro", "ssdart_intro", "SSDART_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
	);
}

// $Barnyard: FUNCTION 00522630
void ADartMiniGame::OnPreStart()
{
	SetVisible( TFALSE );
	AGameLoader::Terrain_Set( AGameLoader::Terrain_EnvSSDarts, TFALSE, TTRUE, 0, 0, 0, 0 );
}

// $Barnyard: FUNCTION 00522060
TBOOL ADartMiniGame::IsGameOver()
{
	return m_bGameOver;
}

// $Barnyard: FUNCTION 005231a0
void ADartMiniGame::UpdateGame( TFLOAT a_fDeltaTime )
{
	// DOES_NOTHING (005acd80) is called on the board
	m_oBoard.UpdateTargets( a_fDeltaTime );

	if ( m_bCogsSpin )
		m_oBoard.UpdateCogs( a_fDeltaTime );

	for ( TINT i = 0; i < ADartBoard::NUM_RINGS; i++ )
		m_oBoard.RotateRing( i, m_oBoard.GetRingSpeed( i ) * a_fDeltaTime );

	for ( TINT i = 0; i < NUM_PLAYERS; i++ )
	{
		Player& rPlayer = m_aPlayers[ i ];

		rPlayer.fBonusAlpha -= BONUS_FADE_SPEED * a_fDeltaTime;

		if ( rPlayer.fBonusAlpha < 0.0f )
			rPlayer.fBonusAlpha = 0.0f;

		rPlayer.pBonusText->SetAlpha( rPlayer.fBonusAlpha );

		for ( TINT j = 0; j < NUM_DARTS; j++ )
		{
			ADart& rDart = rPlayer.aDarts[ j ];

			if ( !rDart.m_bOnTarget )
			{
				if ( rDart.m_iRing != ADartThrow::RING_NONE )
					rDart.UpdateSpin( m_oBoard.GetRingSpeed( rDart.m_iRing ) * a_fDeltaTime );

				rDart.Update();
			}
		}

		ADart::UpdateFalling( a_fDeltaTime, rPlayer.aDarts, NUM_DARTS );
	}

	TVector4 vTop;
	TVector4 vRight;
	TVector4 vLeft;
	TVector4 vBottom;
	ADartBoard::GetCorners( vTop, vRight, vLeft, vBottom );
	m_oFireHud.SetBoardCorners( vTop, vRight, vLeft, vBottom );

	if ( m_bFading )
	{
		if ( m_fFadeTimer > 0.9f && m_bFadeIn )
		{
			// NOTE: retail StartFade (0045cb20) has a 4th argument, TTRUE here, that our signature doesn't have yet
			AFadeManager::GetSingleton()->StartFade( AFade::Color( 255, 0, 0, 0 ), AFade::Color( 0, 0, 0, 0 ), 1.0f );
		}

		if ( m_fFadeTimer < 0.0f )
			m_bFading = TFALSE;

		m_fFadeTimer -= a_fDeltaTime;
		return;
	}

	Player& rPlayer = GetCurrentPlayer();

	if ( rPlayer.iCurrentDart >= NUM_DARTS || m_bGameOver )
	{
		// Wait until both players have thrown their darts
		TINT iNumDone = 0;

		for ( TINT i = 0; i < NUM_PLAYERS; i++ )
		{
			if ( m_aPlayers[ i ].iCurrentDart == NUM_DARTS )
				iNumDone++;
		}

		if ( iNumDone == NUM_PLAYERS )
		{
			for ( TINT i = 0; i < NUM_PLAYERS; i++ )
				m_aPlayers[ i ].aRoundScores[ m_iRound ] = m_aPlayers[ i ].iScore;

			if ( m_iRound == NUM_ROUNDS - 1 || m_aPlayers[ 0 ].iScore <= m_aPlayers[ 1 ].iScore )
			{
				m_bGameOver = TTRUE;
			}
			else
			{
				// Next round
				m_iRound += 1;
				SetupRound( m_iRound );
				m_oBoard.ResetTargets();

				// NOTE: retail StartFade (0045cb20) has a 4th argument, TTRUE here, that our signature doesn't have yet
				AFadeManager::GetSingleton()->StartFade( AFade::Color( 255, 0, 0, 0 ), AFade::Color( 0, 0, 0, 0 ), 1.0f );

				OnStartHudFinished();
			}
		}

		return;
	}

	ADart& rDart = GetCurrentDart();

#ifdef BARNYARD_COMMUNITY_PATCH
	// Development: "-devautoplay" runs stop the aim bars of the first player
	if ( !rPlayer.bComputer && T2String8::FindString( GetCommandLineA(), "-devautoplay" ) )
	{
		// Different timings for each dart so that the aim varies
		TFLOAT fFireTimeX = 0.3f + 0.37f * TFLOAT( rPlayer.iCurrentDart % 4 );
		TFLOAT fFireTimeY = 0.2f + 0.29f * TFLOAT( ( rPlayer.iCurrentDart + 1 ) % 3 );

		if ( rDart.m_eState == ADart::STATE_AIM_X && m_fAimTimerX > fFireTimeX )
			m_bFireX = TTRUE;
		else if ( rDart.m_eState == ADart::STATE_AIM_Y && m_fAimTimerY > fFireTimeY )
			m_bFireY = TTRUE;
	}
#endif

	switch ( rDart.m_eState )
	{
		case ADart::STATE_IDLE:
		{
			m_oFireHud.SetColour( rPlayer.afColour[ 0 ], rPlayer.afColour[ 1 ], rPlayer.afColour[ 2 ] );
			m_oThrow.Reset();

			TSTATICCAST( AVisibleUnit, rDart.m_pUnit )->SetVisible( TFALSE );
			GetThrowController( rPlayer )->Raise();
			rPlayer.pPortrait->SetClockDuration( m_fAimTime + m_fAimTimeBonus );

			m_oThrow.m_fTimeUsed = 0.0f;
			TSTATICCAST( AAnimatedUnit, rPlayer.pUnit )->AttachUnit( rDart.m_pUnit, rPlayer.strDartBone );

			m_fRaiseTimer = 0.0f;
			m_bUnk1       = TFALSE;
			m_bUnk2       = TFALSE;

			const ADartsRoundData& rcData = GetRoundData();

			if ( rPlayer.bComputer )
			{
				m_fAimTime       = rcData.fAiAimTime;
				m_fAimTimeBonus  = rcData.fAiAimTimeBonus;
				m_oThrow.m_fAimX = 0.0f;
				m_oThrow.m_fAimY = 0.0f;
			}
			else
			{
				m_fAimTime       = rcData.fAimTime;
				m_fAimTimeBonus  = rcData.fAimTimeBonus;
				m_oThrow.m_fAimX = 1.0f;
				m_oThrow.m_fAimY = 1.0f;
			}

			rPlayer.pPortrait->ResetClock( TFALSE );
			rPlayer.pPortrait->Show();
			rPlayer.pPortrait->SetClockDuration( m_fAimTime + m_fAimTimeBonus );
			rPlayer.pPortrait->SetClockTime( m_fAimTime + m_fAimTimeBonus );
			rPlayer.pPortrait->StartClockFade( 0.0f, 0.5f, 0.5f );

			UpdatePlayerFacing();
			GetCurrentDart().m_eState = ADart::STATE_RAISE;
			return;
		}
		case ADart::STATE_AIM_X:
			UpdateThrowTime( m_oThrow, m_fAimTime + m_fAimTimeBonus, a_fDeltaTime );

			if ( GetRoundData().fAimFrequencyX + m_fAimTime + m_fAimTimeBonus < m_fAimTimerX || m_bFireX )
			{
				ASoundManager::GetSingleton()->PlayCue( soundbank::DARTS_PING );
				m_oFireHud.SetBarXVisible( TTRUE );
				m_oInteractionHud.SetText( ALocaleManager::Interface()->GetString( 782 ), 2, TINT( m_pInputMap ), AInputCommand_Interact, 0, 0xFFFFFFFF, -1.0f );
				GetCurrentDart().m_eState = ADart::STATE_AIM_Y;
				m_oFireHud.SetBullseyeVisible( TTRUE );
				m_oFireHud.SetBullseyePosition( m_oThrow.m_fAimX * -1.0f, m_oThrow.m_fAimY * -1.0f );

				if ( m_iCurrentPlayer == 0 )
					m_oFireHud.ResetColour();
			}
			else
			{
				UpdateAimX( m_oThrow, a_fDeltaTime );
			}

			return;
		case ADart::STATE_AIM_Y:
			UpdateThrowTime( m_oThrow, m_fAimTime + m_fAimTimeBonus, a_fDeltaTime );

			if ( m_fAimTime + m_fAimTimeBonus < m_fAimTimerX + m_fAimTimerY || m_bFireY )
			{
				m_oFireHud.SetBarXLocked( TTRUE );
				m_oInteractionHud.SetText( TNULL, 2, 0, -1, 0, 0xFFFFFFFF, -1.0f );
				m_oInteractionHud.SetForceHide( TTRUE );
				ASoundManager::GetSingleton()->PlayCue( soundbank::DARTS_PING );
				GetCurrentDart().m_eState = ADart::STATE_THROW;

				if ( m_iCurrentPlayer == 0 )
					m_oFireHud.ResetColour();
			}
			else
			{
				UpdateAimY( m_oThrow, a_fDeltaTime );
			}

			return;
		case ADart::STATE_THROW:
		{
			m_oFireHud.SetColour( rPlayer.afColour[ 0 ], rPlayer.afColour[ 1 ], rPlayer.afColour[ 2 ] );
			GetThrowController( rPlayer )->Throw();
			ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::DARTS_THROW01, *rPlayer.pUnit->GetSteer()->GetPosition() );

			m_oThrow.ComputeDirection();
			m_oBoard.ComputeTargetEx( m_oThrow );

			TVector4 vPosition;
			m_oAimDart.GetPosition( vPosition );
			m_oFireHud.SetTargetLockPosition( vPosition );

			m_fFlightTime             = 0.0f;
			GetCurrentDart().m_eState = ADart::STATE_THROWING;
			return;
		}
		case ADart::STATE_AI_START:
		{
			const ADartsRoundData& rcData  = GetRoundData();
			TRandom&               rRandom = ARandom::GetSingleton()->m_oRandom;

			m_fAiFireTimeY = rRandom.GetFloatMinMax( rcData.fAiFireTimeYMin, rcData.fAiFireTimeYMax );
			m_fAiFireTimeX = rRandom.GetFloatMinMax( rcData.fAiFireTimeXMin, rcData.fAiFireTimeXMax );
			m_fAiErrorX    = rRandom.GetFloatMinMax( rcData.fAiErrorX * -0.01f, rcData.fAiErrorX * 0.01f );
			m_fAiErrorY    = rRandom.GetFloatMinMax( rcData.fAiErrorY * -0.01f, rcData.fAiErrorY * 0.01f );

			GetCurrentDart().m_eState = ADart::STATE_AI_TARGET;
			m_oFireHud.SetBarYVisible( TFALSE );
			return;
		}
		case ADart::STATE_AI_TARGET:
			if ( m_oBoard.FindBestTarget( rPlayer.iTeam ) == ADartThrow::TARGET_NONE )
			{
				// Nothing to aim at, throw anywhere
				m_fAiTargetX = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( -1.0f, 1.0f );
				m_fAiTargetY = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( -1.0f, 1.0f );
			}

			GetCurrentDart().m_eState = ADart::STATE_AI_MOVE;
			m_fAiTimer                = 0.0f;
			return;
		case ADart::STATE_AI_MOVE:
		{
			TFLOAT fMoveTime = m_fAiFireTimeX + 0.08f + m_fAiFireTimeY + 0.08f;

			if ( fMoveTime > m_fAiTimer )
			{
				TINT iTarget = m_oBoard.FindBestTarget( rPlayer.iTeam );

				if ( iTarget != ADartThrow::TARGET_NONE )
				{
					// Aim where the target will be when the dart lands
					TVector4 vPosition;
					m_oBoard.GetTargetPosition( vPosition, iTarget, m_fFlightTime + 0.08f );
					ADartBoard::GetAimFromPosition( vPosition, m_fAiTargetX, m_fAiTargetY );
				}

				TFLOAT fInvMoveTime = 1.0f / fMoveTime;
				m_oThrow.m_fAimX    = TMath::LERP( 0.0f, m_fAiErrorX + m_fAiTargetX, fInvMoveTime * m_fAiTimer );
				m_oThrow.m_fAimY    = TMath::LERP( 0.0f, m_fAiErrorY + m_fAiTargetY, fInvMoveTime * m_fAiTimer );
				m_fAiTimer += a_fDeltaTime;
			}
			else
			{
				m_oThrow.m_fAimX = m_fAiTargetX;
				m_oThrow.m_fAimY = m_fAiTargetY;
				rDart.m_eState   = ADart::STATE_THROW;
			}

			UpdatePlayerFacing();
			UpdateThrowTime( m_oThrow, m_fAimTimeBonus + m_fAimTime, a_fDeltaTime );
			return;
		}
		case ADart::STATE_AI_AIM_X:
		{
			const ADartsRoundData& rcData = GetRoundData();

			TBOOL  bFire    = TFALSE;
			TFLOAT fCos     = TMath::Cos( TMath::PI * rcData.fAimFrequencyX * m_fAimTimerX );
			TFLOAT fCosNext = TMath::Cos( ( a_fDeltaTime + m_fAimTimerX ) * rcData.fAimFrequencyX * TMath::PI );

			UpdateThrowTime( m_oThrow, m_fAimTime + m_fAimTimeBonus, a_fDeltaTime );

			TINT iTarget = m_oBoard.FindClosestTarget( rPlayer.iTeam, m_oThrow.m_fAimX, ( fCosNext + fCos ) * 0.5f );

			if ( m_fAimTimerX > m_fAiFireTimeX )
			{
				if ( iTarget == ADartThrow::TARGET_NONE )
				{
					bFire = TTRUE;
				}
				else
				{
					TVector4 vPosition;
					m_oBoard.GetTargetPosition( vPosition, iTarget, 0.1f );
					ADartBoard::GetAimFromPosition( vPosition, m_fAiTargetX, m_fAiTargetY );

					TFLOAT fTolerance = TMath::Abs( fCosNext - fCos ) + TMath::Abs( m_fAiErrorY );

					if ( TMath::Abs( m_fAiTargetY - fCosNext ) < fTolerance &&
					     TMath::Abs( m_fAiTargetY - fCos ) < fTolerance &&
					     m_oBoard.WillHitTarget( iTarget, fCos + m_fAiErrorY, m_fAiTargetX, 0.1f ) )
					{
						bFire = TTRUE;
					}
				}
			}

			if ( m_fAimTime + m_fAimTimeBonus < m_fAimTimerX )
				bFire = TTRUE;

			UpdateAimX( m_oThrow, a_fDeltaTime );

			if ( bFire == TTRUE )
			{
				ASoundManager::GetSingleton()->PlayCue( soundbank::DARTS_PING );
				m_oFireHud.SetBarXVisible( TTRUE );
				m_oFireHud.SetBullseyeVisible( TTRUE );
				m_oFireHud.SetBullseyePosition( m_oThrow.m_fAimX * -1.0f, m_oThrow.m_fAimY * -1.0f );
				GetCurrentDart().m_eState = ADart::STATE_AI_AIM_Y;
			}

			return;
		}
		case ADart::STATE_AI_AIM_Y:
		{
			TBOOL bFire = TFALSE;
			UpdateThrowTime( m_oThrow, m_fAimTime + m_fAimTimeBonus, a_fDeltaTime );

			if ( m_fAimTimerY > m_fAiFireTimeY )
			{
				TINT iTarget = m_oBoard.FindClosestTarget( rPlayer.iTeam, m_oThrow.m_fAimX, m_oThrow.m_fAimY );

				if ( iTarget == ADartThrow::TARGET_NONE )
				{
					bFire = TTRUE;
				}
				else
				{
					TVector4 vBefore;
					TVector4 vNow;
					TVector4 vAfter;
					m_oBoard.GetTargetPosition( vBefore, iTarget, 0.1f - a_fDeltaTime );
					m_oBoard.GetTargetPosition( vNow, iTarget, 0.1f );
					m_oBoard.GetTargetPosition( vAfter, iTarget, a_fDeltaTime + 0.1f );

					TFLOAT fBeforeX;
					TFLOAT fBeforeY;
					TFLOAT fAfterX;
					TFLOAT fAfterY;
					ADartBoard::GetAimFromPosition( vBefore, fBeforeX, fBeforeY );
					ADartBoard::GetAimFromPosition( vNow, m_fAiTargetX, m_fAiTargetY );
					ADartBoard::GetAimFromPosition( vAfter, fAfterX, fAfterY );

					TFLOAT fAimX = m_oThrow.m_fAimX;

					// The target passes the aim this frame
					if ( ( fBeforeX + m_fAiErrorX + AI_TOLERANCE > fAimX && m_fAiErrorX + fAimX + AI_TOLERANCE > fAfterX ) ||
					     ( m_fAiErrorX + fAimX + AI_TOLERANCE > fBeforeX && fAfterX + m_fAiErrorX + AI_TOLERANCE > fAimX ) )
					{
						if ( AI_TOLERANCE + m_fAiErrorY > TMath::Abs( m_fAiTargetY - m_oThrow.m_fAimY ) )
							bFire = TTRUE;
					}
				}
			}

			if ( m_fAimTime + m_fAimTimeBonus < m_fAimTimerX + m_fAimTimerY || bFire == TTRUE )
			{
				ASoundManager::GetSingleton()->PlayCue( soundbank::DARTS_PING );
				GetCurrentDart().m_eState = ADart::STATE_THROW;
			}
			else
			{
				UpdateAimY( m_oThrow, a_fDeltaTime );
				m_oFireHud.SetBullseyePosition( m_oThrow.m_fAimX * -1.0f, m_oThrow.m_fAimY * -1.0f );
			}

			return;
		}
		case ADart::STATE_THROWING:
		{
			TVector4 vPosition;
			m_oAimDart.GetPosition( vPosition );
			m_oFireHud.SetTargetLockPosition( vPosition );
			return;
		}
		case ADart::STATE_RAISE:
			m_fRaiseTimer += a_fDeltaTime;
			rPlayer.pPortrait->SetClockVisible( TTRUE );

			if ( m_fRaiseTimer > GetRoundData().fRaiseTime )
			{
				TSTATICCAST( AVisibleUnit, GetCurrentDart().m_pUnit )->SetVisible( TTRUE );

				m_fAimTimerY = 0.0f;
				m_fAimTimerX = 0.0f;
				m_bFireY     = TFALSE;
				m_bFireX     = TFALSE;

				GetThrowController( GetCurrentPlayer() )->SetPlayIdle( TTRUE );
				GetThrowController( GetCurrentPlayer() )->SetAim( 0.0f, 0.0f );

				if ( GetCurrentPlayer().bComputer )
				{
					GetCurrentDart().m_eState = ADart::STATE_AI_START;
				}
				else
				{
					m_oFireHud.SetBarXVisible( TFALSE );
					m_oFireHud.SetBarX( -1.0f );
					m_oFireHud.SetBarY( -1.0f );
					m_oFireHud.SetBarYVisible( TTRUE );
					m_oInteractionHud.SetForceHide( TFALSE );
					m_oInteractionHud.SetText( ALocaleManager::Interface()->GetString( 783 ), 2, TINT( m_pInputMap ), AInputCommand_Interact, 0, 0xFFFFFFFF, -1.0f );
					GetCurrentDart().m_eState = ADart::STATE_AIM_X;
				}
			}

			return;
		case ADart::STATE_FLYING:
		{
			TVector4 vLockPosition;
			m_oAimDart.GetPosition( vLockPosition );
			m_oFireHud.SetTargetLockPosition( vLockPosition );

			m_fFlightTime += a_fDeltaTime;
			TFLOAT fProgress = m_fFlightTime * 10.0f;

			TVector4 vPosition;
			vPosition.Lerp4( m_vReleasePosition, m_oThrow.m_vTarget, ( fProgress >= 1.0f ) ? 1.0f : fProgress );
			GetCurrentDart().m_pUnit->GetSteer()->SetPosition( vPosition );

			TVector4 vDirection = m_oThrow.m_vTarget - m_vReleasePosition;
			TFLOAT   fDistance  = TVector4::Distance( m_oThrow.m_vTarget, m_vReleasePosition );
			vDirection.Normalise();

			// The dart gets bigger as it flies away
			TVector4 vDartPosition;
			GetCurrentDart().GetPosition( vDartPosition );
			GetCurrentDart().m_fScale = TVector4::Distance( m_vReleasePosition, vDartPosition ) / fDistance * 9.0f + 1.0f;
			GetCurrentDart().SetDirection( vDirection );

			if ( m_fFlightTime <= 0.1f )
				return;

			// The dart has landed
			m_oBoard.Score( m_oThrow );
			GetCurrentDart().m_iScore = m_oThrow.m_iScore;
			m_fResultTimer            = 0.0f;
			GetCurrentDart().m_eState = ADart::STATE_RESULT;

			rPlayer.pPortrait->StartClockFade( 0.5f, 0.0f, 0.5f );
			m_oFireHud.SetBarXVisible( TFALSE );
			m_oFireHud.SetBarYVisible( TFALSE );
			m_oFireHud.SetBullseyeVisible( TFALSE );

			ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::DARTS_IMPACT, *GetCurrentDart().m_pUnit->GetSteer()->GetPosition() );

			TFLOAT fRed   = 1.0f;
			TFLOAT fGreen = 1.0f;
			TFLOAT fBlue  = 1.0f;

			if ( m_oThrow.m_iTeam == ADartThrow::TEAM_YELLOW )
			{
				fGreen = YELLOW_G;
				fBlue  = YELLOW_B;
			}
			else if ( m_oThrow.m_iTeam == ADartThrow::TEAM_BLUE )
			{
				fRed   = BLUE_R;
				fGreen = BLUE_G;
				fBlue  = BLUE_B;
			}

			const TWCHAR* wszHitText = m_oBoard.GetHitText( m_oThrow, rPlayer.iTeam );
			m_oFireHud.ShowScore( m_oThrow.m_fAimX * -1.0f, m_oThrow.m_fAimY * -1.0f, m_oThrow.m_iScore, wszHitText, fRed, fGreen, fBlue );

			if ( m_oThrow.m_iTarget == ADartThrow::TARGET_NONE )
			{
				GetCurrentDart().m_iRing = m_oThrow.m_iRing;
			}
			else
			{
				if ( rPlayer.iTeam == m_oThrow.m_iTeam )
				{
					// Own target: points and a time bonus
					ASoundManager::GetSingleton()->PlayCue( soundbank::UI_DING );

					if ( ADartThrow::IsBullseye( m_oThrow ) )
						ASoundManager::GetSingleton()->PlayCue( soundbank::DARTS_CHEER_GOOD );

					rPlayer.aCategoryScores[ m_oThrow.m_iCategory ] += m_oThrow.m_iScore;
					rPlayer.iScore += m_oThrow.m_iScore;

					const ADartsRoundData& rcFirstRound = m_pData[ 0 ];

					TFLOAT fTimeLeft = m_fAimTimeBonus + m_fAimTime;

					if ( rcFirstRound.fUnk40 / rcFirstRound.fAimFrequencyY + rcFirstRound.fUnk40 / rcFirstRound.fAimFrequencyX <= m_oThrow.m_fTimeUsed )
						fTimeLeft -= m_oThrow.m_fTimeUsed;

					TFLOAT fBonus = fTimeLeft * ( ( m_iCurrentPlayer != 0 ) ? rcFirstRound.fTimeBonusPlayer2 : rcFirstRound.fTimeBonusPlayer1 );
					TINT   iBonus = ( fmod( fBonus, 1.0 ) <= 0.5 ) ? TMath::FloorToInt( fBonus ) : TMath::CeilToInt( fBonus );

					if ( iBonus < 0 )
						iBonus = 0;

					ShowTimeBonus( m_iCurrentPlayer, iBonus );
					rPlayer.iTimeBonus += iBonus;
					rPlayer.iScore += iBonus;
					rPlayer.pPortrait->SetScore( rPlayer.iScore );
				}
				else
				{
					// The other player's target: the points go to them
					ASoundManager::GetSingleton()->PlayCue( soundbank::UI_NEGATIVE );

					Player& rOther = m_aPlayers[ ( m_iCurrentPlayer + 1 ) % NUM_PLAYERS ];
					rOther.iScore += m_oThrow.m_iScore;
					rOther.pPortrait->SetScore( rOther.iScore );
				}

				m_oBoard.KnockOffTarget( m_oThrow.m_iTarget, &GetCurrentDart() );
			}

			PlayHitSound( m_iCurrentPlayer, m_oThrow );

			if ( m_oThrow.m_iScore > 0 || GetCurrentDart().m_iRing != ADartThrow::RING_NONE || m_oThrow.m_iCog != ADartThrow::COG_NONE )
			{
				GetCurrentDart().SetSpinning( TTRUE );

				// Hitting a cog drops the dart
				if ( m_oThrow.m_iCog != ADartThrow::COG_NONE )
					GetCurrentDart().Drop();
			}

			if ( m_oThrow.m_iScore == 0 )
				ASoundManager::GetSingleton()->PlayCue( soundbank::DARTS_CHEER_BAD );


			return;
		}
		case ADart::STATE_RESULT:
		{
			TVector4 vPosition;
			m_oAimDart.GetPosition( vPosition );
			m_oFireHud.SetTargetLockPosition( vPosition );

			m_fResultTimer += a_fDeltaTime;
			m_oFireHud.SetTargetLockAlpha( TMath::Max( 1.0f - ( m_fResultTimer + m_fResultTimer ), 0.0f ) );
			m_oFireHud.SetScoreAlpha( TMath::Max( 1.0f - m_fResultTimer, 0.0f ) );

			if ( m_fResultTimer > 1.0f )
			{
				m_oAimDart.SetSpinning( TFALSE );
				GetCurrentDart().m_eState = ADart::STATE_IDLE;

				GetCurrentPlayer().pPortrait->ResetClock( TFALSE );
				GetCurrentPlayer().pPortrait->SetClockVisible( TFALSE );

				TINT& riCurrentDart = GetCurrentPlayer().iCurrentDart;

				if ( riCurrentDart >= NUM_DARTS - 1 )
				{
					riCurrentDart = NUM_DARTS;
				}
				else
				{
					riCurrentDart += 1;
					GetCurrentDart().m_eState = ADart::STATE_IDLE;

					m_oFireHud.SetBarXVisible( TFALSE );
					m_oFireHud.SetBarYVisible( TFALSE );
					m_oFireHud.SetTargetLockVisible( TFALSE );
					m_oFireHud.SetTargetLockAlpha( 1.0f );
				}

				// Next player
				SetPlayerActive( m_iCurrentPlayer, TFALSE );
				m_iCurrentPlayer = ( m_iCurrentPlayer + 1 ) % NUM_PLAYERS;
				SetPlayerActive( m_iCurrentPlayer, TTRUE );
			}

			return;
		}
		default:
			return;
	}
}

// $Barnyard: FUNCTION 005228e0
void ADartMiniGame::UpdateGameOver( TFLOAT a_fDeltaTime )
{
	if ( !m_bGameOverPushed )
	{
		AGameStateController::GetSingleton()->PushState( new AAnticGameOverState( GetResults(), TFALSE ) );
		m_bGameOverPushed = TTRUE;
	}
}

// $Barnyard: FUNCTION 00523170
void ADartMiniGame::UpdatePreStart( TFLOAT a_fDeltaTime )
{
	SetupCamera();
	m_GUIElement.Hide();
	FacePlayersToCamera();
}

// $Barnyard: FUNCTION 00524b80
AMiniGameState::S1* ADartMiniGame::GetResults()
{
	m_oResults.ComputeTotal( 0 );

	if ( m_aPlayers[ 0 ].iScore <= m_aPlayers[ 1 ].iScore )
	{
		// NOTE: retail stores -1 and then 0
		m_oResults.iResult                = -1;
		m_oResults.iResult                = 0;
		m_oResults.bWon                   = m_iRound != 0;
		m_oResults.oPlayerList[ 1 ].bUnk1 = TFALSE;
	}
	else
	{
		m_oResults.iResult = 0;
		m_oResults.bWon    = TTRUE;
	}

	PlayerInfo& rInfo                 = m_oResults.oPlayerList[ 0 ];
	rInfo.iUnk2                       = m_aPlayers[ 0 ].iScore;
	m_oResults.oPlayerList[ 1 ].iUnk2 = m_aPlayers[ 1 ].iScore;
	rInfo.bUnk1                       = TTRUE;
	m_oResults.oPlayerList[ 1 ].bUnk1 = TFALSE;
	m_oResults.iMiniGameIndex         = AMiniGameManager::GetSingleton()->GetNextMiniGameIndex();

	while ( rInfo.vecRows.Size() != 0 )
	{
		rInfo.vecRows[ rInfo.vecRows.Size() - 1 ] = ResultRow();
		rInfo.vecRows.PopBack();
	}

	// One row per round played
	TINT iTotal = 0;

	for ( TINT i = 0; i < NUM_ROUNDS; i++ )
	{
		if ( m_iRound >= i )
		{
			ALocale::FormatTags( m_awszRoundNames[ i ], 64, 994, PPropertyValue( i + 1 ) );
			rInfo.vecRows.PushBack( ResultRow{ m_awszRoundNames[ i ], m_aPlayers[ 0 ].aRoundScores[ i ] } );
			iTotal += m_aPlayers[ 0 ].aRoundScores[ i ];
		}
	}

	rInfo.iTotal = iTotal;
	m_oResults.ComputeTotal( 0 );

	if ( m_bSubmitHighScore )
	{
		m_bSubmitHighScore = TFALSE;

		if ( AMiniGameManager::GetSingleton()->GetMiniGameVariant() == 0 )
			AHighScoreManager::AddScore( HIGHSCORE_TABLE, TFLOAT( rInfo.iTotal ), m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
	}

	return &m_oResults;
}

// $Barnyard: FUNCTION 00522f80
void ADartMiniGame::OnStartHudFinished()
{
	AMiniGameState::OnStartHudFinished();

	m_GUIElement.Show();
	SetupCamera();

	for ( TINT i = 0; i < NUM_PLAYERS; i++ )
	{
		Player& rPlayer = m_aPlayers[ i ];

		rPlayer.iScore       = 0;
		rPlayer.iCurrentDart = 0;
		rPlayer.iTimeBonus   = 0;
		rPlayer.pPortrait->SetScore( 0 );

		rPlayer.aCategoryScores[ 0 ] = 0;
		rPlayer.aCategoryScores[ 1 ] = 0;
		rPlayer.aCategoryScores[ 2 ] = 0;

		rPlayer.pCountHud->SetCount( NUM_DARTS );

		for ( TINT j = 0; j < NUM_DARTS; j++ )
		{
			TSTATICCAST( AVisibleUnit, rPlayer.aDarts[ j ].m_pUnit )->SetVisible( TFALSE );
			rPlayer.aDarts[ j ].Reset();
		}

		TSTATICCAST( AVisibleUnit, rPlayer.pUnit )->SetVisible( TTRUE );

		rPlayer.pPortrait->SetNameAlpha( INACTIVE_ALPHA );
		rPlayer.pCountHud->FadeTo( INACTIVE_ALPHA, FADE_DURATION );
		rPlayer.pCountHud->SetCount( NUM_DARTS - rPlayer.iCurrentDart );
		rPlayer.pPortrait->SetClockEnabled( TFALSE );
		rPlayer.pPortrait->SetClockVisible( TFALSE );
	}

	m_iCurrentPlayer = 0;

	Player& rFirst = m_aPlayers[ 0 ];
	rFirst.pPortrait->SetNameAlpha( 1.0f );
	rFirst.pCountHud->FadeTo( 1.0f, FADE_DURATION );
	rFirst.pCountHud->SetCount( NUM_DARTS - rFirst.iCurrentDart );
	rFirst.pPortrait->SetClockEnabled( TTRUE );
	rFirst.pPortrait->SetClockVisible( TFALSE );

	Player& rCurrent = GetCurrentPlayer();
	m_oFireHud.SetColour( rCurrent.afColour[ 0 ], rCurrent.afColour[ 1 ], rCurrent.afColour[ 2 ] );

	m_fFadeTimer = 3.0f;
	m_bFading    = TTRUE;

	m_oRoundHud.SetDuration( 2.0f );
	m_oRoundHud.Start( TFALSE, m_iRound + 1 );

	m_bGameOver       = TFALSE;
	m_bGameOverPushed = TFALSE;
}

// $Barnyard: FUNCTION 005224a0
void ADartMiniGame::OnUnitAction( AAnimationBreakpointUnitAction::Event* a_pEvent )
{
	if ( a_pEvent->eAction != AAnimationBreakpointUnitAction::ACTION_HD_THROW )
		return;

	Player& rPlayer = GetCurrentPlayer();
	ADart&  rDart   = GetCurrentDart();

	if ( rDart.m_eState != ADart::STATE_THROWING )
		return;

	// The dart leaves the hand
	TSTATICCAST( AAnimatedUnit, rPlayer.pUnit )->DetachUnit( rDart.m_pUnit, TTRUE, TFALSE );
	rPlayer.pCountHud->SetCount( NUM_DARTS - 1 - rPlayer.iCurrentDart );

	TSTATICCAST( AVisibleUnit, rDart.m_pUnit )->GetModelInstance()->GetPosition( m_vReleasePosition );
	rDart.m_pUnit->GetSteer()->SetPosition( m_vReleasePosition );
	rDart.SetDirection( m_oThrow.m_vDirection );

	GetCurrentDart().m_eState = ADart::STATE_FLYING;
}

// $Barnyard: FUNCTION 00522070
void ADartMiniGame::SetupRound( TINT a_iRound )
{
	// NOTE: the round passed isn't used, m_iRound is
	const ADartsRoundData& rcData = GetRoundData();

	m_bRoundFlag = rcData.iUnk == 1;

	m_oBoard.GetRing( ADartBoard::RING_INNER )->fAngle  = 0.0f;
	m_oBoard.GetRing( ADartBoard::RING_MIDDLE )->fAngle = 0.0f;
	m_oBoard.GetRing( ADartBoard::RING_OUTER )->fAngle  = 0.0f;

	m_oBoard.GetRing( ADartBoard::RING_INNER )->fSpeed  = GetRoundData().fInnerRingSpeed * TMath::DegToRad( 1.0f );
	m_oBoard.GetRing( ADartBoard::RING_MIDDLE )->fSpeed = GetRoundData().fMiddleRingSpeed * TMath::DegToRad( 1.0f );
	m_oBoard.GetRing( ADartBoard::RING_OUTER )->fSpeed  = GetRoundData().fOuterRingSpeed * TMath::DegToRad( 1.0f );

	m_oBoard.SetScoreMultiplier( TMath::CeilToInt( GetRoundData().fScoreMultiplier ) );

	if ( TMath::Abs( GetRoundData().fOuterRingSpeed ) > 0.1f )
		m_bCogsSpin = TTRUE;
}

// $Barnyard: FUNCTION 005221a0
void ADartMiniGame::UpdatePlayerFacing()
{
	ASteer* pSteer = GetCurrentPlayer().pUnit->GetSteer();

	m_oThrow.ComputeDirection();

	static TVector4 s_vForward = TVector4::VEC_POSX;

	TVector4 vThrow;
	vThrow.Multiply( m_oThrow.m_vDirection, 0.25f );

	TVector4 vForward;
	vForward.Multiply( s_vForward, 1.0f - 0.25f );

	TVector4 vDirection( vThrow.x + vForward.x, vThrow.y + vForward.y, vThrow.z + vForward.z, vThrow.w );
	vDirection.Normalise();

	pSteer->SetDirection( vDirection );
}

// $Barnyard: FUNCTION 00522300
void ADartMiniGame::UpdateThrowTime( ADartThrow& a_rThrow, TFLOAT a_fTime, TFLOAT a_fDeltaTime )
{
	a_rThrow.m_fTimeUsed += a_fDeltaTime;
	GetCurrentPlayer().pPortrait->SetClockTime( a_fTime - a_rThrow.m_fTimeUsed );
}

// $Barnyard: FUNCTION 00522340
void ADartMiniGame::SetPlayerActive( TINT a_iPlayer, TBOOL a_bActive )
{
	TFLOAT  fAlpha  = a_bActive ? 1.0f : INACTIVE_ALPHA;
	Player& rPlayer = m_aPlayers[ a_iPlayer ];

	rPlayer.pPortrait->SetNameAlpha( fAlpha );
	rPlayer.pCountHud->FadeTo( fAlpha, FADE_DURATION );
	rPlayer.pCountHud->SetCount( NUM_DARTS - rPlayer.iCurrentDart );
	rPlayer.pPortrait->SetClockEnabled( a_bActive );
	rPlayer.pPortrait->SetClockVisible( TFALSE );
}

// $Barnyard: FUNCTION 005223d0
void ADartMiniGame::ShowTimeBonus( TINT a_iPlayer, TINT a_iBonus )
{
	Player& rPlayer = m_aPlayers[ a_iPlayer ];

	TStringManager::String16Format( rPlayer.wszBonus, 24, L"%d", a_iBonus );
	rPlayer.pBonusText->SetTextAndFitWidth( rPlayer.wszBonus );
	rPlayer.pBonusText->SetColour( MakeColour( rPlayer.afColour ) );

	rPlayer.fBonusAlpha = 1.0f;
	rPlayer.pBonusText->SetAlpha( 1.0f );
}

// $Barnyard: FUNCTION 005225f0
void ADartMiniGame::SetVisible( TBOOL a_bVisible )
{
	for ( TINT i = 0; i < NUM_PLAYERS; i++ )
		TSTATICCAST( AVisibleUnit, m_aPlayers[ i ].pUnit )->SetVisible( a_bVisible );

	m_oBoard.SetSkeletonUpdating( a_bVisible );
}

// $Barnyard: FUNCTION 00522650
ADartThrowAnimController* ADartMiniGame::GetThrowController( Player& a_rPlayer )
{
	return TSTATICCAST( ADartThrowAnimController, TSTATICCAST( AAnimatedUnit, a_rPlayer.pUnit )->GetAnimatableObject()->FindAnimController( &TGetClass( ADartThrowAnimController ) ) );
}

// $Barnyard: FUNCTION 00522970
void ADartMiniGame::SetupCamera()
{
	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

	static TVector4 s_vPosition = CAMERA_POSITION;

	TVector4 vZero = TVector4::VEC_ZERO;

	TVector4 vDirection( vZero.x - s_vPosition.x, vZero.y - s_vPosition.y, vZero.z - s_vPosition.z, vZero.w );
	vDirection.Normalise();

	if ( s_bSetupCamera )
	{
		pCamera->GetMatrix().LookAtDirection( vDirection, TVector4::VEC_NEGY );
		pCamera->GetMatrix().SetTranslation( s_vPosition );
		pCamera->SetFOV( TMath::DegToRad( CAMERA_FOV ) );
	}
}

// $Barnyard: FUNCTION 00522a80
void ADartMiniGame::FacePlayersToCamera()
{
	TVector4 vZero = TVector4::VEC_ZERO;

	static TVector4 s_vPlayer1Position = CAMERA_POSITION;
	static TVector4 s_vPlayer2Position = CAMERA_POSITION;

	const TVector4& rcCameraPos = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();

	TVector4 vDirection( vZero.x - rcCameraPos.x, vZero.y - rcCameraPos.y, vZero.z - rcCameraPos.z, vZero.w );
	vDirection.Normalise();

	m_aPlayers[ 0 ].pUnit->GetSteer()->SetPosition( s_vPlayer1Position );
	m_aPlayers[ 1 ].pUnit->GetSteer()->SetPosition( s_vPlayer2Position );

	TVector4 vDirectionXZ( vDirection.x, 0.0f, vDirection.z, vDirection.w );
	vDirectionXZ.Normalise();

	m_aPlayers[ 0 ].pUnit->GetSteer()->SetDirection( vDirectionXZ );
	m_aPlayers[ 1 ].pUnit->SetSteerDirection( vDirectionXZ );
}

// $Barnyard: FUNCTION 00522bf0
void ADartMiniGame::UpdateAimX( ADartThrow& a_rThrow, TFLOAT a_fDeltaTime )
{
	Player& rPlayer = GetCurrentPlayer();
	m_oFireHud.SetColour( rPlayer.afColour[ 0 ], rPlayer.afColour[ 1 ], rPlayer.afColour[ 2 ] );

	a_rThrow.m_fAimY = TMath::Cos( GetRoundData().fAimFrequencyX * m_fAimTimerX * TMath::PI );
	m_oFireHud.SetBarY( m_oThrow.m_fAimY * -1.0f );

	GetThrowController( GetCurrentPlayer() )->SetAim( a_rThrow.m_fAimX, a_rThrow.m_fAimY );

	m_fAimTimerX += a_fDeltaTime;
	UpdatePlayerFacing();
}

// $Barnyard: FUNCTION 00522cc0
void ADartMiniGame::UpdateAimY( ADartThrow& a_rThrow, TFLOAT a_fDeltaTime )
{
	Player& rPlayer = GetCurrentPlayer();
	m_oFireHud.SetColour( rPlayer.afColour[ 0 ], rPlayer.afColour[ 1 ], rPlayer.afColour[ 2 ] );

	a_rThrow.m_fAimX = TMath::Cos( GetRoundData().fAimFrequencyY * m_fAimTimerY * TMath::PI );
	m_oFireHud.SetBarX( m_oThrow.m_fAimX * -1.0f );
	m_oFireHud.SetBarY( m_oThrow.m_fAimY * -1.0f );

	GetThrowController( GetCurrentPlayer() )->SetAim( m_oThrow.m_fAimX, m_oThrow.m_fAimY );

	m_fAimTimerY += a_fDeltaTime;
	m_oFireHud.SetBullseyePosition( a_rThrow.m_fAimX * -1.0f, a_rThrow.m_fAimY * -1.0f );
	UpdatePlayerFacing();
}

// $Barnyard: FUNCTION 00522dd0
void ADartMiniGame::PlayHitSound( TINT a_iPlayer, ADartThrow& a_rThrow )
{
	if ( a_iPlayer == 1 )
	{
		// Root
		if ( a_rThrow.m_iTeam != ADartThrow::TEAM_NONE )
		{
			if ( m_aPlayers[ 1 ].iTeam == a_rThrow.m_iTeam )
			{
				if ( ADartThrow::IsBullseye( a_rThrow ) )
					ASoundManager::GetSingleton()->PlayCue( soundbank::DARTS_ROOT_SCORE );
			}
			else
			{
				ASoundManager::GetSingleton()->PlayCue( soundbank::DARTS_ROOT_MISS );
			}
		}
	}
	else
	{
		Player& rPlayer = m_aPlayers[ a_iPlayer ];

		if ( rPlayer.iTeam == a_rThrow.m_iTeam && ( a_rThrow.m_iScore == 10 || a_rThrow.m_iScore == 5 ) )
			ASoundManager::GetSingleton()->PlayCue( ASoundId( ( rPlayer.eGender != APlayerProfile::GENDER_MALE ) ? 677 : 636 ) );
	}
}
