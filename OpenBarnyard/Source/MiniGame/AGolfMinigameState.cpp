#include "pch.h"
#include "AGolfMinigameState.h"
#include "AMiniGameManager.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AModel.h"
#include "Assets/AMaterialLibraryManager.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACameraHelper.h"
#include "FX/AGameTimeFXManager.h"
#include "FX/AParticleEffectManager.h"
#include "FX/AParticleEffectLocatorManager.h"
#include "GUI/AGUI2.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AHighScoreManager.h"
#include "Helpers/APlayerProgress.h"
#include "Helpers/ASimAnimModelHelperManager.h"
#include "Helpers/ATreeManager.h"
#include "HUD/AHudManager.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Render/ARenderer.h"
#include "Render/AShadowManager.h"
#include "Render/ASysShader/ASysShader.h"
#include "Render/ASysShader/ASysMaterial.h"
#include "Render/ASysShader/ASysMesh.h"
#include "Sound/APositionalSoundSet.h"
#include "Sound/ASoundManager.h"
#include "Input/AInputMapManager.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitModelStaticData.h"
#include "Units/AUnitType.h"
#include "Units/AUnitTransform.h"
#include "Units/AAnimatedUnit.h"
#include "UnitMovement/Steering/ABallSteer.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>
#include <Render/TCameraObject.h>
#include <Render/TRenderInterface.h>
#include <Render/TViewport.h>

#include <Render/TBackend.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( AGolfMinigameState );
TPSTRING8_DECLARE( MiniGameLevel1 );
TPSTRING8_DECLARE( MiniGameLevel2 );
TPSTRING8_DECLARE( MiniGameLevel3 );
TPSTRING8_DECLARE( MiniGameLevel4 );
TPSTRING8_DECLARE( MiniGameLevel5 );
TPSTRING8_DECLARE( MiniGameLevel6 );
TPSTRING8_DECLARE( MiniGameLevel7 );
TPSTRING8_DECLARE( MiniGameLevel8 );
TPSTRING8_DECLARE( player_golf );
TPSTRING8_DECLARE( golfball );
TPSTRING8_DECLARE( golfgopher );
TPSTRING8_DECLARE( golf_tee );
TPSTRING8_DECLARE( golf_flag );
TPSTRING8_DECLARE( sock_flag );
TPSTRING8_DECLARE( gopher_golf );
TPSTRING8_DECLARE( gopher_masksnorkel );
TPSTRING8_DECLARE( gopher_floatie );
TPSTRING8_DECLARE( barnbuck );
TPSTRING8_DECLARE( waveflag );
TPSTRING8_DECLARE( idle );
TPSTRING8_DECLARE( c_pelvis );
TPSTRING8_DECLARE( c_head );
TPSTRING8_DECLARE( c_r_upperarm );
TPSTRING8_DECLARE( c_l_upperarm );
TPSTRING8_DECLARE( cb_llocator );

// NOTE: GetClass 00471420, CreateObject 00475450, CreateObjectInPlace 004754b0 (the constructor is inlined in both)
TDEFINE_CLASS( AGolfMinigameState );

// Sounds
static constexpr ASoundId SOUND_MUSIC = 16;

// Locale strings
static constexpr TINT LOCALE_INSTRUCTIONS    = 0x202; // the text box
static constexpr TINT LOCALE_INSTRUCTIONS2   = 0x206; // bottom right
static constexpr TINT LOCALE_RESULTS_STROKES = 636;   // score row of the stroke limit courses
static constexpr TINT LOCALE_RESULTS_SCORE   = 606;   // score row of the time limit courses
static constexpr TINT LOCALE_RESULTS_LIVES   = 584;   // 20 points per life left
static constexpr TINT POINTS_PER_LIFE        = 20;
static constexpr TINT NUM_LIVES              = 5;

#ifdef BARNYARD_LATEST
// Lives and stroke limits (holes 1-9 and the others but 16 and 17) of the difficulty, set by OnInsertion.
// Latest version (Wii): 806af124, 806af128, 806af12c. Normal is the PC retail values.
static TINT s_iNumLives          = NUM_LIVES;
static TINT s_iMaxStrokesFront   = 9;
static TINT s_iMaxStrokesDefault = 12;
#endif

// High score tables (AHighScoreManager::AddScore) by variant
static constexpr TINT HIGHSCORE_TABLE_1 = 31; // variants 0 and 2
static constexpr TINT HIGHSCORE_TABLE_2 = 32; // variant 1
static constexpr TINT HIGHSCORE_TABLE_4 = 34; // variant 3
static constexpr TINT HIGHSCORE_TABLE_5 = 35; // variant 4
static constexpr TINT HIGHSCORE_TABLE_6 = 36; // variant 5

// The course of each variant (symbols of golfdata.trb), 0071d424
static const TPString8* s_apCourseNames[] = {
	&TPS8( MiniGameLevel1 ),
	&TPS8( MiniGameLevel2 ),
	&TPS8( MiniGameLevel3 ),
	&TPS8( MiniGameLevel4 ),
	&TPS8( MiniGameLevel5 ),
	&TPS8( MiniGameLevel6 ),
	&TPS8( MiniGameLevel7 ),
	&TPS8( MiniGameLevel8 ),
};

// Marker colours of the players, 0071dc2c
// NOTE: retail only has two colours here, players 3 and 4 read the next values of the data (0 and 0x3c)
static const TUINT32 s_aPlayerColours[ AMiniGameState::MAX_PLAYERS ] = {
	0xfff69719,
	0xff6c5ce7,
#ifdef BARNYARD_COMMUNITY_PATCH
	0xff6ce76c,
	0xffe7e76c,
#else
	0x00000000,
	0x0000003c,
#endif
};

// $Barnyard: FUNCTION 004728f0
AGolfMinigameState::Player::Player()
    : pGopher( TNULL )
    , pBall( TNULL )
{
}

// $Barnyard: FUNCTION 00472950
AGolfMinigameState::Player::~Player()
{
	pBall   = TNULL;
	pGopher = TNULL;
	oAnimListener.Disconnect();
}

