#include "pch.h"
#include "ABarnYardPoolMiniGameState.h"
#include "Helpers/APlayerProgress.h"
#include "AMiniGameManager.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AModel.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/APoolCamera.h"
#include "FX/AGameTimeFXManager.h"
#include "FX/AParticleSystem.h"
#include "FX/AParticleEffectManager.h"
#include "GameInterface/AAnticGameOverState.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/ASSeqState.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AHighScoreManager.h"
#include "Input/AInputHandler.h"
#include "Input/AInputMapManager.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Render/ARenderer.h"
#include "Render/ASkyDome.h"
#include "Render/ASunFlareEffect.h"
#include "Render/ASkinShader/ASkinMaterial.h"
#include "Sound/ASoundManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitTransform.h"
#include "Units/AUnitPoolBall.h"
#include "Units/AUnitPoolBfa.h"
#include "Units/AUnitPoolCue.h"
#include "Units/ASSeqController.h"
#include "UnitMovement/Steering/APoolBallSteer.h"
#include "UnitMovement/Steering/AStationarySteer.h"

#include <Math/TMathInline.h>
#include <Render/TModel.h>
#include <Render/TRenderInterface.h>
#include <Toshi/TSystem.h>
#include "SoundBank/pool.h"

#include <Render/TBackend.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( ABarnYardPoolMiniGame );
TPSTRING8_DECLARE( poolball );
TPSTRING8_DECLARE( poolcue );
TPSTRING8_DECLARE( poolbfa );
TPSTRING8_DECLARE( pool_fish );
TPSTRING8_DECLARE( pool_float );
TPSTRING8_DECLARE( pool_oarlock_left );
TPSTRING8_DECLARE( pool_oarlock_right );

// NOTE: GetClass 004d1c30, CreateObject 004d4fa0, CreateObjectInPlace 004d5000 (the constructor is inlined in both)
TDEFINE_CLASS( ABarnYardPoolMiniGameState );

//-----------------------------------------------------------------------------
// ABarnYardPoolMiniGameState::Team
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 004d7a80
ABarnYardPoolMiniGameState::Team::Team()
{
	Reset();
}

// $Barnyard: FUNCTION 004d7960
void ABarnYardPoolMiniGameState::Team::Reset()
{
	eBallType      = BALLTYPE_NONE;
	iNumPotted     = 0;
	iNumPlayers    = 1;
	iCurrentPlayer = 0;
	iNumShots      = 0;

	for ( TINT i = MAX_PLAYERS - 1; i >= 0; i-- )
		aPlayers[ i ] = 667;

	aPlayers[ 0 ] = 0;
}

// $Barnyard: FUNCTION 004d79b0
void ABarnYardPoolMiniGameState::Team::SetOppositeBallType( TINT a_eBallType )
{
	eBallType = ( a_eBallType != BALLTYPE_GROUP_B ) + BALLTYPE_GROUP_A;
}

// $Barnyard: FUNCTION 004d79d0
void ABarnYardPoolMiniGameState::Team::OnBallPotted( TINT a_eBallType )
{
	if ( a_eBallType == eBallType )
		iNumPotted++;
	else if ( a_eBallType == BALLTYPE_BLACK && iNumPotted == 7 )
		iNumPotted = 8;
}

// $Barnyard: FUNCTION 004d7a00
TINT ABarnYardPoolMiniGameState::Team::NextPlayer()
{
	if ( ++iCurrentPlayer >= iNumPlayers )
		iCurrentPlayer = 0;

	return iCurrentPlayer;
}

// $Barnyard: FUNCTION 004d7a20
void ABarnYardPoolMiniGameState::Team::SetNumPlayers( TINT a_iNumPlayers )
{
	iNumPlayers = a_iNumPlayers;
	TMath::Clip( iNumPlayers, 1, 2 );
}

// $Barnyard: FUNCTION 004d7a60
TINT ABarnYardPoolMiniGameState::Team::GetNextPlayer() const
{
	TINT iPlayer = iCurrentPlayer + 1;

	if ( iPlayer >= iNumPlayers )
		iPlayer = 0;

	return aPlayers[ iPlayer ];
}

// $Barnyard: FUNCTION 004d7a90
TINT ABarnYardPoolMiniGameState::Team::GetTargetBallType() const
{
	if ( eBallType != BALLTYPE_NONE && iNumPotted == 7 )
		return BALLTYPE_BLACK;

	return eBallType;
}

