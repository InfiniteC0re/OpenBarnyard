#include "pch.h"
#include <Render/TCameraObject.h>
#include "ABikeRaceMicroGame.h"
#include "AMiniGameManager.h"
#include "AGopherShopInterface.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AModelRepos.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACameraHelper.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AAnticGameOverState.h"
#include "GameInterface/ASSeqState.h"
#include "GUI/AFadeManager.h"
#include "Helpers/ABikeManager.h"
#include "Helpers/ABikeHelpers.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AGameTimeManager.h"
#include "Helpers/AHighScoreManager.h"
#include "Helpers/APlayerProgress.h"
#include "HUD/AHudManager.h"
#include "Input/AInputHandler.h"
#include "Input/AInputMapManager.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerSet.h"
#include "Player/APlayerProfileManager.h"
#include "Sound/AMusicManager.h"
#include "Sound/AMultiplexStream.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitTransform.h"
#include "Units/AAnimatedUnit.h"
#include "Units/ASSeqController.h"
#include "Units/ASSeq.h"
#include "UnitMovement/Steering/ABikeSteer.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "FX/AAniEffectManager.h"
#include "Render/AShadowManager.h"
#include "Physics/AVehicleSkidManager.h"
#include "Physics/AVehicleManager.h"
#include "Physics/APhysicsBike.h"
#include "Physics/ACollisionObjectMesh.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( ABikeMicroGame );
TPSTRING8_DECLARE( win );

// NOTE: GetClass 005272b0, CreateObject 00528df0, CreateObjectInPlace 00528e50 (the constructor is inlined in both)
TDEFINE_CLASS( ABikeRaceMicroGame );

// 0x00722eb0
const ABikeRaceMicroGame::TrackInfo ABikeRaceMicroGame::ms_aTracks[ NUM_TRACKS ] = {
	{ "race1", "bikerace1", 2, 3, { 0, 1, 2 }, { 3, 1, 2 } },
	{ "race2", "bikerace2", 2, 3, { 0, 1, 2 }, { 3, 1, 2 } },
	{ "race3", "bikerace3", 2, 3, { 0, 1, 2 }, { 3, 1, 2 } },
	{ "race4", "bikerace4", 2, 3, { 0, 1, 2 }, { 3, 1, 2 } },
	{ "race5", "bikerace5", 2, 3, { 0, 1, 2 }, { 3, 1, 2 } },
	{ "race6", "bikerace6", 1, 3, { 0, 1, 2 }, { 3, 1, 2 } },
};

// The variant with the charity actors next to the course
static constexpr TINT CHARITY_VARIANT = 3;

// $Barnyard: FUNCTION 005285d0
ABikeRaceMicroGame::ABikeRaceMicroGame()
    : m_oMiniMap( ASimMiniMap::Mask( AHudElement::MASK_MINIGAME ) )
{
	m_pInputMap = AInputMapManager::GetSingleton()->FindMap( TPS8( ABikeMicroGame ) );
}

// $Barnyard: FUNCTION 00527390
// $Barnyard: FUNCTION 005272c0
ABikeRaceMicroGame::~ABikeRaceMicroGame()
{
}

// $Barnyard: FUNCTION 005273b0
TBOOL ABikeRaceMicroGame::ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	ACameraHelper* pHelper = ACameraManager::GetSingleton()->GetCurrentCameraHelper();

	if ( ( pHelper && pHelper->ProcessInputCommand( (void*)a_eInputCommand, (void*)a_pInputEvent ) ) || a_eInputCommand == AInputCommand_Leave )
		return TTRUE;

	return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// NOTE: shares 004ee730 with the Joy Ride state
void ABikeRaceMicroGame::OnRender( TFLOAT a_fDeltaTime )
{
	AVehicleSkidManager::GetSingleton()->Render();
}

// $Barnyard: FUNCTION 00527400
void ABikeRaceMicroGame::OnActivate()
{
	m_oWrongWayPopup.ShowIfHasText();
	AMultiplexStream::GetSingleton()->Pause( TFALSE );
	AMiniGameState::OnActivate();
}