// $Barnyard: FUNCTION 00472880
AGolfMinigameState::AnimListeners::Node::Node( TGenericEmitter* a_pEmitter, void* a_pCaller, TGenericListener::t_Callback a_fnCallback, TINT a_iPriority )
{
	pEmitter = a_pEmitter;
	oListener.Connect( *a_pEmitter, a_pCaller, a_fnCallback, a_iPriority );
}

// $Barnyard: FUNCTION 004714d0
AGolfMinigameState::AnimListeners::Node::~Node()
{
	oListener.Disconnect();
}

// $Barnyard: FUNCTION 00474050
AGolfMinigameState::AnimListeners::~AnimListeners()
{
	Clear();
}

// $Barnyard: FUNCTION 00472c70
void AGolfMinigameState::AnimListeners::Clear()
{
	while ( !llNodes.IsEmpty() )
	{
		Node* pNode = llNodes.Head();
		pNode->Remove();
		delete pNode;
	}
}

// $Barnyard: FUNCTION 00472bf0
AGolfMinigameState::AnimListeners::Node* AGolfMinigameState::AnimListeners::Add( TGenericEmitter* a_pEmitter, TINT a_iPriority )
{
	Node* pNode = new Node( a_pEmitter, pCaller, fnCallback, a_iPriority );
	llNodes.InsertTail( pNode );

	return pNode;
}

// $Barnyard: FUNCTION 00474550
AGolfMinigameState::AGolfMinigameState()
    : m_oViewport( TTRUE )
    , m_vHolePosition( TVector4::VEC_ZERO )
    , m_vPlayerPosition( TVector4::VEC_ZERO )
    , m_vPlayerDirection( TVector4::VEC_POSX )
{
	m_oPlayListeners.pCaller    = this;
	m_oPlayListeners.fnCallback = TREINTERPRETCAST( TGenericListener::t_Callback, OnPlayFinishedCallback );

	m_pPlayerUnit     = TNULL;
	m_pFlag           = TNULL;
	m_fIdleTimer      = 0.0f;
	m_fTimer          = 0.0f;
	m_eGameState      = GAMESTATE_INTRO;
	m_bReloadHole     = TFALSE;
	m_bHoleLoaded     = TFALSE;
	m_bCourseFinished = TFALSE;

	m_pInputMap = AInputMapManager::GetSingleton()->FindMap( TPS8( AGolfMinigameState ) );

	TRenderInterface::DISPLAYPARAMS* pDisplayParams = TRenderInterface::GetSingleton()->GetCurrentDisplayParams();
	m_oViewport.SetX( 0.0f );
	m_oViewport.SetY( 0.0f );
	m_oViewport.SetWidth( TFLOAT( pDisplayParams->uiWidth ) );
	m_oViewport.SetHeight( TFLOAT( pDisplayParams->uiHeight ) );
	m_oViewport.SetMinZ( 0.2f );
	m_oViewport.SetMaxZ( 1.0f );
	m_oViewport.AllowBackgroundClear( TFALSE );
	m_oViewport.AllowDepthClear( TTRUE );
}

// $Barnyard: FUNCTION 004740a0
// $Barnyard: FUNCTION 00474810
AGolfMinigameState::~AGolfMinigameState()
{
	m_pPlayerUnit = TNULL;
	m_oSwingListener.Disconnect();
}

// $Barnyard: FUNCTION 004714c0
ASoundId AGolfMinigameState::GetSound()
{
	return 16;
}

// $Barnyard: FUNCTION 00471430
TINT AGolfMinigameState::GetMaxStrokes()
{
	TINT iHole = m_pCourse->aHoles[ m_iCurrentHole ];

#ifdef BARNYARD_LATEST
	// Latest version (Wii 80196234)
	if ( iHole < 10 ) return s_iMaxStrokesFront;
	if ( iHole == 16 ) return 15;
	if ( iHole == 17 ) return 18;

	return s_iMaxStrokesDefault;
#else
	if ( iHole < 10 ) return 9;
	if ( iHole == 16 ) return 15;
	if ( iHole == 17 ) return 18;

	return 12;
#endif
}

// $Barnyard: FUNCTION 00471470
void AGolfMinigameState::ShowPopup( const TWCHAR* a_wszText )
{
	m_oPopup.m_fPulseSpeed = 1.0f;
	m_oPopup.m_fStartScale = 1.0f;
	m_oPopup.m_eMode       = AGUI2AnythingPopup::MODE_FADE;
	m_oPopup.m_fScale      = 1.2f;
	m_oPopup.m_fDuration   = 2.0f;
	m_oPopup.SetText( a_wszText, 0xff77ffff, TNULL );
}

// $Barnyard: FUNCTION 00471ff0
TBOOL AGolfMinigameState::IsCourseOver()
{
	if ( !m_pCourse->bStrokeLimit )
		return m_fElapsedTime >= m_fTimeLimit;

	TINT iHole = m_pCourse->aHoles[ m_iCurrentHole ];

#ifdef BARNYARD_LATEST
	// Latest version (Wii 80199f0c): GetMaxStrokes, which depends on the difficulty
	return m_aPlayers[ 0 ].iStrokes >= GetMaxStrokes();
#else
	if ( iHole < 10 ) return m_aPlayers[ 0 ].iStrokes >= 9;
	if ( iHole == 16 ) return m_aPlayers[ 0 ].iStrokes >= 15;

	return m_aPlayers[ 0 ].iStrokes >= ( ( iHole != 17 ) ? 12 : 18 );
#endif
}

// $Barnyard: FUNCTION 00475000
TBOOL AGolfMinigameState::NextPlayer()
{
	if ( ++m_iCurrentPlayer < m_iNumPlayers )
	{
#ifdef BARNYARD_COMMUNITY_PATCH
		// NOTE: retail calls the whole hole set-up here, which sets m_iCurrentPlayer back to 0 and restarts the
		// intro, so with 2+ players player 0 tees off forever. Only set the tee up
		// for the next player instead (the dead "not in the intro" branch of the tee set-up suggests that intent)
		if ( m_iNumPlayers > 1 )
		{
			StartFlyOver();
			m_eGameState = GAMESTATE_INTRO;
			SetupTee();
		}
		else
#endif
			SetupPlayerTurn();
		return TFALSE;
	}

	m_iCurrentPlayer = -1;
	return TTRUE;
}