//-----------------------------------------------------------------------------
// ABarnYardPoolMiniGameState
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 004d3480
ABarnYardPoolMiniGameState::ABarnYardPoolMiniGameState()
    : m_oCueViewport( TTRUE )
{
	TSTATICASSERT( offsetof( ABarnYardPoolMiniGameState, m_eNextState ) == 0x20710 );

	m_bPropsCreated   = TFALSE;
	m_iVariant        = 1;
	m_iDifficulty     = 4;
	m_eNextState      = STATE_NONE;
	m_afAimSpeed[ 0 ] = 0.007f;
	m_afAimSpeed[ 1 ] = 0.007f;
	m_abAimInput[ 0 ] = TFALSE;
	m_abAimInput[ 1 ] = TFALSE;
	m_pInputMap       = AInputMapManager::GetSingleton()->FindMap( TPS8( ABarnYardPoolMiniGame ) );

	TRenderInterface::DISPLAYPARAMS* pDisplayParams = TRenderInterface::GetSingleton()->GetCurrentDisplayParams();
	m_oCueViewport.SetX( 0.0f );
	m_oCueViewport.SetY( 0.0f );
	m_oCueViewport.SetWidth( TFLOAT( pDisplayParams->uiWidth ) );
	m_oCueViewport.SetHeight( TFLOAT( pDisplayParams->uiHeight ) );
	m_oCueViewport.SetMinZ( 0.2f );
	m_oCueViewport.SetMaxZ( 1.0f );
	m_oCueViewport.AllowBackgroundClear( TFALSE );
	m_oCueViewport.AllowDepthClear( TTRUE );

	m_fInputDelay      = 0.0f;
	m_fMessageTimer    = 0.0f;
	m_bGroupsAnnounced = TFALSE;

#ifdef BARNYARD_COMMUNITY_PATCH
	m_fDevAutoPlayTimer = 0.0f;
	m_iDevAutoPlayShots = 0;
#endif
}

// $Barnyard: FUNCTION 004d3680
// $Barnyard: FUNCTION 004d5040
ABarnYardPoolMiniGameState::~ABarnYardPoolMiniGameState()
{
}

// $Barnyard: FUNCTION 004d1c40
void ABarnYardPoolMiniGameState::OnActivate()
{
	if ( m_oStartHudElement.IsForceHidden() == TTRUE )
	{
		m_oHud.SetTeamsVisible( TTRUE );
		// NOTE: retail calls DOES_NOTHING (005acd80) on m_oPowerBar here
	}

	AMiniGameState::OnActivate();
}

// $Barnyard: FUNCTION 004d1c70
void ABarnYardPoolMiniGameState::OnDeactivate()
{
	m_oHud.SetTeamsVisible( TFALSE );
	// NOTE: retail calls DOES_NOTHING (005acd80) on m_oPowerBar here

	AMiniGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 004d2a80
void ABarnYardPoolMiniGameState::OnStartHudFinished()
{
	m_oHud.SetTeamsVisible( TTRUE );
	ASoundManager::GetSingleton()->PlayCue( soundbank::POOL_INTRO );
}

// $Barnyard: FUNCTION 004d32d0
void ABarnYardPoolMiniGameState::PlayLoseSequence()
{
	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "sspool_lose", "sspool_lose", "SSpool_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
	);
}

// $Barnyard: FUNCTION 004d3360
void ABarnYardPoolMiniGameState::PlayWinSequence()
{
	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "sspool_win", "sspool_win", "SSpool_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
	);
}

// $Barnyard: FUNCTION 004d33f0
// $BarnyardWiiJP: FUNCTION 80216b44
void ABarnYardPoolMiniGameState::Pause()
{
#ifdef BARNYARD_LATEST
	// Latest version: the small table has no intro sequence, the cue ball (hidden by OnPreStart) is just shown again
	// NOTE: the Wii leaves the cue ball's shadow hidden here
	if ( m_iVariant == 5 )
	{
		if ( m_pCueBall )
			m_pCueBall->SetVisible( TTRUE );

		return;
	}
#endif

	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "sspool_intro", "sspool_intro", "SSpool_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
	);
}

// $Barnyard: FUNCTION 004d4f40
void ABarnYardPoolMiniGameState::OnPause()
{
	m_pCueBall->SetVisible( TTRUE );
	TSTATICCAST( AAnimatedUnit, m_aShadows[ CUE_BALL ]->GetUnit() )->SetVisible( TTRUE );
	AMiniGameState::OnPause();
}