// $Barnyard: FUNCTION 00527430
void ABikeRaceMicroGame::OnDeactivate()
{
	m_oWrongWayPopup.Hide();
	AMultiplexStream::GetSingleton()->Pause( TTRUE );
	// NOTE: the original calls DOES_NOTHING (00628f40) here
	AMiniGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 00527bb0
AUnit* ABikeRaceMicroGame::GetRivalUnit( TINT a_iRival )
{
	return GetBikerUnit( a_iRival + 1 );
}

AUnit* ABikeRaceMicroGame::GetBikerUnit( TINT a_iBiker ) const
{
	AUnitProxy* pProxy = m_aBikers[ a_iBiker ].pSlot->m_pUnitProxy;
	return pProxy ? pProxy->GetUnit() : TNULL;
}

ABikeSteer* ABikeRaceMicroGame::GetBikerSteer( TINT a_iBiker ) const
{
	return TSTATICCAST( ABikeSteer, GetBikerUnit( a_iBiker )->GetSteer() );
}

// $Barnyard: FUNCTION 00527460
void ABikeRaceMicroGame::OnStartHudFinished()
{
	for ( TINT i = 0; i <= m_pTrack->iNumRivals; i++ )
	{
		ABikeSteer* pSteer = GetBikerSteer( i );
		pSteer->Start();
		pSteer->GetPhysics()->SetFrozen( TTRUE );
	}
}

// $Barnyard: FUNCTION 005274c0
void ABikeRaceMicroGame::UpdatePreStart( TFLOAT a_fDeltaTime )
{
	GetBikerSteer( 0 )->Stop( TTRUE );
}

// $Barnyard: FUNCTION 00527f70
AMiniGameState::S1* ABikeRaceMicroGame::GetResults()
{
	m_oResults.iResult = m_bWon ? 0 : -1;

	PlayerInfo& rInfo = m_oResults.oPlayerList[ 0 ];
	rInfo.bUnk1       = TTRUE;
	rInfo.iUnk2       = m_oResults.iResult == 0;

	m_oResults.iMiniGameIndex = AMiniGameManager::GetSingleton()->GetNextMiniGameIndex();
	m_oResults.bWon           = m_oResults.iResult == 0;

	while ( rInfo.vecRows.Size() != 0 )
	{
		rInfo.vecRows[ rInfo.vecRows.Size() - 1 ] = ResultRow();
		rInfo.vecRows.PopBack();
	}

	if ( m_bSubmitHighScore )
	{
		m_bSubmitHighScore = TFALSE;
		TINT iVariant      = AMiniGameManager::GetSingleton()->GetMiniGameVariant();

		// NOTE: iTotal is never set by the bike race, the score is always 0
		if ( iVariant >= 0 && iVariant < NUM_TRACKS )
			AHighScoreManager::AddScore( HIGHSCORE_TABLE + iVariant, TFLOAT( rInfo.iTotal ), m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
	}

	m_oResults.bFlag5 = TTRUE;
	return &m_oResults;
}

// $Barnyard: FUNCTION 00528280
void ABikeRaceMicroGame::SetBikersVisible( TBOOL a_bVisible )
{
	for ( TINT i = 0; i <= m_pTrack->iNumRivals; i++ )
	{
		AAnimatedUnit* pUnit = TSTATICCAST( AAnimatedUnit, GetBikerUnit( i ) );
		pUnit->SetVisible( a_bVisible );

		if ( a_bVisible == TTRUE )
		{
			AAnimatableObject* pObject = pUnit->GetAnimatableObject();
			pObject->Update( 0.0f );
			pObject->UpdateAttachedObjectTransforms();
		}
	}
}

// The intro sequence is over, the race begins
// $Barnyard: FUNCTION 00528300
void ABikeRaceMicroGame::OnPause()
{
	AAnimatedUnit* pPlayer = TSTATICCAST( AAnimatedUnit, GetBikerUnit( 0 ) );
	pPlayer->SetMovement( TTRUE );
	pPlayer->SetAIEnabled( TTRUE );
	pPlayer->GetAnimatableObject()->SetAnimControllersDisabled( TFALSE );

	ABikeSteer* pSteer = TSTATICCAST( ABikeSteer, pPlayer->GetSteer() );

	TCHAR szGoal[ 16 ];
	TStringManager::String8Format( szGoal, sizeof( szGoal ), "player0start" );

	TVector4 vecStart;
	m_pNetwork->FindGoalOrFirst( szGoal )->GetPosition( vecStart );
	pSteer->SetPosition( vecStart );
	pSteer->Stop( TTRUE );

	ACameraManager::GetSingleton()->GetCurrentCameraHelper()->ResetPosition( TTRUE );
	SetBikersVisible( TTRUE );

	AMiniGameState::OnPause();
}

// $Barnyard: FUNCTION 005283d0
void ABikeRaceMicroGame::PlayLoseSequence()
{
	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "SSBR_Defeat", "SSBR_Defeat", "SSBR_CameraB_Defeat", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
	);
}

// $Barnyard: FUNCTION 00528460
void ABikeRaceMicroGame::PlayWinSequence()
{
	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "SSBR_Victory", "SSBR_Victory", "SSBR_CameraB_Victory", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
	);
}