// $Barnyard: FUNCTION 004715d0
TBOOL AGolfMinigameState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	TINT iPlayer = APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetPlayerSlotIndexByDevice( a_pInputEvent->GetSource() );

	switch ( a_eInputCommand )
	{
		case AInputCommand_Quit:
		case AInputCommand_Start:
			// Pausing is only possible once the hole is loaded
			if ( !m_bHoleLoaded )
				return TTRUE;

			break;
		case AInputCommand_Move:
			m_aPlayers[ iPlayer ].fStickY = -a_pInputEvent->GetMagnitudeFloat( 1 );
			m_aPlayers[ iPlayer ].fStickX = a_pInputEvent->GetMagnitudeFloat( 0 );
			return TTRUE;
		case AInputCommand_Up:
			a_pInputEvent->StartRepeat( 0.0f, 0.0f );
			m_aPlayers[ iPlayer ].fDPadY = 1.0f;
			return TTRUE;
		case AInputCommand_Down:
			a_pInputEvent->StartRepeat( 0.0f, 0.0f );
			m_aPlayers[ iPlayer ].fDPadY = -1.0f;
			return TTRUE;
		case AInputCommand_Left:
			a_pInputEvent->StartRepeat( 0.0f, 0.0f );
			m_aPlayers[ iPlayer ].fDPadX = -1.0f;
			return TTRUE;
		case AInputCommand_Right:
			a_pInputEvent->StartRepeat( 0.0f, 0.0f );
			m_aPlayers[ iPlayer ].fDPadX = 1.0f;
			return TTRUE;
		case AInputCommand_ZoomIn:
			a_pInputEvent->StartRepeat( 0.0f, 0.0f );
			m_bCommand30 = TTRUE;
			return TTRUE;
		case AInputCommand_ZoomOut:
			a_pInputEvent->StartRepeat( 0.0f, 0.0f );
			m_bCommand31 = TTRUE;
			return TTRUE;
		case AInputCommand_Shoot:
			if ( a_pInputEvent->GetEventType() != TInputInterface::EVENT_TYPE_GONE_DOWN )
				m_aPlayers[ iPlayer ].bSwingReleased = TTRUE;
			else
				m_aPlayers[ iPlayer ].bSwingPressed = TTRUE;

			return TTRUE;
	}

	return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $Barnyard: FUNCTION 00472d00
void AGolfMinigameState::OnInsertion()
{
#ifdef BARNYARD_LATEST
	// Latest version (Wii 80191eb4): more lives and strokes on Easy
	if ( APlayerProgress::GetSingleton()->IsEasyDifficulty() )
	{
		s_iNumLives          = 7;
		s_iMaxStrokesFront   = 11;
		s_iMaxStrokesDefault = 14;
	}
	else
	{
		s_iNumLives          = NUM_LIVES;
		s_iMaxStrokesFront   = 9;
		s_iMaxStrokesDefault = 12;
	}
#endif

	m_bShowPlayerFloats = TFALSE;
	m_pPlayerUnit       = TNULL;

	for ( TINT i = 0; i < MAX_PLAYERS; i++ )
	{
		m_aPlayers[ i ].pGopher = TNULL;
		m_aPlayers[ i ].pBall   = TNULL;
	}

	m_bCommand31    = TFALSE;
	m_bCommand30    = TFALSE;
	m_bCameraOnBall = TFALSE;
	m_iNumPlayers   = APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetHumanPlayerCount();

	AMiniGameManager* pMiniGameManager = AMiniGameManager::GetSingleton();

	if ( m_oGolfData.Load( "Data\\MinigameData\\Golf\\golfdata.trb" ) == TTRB::ERROR_OK )
	{
		m_pCourse = m_oGolfData.CastSymbol<CourseData>( s_apCourseNames[ pMiniGameManager->GetMiniGameVariant() ]->GetString() );

		if ( !m_pCourse )
		{
			// Unknown variant, play the first course
			pMiniGameManager->SetMiniGameVariant( 0 );
			m_pCourse = m_oGolfData.CastSymbol<CourseData>( s_apCourseNames[ pMiniGameManager->GetMiniGameVariant() ]->GetString() );
		}

		m_ppHoleData = m_oGolfData.CastSymbol<HoleData*>( "HoleData" );
	}

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		Player& rPlayer = m_aPlayers[ i ];

		rPlayer.fStickX        = 0.0f;
		rPlayer.fStickY        = 0.0f;
		rPlayer.fDPadX         = TVector2::VEC_ZERO.x;
		rPlayer.fDPadY         = TVector2::VEC_ZERO.y;
		rPlayer.bSwingPressed  = TFALSE;
		rPlayer.bSwingReleased = TFALSE;
		rPlayer.iGopherState   = GOPHERSTATE_FREE;
		rPlayer.iBallState     = BALLSTATE_ON_COURSE;
		rPlayer.iHeldBall      = -1;
		rPlayer.iScore         = 0;
	}

	// The player
	APlayerProfileManager* pProfileManager = APlayerProfileManager::GetSingleton();

	AUnitTransform oPlayerTransform( m_vPlayerPosition, TVector4::VEC_POSX, 0, pProfileManager->GetMainGender() == APlayerProfile::GENDER_FEMALE, 0, TTRUE );
	AAnimatedUnit* pPlayer = TSTATICCAST( AAnimatedUnit, AUnitManager::GetSingleton()->SpawnUnit( TPS8( player_golf ), &oPlayerTransform ) );
	m_pPlayerUnit          = pPlayer->GetSelfProxy();

	{
		AModelInstanceRef pModelInstance = pPlayer->GetAnimatableObject()->GetModelInstance();
		AShadowManager::GetSingleton()->AddUnitShadow( pPlayer, 1.25f, pModelInstance->GetBoneIndex( TPS8( c_pelvis ) ), -0.1f, AShadowManager::FLAGS_UNIFORM_SCALE, AShadowManager::TYPE_DEFAULT );
	}

	pPlayer->ApplyMaterial( pProfileManager->GetMainCostume(), TTRUE );

	m_iCurrentPlayer = 0;
	m_iCurrentHole   = -1;

	m_iWaterSplashAEffect = AParticleEffectManager::FindEffect( "watersplash2a" );
	m_iWaterSplashBEffect = AParticleEffectManager::FindEffect( "watersplash2b" );
	m_iWaterRippleEffect  = AParticleEffectManager::FindEffect( "waterripple" );

	// Materials of the power ring around the gopher
	m_pPowerPointerMaterial = ASysShader::GetSingleton()->CreateMaterial( TNULL );

	if ( m_pPowerPointerMaterial && m_pPowerPointerMaterial->Create( ASysMaterial::BLENDMODE_1 ) )
	{
		m_pPowerPointerMaterial->SetTextureNum( 1 );
		m_pPowerPointerMaterial->SetTexture( AMaterialLibraryManager::GetSingleton()->FindTexture( "particles\\gopher_power_pointer.tga" ) );
	}

	m_pPowerChunkMaterial = ASysShader::GetSingleton()->CreateMaterial( TNULL );

	if ( m_pPowerChunkMaterial && m_pPowerChunkMaterial->Create( ASysMaterial::BLENDMODE_1 ) )
	{
		m_pPowerChunkMaterial->SetTextureNum( 1 );
		m_pPowerChunkMaterial->SetTexture( AMaterialLibraryManager::GetSingleton()->FindTexture( "particles\\gopher_power_chunk.tga" ) );
	}

	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();