// $Barnyard: FUNCTION 004d4f70
void ABarnYardPoolMiniGameState::OnPreStart()
{
	m_pCueBall->SetVisible( TFALSE );
	TSTATICCAST( AAnimatedUnit, m_aShadows[ CUE_BALL ]->GetUnit() )->SetVisible( TFALSE );
}

// $Barnyard: FUNCTION 004d4e00
void ABarnYardPoolMiniGameState::OnRenderOverlay( TFLOAT a_fDeltaTime )
{
	// Renders the cue again on top of everything so the table doesn't hide it
	if ( !m_aCue[ 0 ] || !m_aCue[ 0 ]->GetUnit() || !m_pCue || m_pCue->IsVisible() != TTRUE )
		return;

	TRenderD3DInterface* pRender = TRenderD3DInterface::Interface();
	pRender->FlushShaders();

	TRenderContext* pOldContext = pRender->GetCurrentContext();
	auto&           rTransforms = pRender->GetTransforms();

	m_oCueViewport.Begin();
	pRender->SetCurrentRenderContext( m_oCueViewport.GetRenderContext() );

	TRenderContext* pContext   = m_oCueViewport.GetRenderContext();
	TCameraObject*  pOldCamera = pContext->GetCameraObject();
	pContext->SetCameraObject( ARenderer::GetSingleton()->GetCameraObject() );
	ARenderer::GetSingleton()->GetCameraObject()->Render();

	rTransforms.Push( m_oCueViewport.GetRenderContext()->GetWorldViewMatrix() );

	m_pCue->GetModelInstance()->GetSceneObject()->EnableSkeletonUpdate();
	m_pCue->GetModelInstance()->GetSceneObject()->RenderIfVisible();
	m_pCue->GetModelInstance()->GetSceneObject()->DisableSkeletonUpdate();

	m_oCueViewport.GetRenderContext()->SetCameraObject( pOldCamera );
	rTransforms.Pop();

	pRender->SetCurrentRenderContext( pOldContext );
	m_oCueViewport.End();
}

// $Barnyard: FUNCTION 004d5060
void ABarnYardPoolMiniGameState::SetMaterialsFlag( AAnimatedUnit* a_pUnit, TBOOL a_bFlag )
{
	TModel*    pModel = a_pUnit->GetAnimatableObject()->GetModelInstance()->GetSceneObject()->GetModelRef()->GetModel();
	TModelLOD& rLOD   = pModel->GetLOD( 0 );

	for ( TINT i = 0; i < rLOD.iNumMeshes; i++ )
		TSTATICCAST( ASkinMaterial, rLOD.ppMeshes[ i ]->GetMaterial() )->SetFlag( a_bFlag );
}