// $Barnyard: FUNCTION 005284f0
void ABikeRaceMicroGame::Pause()
{
	// Play the intro, the race starts once the sequence state is removed (see AMiniGameState::OnActivate)
	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "SSBR_Main", "SSBR_Main", "SSBR_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
	);
}

// Hides the bikers while the intro plays
// $Barnyard: FUNCTION 00528580
void ABikeRaceMicroGame::OnPreStart()
{
	SetBikersVisible( TFALSE );

	AAnimatedUnit* pPlayer = TSTATICCAST( AAnimatedUnit, GetBikerUnit( 0 ) );
	pPlayer->SetMovement( TFALSE );
	pPlayer->SetAIEnabled( TFALSE );
	pPlayer->GetAnimatableObject()->SetAnimControllersDisabled( TTRUE );
}

// $Barnyard: FUNCTION 0052be60
void ABikeRaceMicroGame::OnInsertion()
{
	SaveCameraFOV();

	TUtil::MemSet( m_apSpectators, 0, sizeof( m_apSpectators ) );
	m_szOldSkeleton = TNULL;

	TINT iVariant = AMiniGameManager::GetSingleton()->GetMiniGameVariant();

	if ( iVariant >= NUM_TRACKS )
		iVariant = 0;

	m_pTrack = &ms_aTracks[ iVariant ];

	// The interior network with the start goals
	TPString8 strName = TPS8D( m_pTrack->szNetwork );
	m_pNetwork        = AInteriorNetworkManager::GetSingleton()->LoadNetwork( strName );
	AInteriorNetworkManager::GetSingleton()->SetDefaultNetwork( strName );

	Terrain_SetMain( TTRUE, m_pNetwork->FindGoalOrFirst( "player0start" )->m_iGroupId, TFALSE );
	AGameLoader::Terrain_Load();

	// The collision of the course
	TCHAR szModel[ 32 ];
	TStringManager::String8Format( szModel, sizeof( szModel ), "data\\models\\%s_collision", m_pTrack->szName );
	strName = TPS8D( szModel );

	m_pTrackModel = AModelRepos::GetSingleton()->InstantiateNewModel( strName, TNULL );

	// NOTE: retail copies the matrix of the transform object directly
	TMatrix44 matWorld = TREINTERPRETCAST( const TMatrix44&, ATerrainInterface::GetSingleton()->GetLocatorList()->GetWorldTransform() );
	m_pTrackModel->SetTransform( matWorld );
	m_pTrackModel->SetSkeletonUpdating( TTRUE );

	m_pTrackCollision = new ACollisionObjectMesh();
	m_pTrackCollision->Create( m_pTrackModel->GetSceneObject() );

	if ( iVariant < 4 )
	{
		m_pBarriersModel = TNULL;
	}
	else
	{
		strName          = TPS8D( "data\\models\\overpassbarriers" );
		m_pBarriersModel = AModelRepos::GetSingleton()->InstantiateNewModel( strName, TNULL );
		m_pBarriersModel->SetTransform( matWorld );
		m_pBarriersModel->SetSkeletonUpdating( TTRUE );
	}

	// The player and the rivals
	TInputDeviceController* pController = AInputHandler::GetSingleton()->GetMainController();
	APlayerManager*         pPlayers    = APlayerManager::GetSingleton();
	pPlayers->GetPlayerSet( APlayerManager::PLAYERSET_MINIGAME )->Reset();
	pPlayers->GetPlayerSet( APlayerManager::PLAYERSET_MINIGAME )->AddHumanPlayer( pController, -1 );

	for ( TINT i = 0; i < m_pTrack->iNumRivals; i++ )
		pPlayers->GetCurrentPlayerSet()->AddAIPlayer();

	pPlayers->GetCurrentPlayerSet()->FinishUpAddingAIs();

	m_oMainHud.SetMask( AHudElement::Mask( AHudElement::MASK_MINIGAME_DEFAULT ) );
	AHudManager::GetSingleton()->AddElement( &m_oMainHud, TFALSE, TTRUE, TFALSE );

	m_aBikers[ 0 ].iSegment  = 0;
	m_aBikers[ 0 ].iWaypoint = 0;
	m_aBikers[ 0 ].iLap      = 0;

	APlayerProfileManager* pProfileManager = APlayerProfileManager::GetSingleton();

	AUnitTransform oPlayerTransform( TVector4::VEC_ZERO, TVector4::VEC_POSX, 0, pProfileManager->GetMainGender() == APlayerProfile::GENDER_FEMALE, -1, TTRUE );
	AAnimatedUnit* pPlayer = TSTATICCAST( AAnimatedUnit, AUnitManager::GetSingleton()->CreateUnit( AUnitManager::GetSingleton()->FindUnit( "player_bikerace" ), &oPlayerTransform ) );

	if ( pPlayer->GetNumSkins() > 0 )
	{
		TINT iSkin = pProfileManager->GetMainCostume();
		TMath::Clip( iSkin, 0, pPlayer->GetNumSkins() - 1 );
		pPlayer->ApplyMaterial( iSkin, TTRUE );
	}

	// The bikes bought in the gopher shop
	for ( TINT i = 0; i < 5; i++ )
	{
		if ( AGopherShopInterface::HasBought( i + 309 ) )
			ABikeManager::GetSingleton()->SetBikeTypeUnlocked( ABikeManager::GetSingleton()->GetBikeTypeFlag( i ), TTRUE );
	}

#ifdef BARNYARD_COMMUNITY_PATCH
	pProfileManager->GetMainProfile()->SetStamina( 5.0f );
#else
	// NOTE: retail calls it on the profile manager itself (the main profile is at +4)
	TREINTERPRETCAST( APlayerSimProfile*, pProfileManager )->SetStamina( 5.0f );
#endif

	ABikeManager* pBikeManager = ABikeManager::GetSingleton();
	pBikeManager->CreateBikes();
	pBikeManager->StartupPlayerBike();
	ACameraManager::GetSingleton()->SetCameraHelper( ACameraManager::CAMERAHELPER_BIKE );
	pBikeManager->CreatePlayerBike();

	m_aBikers[ 0 ].pSlot = pPlayers->GetCurrentPlayerSet()->GetPlayerSlot( 0 );
	GetBikerSteer( 0 )->Stop( TTRUE );

	for ( TINT i = 1; i <= m_pTrack->iNumRivals; i++ )
	{
		Biker& rBiker    = m_aBikers[ i ];
		rBiker.iSegment  = 0;
		rBiker.iWaypoint = 0;
		rBiker.iLap      = 0;

		AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_POSX, 0, m_pTrack->aiRivalSkins[ i - 1 ], -1, TTRUE );
		AAnimatedUnit* pUnit = TSTATICCAST( AAnimatedUnit, AUnitManager::GetSingleton()->CreateUnit( AUnitManager::GetSingleton()->FindUnit( "jersey_bike" ), &oTransform ) );
		rBiker.pSlot         = pPlayers->GetCurrentPlayerSet()->GetPlayerSlot( i );

		pUnit->GetAnimatableObject()->GetModelInstance()->SetShadowMode( AModelInstance::SHADOWMODE_DYNAMIC );
		ABikeHelpers::AttachBikeModel( pUnit, m_pTrack->aiRivalBikes[ i - 1 ] );

		ABikeSteer* pSteer = TSTATICCAST( ABikeSteer, pUnit->GetSteer() );
		pSteer->Stop( TTRUE );
#ifdef BARNYARD_LATEST
		// Latest version (Wii 8027cc0c): the rivals ride bike type 0 on Easy, whatever the variant
		pSteer->SetBikeType( APlayerProgress::GetSingleton()->IsEasyDifficulty() ? 0 : iVariant );
#else
		pSteer->SetBikeType( iVariant );
#endif
		pSteer->GetPhysics()->m_bUnkCD2 = TFALSE;
		pSteer->SetAIControlled( TTRUE );

		if ( ACollisionObject* pCollision = pUnit->GetCollisionObject() )
			pCollision->SetType( ACollisionObject::TYPE_PLAYER );

		Rival& rRival      = m_aRivals[ i - 1 ];
		rRival.eState      = RIVALSTATE_RACING;
		rRival.fTimer      = 0.0f;
		rRival.bAhead      = TFALSE;
		rRival.fTauntTimer = 0.0f;
	}

	m_HUDParams.SetFlags( AHudElement::MASK_MINIGAME );

	// The portrait of the player on the main HUD
	m_oPortrait.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( pProfileManager->GetMainProfile()->GetHideTextureName( -1, APlayerProfile::GENDER_NONE ) ) );
	m_oPortrait.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oPortrait.SetTransform( -10.0f, 0.0f, 0.0f );
	m_oMainHud.m_oBacking.AddChildTail( m_oPortrait );

	// The placing, under the pick up counter
	AHudManager::GetSingleton()->AddElement( &m_oPlacingHud, TFALSE, TTRUE, TFALSE );
	m_oPlacingHud.SetPlacing( 1 );

	TFLOAT fY = m_oPickUpHudElement.m_oBacking.GetHeight() + m_oPlacingHud.GetBacking().GetTransform().GetTranslation().y;
	m_oPlacingHud.GetBacking().SetTransform( 0.0f, fY, 0.0f );

	// The mini map
	m_oMiniMap.Create();
	m_oMiniMap.SetPlayer( m_aBikers[ 0 ].pSlot->m_pUnitProxy );

	for ( TINT i = 1; i <= m_pTrack->iNumRivals; i++ )
		m_oMiniMap.AddUnitIcon( m_aBikers[ i ].pSlot->m_pUnitProxy, ASimMiniMap::ICON_ENEMY, 0.5f, TFALSE );

	// The lap, under the placing
	m_oLapText.Create( T2GUIFontManager::FindFont( "Rekord26" ), 0.0f );
	m_oLapText.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );
	TStringManager::String16Format( m_wszLapText, TARRAYSIZE( m_wszLapText ), ALocaleManager::Interface()->GetString( LOCALE_LAP ), 1, m_pTrack->iNumLaps );
	m_oLapText.SetTextAndFitWidth( m_wszLapText );

	fY = m_oPlacingHud.GetBacking().GetHeight() + fY + 10.0f;
	m_oLapText.SetTransform( -10.0f, fY, 0.0f );
	m_GUIElement.AddChildTail( m_oLapText );

	AGameTimeManager::GetSingleton()->ChangePhaseOfDay( AGameTimeManager::DAYPHASE_MORNING, TTRUE, TTRUE );

	m_fWrongWayTime = 0.0f;
	m_iTauntCue     = 1;
	m_bGameOver     = TFALSE;
	m_bWon          = TFALSE;
	m_bFallStarted  = TFALSE;
	m_pFade         = TNULL;

	if ( AMiniGameManager::GetSingleton()->GetMiniGameVariant() == CHARITY_VARIANT )
	{
		CreateCharityActors( TFALSE );
		SetCharityActorsSkeletonUpdating( TTRUE );
	}

	AVehicleSkidManager::GetSingleton()->HideAllTracks();
	AMiniGameState::OnInsertion();
	m_oPickUpHudElement.SetForceHide( TFALSE );

	ATerrainInterface::GetSingleton()->WaitUntilLoaded();
	SetupTrack();

	// The finish line, 3.5 back along its axis
	ATerrainInterface::GetSingleton()->FindLocator( "bikefinishline" )->GetMatrix( m_matFinishLine );

	TVector4 vecAxis = m_matFinishLine.AsBasisVector4( BASISVECTOR_RIGHT );
	TVector4 vecOffset;
	vecOffset.Multiply( vecAxis, 3.5f );

	TVector4& rTranslation = m_matFinishLine.AsBasisVector4( BASISVECTOR_TRANSLATION );
	rTranslation.x -= vecOffset.x;
	rTranslation.y -= vecOffset.y;
	rTranslation.z -= vecOffset.z;

	// The intro sequence plays at the finish line
	ASSeqController::GetSingleton()->LoadScript( "ssbr_main.tso", TVector4::VEC_ZERO );
	ASSeq* pIntro = ASSeqController::GetSingleton()->FindLoadedScriptByName( "SSBR_Main" );
	m_matFinishLine.RotateZ( TMath::PI * 0.5f );
	pIntro->SetTransform( m_matFinishLine );

	AMusicManager::GetSingleton()->SetEnabled( TFALSE, TTRUE );
	AMultiplexStream::GetSingleton()->Open( "BikeRace" );
	AVehicleManager::SetUseTriangleNormal( TTRUE );
	AVehicleManager::SetSlideOnObjects( TTRUE );
}