#ifdef BARNYARD_COMMUNITY_PATCH
	// Development: "-devplayers <n>" (multiplayer turns). The golf library of the PC data only has the
	// first player's ball, so load the models of the other players' balls and gophers from Data/ModelTypes
	if ( m_iNumPlayers > 1 && T2String8::FindString( GetCommandLineA(), "-devplayers" ) )
	{
		const TCHAR* aUnitTypes[] = { "golfball", "golfgopher" };

		for ( TINT iType = 0; iType < TARRAYSIZE( aUnitTypes ); iType++ )
		{
			AUnitType*            pUnitType  = AUnitManager::GetSingleton()->FindUnit( aUnitTypes[ iType ] );
			AUnitModelStaticData* pModelData = pUnitType ? TSTATICCAST( AUnitModelStaticData, pUnitType->FindStaticData( &TGetClass( AUnitModelStaticData ) ) ) : TNULL;

			for ( TINT i = 0; pModelData && i < m_iNumPlayers; i++ )
			{
				const TPString8& rcModelName = pModelData->GetModelName( i % pModelData->GetNumModels() );

				if ( !pAnimObjManager->FindType( rcModelName ) )
				{
					TINFO( "-devplayers: loading model type '%s'\n", rcModelName.GetString() );
					pAnimObjManager->LoadAnimObjType( rcModelName, TNULL, TFALSE );
				}
			}
		}
	}