// $Barnyard: FUNCTION 004d62d0
// $BarnyardWiiJP: FUNCTION 80210860
void ABarnYardPoolMiniGameState::OnInsertion()
{
	m_fLineOffset       = 0.0f;
	m_bShowPlayerFloats = TFALSE;
	m_bMouseAim         = TFALSE;
	m_fInputDelay       = 0.0f;

	APlayerSet* pPlayerSet = APlayerManager::GetSingleton()->GetCurrentPlayerSet();
	m_iNumPlayers          = pPlayerSet->GetHumanPlayerCount();
	TMath::Clip( m_iNumPlayers, 1, 4 );
	m_iNumAIPlayers = pPlayerSet->GetNumAIPlayers();

	AMiniGameState::OnInsertion();

	m_iVariant = AMiniGameManager::GetSingleton()->GetMiniGameVariant();
	TMath::Clip( m_iVariant, 0, 5 );

	switch ( m_iVariant )
	{
		case 0:
		case 1:
		case 2:
		case 3:
			AGameLoader::Terrain_Set( AGameLoader::Terrain_EnvPoolBoat, TFALSE, TTRUE, 0, 0, 0, 0 );
			m_iDifficulty   = m_iVariant;
			m_fPocketRadius = 0.15f;
			break;
		case 5:
			AGameLoader::Terrain_Set( AGameLoader::Terrain_EnvPoolTable, TFALSE, TTRUE, 0, 0, 0, 0 );
#ifdef BARNYARD_LATEST
			// Latest version: an easier opponent on the small table. Made for the Wii controls, so only on Easy here.
			m_iDifficulty = APlayerProgress::GetSingleton()->IsEasyDifficulty() ? 2 : 4;
#else
			m_iDifficulty = 4;
#endif
			m_fPocketRadius = 0.07f;
			break;
		default:
			AGameLoader::Terrain_Set( AGameLoader::Terrain_EnvPoolBoat, TFALSE, TTRUE, 0, 0, 0, 0 );
			m_iDifficulty   = 4;
			m_fPocketRadius = 0.15f;
			break;
	}

	AGameLoader::Terrain_Load();

	ACameraManager* pCameraManager = ACameraManager::GetSingleton();
	pCameraManager->SetCameraHelper( ACameraManager::CAMERAHELPER_POOL );
	m_pCamera = TSTATICCAST( APoolCamera, pCameraManager->GetCurrentCameraHelper() );

	m_fSavedNear = ARenderer::GetSingleton()->SetNear( 0.1f );

	// The balls
	for ( TINT i = 0; i < NUM_BALLS; i++ )
	{
		TINT eBallType;

		if ( i == CUE_BALL )
			eBallType = BALLTYPE_CUE;
		else if ( i == BLACK_BALL )
			eBallType = BALLTYPE_BLACK;
		else if ( i < BLACK_BALL )
			eBallType = BALLTYPE_GROUP_A;
		else
			eBallType = BALLTYPE_GROUP_B;

		AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_POSX, 0, eBallType, 0, TTRUE );
		AAnimatedUnit* pUnit = TSTATICCAST( AAnimatedUnit, AUnitManager::GetSingleton()->SpawnUnit( TPS8( poolball ), &oTransform ) );
		m_aBalls[ i ]        = pUnit->GetSelfProxy();

		pUnit->ChangeSteer( &TGetClass( APoolBallSteer ) );
		pUnit->GetAnimatableObject()->SetGlow( TFALSE );
		SetMaterialsFlag( pUnit, TFALSE );

		AUnitPoolBall* pBall = TSTATICCAST( AUnitPoolBall, GetBallUnit( i ) );
		pBall->SetupSteer();
		pBall->SetBallType( eBallType );
		SetMaterialsFlag( pBall, TFALSE );
	}

	// The shadows of the balls
	for ( TINT i = 0; i < NUM_BALLS; i++ )
	{
		AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_POSX, 0, BALLTYPE_NONE, 0, TTRUE );
		AUnit*         pUnit = AUnitManager::GetSingleton()->SpawnUnit( TPS8( poolball ), &oTransform );
		m_aShadows[ i ]      = pUnit->GetSelfProxy();

		pUnit->ChangeSteer( &TGetClass( AStationarySteer ) );
	}

	// The cue
	AUnitTransform oCueTransform( TVector4::VEC_ZERO, TVector4::VEC_POSZ, 0, 0, 0, TTRUE );
	AAnimatedUnit* pCue = TSTATICCAST( AAnimatedUnit, AUnitManager::GetSingleton()->SpawnUnit( TPS8( poolcue ), &oCueTransform ) );
	m_aCue[ 0 ]         = pCue->GetSelfProxy();

	pCue->ChangeSteer( &TGetClass( AStationarySteer ) );
	pCue->GetAnimatableObject()->SetGlow( TFALSE );
	SetMaterialsFlag( pCue, TFALSE );

	// The aim target
	AUnitTransform oBfaTransform( TVector4::VEC_ZERO, TVector4::VEC_POSZ, 0, 0, 0, TTRUE );
	AAnimatedUnit* pBfa = TSTATICCAST( AAnimatedUnit, AUnitManager::GetSingleton()->SpawnUnit( TPS8( poolbfa ), &oBfaTransform ) );
	m_pBfa              = pBfa->GetSelfProxy();

	pBfa->ChangeSteer( &TGetClass( AStationarySteer ) );
	pBfa->GetAnimatableObject()->SetGlow( TFALSE );
	SetMaterialsFlag( pBfa, TFALSE );

	// HUD
	m_oPowerBar.Create( 0.33f, &m_GUIElement, AHudElement::Mask( AHudElement::MASK_MINIGAME_DEFAULT ) );
	m_oPowerBar.SetValue( 0.0f, TFALSE );

	m_oHud.Create( &m_oPowerBar.GetBarElement(), &m_GUIElement );

	APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();
	m_oHud.SetTeamPlayer( pProfile->GetHideTextureName( -1, APlayerProfile::GENDER_NONE ), pProfile->GetName().GetString( 0 ), 0 );
	m_oHud.SetTeamsVisible( TFALSE );

	ResetGame();

	// NOTE: the original sets a copy of the effect (AGameTimeFX copy constructor 00492d30)
	AGameTimeFXManager* pGameTimeFX = AGameTimeFXManager::GetSingleton();
	pGameTimeFX->SetForcedFX( pGameTimeFX->FindFX( "Pool" ) );

	m_wszOpponentName = ALocaleManager::Interface()->GetString( LOCALE_DUKE );
	m_oHud.SetTeamPlayer( "Duke", m_wszOpponentName, 1 );

	m_bUnkD9B = TTRUE;
	m_bUnkD9A = TTRUE;

	// NOTE: retail calls DOES_NOTHING (nullsub_7) with the main controller and TVector4::VEC_ZERO here