// $Barnyard: FUNCTION 005293b0
void ABikeRaceMicroGame::OnRemoval()
{
	RestoreCameraFOV();
	AVehicleSkidManager::GetSingleton()->HideAllTracks();

	ACameraManager::GetSingleton()->DetachCameraHelpers();
	ACameraManager::GetSingleton()->ResetHelperTargets();

	m_oMainHud.Detach();
	m_oPlacingHud.Detach();
	m_oMiniMap.Destroy();
	m_oMiniMap.Detach();

	APlayerManager* pPlayers = APlayerManager::GetSingleton();

	if ( pPlayers->GetCurrentPlayerSet() == pPlayers->GetPlayerSet( APlayerManager::PLAYERSET_SIM ) )
		pPlayers->SetCurrentPlayerSet( APlayerManager::PLAYERSET_MINIGAME );

	ABikeManager* pBikeManager = ABikeManager::GetSingleton();
	pBikeManager->DestroyPlayerBike( TFALSE );
	pBikeManager->EndRide( TFALSE );
	pBikeManager->DestroyBikes();

	for ( TINT i = 0; i <= m_pTrack->iNumRivals; i++ )
		AUnitManager::GetSingleton()->DestroyUnit( GetBikerUnit( i ) );

	AAnimatableObjectManager* pObjectManager = AAnimatableObjectManager::GetSingleton();

	for ( TINT i = 0; i < m_iNumSpectators; i++ )
		pObjectManager->DestroyAnimatableObject( m_apSpectators[ i ] );

	AAniEffectManager::GetSingleton()->DeleteAllEffects();

	for ( TINT i = 0; i < m_iNumCoins; i++ )
	{
		AShadowManager::GetSingleton()->RemoveShadow( m_aCoins[ i ].pObject->GetModelInstance() );
		pObjectManager->DestroyAnimatableObject( m_aCoins[ i ].pObject );
	}

	for ( TINT i = 0; i < m_iNumBerries; i++ )
	{
		AShadowManager::GetSingleton()->RemoveShadow( m_aBerries[ i ].pObject->GetModelInstance() );
		pObjectManager->DestroyAnimatableObject( m_aBerries[ i ].pObject );
	}

	if ( m_pGateModel )
	{
		m_pGateCollision->UnlinkCollisionObject();

		if ( m_pGateCollision )
			delete m_pGateCollision;

		AModelRepos::GetSingleton()->DestroyModelInstance( m_pGateModel );
		m_pGateModel = TNULL;
	}

	m_pTrackCollision->UnlinkCollisionObject();

	if ( m_pTrackCollision )
		delete m_pTrackCollision;

	AModelRepos::GetSingleton()->DestroyModelInstance( m_pTrackModel );
	m_pTrackModel = TNULL;

	if ( m_pBarriersModel )
	{
		AModelRepos::GetSingleton()->DestroyModelInstance( m_pBarriersModel );
		m_pBarriersModel = TNULL;
	}

	AInteriorNetworkManager::GetSingleton()->DestroyNetwork( m_pNetwork, TFALSE );

	// Put the locators of the terrain back
	if ( m_szOldSkeleton && !TSystemManager::Quitted() )
		ATerrainInterface::GetSingleton()->ReloadSkeleton( m_szOldSkeleton );

	if ( AMiniGameManager::GetSingleton()->GetMiniGameVariant() == CHARITY_VARIANT )
		DestroyCharityActors();

	AMusicManager::GetSingleton()->SetEnabled( TTRUE, TTRUE );
	AMultiplexStream::GetSingleton()->Close( TTRUE );
	AVehicleManager::SetUseTriangleNormal( TFALSE );
	AVehicleManager::SetSlideOnObjects( TFALSE );

	AMiniGameState::OnRemoval();
}