#endif

	// The balls and the gophers
	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		Player& rPlayer = m_aPlayers[ i ];

		AUnitTransform oBallTransform( TVector4::VEC_ZERO, TVector4::VEC_POSX, 0, i, 0, TTRUE );
		AUnit*         pBall = AUnitManager::GetSingleton()->SpawnUnit( TPS8( golfball ), &oBallTransform );
		rPlayer.pBall        = pBall->GetSelfProxy();

		AShadowManager::GetSingleton()->AddUnitShadow( pBall, 0.3f, -1, -0.05f, AShadowManager::FLAGS_UNIFORM_SCALE, AShadowManager::TYPE_DEFAULT );
		TSTATICCAST( ABallSteer, pBall->GetSteer() )->SetPushOutIteratively( TTRUE );

		AUnitTransform oGopherTransform( TVector4::VEC_ZERO, TVector4::VEC_POSX, 0, i, 0, TTRUE );
		AAnimatedUnit* pGopher = TSTATICCAST( AAnimatedUnit, AUnitManager::GetSingleton()->SpawnUnit( TPS8( golfgopher ), &oGopherTransform ) );
		pGopher->GetAnimatableObject()->PlayAnimation( TPS8( idle ) );
		rPlayer.pGopher = pGopher->GetSelfProxy();

		AShadowManager::GetSingleton()->AddUnitShadow( pGopher, 1.25f, -1, -0.1f, AShadowManager::FLAGS_UNIFORM_SCALE, AShadowManager::TYPE_DEFAULT );
		AShadowManager::GetSingleton()->SetShadowEnabled( pGopher, TFALSE );

		AAnimatedUnit* pBallUnit = TSTATICCAST( AAnimatedUnit, rPlayer.pBall ? rPlayer.pBall->GetUnit() : TNULL );
		rPlayer.pBallModel       = pAnimObjManager->CreateAnimatableObject( pBallUnit->GetAnimatableObject()->GetObjectType(), TNULL, 0 );
		rPlayer.pBallModel->SetSkeletonUpdating( TFALSE, TTRUE );

		rPlayer.pPowerPointerMesh = ASysShader::GetSingleton()->CreateMesh( TNULL );

		if ( rPlayer.pPowerPointerMesh && rPlayer.pPowerPointerMesh->CreatePools( ASysMesh::FLAGS_DEFAULT, 4, 4 ) )
			rPlayer.pPowerPointerMesh->SetMaterial( m_pPowerPointerMaterial );

		for ( TINT k = 0; k < NUM_POWER_CHUNKS; k++ )
		{
			rPlayer.apPowerChunkMeshes[ k ] = ASysShader::GetSingleton()->CreateMesh( TNULL );

			if ( rPlayer.apPowerChunkMeshes[ k ] && rPlayer.apPowerChunkMeshes[ k ]->CreatePools( ASysMesh::FLAGS_DEFAULT, 4, 4 ) )
				rPlayer.apPowerChunkMeshes[ k ]->SetMaterial( m_pPowerChunkMaterial );
		}
	}

	// The tee and the flags
	m_pTee = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( TPS8( golf_tee ) ), TNULL, 0 );
	m_pTee->SetSkeletonUpdating( TTRUE, TTRUE );
	m_pTee->SetVisible( TTRUE );

	m_pFlag = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( TPS8( golf_flag ) ), TNULL, 0 );
	m_pFlag->SetSkeletonUpdating( TTRUE, TTRUE );
	m_pFlag->SetVisible( TTRUE );
	m_pFlag->PlayAnimation( TPS8( waveflag ) );

	m_pSockFlag = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( TPS8( sock_flag ) ), TNULL, 0 );
	m_pSockFlag->SetSkeletonUpdating( TFALSE, TTRUE );
	m_pSockFlag->SetVisible( TTRUE );
	m_pSockFlag->PlayAnimation( TPS8( waveflag ) );

	// The gophers swimming with the ball (in the snorkel mask and the floaties)
	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		Player& rPlayer = m_aPlayers[ i ];

		AAnimatableObject* pGopherGolf = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( TPS8( gopher_golf ) ), TNULL, 0 );
		rPlayer.pGopherGolf            = pGopherGolf;

		AAnimatedUnit*     pBallUnit = TSTATICCAST( AAnimatedUnit, rPlayer.pBall ? rPlayer.pBall->GetUnit() : TNULL );
		AAnimatableObject* pBall     = pAnimObjManager->CreateAnimatableObject( pBallUnit->GetAnimatableObject()->GetObjectType(), TNULL, 0 );
		pAnimObjManager->AttachObject( pBall, pGopherGolf, TPString8(), TPS8( cb_llocator ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );

		AAnimatableObject* pSnorkel = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( TPS8( gopher_masksnorkel ) ), TNULL, 0 );
		pAnimObjManager->AttachObject( pSnorkel, pGopherGolf, TPString8(), TPS8( c_head ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
		pSnorkel->SetSkeletonUpdating( TFALSE, TTRUE );

		AAnimatableObjectType* pFloatieType = pAnimObjManager->FindType( TPS8( gopher_floatie ) );

		AAnimatableObject* pFloatieR = pAnimObjManager->CreateAnimatableObject( pFloatieType, TNULL, 0 );
		pAnimObjManager->AttachObject( pFloatieR, pGopherGolf, TPString8(), TPS8( c_r_upperarm ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
		pFloatieR->SetSkeletonUpdating( TFALSE, TTRUE );

		AAnimatableObject* pFloatieL = pAnimObjManager->CreateAnimatableObject( pFloatieType, TNULL, 0 );
		pAnimObjManager->AttachObject( pFloatieL, pGopherGolf, TPString8(), TPS8( c_l_upperarm ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
		pFloatieL->SetSkeletonUpdating( TFALSE, TTRUE );
	}

	if ( m_pCourse->bCharity )
	{
		AAnimatableObjectType* pBuckType = pAnimObjManager->FindType( TPS8( barnbuck ) );

		for ( TINT i = 0; i < NUM_BUCKS; i++ )
			m_apBucks[ i ] = pAnimObjManager->CreateAnimatableObject( pBuckType, TNULL, 0 );

		CreateCharityActors( TFALSE );
	}

	ATreeManager* pTreeManager = ATreeManager::GetSingleton();
	pTreeManager->GetDistances( m_fSavedTreeLODDistance, m_fSavedTreeRenderDistance, m_fSavedTreeBlendingDistance );
	pTreeManager->SetDistances( 200.0f, 200.0f, 200.0f );

	m_fSavedFarDistance = ARenderer::GetSingleton()->GetFar();
	ARenderer::GetSingleton()->SetFar( 400.0f );

	m_iMusicCue = ASoundManager::GetSingleton()->PlayCue( SOUND_MUSIC );

	AMiniGameState::OnInsertion();

	if ( m_pCourse->bCharity )
		m_oPickUpHudElement.SetForceHide( TFALSE );

	// The power bar
	AHudManager::GetSingleton()->AddElement( &m_oPowerBar, TFALSE, TTRUE, TFALSE );
	m_oPowerBar.SetForceHide( TTRUE );
	m_oPowerBar.SetMask( AHudElement::MASK_MINIGAME_DEFAULT );
	m_oPowerBar.m_fFillSpeed = 1000.0f;

	// The text box with the instructions
	m_oTextBox.Create( T2GUIFontManager::FindFont( "Rekord18" ), 200.0f );
	ALocale::FormatInputTags( m_wszInstructions, TARRAYSIZE( m_wszInstructions ), LOCALE_INSTRUCTIONS );
	m_oTextBox.SetText( m_wszInstructions );
	m_oTextBox.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oTextBox.SetTransform( 0.0f, -5.0f, 0.0f );
	m_GUIElement.AddChildTail( m_oTextBox );
	m_oTextBox.Hide();

	if ( m_iNumPlayers == 1 )
	{
#ifdef BARNYARD_LATEST
		m_iLives = s_iNumLives;
#else
		m_iLives = NUM_LIVES;
#endif

		if ( m_pCourse->bStrokeLimit )
		{
			// The strokes counter
			m_oStrokesBack.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Golf" ) );
			m_oStrokesBack.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
			m_GUIElement.AddChildTail( m_oStrokesBack );

			TFLOAT fWidth, fHeight;
			m_oStrokesBack.GetDimensions( fWidth, fHeight );
			fHeight *= 1.4f;
			fWidth *= 1.4f;
			m_oStrokesBack.SetDimensions( fWidth, fHeight );

			m_oStrokesText.SetFont( T2GUIFontManager::FindFont( "Rekord26" ) );
			m_oStrokesText.SetText( m_wszStrokes );
			m_oStrokesText.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
			m_oStrokesText.SetColour( 0xffffff00 );
			m_oStrokesText.SetTransform( 0.0f, fHeight * -0.09f, 0.0f );
			m_oStrokesBack.AddChildTail( m_oStrokesText );

			m_pTimerOrStrokes = &m_oStrokesBack;
		}
		else
		{
			// The timer
			m_oTimer.Create( 0.0f, 0.0f );
			m_oTimer.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
			m_oTimer.GetTransform().SetTranslation( 10.0f, 10.0f );
			m_GUIElement.AddChildTail( m_oTimer );

			TFLOAT fWidth, fHeight;
			m_oTimer.GetDimensions( fWidth, fHeight );
			fHeight *= 0.75f;
			fWidth *= 0.75f;
			m_oTimer.SetDimensions( fWidth, fHeight );

			m_pTimerOrStrokes = &m_oTimer;
		}

		m_oLifeHud.Create( &m_GUIElement, TNULL, 8, "Score_Wood_Thin", T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT, 1.0f, TFALSE );
		m_oLifeHud.SetLives( m_iLives );
		m_oLifeHud.SetVisible( TFALSE );

		if ( m_pCourse->bCharity )
		{
			// Below the bucks counter
			m_oLifeHud.SetTransform( 0.0f, m_oPickUpHudElement.m_oBacking.GetHeight() + 10.0f, 0.0f );
		}
	}
#ifdef BARNYARD_COMMUNITY_PATCH
	else
	{
		// NOTE: with 2+ players retail leaves m_pTimerOrStrokes uninitialised and never creates the life HUD,
		// but still uses both (crash), and StartPlay sets up the timer that isn't created either. Create the timer
		// and point m_pTimerOrStrokes at it, and create the life HUD hidden. The time limit still ends the hole and
		// scores it, so on timed courses the timer is shown as in single player (the strokes counter is per player,
		// so it isn't shown on stroke limit courses)
		m_oTimer.Create( 0.0f, 0.0f );
		m_pTimerOrStrokes = &m_oTimer;

		if ( !m_pCourse->bStrokeLimit )
		{
			m_oTimer.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
			m_oTimer.GetTransform().SetTranslation( 10.0f, 10.0f );
			m_GUIElement.AddChildTail( m_oTimer );

			TFLOAT fWidth, fHeight;
			m_oTimer.GetDimensions( fWidth, fHeight );
			m_oTimer.SetDimensions( fWidth * 0.75f, fHeight * 0.75f );
		}

#  ifdef BARNYARD_LATEST
		m_iLives = s_iNumLives;
#  else
		m_iLives = NUM_LIVES;
#  endif
		m_oLifeHud.Create( &m_GUIElement, TNULL, 8, "Score_Wood_Thin", T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT, 1.0f, TFALSE );
		m_oLifeHud.SetLives( m_iLives );
		m_oLifeHud.SetVisible( TFALSE );
	}
#endif

	// The markers
	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		m_aPlayers[ i ].oReticle.Create();
		m_aPlayers[ i ].oReticle.SetMarkerColour( s_aPlayerColours[ i ] );
	}

	m_oFlagReticle.Create();
	m_oFlagReticle.SetTextures( "Putt_Flag", TNULL );
	m_oFlagReticle.SetMarkerColour( 0xffffffff );

	// The hole text
	m_oHoleText.SetFont( T2GUIFontManager::FindFont( "Rekord26" ) );
	m_oHoleText.Hide();
	m_oHoleText.SetAlpha( 0.0f );
	m_oHoleText.GetTransform().SetTranslation( 0.0f, -50.0f );
	m_GUIElement.AddChildTail( m_oHoleText );

	// The instructions at the bottom right
	m_oInstructionsText.SetFont( T2GUIFontManager::FindFont( "Rekord18" ) );
	m_oInstructionsText.Hide();
	m_oInstructionsText.SetAttachment( T2GUIATTACHMENT_BOTTOMRIGHT, T2GUIATTACHMENT_BOTTOMRIGHT );
	m_oInstructionsText.SetTransform( -5.0f, -5.0f, 0.0f );
	ALocale::FormatInputTags( m_wszInstructions2, TARRAYSIZE( m_wszInstructions2 ), LOCALE_INSTRUCTIONS2 );
	m_oInstructionsText.SetText( m_wszInstructions2 );
	m_GUIElement.AddChildTail( m_oInstructionsText );
}

// $Barnyard: FUNCTION 004717a0
void AGolfMinigameState::OnRemoval()
{
	ASimAnimModelHelperManager::GetSingleton()->RemoveModelHelper( &m_oWindmillHelper );
	m_oWindmillHelper.OnDestroy();

	AParticleEffectLocatorManager::GetSingleton()->Reset();
	APositionalSoundSet::GetSingleton()->Unload();
	AGameLoader::DestroyLevel();

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		Player& rPlayer = m_aPlayers[ i ];

		rPlayer.pPowerPointerMesh->DestroyResource();
		rPlayer.pPowerPointerMesh = TNULL;

		for ( TINT k = 0; k < NUM_POWER_CHUNKS; k++ )
		{
			rPlayer.apPowerChunkMeshes[ k ]->DestroyResource();
			rPlayer.apPowerChunkMeshes[ k ] = TNULL;
		}
	}

	m_pPowerPointerMaterial->Destroy();
	m_pPowerPointerMaterial = TNULL;
	m_pPowerChunkMaterial->Destroy();
	m_pPowerChunkMaterial = TNULL;

	TRenderInterface::GetSingleton()->BeginEndSceneHAL();
	m_oPowerBar.Detach();

	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();
	pAnimObjManager->DestroyAnimatableObject( m_pTee );
	pAnimObjManager->DestroyAnimatableObject( m_pFlag );
	pAnimObjManager->DestroyAnimatableObject( m_pSockFlag );

	if ( m_pCourse->bCharity )
	{
		for ( TINT i = 0; i < NUM_BUCKS; i++ )
			pAnimObjManager->DestroyAnimatableObject( m_apBucks[ i ] );

		DestroyCharityActors();
	}

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		pAnimObjManager->DestroyAnimatableObject( m_aPlayers[ i ].pBallModel );
		pAnimObjManager->DestroyAnimatableObject( m_aPlayers[ i ].pGopherGolf );
	}

	AShadowManager::GetSingleton()->RemoveShadow( GetPlayerUnit() );
	AUnitManager::GetSingleton()->DestroyUnit( GetPlayerUnit() );

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		Player& rPlayer = m_aPlayers[ i ];

		AUnit* pBall = rPlayer.pBall ? rPlayer.pBall->GetUnit() : TNULL;
		AShadowManager::GetSingleton()->RemoveShadow( pBall );
		AUnitManager::GetSingleton()->DestroyUnit( rPlayer.pBall ? rPlayer.pBall->GetUnit() : TNULL );
		rPlayer.pBall = TNULL;

		AUnit* pGopher = rPlayer.pGopher ? rPlayer.pGopher->GetUnit() : TNULL;
		AShadowManager::GetSingleton()->RemoveShadow( pGopher );
		AUnitManager::GetSingleton()->DestroyUnit( rPlayer.pGopher ? rPlayer.pGopher->GetUnit() : TNULL );
		rPlayer.pGopher = TNULL;

		rPlayer.oAnimListener.Disconnect();
	}

	m_oSwingListener.Disconnect();

	ACameraHelper* pMultiCameraHelper = ACameraManager::GetSingleton()->GetCameraHelper( ACameraManager::CAMERAHELPER_MULTICAMERA );
	pMultiCameraHelper->OnDetach();
	pMultiCameraHelper->ResetTarget();
	ACameraManager::GetSingleton()->ResetHelperTargets();
	ACameraManager::GetSingleton()->DetachCameraHelpers();

	m_pPlayerUnit = TNULL;

	for ( TINT i = 0; i < MAX_PLAYERS; i++ )
	{
		m_aPlayers[ i ].pGopher = TNULL;
		m_aPlayers[ i ].pBall   = TNULL;
	}

	AGameTimeFXManager::GetSingleton()->ClearForcedFX();
	AUnitManager::GetSingleton()->RemoveAllUnits();
	AUnitManager::GetSingleton()->DeleteDyingUnits();
	TRenderInterface::GetSingleton()->FlushDyingResources();

	ARenderer::GetSingleton()->SetFar( m_fSavedFarDistance );
	ATreeManager::GetSingleton()->SetDistances( m_fSavedTreeLODDistance, m_fSavedTreeRenderDistance, m_fSavedTreeBlendingDistance );

	m_oGolfData.Close();
	AMiniGameState::OnRemoval();
}