#ifdef BARNYARD_LATEST
	// Latest version: no intro sequence on the small table
	if ( m_iVariant != 5 )
#endif
		ASSeqController::GetSingleton()->LoadScript( "sspool_intro.tso", TVector4::VEC_ZERO );

	// No sky and no sun flare
	ASkyDome* pSkyDome = ARenderer::GetSingleton()->GetSkyDome();
	pSkyDome->SetFlags( pSkyDome->GetFlags() & ~ASkyDome::FLAGS_RENDER );
	pSkyDome->SetFlags( ( pSkyDome->GetFlags() & ~( ASkyDome::FLAGS_FROZEN | ASkyDome::FLAGS_ROTATE ) ) | ASkyDome::FLAGS_FROZEN );
	ASunFlareEffect::GetSingleton()->SetEnabled( TFALSE );

	// The arrow at the end of the aim guide
	m_pArrowEffect = AParticleEffectManager::CloneEffect( AParticleEffectManager::FindEffect( "poolspot_arrow" ) );
	AParticleEmitLine::SetEffectColour( m_pArrowEffect, 0 );
	m_pArrowHandle = AParticleEffectManager::GetSingleton()->SpawnEffect( m_pArrowEffect, TVector4::VEC_ZERO, TVector4::VEC_NEGY );
	m_pArrowHandle->GetInstance()->GetTransform().Scale( 0.3f, 0.3f, 0.3f );

	m_oAimLine.Create( "poolspot" );
}

// $Barnyard: FUNCTION 004d37e0
void ABarnYardPoolMiniGameState::OnRemoval()
{
	DestroyProps();
	AMiniGameState::OnRemoval();

	AUnitManager* pUnitManager = AUnitManager::GetSingleton();

	for ( TINT i = 0; i < NUM_BALLS; i++ )
	{
		if ( m_aBalls[ i ]->GetUnit() )
			pUnitManager->DestroyUnit( m_aBalls[ i ]->GetUnit() );

		m_aBalls[ i ] = TNULL;

		if ( m_aShadows[ i ]->GetUnit() )
			pUnitManager->DestroyUnit( m_aShadows[ i ]->GetUnit() );

		m_aShadows[ i ] = TNULL;
	}

	if ( m_aCue[ 0 ]->GetUnit() )
		pUnitManager->DestroyUnit( m_aCue[ 0 ]->GetUnit() );

	m_aCue[ 0 ] = TNULL;
	m_pCue      = TNULL;

	if ( m_pBfa->GetUnit() )
		pUnitManager->DestroyUnit( m_pBfa->GetUnit() );

	m_pBfa    = TNULL;
	m_pCamera = TNULL;

	AGameLoader::DestroyLevel();
	pUnitManager->RemoveAllUnits();
	pUnitManager->DeleteDyingUnits();
	TRenderInterface::GetSingleton()->FlushDyingResources();
	AGameTimeFXManager::GetSingleton()->ClearForcedFX();

	AInputHandler::GetSingleton()->StopAllRumbles();
	// NOTE: retail calls DOES_NOTHING (00628f40) with 0 here
	AInputHandler::GetSingleton()->StopRumble( AInputHandler::GetSingleton()->GetMainController() );

	// Turn the sky and the sun flare back on
	ASkyDome* pSkyDome = ARenderer::GetSingleton()->GetSkyDome();
	pSkyDome->SetFlags( pSkyDome->GetFlags() | ASkyDome::FLAGS_RENDER );
	pSkyDome->SetFlags( ( pSkyDome->GetFlags() & ~( ASkyDome::FLAGS_FROZEN | ASkyDome::FLAGS_ROTATE ) ) | ASkyDome::FLAGS_ROTATE );
	ASunFlareEffect::GetSingleton()->SetEnabled( TTRUE );

	m_oAimLine.Destroy();
	m_pArrowHandle->GetInstance()->Kill();

	if ( m_pArrowEffect )
	{
		m_pArrowEffect->DeleteClone();
		TFree( m_pArrowEffect );
	}

	ARenderer::GetSingleton()->SetNear( m_fSavedNear );
}