// $Barnyard: FUNCTION 0052c840
void ABikeRaceMicroGame::UpdateAlways( TFLOAT a_fDeltaTime )
{
	UpdateHUD( a_fDeltaTime );
	UpdatePickups( a_fDeltaTime );
	UpdateSpectators();

	AMultiplexStream* pStream = AMultiplexStream::GetSingleton();

	if ( pStream->GetState() != AMultiplexStream::STATE_PLAYING )
	{
		pStream->Play( 1.0f );
		pStream->SetVolume( 1.0f, 0, TTRUE );
		pStream->SetVolume( 1.0f, 1, TTRUE );
		pStream->SetVolume( 0.0f, 2, TTRUE );
		pStream->SetVolume( 0.0f, 3, TTRUE );
	}
}

// $Barnyard: FUNCTION 0052c930
void ABikeRaceMicroGame::UpdateGame( TFLOAT a_fDeltaTime )
{
	UpdateRivals( a_fDeltaTime );
	UpdateProgress();
	UpdateSunkBikes();
	UpdatePlayerFall( a_fDeltaTime );

#ifdef BARNYARD_COMMUNITY_PATCH
	Dev_UpdateAutoPlay();
#endif
}

// $Barnyard: FUNCTION 0052baf0
void ABikeRaceMicroGame::UpdateGameOver( TFLOAT a_fDeltaTime )
{
	if ( 0.0f == m_fGameOverTime )
	{
		// Look at the player
		ACameraManager::GetSingleton()->SetCameraHelper( ACameraManager::CAMERAHELPER_VICTORY );
		ACameraManager::GetSingleton()->GetCurrentCameraHelper()->SetTarget( m_aBikers[ 0 ].pSlot->m_pUnitProxy );

		for ( TINT i = 0; i < m_iNumSpectators; i++ )
		{
			AModelInstance* pModelInstance = m_apSpectators[ i ]->GetModelInstance();

			// NOTE: retail tests the skeleton-updating bit (0x1), not the visible bit
			if ( pModelInstance && pModelInstance->IsUpdatingSkeleton() )
				m_apSpectators[ i ]->PlayAnimation( TPS8( win ) );
		}
	}

	ASteer* pSteer = GetBikerUnit( 0 )->GetSteer();

	if ( pSteer->IsA( &TGetClass( ABikeSteer ) ) )
		TSTATICCAST( ABikeSteer, pSteer )->Stop( TFALSE );

	m_fGameOverTime += a_fDeltaTime;

	if ( m_fGameOverTime > 3.0f )
	{
		for ( TINT i = 1; i <= m_pTrack->iNumRivals; i++ )
			GetBikerSteer( i )->Stop( TFALSE );

		ATerrainInterface::GetSingleton()->CancelStreaming();
		ATerrainInterface::GetSingleton()->WaitUntilLoaded();

		AGameStateController::GetSingleton()->PushState( new AAnticGameOverState( GetResults(), TFALSE ) );
	}
}