// $Barnyard: FUNCTION 00474250
void AGolfMinigameState::OnActivate()
{
	AMiniGameState::OnActivate();
	m_GUIElement.SetVisible( m_bHoleLoaded == TTRUE );

	if ( m_eGameState == GAMESTATE_PLAY )
	{
		m_pTimerOrStrokes->Show();

		if ( m_bPlayFinished )
		{
			if ( m_bLoseLife || m_aPlayers[ 0 ].iStrokes == 1 )
				m_oLifeHud.SetVisible( TTRUE );
		}
		else
		{
			SetupHoleCamera();

			for ( TINT i = 0; i < m_iNumPlayers; i++ )
				m_aPlayers[ i ].oReticle.SetVisible( TTRUE );

			m_oFlagReticle.SetVisible( TTRUE );
		}
	}

	m_oPopup.ShowIfHasText();

	if ( m_wszHoleText[ 0 ] != L'\0' )
		m_oHoleText.Show();

	if ( m_eGameState == GAMESTATE_TEE )
		m_oTextBox.Show();
}

// $Barnyard: FUNCTION 00471b50
void AGolfMinigameState::OnDeactivate()
{
	for ( TINT i = 0; i < m_iNumPlayers; i++ )
		m_aPlayers[ i ].oReticle.SetVisible( TFALSE );

	m_pTimerOrStrokes->Hide();
	m_oFlagReticle.SetVisible( TFALSE );
	m_oPopup.Hide();
	m_oHoleText.Hide();
	m_oLifeHud.SetVisible( TFALSE );
	m_oTextBox.Hide();
	m_oInstructionsText.Hide();

	AMiniGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 00472a10
AMiniGameState::S1* AGolfMinigameState::GetResults()
{
	// The winner is the player with the best score (the first one of them)
	TINT iBest = 0;

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		m_oResults.oPlayerList[ i ].bUnk1 = TTRUE;
		m_oResults.oPlayerList[ i ].iUnk2 = m_aPlayers[ i ].iScore;

		if ( m_aPlayers[ i ].iScore > m_oResults.oPlayerList[ iBest ].iUnk2 )
			iBest = i;
	}

	m_oResults.iResult = iBest;

	if ( !m_bCourseFinished )
		m_oResults.iResult = -1;

	m_oResults.iMiniGameIndex = AMiniGameManager::GetSingleton()->GetNextMiniGameIndex();
	m_oResults.bWon           = m_oResults.iResult == 0;

	PlayerInfo& rInfo = m_oResults.oPlayerList[ 0 ];

	while ( rInfo.vecRows.Size() != 0 )
	{
		rInfo.vecRows[ rInfo.vecRows.Size() - 1 ] = ResultRow();
		rInfo.vecRows.PopBack();
	}

	rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( m_pCourse->bStrokeLimit ? LOCALE_RESULTS_STROKES : LOCALE_RESULTS_SCORE ), m_aPlayers[ 0 ].iScore } );
	rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( LOCALE_RESULTS_LIVES ), POINTS_PER_LIFE * m_iLives } );
	m_oResults.ComputeTotal( 0 );

	if ( m_bSubmitHighScore )
	{
		m_bSubmitHighScore = TFALSE;

		TINT iTable;

		switch ( AMiniGameManager::GetSingleton()->GetMiniGameVariant() )
		{
			case 0:
			case 2:
				iTable = HIGHSCORE_TABLE_1;
				break;
			case 1:
				iTable = HIGHSCORE_TABLE_2;
				break;
			case 3:
				iTable = HIGHSCORE_TABLE_4;
				break;
			case 4:
				iTable = HIGHSCORE_TABLE_5;
				break;
			case 5:
				iTable = HIGHSCORE_TABLE_6;
				break;
			default:
				return &m_oResults;
		}

		AHighScoreManager::AddScore( iTable, TFLOAT( rInfo.iTotal ), m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
	}

	return &m_oResults;
}