// $Barnyard: FUNCTION 004d39c0
AMiniGameState::S1* ABarnYardPoolMiniGameState::GetResults()
{
	m_oResults.iMiniGameIndex         = AMiniGameManager::GetSingleton()->GetNextMiniGameIndex();
	m_oResults.oPlayerList[ 0 ].bUnk1 = TTRUE;
	m_oResults.oPlayerList[ 1 ].bUnk1 = TTRUE;
	m_oResults.oPlayerList[ 0 ].iUnk2 = 0;
	m_oResults.oPlayerList[ 1 ].iUnk2 = 0;

	TINT iNumPotted      = m_aTeams[ 0 ].GetNumPotted();
	TINT iOpponentPotted = m_aTeams[ 1 ].GetNumPotted();

	m_oResults.iMiniGameVariant = m_iVariant;

	if ( m_iWinner == 6 )
	{
		m_oResults.oPlayerList[ 0 ].iUnk2 = 0;
		m_oResults.oPlayerList[ 1 ].iUnk2 = 0;
	}

	m_oResults.iResult = m_iWinner;
	m_oResults.bFlag1  = TTRUE;
	m_oResults.bWon    = m_iWinner == 0;

	PlayerInfo& rInfo = m_oResults.oPlayerList[ 0 ];

	while ( rInfo.vecRows.Size() != 0 )
	{
		rInfo.vecRows[ rInfo.vecRows.Size() - 1 ] = ResultRow();
		rInfo.vecRows.PopBack();
	}

	// 100 points for every ball potted more than the opponent
	TINT iScore = 100 * ( iNumPotted - iOpponentPotted );
	TMath::Clip( iScore, 0, 1000 );

	const TWCHAR* wszResult = ALocaleManager::Interface()->GetString( LOCALE_RESULT_LOST );

	if ( m_oResults.bWon )
		wszResult = ALocaleManager::Interface()->GetString( LOCALE_RESULT_WON );

	const TWCHAR* wszAccuracy = ALocaleManager::Interface()->GetString( LOCALE_RESULT_ACCURACY );

	// The percentage of the shots that potted a ball
	TINT iAccuracy = 1;

	if ( m_aTeams[ 0 ].iNumShots > 0 )
		iAccuracy = 100 * iNumPotted / m_aTeams[ 0 ].iNumShots;

	TMath::Clip( iAccuracy, 1, 100 );

	m_bSubmitHighScore = TFALSE;

	TINT iTable = -1;

	switch ( AMiniGameManager::GetSingleton()->GetMiniGameVariant() )
	{
		case 0:
		case 2:
			iTable = HIGHSCORE_TABLE_BARN;
			break;
		case 1:
			iTable = HIGHSCORE_TABLE_VARIANT1;
			break;
		case 3:
			iTable = HIGHSCORE_TABLE_VARIANT3;
			break;
		case 4:
			iTable = HIGHSCORE_TABLE_VARIANT4;
			break;
		case 5:
			iTable = HIGHSCORE_TABLE_VARIANT5;
			break;
	}

	if ( m_oResults.bWon )
	{
		// A win is at least worth a D: the accuracy is raised to the limit of the grade
		if ( AHighScoreManager::GetGrade( iTable, TFLOAT( iScore + iAccuracy ) ) == AHighScoreManager::GRADE_F )
			iAccuracy = TINT( TFLOAT( TMath::CeilToInt( AHighScoreManager::GetGradeLimit( iTable, AHighScoreManager::GRADE_D ) ) ) );
	}

	rInfo.vecRows.PushBack( ResultRow{ wszResult, iScore } );
	rInfo.vecRows.PushBack( ResultRow{ wszAccuracy, iAccuracy } );
	m_oResults.ComputeTotal( 0 );

	if ( iTable >= 0 )
		AHighScoreManager::AddScore( iTable, TFLOAT( rInfo.iTotal ), m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );

	return &m_oResults;
}