// Fills the quad of a power ring mesh: the corners (+-a_fSize, 0, +-a_fSize) transformed by a_rcTransform.
// NOTE: inlined five times by AGolfMinigameState::OnRenderOverlay
static void FillPowerQuad( ASysMesh* a_pMesh, const TMatrix44& a_rcTransform, TFLOAT a_fSize, TUINT32 a_uiColour )
{
	static constexpr TFLOAT s_aCorners[ 4 ][ 2 ] = {
		{ -1.0f, 1.0f },
		{ -1.0f, -1.0f },
		{ 1.0f, 1.0f },
		{ 1.0f, -1.0f },
	};

	ASysMesh::LockBuffer oLockBuffer;
	a_pMesh->Lock( oLockBuffer );

	ASysMesh::Vertex* pVertices = TREINTERPRETCAST( ASysMesh::Vertex*, oLockBuffer.VertexLock.apStreams[ 0 ] );
	TIndexType*       pIndices  = oLockBuffer.IndexLock.pBuffer;

	for ( TINT i = 0; i < 4; i++ )
	{
		TVector4 vCorner( s_aCorners[ i ][ 0 ] * a_fSize, 0.0f, s_aCorners[ i ][ 1 ] * a_fSize, 1.0f );
		TVector4 vTransformed;
		TMatrix44::TransformVector( vTransformed, a_rcTransform, vCorner );

		pVertices[ i ].vPosition.x = vTransformed.x;
		pVertices[ i ].vPosition.y = vTransformed.y;
		pVertices[ i ].vPosition.z = vTransformed.z;
		pVertices[ i ].uiColor     = a_uiColour;
		pVertices[ i ].vUV.x       = TFLOAT( i / 2 );
		pVertices[ i ].vUV.y       = TFLOAT( i % 2 );
	}

	pIndices[ 0 ] = 0;
	pIndices[ 1 ] = 1;
	pIndices[ 2 ] = 2;
	pIndices[ 3 ] = 3;

	a_pMesh->Unlock( 4, 4 );
	a_pMesh->Validate();
	a_pMesh->Render();
}