// $Barnyard: FUNCTION 004d50d0
void ABarnYardPoolMiniGameState::CreateProps()
{
	if ( m_iVariant >= 5 || m_bPropsCreated )
		return;

	m_bPropsCreated = TTRUE;

	AAnimatableObjectManager* pObjectManager = AAnimatableObjectManager::GetSingleton();

	AAnimatableObjectType* apTypes[ NUM_PROPS ];
	apTypes[ 0 ] = pObjectManager->FindType( TPS8( pool_fish ) );
	apTypes[ 1 ] = pObjectManager->FindType( TPS8( pool_float ) );
	apTypes[ 2 ] = pObjectManager->FindType( TPS8( pool_oarlock_left ) );
	apTypes[ 3 ] = pObjectManager->FindType( TPS8( pool_oarlock_right ) );

	TMatrix44 aTransforms[ NUM_PROPS ];

	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
	pTerrain->GetLocatorTransform( "pool_fish", aTransforms[ 0 ] );
	pTerrain->GetLocatorTransform( "pool_float", aTransforms[ 1 ] );
	pTerrain->GetLocatorTransform( "pool_oarlock_left", aTransforms[ 2 ] );
	pTerrain->GetLocatorTransform( "pool_oarlock_right", aTransforms[ 3 ] );

	for ( TINT i = 0; i < NUM_PROPS; i++ )
	{
		m_aProps[ i ].pObject = pObjectManager->CreateAnimatableObject( apTypes[ i ], TNULL, 0 );
		m_aProps[ i ].pObject->GetModelInstance()->SetTransform( aTransforms[ i ] );
		m_aProps[ i ].pObject->SetSkeletonUpdating( TTRUE, TTRUE );
		m_aProps[ i ].pObject->SetVisible( TTRUE );
		m_aProps[ i ].pObject->SetOpacity( 1.0f, TTRUE );
		m_aProps[ i ].pObject->GetModelInstance()->DisableDrawWithDefaultMaterials();
	}
}

// $Barnyard: FUNCTION 004d2920
void ABarnYardPoolMiniGameState::DestroyProps()
{
	if ( m_iVariant >= 5 )
		return;

	m_bPropsCreated = TFALSE;

	AAnimatableObjectManager* pObjectManager = AAnimatableObjectManager::GetSingleton();

	for ( TINT i = 0; i < NUM_PROPS; i++ )
		pObjectManager->DestroyAnimatableObject( m_aProps[ i ].pObject );
}

// $Barnyard: FUNCTION 004d52d0
void ABarnYardPoolMiniGameState::UpdateProps( TFLOAT a_fDeltaTime )
{
	if ( m_iVariant >= 5 )
		return;

	// The props fade out when they are this close to the cue (squared)
	static constexpr TFLOAT s_afFadeDistances[ NUM_PROPS ] = { 0.5f, 0.5f, 0.1f, 0.1f };

	TVector4 vCueStart;
	m_pCue->GetModelInstance()->GetPosition( vCueStart );

	TVector4 vCueEnd;
	vCueEnd.Multiply( m_pCue->m_vecDirection, m_pCue->m_fCueLength );
	vCueEnd.x += vCueStart.x;
	vCueEnd.y += vCueStart.y;
	vCueEnd.z += vCueStart.z;

	for ( TINT i = 0; i < NUM_PROPS; i++ )
	{
		Prop& rProp = m_aProps[ i ];

		TFLOAT fTargetOpacity = 1.0f;

		TVector4 vPropPos;
		rProp.pObject->GetModelInstance()->GetPosition( vPropPos );

		if ( m_pCue->IsVisible() && TMath::DistancePointToSegmentSq( vPropPos, vCueStart.AsVector3(), vCueEnd.AsVector3() ) < s_afFadeDistances[ i ] )
			fTargetOpacity = 0.0f;

		if ( rProp.fTargetOpacity != fTargetOpacity )
		{
			rProp.fTargetOpacity = fTargetOpacity;
			rProp.fTime          = 0.0f;
			rProp.fStartOpacity  = rProp.pObject->GetModelInstance()->GetOpacity();
		}

		rProp.fTime += a_fDeltaTime;
		TMath::Clip( rProp.fTime, 0.0f, 1.0f );

		rProp.pObject->SetOpacity( TMath::LERP( rProp.fStartOpacity, fTargetOpacity, rProp.fTime ), TTRUE );
	}
}