// Renders the power ring around the gopher of every player that aims (iGopherState != GOPHERSTATE_FREE):
// the pointer in front of the gopher and, unless iGopherState is 1, up to 36 chunks around it (fWindupTime of them).
// $Barnyard: FUNCTION 00472080
void AGolfMinigameState::OnRenderOverlay( TFLOAT a_fDeltaTime )
{
#ifdef TRENDERINTERFACE_DX8
	TRenderD3DInterface* pRender = TRenderD3DInterface::Interface();
#else
	TRenderInterface* pRender = TRenderInterface::GetSingleton();
#endif

	TRenderContext* pOldContext = pRender->GetCurrentContext();

	m_oViewport.Begin();
	pRender->SetCurrentRenderContext( m_oViewport.GetRenderContext() );

	TRenderContext* pContext   = m_oViewport.GetRenderContext();
	TCameraObject*  pOldCamera = pContext->GetCameraObject();
	pContext->SetCameraObject( ARenderer::GetSingleton()->GetCameraObject() );
	ARenderer::GetSingleton()->GetCameraObject()->Render();

	pRender->GetTransforms().Push( m_oViewport.GetRenderContext()->GetWorldViewMatrix() );

	// NOTE: the colour is kept between the players (the pointer of a player after the first one
	// has the colour of the last chunk of the previous player)
	TUINT32 uiColour = 0xffff8200;

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		Player& rPlayer = m_aPlayers[ i ];

		if ( rPlayer.iGopherState == GOPHERSTATE_FREE )
			continue;

		// The pointer
		ASteer* pGopherSteer = ( rPlayer.pGopher ? rPlayer.pGopher->GetUnit() : TNULL )->GetSteer();

		const TVector4* pDirection = pGopherSteer->GetDirection();
		TFLOAT          fCos       = pDirection->z;
		TMath::Clip( fCos, -1.0f, 1.0f );

		TFLOAT fAngle = TMath::ACos( fCos );

		if ( pDirection->x < 0.0f )
			fAngle *= -1.0f;

		const TVector4* pPosition = pGopherSteer->GetPosition();
		TVector4        vPointerPos( pDirection->x + pPosition->x, pPosition->y + pDirection->y, pPosition->z + pDirection->z, pPosition->w );

		TMatrix44 mTransform;
		mTransform.Identity();
		mTransform.RotateY( fAngle );
		mTransform.SetTranslation( TVector4( vPointerPos.x, vPointerPos.y - 0.3f, vPointerPos.z, vPointerPos.w ) );

		FillPowerQuad( rPlayer.pPowerPointerMesh, mTransform, 0.5f, uiColour );

		if ( rPlayer.iGopherState == GOPHERSTATE_HOLDING )
			continue;

		// The chunks
		TFLOAT fNumChunks = 18.0f - TMath::Cos( TMath::PI * rPlayer.fWindupTime ) * 18.0f;

		const TVector4* pGopherPos = ( rPlayer.pGopher ? rPlayer.pGopher->GetUnit() : TNULL )->GetSteer()->GetPosition();
		TVector4        vCentre( pGopherPos->x, pGopherPos->y - 0.3f, pGopherPos->z, pGopherPos->w );

		TINT iGreen = 4680;

		for ( TINT k = 0; k < NUM_POWER_CHUNKS; k++, iGreen -= 130 )
		{
			TFLOAT fChunkAngle = TFLOAT( k ) * TMath::TWO_PI * ( 1.0f / 36.0f ) + fAngle;

			if ( fChunkAngle >= TMath::TWO_PI )
				fChunkAngle -= TMath::TWO_PI;

			TMatrix44 mChunkTransform;
			mChunkTransform.Identity();
			mChunkTransform.RotateY( fChunkAngle );
			mChunkTransform.SetTranslation( vCentre );

			// Fades in the last chunk, orange to red around the ring
			TFLOAT fAlpha = fNumChunks - TFLOAT( k );
			TMath::Clip( fAlpha, 0.0f, 1.0f );

			uiColour = ( ( ( TUINT32( fAlpha * 255.0f ) << 16 ) | TUINT32( iGreen / 36 ) | 0xff00 ) << 8 );

			FillPowerQuad( rPlayer.apPowerChunkMeshes[ k ], mChunkTransform, 1.0f, uiColour );
		}
	}

	pContext->SetCameraObject( pOldCamera );
	pRender->GetTransforms().Pop();
	pRender->SetCurrentRenderContext( pOldContext );
	m_oViewport.End();

#ifdef TRENDERINTERFACE_DX8
	pRender->FlushShaders();
#endif
}
