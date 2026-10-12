#include "pch.h"
#include <Render/TCameraObject.h> // before the Physics headers (Opcode #defines PI)
#include "Physics/ACollisionObject.h"
#include "ACarChaseMiniGameState.h"
#include "AMiniGameManager.h"
#include "ALoadScreen.h"
#include "Input/AInputMapManager.h"
#include "Input/AInputHandler.h"
#include "FX/AParticleEffectManager.h"
#include "FX/AGameTimeFXManager.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "Cameras/ABikePhysicsCameraHelper.h"
#include "GameInterface/AAnticGameOverState.h"
#include "GameInterface/ASSeqState.h"
#include "Units/ASSeqController.h"
#include "Units/ASSeq.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitTransform.h"
#include "Units/AUnitProxy.h"
#include "Units/AUnitCarChaseCar.h"
#include "UnitMovement/Steering/ARacingCarSteer.h"
#include "UnitMovement/Steering/AKidBikeSteer.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerSet.h"
#include "Player/APlayerProfileManager.h"
#include "Helpers/AHighScoreManager.h"
#include "Helpers/APlayerProgress.h"
#include "Helpers/AInstanceManager.h"
#include "Helpers/ATreeManager.h"
#include "Helpers/ARegrowthManager.h"
#include "Render/ARenderer.h"
#include "Render/AShadowManager.h"
#include "Sound/ASoundManager.h"
#include "Sound/AMusicManager.h"
#include "Sound/AMultiplexStream.h"
#include "HUD/AHudManager.h"
#include "Terrain/ATerrainSection.h"
#include "Terrain/ATerrainVIS.h"
#include "Physics/ACollisionManager.h"
#include "Physics/ACollisionObjectMesh.h"
#include "Physics/AVehicleManager.h"
#include "Physics/AVehicleSkidManager.h"
#include "Physics/APhysicsCar.h"

#include <Render/TRenderInterface.h>
#include <Platform/DX8/TRenderInterface_DX8.h>
#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Toshi/TStringManager.h>
#include <Math/TQuaternion.h>
#include <cstddef>
#include "SoundBank/carchase.h"

#ifdef PI
#  undef PI
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( ACarChaseMiniGameState );
TPSTRING8_DECLARE( car_chase_car1 );
TPSTRING8_DECLARE( car_finishline );
TPSTRING8_DECLARE( road_block1 );
TPSTRING8_DECLARE( road_block2 );
TPSTRING8_DECLARE( carstar );
TPSTRING8_DECLARE( car_coin );
TPSTRING8_DECLARE( ride );
TPSTRING8_DECLARE( idle );

// NOTE: GetClass 004ed200, CreateObject 004f0260, CreateObjectInPlace 004f02c0 (the constructor is inlined in both)
TDEFINE_CLASS( ACarChaseMiniGameState );

// Camera of the intro (UpdatePreStart): where the camera was when the start HUD showed up.
// The camera moves back to it during the "Go" stage.
// NOTE: globals in the original (0x007832e8, 0x00783324..0x00783344)
static TBOOL    s_bPreStartCameraSaved = TFALSE;
static TFLOAT   s_fPreStartCameraFOV   = 0.0f;
static TVector4 s_vPreStartCameraDir;
static TVector4 s_vPreStartCameraPos;

static constexpr TFLOAT PRESTART_CAMERA_DISTANCE = 5.0f;       // 0x00773c68
static constexpr TFLOAT PRESTART_CAMERA_SPEED    = 5.0f;       // 0x00773c6c
static constexpr TFLOAT PRESTART_CAMERA_FOV      = 1.3962634f; // 80 degrees

// Offsets of the chase camera (ABikePhysicsCameraHelper), in vehicle space
static const TVector4 CAMERA_OFFSET( 0.0f, -3.0f, -8.0f, 1.0f ); // 0x00773c40
static const TVector4 CAMERA_LOOKAT_OFFSET( 0.0f, -1.5f, 2.0f, 1.0f );
static const TVector4 CAMERA_PIVOT_OFFSET( 0.0f, -3.0f, 0.0f, 1.0f );

// AHighScoreManager record of each track
static constexpr TINT TRACK_HIGHSCORE_RECORDS[] = { 46, 47, 48, 49, 50, 51, 52 };

// Materials of the cars that get the night environment map
static const TCHAR* s_apEnvMapMaterials[] = {
	"BeadyMatte",
	"collision_1",
	"newcop_matte",
	"strawnew",
	"utebod_matte",
};

// VIS group of the terrain while the follow the kids variant loads its sections
// $Barnyard: FUNCTION 004ee2b0
static TINT GetLoadingVISGroup()
{
	return 10;
}

// Destroys the unit of the proxy of a_pUnit
// NOTE: inlined in OnRemoval
static void DestroyCarChaseUnit( AUnit* a_pUnit )
{
	AUnit* pUnit = a_pUnit->GetSelfProxy()->GetUnit();

	if ( pUnit )
		AUnitManager::GetSingleton()->DestroyUnit( pUnit );
}

// Sets up the materials and the scene object of a car model
// NOTE: inlined in OnInsertion
static void SetupCarModel( AUnitCarChaseCar* a_pCar, TBOOL a_bUnlit )
{
	AModelInstanceRef pModelInstance = a_pCar->GetAnimatableObject()->GetModelInstance();

	TSceneObject* pSceneObject = pModelInstance->GetSceneObject();
	pSceneObject->EnableOpacity();
	pSceneObject->GetFadeParams().y = 1.0f;

	if ( a_bUnlit )
		pModelInstance->SetUnlit();

	pModelInstance->SetEnvironmentMap( "Particles\\envmap_small_night.tga", TARRAYSIZE( s_apEnvMapMaterials ), s_apEnvMapMaterials );
}

// $Barnyard: FUNCTION 004eca40
ACarChaseMiniGameState::Racer::Racer()
{
	iNumPoints    = 0;
	pSteer        = TNULL;
	pSpline       = TNULL;
	fUnk674       = 0.0f;
	fFarTime      = 0.0f;
	iUnk00        = 100;
	iMode         = MODE_IDLE;
	iUnk08        = 0;
	fPointSpacing = 4.0f;
	fUnk10        = 20.0f;
	fUnk14        = 5.0f;
	fUnk18        = 2.5f;
	fUnk24        = 5.0f;
	iUnk1c        = 0;
}

// $Barnyard: FUNCTION 004f58f0
ACarChaseMiniGameState::SplineFollower::SplineFollower()
{
	pSpline = TNULL;
	pSteer  = TNULL;
	iUnk24  = 0;
	iUnk1c  = 0;
	vUnk08  = TVector4::VEC_ZERO;
	fUnk18  = 4.0f;
	iUnk28  = 3;
	iUnk2c  = 0;
}

// $Barnyard: FUNCTION 004f5cb0
void ACarChaseMiniGameState::SplineFollower::Init( ASpline* a_pSpline )
{
	pSpline   = a_pSpline;
	iUnk24    = 0;
	iUnk1c    = 0;
	fProgress = 0.0f;
}

// $Barnyard: FUNCTION 004ee250
ACarChaseMiniGameState::RaceTrack::~RaceTrack()
{
	oTRB.Close();
}

// $Barnyard: FUNCTION 004ee3a0
ACarChaseMiniGameState::ACarChaseMiniGameState()
    : m_oViewport( TTRUE )
{
	TSTATICASSERT( offsetof( ACarChaseMiniGameState, m_afData ) == 0xbac );
	TSTATICASSERT( offsetof( ACarChaseMiniGameState, m_aMiniMaps ) == 0xdb0 );
	TSTATICASSERT( offsetof( ACarChaseMiniGameState, m_oDirectionArrow ) == 0x12d0 );
	TSTATICASSERT( offsetof( ACarChaseMiniGameState, m_oFadingText ) == 0x17e4 );
	TSTATICASSERT( offsetof( ACarChaseMiniGameState, m_oTimer ) == 0x1958 );
	TSTATICASSERT( offsetof( ACarChaseMiniGameState, m_apCars ) == 0x1dc8 );
	TSTATICASSERT( offsetof( ACarChaseMiniGameState, m_aRacers ) == 0x1e00 );
	TSTATICASSERT( offsetof( ACarChaseMiniGameState, m_oRaceTrack ) == 0x2b2c );
	TSTATICASSERT( offsetof( ACarChaseMiniGameState, m_oViewport ) == 0x3588 );
	TSTATICASSERT( offsetof( ACarChaseMiniGameState, m_bNoTimeLimit ) == 0x35a0 );
	TSTATICASSERT( offsetof( ACarChaseMiniGameState, m_oIconElement ) == 0x3724 );
	TSTATICASSERT( offsetof( ACarChaseMiniGameState, m_oIconRectangle ) == 0x3778 );
	TSTATICASSERT( offsetof( ACarChaseMiniGameState, m_oListener ) == 0x37f4 );
	TSTATICASSERT( offsetof( ACarChaseMiniGameState, m_pCarStar ) == 0x3808 );
	TSTATICASSERT( offsetof( ACarChaseMiniGameState, m_aCoins ) == 0x3830 );
	TSTATICASSERT( offsetof( ACarChaseMiniGameState, m_oTrackDataTRB ) == 0x3e90 );
	TSTATICASSERT( offsetof( ACarChaseMiniGameState, m_oLifeHud ) == 0x3fd4 );
	TSTATICASSERT( offsetof( ACarChaseMiniGameState, m_iLives ) == 0x437c );

	m_aiMapRect[ 0 ] = 0;
	m_aiMapRect[ 1 ] = 0;
	m_aiMapRect[ 2 ] = 0;
	m_aiMapRect[ 3 ] = 0;

	m_bStartEngines        = TTRUE;
	m_bUnk3818             = TFALSE;
	m_fDistanceWarningTime = 0.0f;
	m_fKidDistance         = 0.0f;
	m_pTrackData           = TNULL;
	m_iLives               = 5;
	m_bTrackLoaded         = TFALSE;
	m_iCue4384             = -1;
	m_fDriverVoiceTimer    = 0.0f;
	m_iCue438c             = -1;
	m_fBikeVoiceTimer      = 0.0f;
	m_eVariant             = 0;
	m_pFinishLine          = TNULL;
	m_pFinishLineCollision = TNULL;
	m_fOutroTime           = 0.0f;
	m_fSpeedScale          = 1.0f;
	m_bRaceRunning         = TFALSE;
	m_pCarStar             = TNULL;
	m_pInputMap            = AInputMapManager::GetSingleton()->FindMap( TPS8( ACarChaseMiniGameState ) );
	m_iUnkc30              = 1;
	m_eTrack               = TRACK_1;
	m_iNumCoinsCollected   = 0;
	m_iNumSplineFollowers  = 0;
	m_bInputEnabled        = TFALSE;

	auto pDisplayParams = TRenderInterface::GetSingleton()->GetCurrentDisplayParams();
	m_oViewport.SetX( 0.0f );
	m_oViewport.SetY( 0.0f );
	m_oViewport.SetWidth( TFLOAT( pDisplayParams->uiWidth ) );
	m_oViewport.SetHeight( TFLOAT( pDisplayParams->uiHeight ) );
	m_oViewport.SetMinZ( 0.2f );
	m_oViewport.SetMaxZ( 1.0f );
	m_oViewport.AllowBackgroundClear( TFALSE );
	m_oViewport.AllowDepthClear( TTRUE );

	m_iNumFinishedCars     = 0;
	m_iLastGear            = 0;
	m_fGearShiftCameraTime = 0.0f;
	m_iCue3824             = -1;
	m_iMilkSlickEffect     = AParticleEffectManager::FindEffect( "carmilkslick" );

	for ( TINT i = 0; i < NUM_COINS; i++ )
	{
		m_aCoins[ i ].pObject  = TNULL;
		m_aCoins[ i ].iMapIcon = -1;
	}

	for ( TINT i = 0; i < CAR_NUMOF; i++ )
	{
		m_apCars[ i ]   = TNULL;
		m_apSteers[ i ] = TNULL;
	}

	for ( TINT i = 0; i < NUM_KID_BIKES; i++ )
	{
		m_apKidBikes[ i ]      = TNULL;
		m_apKidBikeSteers[ i ] = TNULL;
	}
}

// $Barnyard: FUNCTION 004ee740
// $Barnyard: FUNCTION 004f0300
ACarChaseMiniGameState::~ACarChaseMiniGameState()
{
}

// $Barnyard: FUNCTION 004ee9f0
TBOOL ACarChaseMiniGameState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( FindChildState( &TGetClass( AAnticGameOverState ) ) && a_eInputCommand == AInputCommand_Quit )
		return TTRUE;

	if ( FindChildState( &TGetClass( AAnticGameOverState ) ) )
		return TFALSE; // RETURNS_FALSE (006528c0)

	if ( !m_bInputEnabled )
	{
		if ( a_eInputCommand == AInputCommand_ForwardWithUp && a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
			a_pInputEvent->StartRepeat( 0.0f, 0.0f );

		return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
	}

	TBOOL bHandled = TFALSE;
	TINT  iSlot    = APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetPlayerSlotIndexByDevice( a_pInputEvent->GetSource() );

	if ( m_apCars[ iSlot ]->m_iState != AUnitCarChaseCar::STATE_RACING )
		return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	AUnitCarChaseCar*           pCar       = m_apCars[ iSlot ];
	TInputInterface::EVENT_TYPE eEventType = a_pInputEvent->GetEventType();

	switch ( a_eInputCommand )
	{
		case AInputCommand_Move:
			pCar->m_iUnk15C   = STEERCOMMAND_ANALOG;
			pCar->m_fSteering = a_pInputEvent->GetMagnitudeFloat( 0 );
			bHandled          = TTRUE;
			break;
		case AInputCommand_Left:
		case AInputCommand_Right:
		case AInputCommand_ZoomIn:
			bHandled = TTRUE;
			break;
		case AInputCommand_LeftWithUp:
		case AInputCommand_RightWithUp:
		{
			STEERCOMMAND iTurn = ( a_eInputCommand == AInputCommand_LeftWithUp ) ? STEERCOMMAND_LEFT : STEERCOMMAND_RIGHT;

			if ( eEventType == TInputInterface::EVENT_TYPE_GONE_DOWN )
			{
				pCar->m_iUnk160 = iTurn;
				a_pInputEvent->StartRepeat( 0.0f, 0.0f );
			}
			else if ( eEventType == TInputInterface::EVENT_TYPE_REPEAT )
			{
				pCar->m_iUnk160 = iTurn;
			}
			else if ( eEventType == TInputInterface::EVENT_TYPE_GONE_UP )
			{
				pCar->m_iUnk160 = STEERCOMMAND_ANALOG;
			}

			bHandled = TTRUE;
			break;
		}
		case AInputCommand_ForwardWithUp:
		case AInputCommand_BackwardWithUp:
		case AInputCommand_Shoot:
		{
			AUnitCarChaseCar::DRIVESTATE iPedal = AUnitCarChaseCar::DRIVESTATE_UNK7;

			if ( a_eInputCommand == AInputCommand_ForwardWithUp )
				iPedal = AUnitCarChaseCar::DRIVESTATE_ACCELERATE;
			else if ( a_eInputCommand == AInputCommand_BackwardWithUp )
				iPedal = AUnitCarChaseCar::DRIVESTATE_BRAKE;

			if ( eEventType == TInputInterface::EVENT_TYPE_GONE_DOWN )
			{
				pCar->m_iDriveState = iPedal;
				a_pInputEvent->StartRepeat( 0.0f, 0.0f );
			}
			else if ( eEventType == TInputInterface::EVENT_TYPE_REPEAT )
			{
				pCar->m_iDriveState = iPedal;
			}
			else if ( eEventType == TInputInterface::EVENT_TYPE_GONE_UP )
			{
				pCar->m_iDriveState = AUnitCarChaseCar::DRIVESTATE_IDLE;
			}

			bHandled = TTRUE;
			break;
		}
		case AInputCommand_Horn:
			if ( eEventType == TInputInterface::EVENT_TYPE_GONE_DOWN )
			{
				ASoundId eHorn = ( m_apCars[ CAR_PLAYER ]->GetCarType() != AUnitCarChaseCar::CARTYPE_BEADY ) ? soundbank::CARCHASE_HORN : soundbank::CARCHASE_B_HORN;
				m_iCue3824     = ASoundManager::GetSingleton()->PlayCueAtPosition( eHorn, *m_apSteers[ CAR_PLAYER ]->GetPosition() );
				a_pInputEvent->StartRepeat( 0.0f, 0.0f );
				return TTRUE;
			}

			if ( eEventType == TInputInterface::EVENT_TYPE_GONE_UP )
			{
				if ( m_iCue3824 != -1 )
					ASoundManager::GetSingleton()->StopCue( m_iCue3824 );

				return TTRUE;
			}

			break;
		default:
			break;
	}

	// Apply the inputs to the car
	if ( pCar->m_iUnk160 != STEERCOMMAND_ANALOG )
		DriveCar( m_apSteers[ iSlot ], pCar->m_iDriveState, pCar->m_iUnk160, pCar->m_fSteering, 90.0f, TTRUE );
	else
		DriveCar( m_apSteers[ iSlot ], pCar->m_iDriveState, pCar->m_iUnk15C, pCar->m_fSteering, 90.0f, TTRUE );

	ACameraHelper* pCameraHelper = ACameraManager::GetSingleton()->GetCurrentCameraHelper();

	if ( pCameraHelper && pCameraHelper->ProcessInputCommand( TREINTERPRETCAST( void*, TUINTPTR( a_eInputCommand ) ), const_cast<TInputInterface::InputEvent*>( a_pInputEvent ) ) )
		return TTRUE;

	if ( !bHandled )
		bHandled = AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	return bHandled;
}

// $Barnyard: FUNCTION 004ee730
void ACarChaseMiniGameState::OnRender( TFLOAT a_fDeltaTime )
{
	AVehicleSkidManager::GetSingleton()->Render();
}

// $Barnyard: FUNCTION 004ed210
void ACarChaseMiniGameState::OnRenderOverlay( TFLOAT a_fDeltaTime )
{
	if ( m_bIsActivated )
		TRenderD3DInterface::FlushShaders();
}

// $Barnyard: FUNCTION 004ee340
void ACarChaseMiniGameState::OnPause()
{
	if ( m_apCars[ CAR_PLAYER ] )
		m_apCars[ CAR_PLAYER ]->SetVisible( TTRUE );

	if ( m_apCars[ CAR_FOLLOWER ] )
		m_apCars[ CAR_FOLLOWER ]->SetVisible( TTRUE );

	if ( m_eVariant == 4 )
	{
		for ( TINT i = 0; i < NUM_KID_BIKES; i++ )
		{
			if ( m_apKidBikes[ i ] )
				TSTATICCAST( AVisibleUnit, m_apKidBikes[ i ] )->SetVisible( TTRUE );
		}
	}
}

// $Barnyard: FUNCTION 004ee8d0
void ACarChaseMiniGameState::OnActivate()
{
	if ( m_bStartHudWasHidden == TTRUE && m_eState != STATE_RESULTS )
		SetHUDVisible( TTRUE );

	AInputHandler::GetSingleton()->SetRumblePaused( TFALSE );
	AMultiplexStream::GetSingleton()->Pause( TFALSE );

	if ( m_apCars[ CAR_PLAYER ] )
		m_apCars[ CAR_PLAYER ]->PauseSounds( TFALSE );

	for ( TINT i = CAR_RACER1; i <= CAR_RACER2; i++ )
	{
		if ( m_apCars[ i ] )
			m_apCars[ i ]->PauseSounds( TFALSE );
	}

	if ( m_apCars[ CAR_FOLLOWER ] )
		m_apCars[ CAR_FOLLOWER ]->PauseSounds( TFALSE );

	AMiniGameState::OnActivate();
}

// $Barnyard: FUNCTION 004ee960
void ACarChaseMiniGameState::OnDeactivate()
{
	AInputHandler::GetSingleton()->SetRumblePaused( TTRUE );

	if ( m_oIconElement.IsVisible() )
		m_oIconElement.Hide();

	SetHUDVisible( TFALSE );
	AMultiplexStream::GetSingleton()->Pause( TTRUE );

	if ( m_apCars[ CAR_PLAYER ] )
		m_apCars[ CAR_PLAYER ]->PauseSounds( TTRUE );

	for ( TINT i = CAR_RACER1; i <= CAR_RACER2; i++ )
	{
		if ( m_apCars[ i ] )
			m_apCars[ i ]->PauseSounds( TTRUE );
	}

	if ( m_apCars[ CAR_FOLLOWER ] )
		m_apCars[ CAR_FOLLOWER ]->PauseSounds( TTRUE );

	AMiniGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 004f1ed0
void ACarChaseMiniGameState::Pause()
{
	AAnimatableObjectManager::GetSingleton()->Activate( TTRUE );

	if ( m_eVariant != 4 && m_eVariant != 7 )
	{
		AMiniGameState::Pause();
		return;
	}

	// Follow the kids: load the terrain sections and play the intro sequence
	{
		TPString8 strPicture;
		g_oLoadScreen.SetPicture( AMiniGameManager::GetSingleton()->GetLoadingScreenName( strPicture, AMiniGameManager::GetSingleton()->GetNextMiniGameIndex() ), TFALSE );
	}

	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
	pTerrain->SetIsLoaded( TFALSE );
	pTerrain->SetGetCurrentVISGroupCallback( GetLoadingVISGroup );
	pTerrain->Update();
	pTerrain->SetIsLoaded( TTRUE );

	if ( !pTerrain->IsLoaded() && !g_oLoadScreen.IsLoadingScreen() )
		g_oLoadScreen.SetLoadingState( TTRUE, TTRUE );

	AGameLoader::Terrain_Load();
	g_oLoadScreen.Reset();
	g_oLoadScreen.SetLoadingState( TFALSE, TTRUE );

	UpdatePreStart( 0.0f );

	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "SSFTK_FollowTheKids", "SSFTK_FollowTheKids", "SSFTK_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
	);
}

// $Barnyard: FUNCTION 004effb0
void ACarChaseMiniGameState::PlayLoseSequence()
{
	AAnimatableObjectManager::GetSingleton()->Activate( TTRUE );

	if ( m_eVariant != 4 )
	{
		AGameStateController::GetSingleton()->PushState(
		    new ASSeqState( "sscop_defeat", "sscop_defeat", "SSCOP_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
		);
	}
	else
	{
		AGameStateController::GetSingleton()->PushState(
		    new ASSeqState( "SSFTK_Lose", "SSFTK_Lose", "SSFTK_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
		);
	}
}

// $Barnyard: FUNCTION 004f00a0
void ACarChaseMiniGameState::PlayWinSequence()
{
	AAnimatableObjectManager::GetSingleton()->Activate( TTRUE );

	if ( m_eVariant != 4 )
	{
		AGameStateController::GetSingleton()->PushState(
		    new ASSeqState( "sscop_victory", "sscop_victory", "SSCOP_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
		);
	}

	// NOTE: the follow the kids variant calls DOES_NOTHING (005acd80) instead
}

// $Barnyard: FUNCTION 004ee2c0
void ACarChaseMiniGameState::OnPreStart()
{
	if ( m_apCars[ CAR_PLAYER ] )
		m_apCars[ CAR_PLAYER ]->SetVisible( TFALSE );

	for ( TINT i = CAR_RACER1; i <= CAR_RACER2; i++ )
	{
		if ( m_apCars[ i ] )
			m_apCars[ i ]->SetVisible( TFALSE );
	}

	if ( m_apCars[ CAR_FOLLOWER ] )
		m_apCars[ CAR_FOLLOWER ]->SetVisible( TFALSE );

	for ( TINT i = 0; i < NUM_KID_BIKES; i++ )
	{
		if ( m_apKidBikes[ i ] )
			TSTATICCAST( AVisibleUnit, m_apKidBikes[ i ] )->SetVisible( TFALSE );
	}
}

// $Barnyard: FUNCTION 004f0160
void ACarChaseMiniGameState::UpdateAlways( TFLOAT a_fDeltaTime )
{
	if ( m_bRaceRunning != TTRUE )
		return;

	if ( m_bStartEngines == TTRUE )
	{
		// Start the engines once
		for ( TINT i = m_iNumPlayers - 1; i >= 0; i-- )
		{
			if ( m_apCars[ i ] )
				m_apCars[ i ]->StartEngine();
		}

		for ( TINT i = CAR_RACER2; i >= CAR_RACER1; i-- )
		{
			if ( m_apCars[ i ] )
				m_apCars[ i ]->StartEngine();
		}

		if ( m_apCars[ CAR_FOLLOWER ] )
			m_apCars[ CAR_FOLLOWER ]->StartEngine();

		m_bStartEngines = TFALSE;
	}

	AMultiplexStream* pStream = AMultiplexStream::GetSingleton();

	if ( m_bRaceRunning == TTRUE && !FindChildState( &TGetClass( AAnticGameOverState ) ) && pStream->GetState() != AMultiplexStream::STATE_PLAYING )
	{
		pStream->Play( 1.0f );
		pStream->SetVolume( 1.0f, 0, TTRUE );
		pStream->SetVolume( 1.0f, 1, TTRUE );
		pStream->SetVolume( 0.0f, 2, TTRUE );
		pStream->SetVolume( 0.0f, 3, TTRUE );
	}
}

// Makes the camera look at a_rcTargetDir from a_rcPos, rotating it from its current direction by a_fProgress
// NOTE: inlined twice in UpdatePreStart
static void RotatePreStartCamera( ACamera* a_pCamera, const TVector4& a_rcPos, const TVector4& a_rcTargetDir, TFLOAT a_fProgress )
{
	TVector4 vCurrentDir;
	a_pCamera->GetDirection( vCurrentDir );
	vCurrentDir.w = 1.0f;

	TMatrix44 matCurrent;
	matCurrent.LookAtDirection( vCurrentDir, TVector4::VEC_NEGY );

	TMatrix44 matTarget;
	matTarget.LookAtDirection( a_rcTargetDir, TVector4::VEC_NEGY );

	TQuaternion quatCurrent;
	TQuaternion quatTarget;
	TMatrix44::MatToQuat( quatCurrent, matCurrent );
	TMatrix44::MatToQuat( quatTarget, matTarget );

	TQuaternion quatResult;
	TQuaternion::Slerp( quatResult, quatCurrent, quatTarget, a_fProgress );

	TVector4 vNewDir;
	RotateVector( vNewDir, quatResult, TVector4::VEC_POSZ );
	vNewDir.Normalise();

	a_pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ) = a_rcPos;
	a_pCamera->LookAtDirection( vNewDir );
}

// $Barnyard: FUNCTION 004eee00
void ACarChaseMiniGameState::UpdatePreStart( TFLOAT a_fDeltaTime )
{
	ACameraManager* pCameraManager = ACameraManager::GetSingleton();
	ACamera*        pCamera        = pCameraManager->GetCurrentCamera();

	if ( !s_bPreStartCameraSaved )
	{
		m_apCameraHelpers[ 0 ] = pCameraManager->GetCurrentCameraHelper();
		m_apCameraHelpers[ 0 ]->OnUpdate( 0.0f );

		s_vPreStartCameraPos = pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );
		pCamera->GetDirection( s_vPreStartCameraDir );
		s_fPreStartCameraFOV   = PRESTART_CAMERA_FOV;
		s_vPreStartCameraDir.w = 1.0f;
		s_bPreStartCameraSaved = TTRUE;

		pCameraManager->DetachCameraHelpers();
	}

	pCamera->SetFOV( s_fPreStartCameraFOV );

	ASteer*  pPlayerSteer = m_apCars[ CAR_PLAYER ]->GetSteer();
	TVector4 vCarPos;
	pPlayerSteer->CopyPosition( vCarPos );
	vCarPos.y -= 2.0f;

	TVector4 vCarDir;
	pPlayerSteer->CopyDirection( vCarDir );
	vCarDir.y = 0.0f;
	vCarDir.Normalise();

	switch ( m_oStartHudElement.m_iImageIndex )
	{
		case AStartMiniGameHudElement::STAGE_READY:
		{
			// In front of the car, looking at it
			TVector4 vOffset;
			vOffset.Multiply( vCarDir, PRESTART_CAMERA_DISTANCE );

			TVector4 vPos;
			vPos.Add( vCarPos, vOffset );
			pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ) = vPos;

			vCarDir.x = -vCarDir.x;
			vCarDir.y = -vCarDir.y;
			vCarDir.z = -vCarDir.z;
			pCamera->LookAtDirection( vCarDir );
			break;
		}
		case AStartMiniGameHudElement::STAGE_SET:
		{
			// Move to the side of the car
			vCarDir.AsVector3().RotateY( TMath::HALF_PI );

			TVector4 vOffset;
			vOffset.Multiply( vCarDir, PRESTART_CAMERA_DISTANCE );

			TVector4 vTargetPos;
			vTargetPos.Substract( vCarPos, vOffset );

			TVector4 vPos = pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );
			vPos.Lerp4( vPos, vTargetPos, PRESTART_CAMERA_SPEED * a_fDeltaTime );

			RotatePreStartCamera( pCamera, vPos, vCarDir, PRESTART_CAMERA_SPEED * a_fDeltaTime );
			break;
		}
		case AStartMiniGameHudElement::STAGE_GO:
		{
			// Back to where the camera was
			TVector4 vPos = pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );
			vPos.Lerp4( vPos, s_vPreStartCameraPos, PRESTART_CAMERA_SPEED * a_fDeltaTime );

			RotatePreStartCamera( pCamera, vPos, s_vPreStartCameraDir, PRESTART_CAMERA_SPEED * a_fDeltaTime );
			break;
		}
		default:
			break;
	}
}

// $Barnyard: FUNCTION 004ef490
void ACarChaseMiniGameState::OnStartHudFinished()
{
	AMiniGameState::OnStartHudFinished();

	if ( m_eTrack == TRACK_FOLLOW_KIDS )
	{
		m_oTimer.SetVisible( TFALSE );
		m_oLifeHud.SetVisible( TFALSE );
	}

	if ( m_eTrack != TRACK_COINS )
		m_oDirectionArrow.ShowArrow();

	m_oSpeedometer.SetVisible( TTRUE );

	if ( m_eTrack == TRACK_FOLLOW_KIDS )
		m_oTargetIndicator.SetVisible( TTRUE );

	SetupCamera();

	ACameraManager::GetSingleton()->GetCurrentCameraHelper()->ResetPosition( TTRUE );

	for ( TINT i = m_iNumPlayers - 1; i >= 0; i-- )
	{
		APhysicsCar* pPhysics            = m_apSteers[ i ]->GetCar();
		pPhysics->m_oGearBox.bAllowDrive = TTRUE;

		if ( pPhysics->m_fThrottle > 0.5f )
			ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::CARCHASE_SKID_LONG, *m_apSteers[ i ]->GetPosition() );

		pPhysics->SetThrottle( 0.0f );
	}

	if ( m_eTrack == TRACK_FOLLOW_KIDS )
	{
		for ( TINT i = 0; i < NUM_KID_BIKES; i++ )
		{
			TSTATICCAST( AVisibleUnit, m_apKidBikes[ i ] )->SetVisible( TTRUE );
			TSTATICCAST( AAnimatedUnit, m_apKidBikes[ i ] )->GetAnimatableObject()->PlayAnimation( TPS8( ride ) );
			m_apKidBikeSteers[ i ]->SetSpeed( 20.0f );
		}
	}

	if ( m_eTrack < TRACK_FOLLOW_KIDS )
		m_oRacePlacingHud.SetForceHide( TFALSE );

	if ( m_apSteers[ CAR_FOLLOWER ] )
		m_apSteers[ CAR_FOLLOWER ]->GetCar()->SetThrottle( 1.0f );

	m_oFadingText.Show();

	if ( m_eTrack != TRACK_FOLLOW_KIDS )
		m_oTimer.SetVisible( TTRUE );

	if ( m_eTrack == TRACK_COINS )
		m_oDirectionArrow.HideArrow();
	else
		m_oDirectionArrow.ShowArrow();

	for ( TINT i = m_iNumPlayers - 1; i >= 0; i-- )
		m_aMiniMaps[ i ].Show();

	m_oSpeedometer.SetVisible( TTRUE );
	m_oTargetIndicator.SetVisible( m_eTrack == TRACK_FOLLOW_KIDS );

	if ( m_eTrack == TRACK_COINS )
		m_oPickUpHudElement.SetForceHide( TFALSE );
	else if ( m_eTrack < TRACK_FOLLOW_KIDS )
		m_oRacePlacingHud.SetForceHide( TFALSE );

	if ( m_eTrack != TRACK_COINS && m_eTrack != TRACK_FOLLOW_KIDS )
		m_oLifeHud.SetVisible( TTRUE );

	m_bStartHudWasHidden = TTRUE;
}

// Attaches the chase camera to the player car.
// NOTE: inlined in OnInsertion, OnStartHudFinished and SetupLevel
void ACarChaseMiniGameState::SetupCamera()
{
	ACameraManager::GetSingleton()->SetCameraHelper( ACameraManager::CAMERAHELPER_BIKE );
	m_apCameraHelpers[ 0 ] = ACameraManager::GetSingleton()->GetCurrentCameraHelper();
	m_apCameraHelpers[ 0 ]->SetTargetUnit( m_apCars[ CAR_PLAYER ] );

	ABikePhysicsCameraHelper* pCameraHelper = TSTATICCAST( ABikePhysicsCameraHelper, m_apCameraHelpers[ 0 ] );
	pCameraHelper->SetCameraOffset( CAMERA_OFFSET );
	pCameraHelper->SetLookAtOffset( CAMERA_LOOKAT_OFFSET );
	pCameraHelper->SetPivotOffset( CAMERA_PIVOT_OFFSET );
}

// $Barnyard: FUNCTION 004f0b70
AMiniGameState::S1* ACarChaseMiniGameState::GetResults()
{
	m_oResults.iMiniGameIndex = AMiniGameManager::GetSingleton()->GetNextMiniGameIndex();

	for ( TINT i = m_iNumPlayers - 1; i >= 0; i-- )
	{
		m_oResults.oPlayerList[ i ].bUnk1 = TTRUE;
		m_oResults.oPlayerList[ i ].iUnk2 = 0;
	}

	AUnitCarChaseCar* pPlayerCar = m_apCars[ CAR_PLAYER ];

	if ( pPlayerCar->m_bUnk18C )
	{
		m_oResults.oPlayerList[ 0 ].iUnk2 = pPlayerCar->GetUnkB0();
		m_oResults.iResult                = 0;
	}
	else
	{
		m_oResults.oPlayerList[ 0 ].iUnk2 = 0;
		m_oResults.iResult                = -1;
	}

	m_oResults.bWon = m_oResults.iResult == 0;

	PlayerInfo& rInfo = m_oResults.oPlayerList[ 0 ];

	while ( rInfo.vecRows.Size() != 0 )
	{
		rInfo.vecRows[ rInfo.vecRows.Size() - 1 ] = ResultRow();
		rInfo.vecRows.PopBack();
	}

	if ( m_eTrack == TRACK_COINS )
	{
		// Coins
		rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( 274 ), m_iNumCoinsCollected } );
		m_oResults.ComputeTotal( 0 );

		m_oResults.bWon   = m_iNumCoinsCollected + 10 * TINT( m_afFinishTimes[ 0 ] ) > 0;
		m_oResults.bFlag5 = TFALSE;
	}
	else
	{
		m_oResults.bFlag5 = TTRUE;
	}

	if ( m_bSubmitHighScore )
	{
		m_bSubmitHighScore = TFALSE;

		if ( m_eTrack >= 0 && m_eTrack < TARRAYSIZE( TRACK_HIGHSCORE_RECORDS ) )
			AHighScoreManager::AddScore( TRACK_HIGHSCORE_RECORDS[ m_eTrack ], TFLOAT( rInfo.iTotal ), m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
	}

	return &m_oResults;
}

// $Barnyard: FUNCTION 004efdd0
TBOOL ACarChaseMiniGameState::ShowGameOver( TINT a_iUnused, const TINT* a_pValue )
{
	// NOTE: UpdateGame, the only caller, always passes a pointer to 0
	if ( *a_pValue != 0 )
		return TTRUE;

	m_eState = STATE_RESULTS;
	SetHUDVisible( TFALSE );
	ACollisionManager::GetSingleton()->SetVehiclesUpdateEnabled( TFALSE );

	GetResults();

	for ( TINT i = m_iNumPlayers - 1; i >= 0; i-- )
	{
		if ( m_apCars[ i ] )
			m_apCars[ i ]->Reset();
	}

	for ( TINT i = CAR_RACER2; i >= CAR_RACER1; i-- )
	{
		if ( m_apCars[ i ] )
			m_apCars[ i ]->Reset();
	}

	if ( m_apCars[ CAR_FOLLOWER ] )
		m_apCars[ CAR_FOLLOWER ]->Reset();

	m_oListener.Disconnect();

	APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();

	if ( ( m_eVariant == 4 && AGameLoader::IsStoryMode() == TTRUE && pProfile->GetSavedAct() == 3 && pProfile->GetSavedTier() == 11 ) || m_eVariant == 7 )
	{
		if ( m_oResults.bWon == TTRUE )
		{
			// The story race has been won, go on with the story instead of showing the results
			m_oResults.bWon    = TFALSE;
			m_oResults.iResult = -1;
			AGameLoader::SetTransitionToScriptedSequence( 7 );
			return TTRUE;
		}
	}

	m_bRaceRunning = TFALSE;
	AMultiplexStream::GetSingleton()->Stop();
	AMusicManager::GetSingleton()->SetEnabled( TTRUE, TTRUE );

	AddChildState( new AAnticGameOverState( &m_oResults, TFALSE ) );
	return TTRUE;
}

// $Barnyard: FUNCTION 004ee080
void ACarChaseMiniGameState::LoadData()
{
	TTRB oTRB;
	oTRB.Load( "Data\\MinigameData\\carchasedata\\carchase.trb", 0 );

	TUtil::MemCopy( m_afData, oTRB.GetSymbolAddress( "Floats" ), sizeof( m_afData ) );
	m_iData = *TSTATICCAST( TINT, oTRB.GetSymbolAddress( "Ints" ) );

	oTRB.Close();
}

// $Barnyard: FUNCTION 004ee140
void ACarChaseMiniGameState::SetHUDVisible( TBOOL a_bVisible )
{
	if ( a_bVisible )
		m_oFadingText.Show();
	else
		m_oFadingText.Hide();

	if ( m_eTrack != TRACK_FOLLOW_KIDS )
		m_oTimer.SetVisible( a_bVisible );

	if ( a_bVisible != TTRUE || m_eTrack == TRACK_COINS )
		m_oDirectionArrow.HideArrow();
	else
		m_oDirectionArrow.ShowArrow();

	for ( TINT i = m_iNumPlayers - 1; i >= 0; i-- )
		m_aMiniMaps[ i ].SetVisible( a_bVisible == TTRUE );

	m_oSpeedometer.SetVisible( a_bVisible );

	if ( m_eTrack == TRACK_FOLLOW_KIDS )
		m_oTargetIndicator.SetVisible( a_bVisible );
	else
		m_oTargetIndicator.SetVisible( TFALSE );

	if ( m_eTrack == TRACK_COINS )
		m_oPickUpHudElement.SetForceHide( !a_bVisible );
	else if ( m_eTrack < TRACK_FOLLOW_KIDS )
		m_oRacePlacingHud.SetForceHide( !a_bVisible );

	if ( m_eTrack != TRACK_COINS && m_eTrack != TRACK_FOLLOW_KIDS )
		m_oLifeHud.SetVisible( a_bVisible );
}

// $Barnyard: FUNCTION 004ed300
void ACarChaseMiniGameState::StartKidBikes()
{
	TFLOAT fInvLength = 1.0f / m_oRaceTrack.oSpline.GetLength();

	// Start positions of the bikes along the track
	TFLOAT afStart[ NUM_KID_BIKES ];
	afStart[ 0 ] = 50.0f * fInvLength;
	afStart[ 1 ] = 47.0f * fInvLength;
	afStart[ 2 ] = fInvLength * 48.0f;

	for ( TINT i = 0; i < NUM_KID_BIKES; i++ )
	{
		m_apKidBikeSteers[ i ]->SetSpline( &m_oRaceTrack.oSpline );
		m_apKidBikeSteers[ i ]->SetProgress( afStart[ i ] );
		m_apKidBikeSteers[ i ]->SetSpeed( 0.0f );

		AAnimatedUnit* pBike = TSTATICCAST( AAnimatedUnit, m_apKidBikes[ i ] );
		pBike->SetVisible( TTRUE );
		pBike->GetAnimatableObject()->PlayAnimation( TPS8( idle ) );
		pBike->GetModelInstance()->GetSceneObject()->DisableDistanceCulling();
	}
}

// $Barnyard: FUNCTION 004f11d0
void ACarChaseMiniGameState::CreateCoins()
{
	AAnimatableObjectType* pCoinType = AAnimatableObjectManager::GetSingleton()->FindType( TPS8( car_coin ) );

	for ( TINT i = 0; i < NUM_COINS; i++ )
	{
		TCHAR szLocator[ 32 ];

		if ( i != 0 && i < 9 )
			TStringManager::String8Format( szLocator, sizeof( szLocator ), "coin_0%d", i + 1 );
		else
			TStringManager::String8Format( szLocator, sizeof( szLocator ), "coin_%d", i + 1 );

		TVector4    vPosition;
		TQuaternion quatRotation;
		ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( szLocator, vPosition, quatRotation );
		vPosition.y -= 2.0f;

		Coin& rCoin = m_aCoins[ i ];

		if ( !rCoin.pObject )
			rCoin.pObject = AAnimatableObjectManager::GetSingleton()->CreateAnimatableObject( pCoinType, TNULL, 0 );

		rCoin.pObject->GetModelInstance()->SetTransform( vPosition, *TREINTERPRETCAST( TVector4*, &quatRotation ) );

		TMatrix44 matTransform;
		rCoin.pObject->GetModelInstance()->GetLocalTransform( matTransform );
		matTransform.Scale( 2.0f, 2.0f, 2.0f );
		rCoin.pObject->GetModelInstance()->SetTransform( matTransform );

		rCoin.pObject->SetSkeletonUpdating( TTRUE, TTRUE );
		AShadowManager::GetSingleton()->AddModelShadow( rCoin.pObject->GetModelInstance(), 1.0f, -1, -0.1f, AShadowManager::FLAGS( 32 ), AShadowManager::TYPE( 0 ) );
		AShadowManager::GetSingleton()->SetShadowStatic( rCoin.pObject->GetModelInstance(), TTRUE );

		rCoin.iMapIcon = m_aMiniMaps[ 0 ].AddIcon( vPosition, TVector4::VEC_POSY, AGUI2MiniMap::ICONTYPE_EDGE, AGUI2MiniMap::ICON_CHALLENGE, 0.1f );
		m_aMiniMaps[ 0 ].SetIconVisible( rCoin.iMapIcon, TTRUE );
		rCoin.bCollected = TFALSE;
	}
}

// $Barnyard: FUNCTION 004f17b0
void ACarChaseMiniGameState::LoadTrack()
{
	// Road block locators of each track, the lists end with an empty string
	static const TCHAR* s_apTrack1RoadBlocks[] = { "21_00", "2_00", "2_01", "3_00", "" };
	static const TCHAR* s_apTrack2RoadBlocks[] = { "8_00", "8_02", "2_03", "2_02", "3_02", "10_01", "" };
	static const TCHAR* s_apTrack3RoadBlocks[] = { "21_00", "8_00", "8_02", "" };
	static const TCHAR* s_apTrack4RoadBlocks[] = { "2_03", "2_02", "3_02", "" };
	static const TCHAR* s_apTrack5RoadBlocks[] = { "3_01", "3_02", "10_01", "8_01", "8_03", "19_00", "21_02", "" };

	m_iNumRoadBlocks = 0;

	for ( TINT i = 0; i < NUM_BARRIERS; i++ )
		m_apBarriers[ i ]->SetSkeletonUpdating( TFALSE, TTRUE );

	const TCHAR** ppRoadBlocks;
	const TCHAR*  szSpline;

	switch ( m_eTrack )
	{
		case TRACK_1:
			ppRoadBlocks          = s_apTrack1RoadBlocks;
			m_iNumSplineFollowers = 1;
			m_iUnkc98             = 0;
			m_fTimeLimit          = 100.0f;
			szSpline              = "track01_spline";
			break;
		case TRACK_2:
			ppRoadBlocks          = s_apTrack2RoadBlocks;
			m_iNumSplineFollowers = 1;
			m_iUnkc98             = 4;
			m_fTimeLimit          = 300.0f;
			szSpline              = "track02_spline";
			break;
		case TRACK_3:
			ppRoadBlocks          = s_apTrack3RoadBlocks;
			m_iNumSplineFollowers = 1;
			m_iUnkc98             = 6;
			m_fTimeLimit          = 300.0f;
			szSpline              = "track03_spline";
			break;
		case TRACK_4:
			ppRoadBlocks          = s_apTrack4RoadBlocks;
			m_iNumSplineFollowers = 1;
			m_iUnkc98             = 8;
			m_fTimeLimit          = 300.0f;
			szSpline              = "track04_spline";
			break;
		case TRACK_FOLLOW_KIDS:
			ppRoadBlocks          = s_apTrack5RoadBlocks;
			m_iNumSplineFollowers = 0;
			m_iUnkc98             = 0;
			m_fTimeLimit          = 300.0f;
			szSpline              = "track05_spline";
			break;
		case TRACK_STAR:
		case TRACK_COINS:
			ppRoadBlocks          = s_apTrack1RoadBlocks;
			m_iUnkc98             = 0;
			m_iNumSplineFollowers = 0;
			m_fTimeLimit          = 300.0f;
			szSpline              = "track01_spline";
			break;
		default:
			ppRoadBlocks          = s_apTrack1RoadBlocks;
			m_iUnkc98             = 2;
			m_eTrack              = TRACK_1;
			m_iNumSplineFollowers = 0;
			m_fTimeLimit          = 300.0f;
			szSpline              = "track01_spline";
			break;
	}

	m_oRaceTrack.oTRB.Close();
	m_oRaceTrack.oTRB.Load( "Data\\MiniGameData\\carchasedata\\car_race.trb", 0 );
	m_oRaceTrack.oSpline.Load( m_oRaceTrack.oTRB, szSpline );
	m_oRaceTrack.oTRB.DeleteSymbolTable();

	// The barriers used by the track
	switch ( m_eTrack )
	{
		case TRACK_3:
			m_apBarriers[ 0 ]->SetSkeletonUpdating( TTRUE, TTRUE );
			m_apBarriers[ 1 ]->SetSkeletonUpdating( TTRUE, TTRUE );
			m_apBarriers[ 2 ]->SetSkeletonUpdating( TTRUE, TTRUE );
			break;
		case TRACK_4:
			m_apBarriers[ 3 ]->SetSkeletonUpdating( TTRUE, TTRUE );
			m_apBarriers[ 5 ]->SetSkeletonUpdating( TTRUE, TTRUE );
			break;
		case TRACK_FOLLOW_KIDS:
			m_apBarriers[ 4 ]->SetSkeletonUpdating( TTRUE, TTRUE );
			break;
		default:
			break;
	}

	// The finish line is at the end of the track
	m_oRaceTrack.oSpline.GetPosition( 1.0f, m_vFinishPosition );

	if ( m_bNoTimeLimit )
		m_fTimeLimit = 99999.0f;

#ifdef BARNYARD_LATEST
	// Latest version (Wii 8023b7fc): more time on Easy
	if ( APlayerProgress::GetSingleton()->IsEasyDifficulty() )
		m_fTimeLimit *= 1.1f;
#endif

	if ( m_eTrack == TRACK_COINS )
	{
		m_iNumRoadBlocks = 0;
	}
	else
	{
		// Park every road block far away
		for ( TINT i = MAX_ROAD_BLOCKS - 1; i >= 0; i-- )
		{
			m_apRoadBlocks[ i ]->GetModelInstance()->SetTranslation( m_vParkingPos.AsVector3() );
			m_apRoadBlocks[ i ]->GetModelInstance()->SetSkeletonUpdating( TFALSE );
		}

		// And place the ones of the track
		TINT i = 0;

		do
		{
			if ( TStringManager::String8Compare( ppRoadBlocks[ i ], "", -1 ) == 0 )
				break;

			TCHAR szLocator[ 64 ];
			TStringManager::String8Format( szLocator, sizeof( szLocator ), "roadblock_%s", ppRoadBlocks[ i ] );

			TVector4    vPosition;
			TQuaternion quatRotation;
			ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( szLocator, vPosition, quatRotation );

			m_apRoadBlocks[ i ]->GetModelInstance()->SetTransform( vPosition, *TREINTERPRETCAST( TVector4*, &quatRotation ) );
			m_apRoadBlocks[ i ]->GetModelInstance()->SetSkeletonUpdating( TTRUE );
			m_apRoadBlockCollisions[ i ]->Create( m_apRoadBlocks[ i ]->GetModelInstance()->GetSceneObject() );

			i++;
		} while ( i < MAX_ROAD_BLOCKS );

		m_iNumRoadBlocks = i;
	}

	if ( m_eTrack < TRACK_FOLLOW_KIDS )
	{
		TCHAR szFileName[ 128 ];
		TStringManager::String8Format( szFileName, sizeof( szFileName ), "Data/MiniGameData/carchasedata/hotlap%d.trb", m_eTrack + 1 );

		if ( !m_bTrackLoaded )
		{
			m_oTrackDataTRB.Load( szFileName, 0 );
			m_pTrackData   = m_oTrackDataTRB.GetSymbolAddress( "Main" );
			m_bTrackLoaded = TTRUE;
		}
	}
}

// Places the car of a_pSteer at a_rcPosition facing a_rcDirection, stopped
// NOTE: inlined in SetupLevel
static void ResetCarSteer( ARacingCarSteer* a_pSteer, const TVector4& a_rcPosition, const TVector4& a_rcDirection )
{
	a_pSteer->Reset( a_rcPosition, a_rcDirection );
	a_pSteer->SetVelocity( TVector4::VEC_ZERO );
	a_pSteer->SetPedal( ARacingCarSteer::PEDAL_NONE, TFALSE );
	a_pSteer->SetSteering( ARacingCarSteer::STEER_ABSOLUTE, 0.0f, 0.0f );
}

// Position and direction (the negated locator rotation) of the start locator a_szLocator,
// moved 10 along the track when a_bOffset is set and put on the ground
// NOTE: inlined in SetupLevel
static void GetStartTransform( const TCHAR* a_szLocator, ASpline& a_rSpline, TBOOL a_bOffset, TVector4& a_rOutPosition, TVector4& a_rOutDirection )
{
	TQuaternion quatRotation;
	ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( a_szLocator, a_rOutPosition, quatRotation );

	TVector4 vTrackDir;
	a_rSpline.GetDirection( 0.0f, vTrackDir );

	if ( a_bOffset )
	{
		TVector4 vOffset;
		vOffset.Multiply( vTrackDir, 10.0f );
		a_rOutPosition.x += vOffset.x;
		a_rOutPosition.y += vOffset.y;
		a_rOutPosition.z += vOffset.z;
	}

	a_rOutPosition.y = ATerrainInterface::GetSingleton()->GetGroundHeightTop( a_rOutPosition.x, a_rOutPosition.z, 0 );
	a_rOutDirection.Set( -quatRotation.x, -quatRotation.y, -quatRotation.z, quatRotation.w );
}

// $Barnyard: FUNCTION 004f2060
void ACarChaseMiniGameState::SetupLevel()
{
	AVehicleSkidManager::GetSingleton()->HideAllTracks();

	m_eState              = STATE_INIT;
	m_eSubState           = SUBSTATE_NONE;
	m_bUnkc34             = TFALSE;
	m_iFinishPlace        = 0;
	m_iNumPlayersFinished = 0;
	m_iNumSplineFollowers = 0;
	m_bInputEnabled       = TTRUE;
	m_fTime               = 0.0f;

	LoadTrack();

	for ( TINT i = m_iNumPlayers - 1; i >= 0; i-- )
		m_afFinishTimes[ i ] = 0.0f;

	m_afFinishTimes[ 1 ] = 0.0f;

	if ( m_eTrack == TRACK_FOLLOW_KIDS )
	{
		m_oTimer.SetVisible( TFALSE );
		StartKidBikes();

		ARigidBodyMaterial& rMaterial = m_apSteers[ CAR_PLAYER ]->GetCar()->m_oBody.m_oMaterial;
		rMaterial.m_uiCollideTypes |= ACollisionObject::TYPE_UNIT;
		rMaterial.m_uiResponseTypes |= ACollisionObject::TYPE_UNIT;
	}

	TCHAR szLocator[ 64 ];

	// The players
	for ( TINT i = m_iNumPlayers - 1; i >= 0; i-- )
	{
		if ( !m_apCars[ i ] )
			continue;

		if ( m_eTrack == TRACK_STAR )
			TStringManager::String8Format( szLocator, sizeof( szLocator ), "track01_start1" );
		else
			TStringManager::String8Format( szLocator, sizeof( szLocator ), "track0%d_start%d", ( m_eTrack == TRACK_COINS ) ? 5 : m_eTrack + 1, i + 1 );

		TBOOL bRoadBlockAtStart = m_eTrack == TRACK_2 || m_eTrack == TRACK_4;

		TVector4 vPosition;
		TVector4 vDirection;
		GetStartTransform( szLocator, m_oRaceTrack.oSpline, bRoadBlockAtStart, vPosition, vDirection );

		if ( bRoadBlockAtStart )
		{
			// Puts a road block on the ground at the start of the track (the player starts 10 further)
			TVector4 vBlockPos;
			m_oRaceTrack.oSpline.GetPosition( 0.0f, vBlockPos );

			TVector4 vTrackDir;
			m_oRaceTrack.oSpline.GetDirection( 0.0f, vTrackDir );

			TFLOAT fDistance = 200.0f;

			if ( ATerrainInterface::GetSingleton()->CollideRay( vBlockPos, TVector4::VEC_POSY, fDistance, 0, TFALSE ) == TTRUE )
			{
				vBlockPos.y += fDistance;
			}
			else if ( ATerrainInterface::GetSingleton()->CollideRay( vBlockPos, TVector4::VEC_NEGY, fDistance, 0, TFALSE ) == TTRUE )
			{
				vBlockPos.y -= fDistance;
			}

			TVector4 vSphere( vBlockPos.x, vBlockPos.y, vBlockPos.z, 2.0f );
			TFLOAT   fSphereDistance = 5.0f;
			TVector4 vNormal;
			ATerrainInterface::GetSingleton()->CollideSphere( vSphere, fSphereDistance, vNormal, -1, 0 );
			vNormal.w = 0.0f;
			vNormal.Normalise();

			TVector4 vRight;
			vRight.CrossProduct( vNormal, vTrackDir );
			vRight.w = 0.0f;
			vRight.Normalise();

			TVector4 vForward;
			vForward.CrossProduct( vRight, vNormal );
			vForward.w = 0.0f;
			vForward.Normalise();

			TMatrix44 matTransform                                 = TMatrix44::IDENTITY;
			matTransform.AsBasisVector4( BASISVECTOR_RIGHT )       = vForward;
			matTransform.AsBasisVector4( BASISVECTOR_UP )          = vRight;
			matTransform.AsBasisVector4( BASISVECTOR_FORWARD )     = vNormal;
			matTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ) = vBlockPos;

			AAnimatableObject* pRoadBlock = m_apRoadBlocks[ m_iNumRoadBlocks ];
			pRoadBlock->GetModelInstance()->SetTransform( matTransform );
			pRoadBlock->GetModelInstance()->SetSkeletonUpdating( TTRUE );
			m_apRoadBlockCollisions[ m_iNumRoadBlocks ]->Create( pRoadBlock->GetModelInstance()->GetSceneObject() );

			m_iNumRoadBlocks++;
		}

		ARacingCarSteer* pSteer = m_apSteers[ i ];
		pSteer->Reset( vPosition, vDirection );
		pSteer->SetVelocity( TVector4::VEC_ZERO );
		pSteer->SetPedal( ARacingCarSteer::PEDAL_NONE, TFALSE );
		pSteer->SetSteering( ARacingCarSteer::STEER_ABSOLUTE, 0.0f, 0.0f );

		APhysicsGearBox& rGearBox = pSteer->GetCar()->m_oGearBox;
		rGearBox.iPrevGear        = rGearBox.iGear;
		rGearBox.iGear            = 1;
		rGearBox.bAllowDrive      = TFALSE;

		pSteer->SetVelocity( TVector4::VEC_ZERO );
		rGearBox.bAllowReverse = TTRUE;

		AUnitCarChaseCar* pCar = m_apCars[ i ];
		pCar->Reset();
		pCar->m_bUnk18C = TFALSE;
		pCar->m_iState  = AUnitCarChaseCar::STATE_RACING;

		if ( pCar->GetCarType() != AUnitCarChaseCar::CARTYPE_PLAYER )
			pCar->m_iEngineLoopSound = 439;
		else
			pCar->m_iEngineLoopSound = 384;
	}

	if ( m_eTrack != TRACK_COINS )
	{
		// The racers start at the cop car locators
		for ( TINT i = NUM_RACERS; i > 0; i-- )
		{
			if ( !m_apCars[ i ] )
				continue;

			if ( i > 9 )
				TStringManager::String8Format( szLocator, sizeof( szLocator ), "copcar_%d", i );
			else
				TStringManager::String8Format( szLocator, sizeof( szLocator ), "copcar_0%d", i );

			TVector4    vPosition;
			TQuaternion quatRotation;
			ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( szLocator, vPosition, quatRotation );
			vPosition.y = ATerrainInterface::GetSingleton()->GetGroundHeightTop( vPosition.x, vPosition.z, 0 );

			TVector4 vDirection( quatRotation.x, quatRotation.y, quatRotation.z, quatRotation.w );

			if ( m_eTrack == TRACK_FOLLOW_KIDS )
			{
				// Not used in this variant, parked away from the track
				ResetCarSteer( m_apSteers[ i ], m_vParkingPos, vDirection );
				m_vParkingPos.z -= 10.0f;
			}
			else
			{
				ResetCarSteer( m_apSteers[ i ], vPosition, vDirection );
			}

			APhysicsCar* pPhysics              = m_apSteers[ i ]->GetCar();
			pPhysics->m_oParams.fUnk00         = 12000.0f;
			pPhysics->m_oGearBox.bAllowReverse = TFALSE;

			Racer& rRacer = m_aRacers[ i - 1 ];
			rRacer.SetMode( Racer::MODE_IDLE, 5 );

			AUnitCarChaseCar* pCar = m_apCars[ i ];
			pCar->Reset();
			pCar->m_iEngineLoopSound = 384;
			pCar->m_iState           = AUnitCarChaseCar::STATE_RACING;
			pCar->SetVisible( TFALSE );
			pCar->m_bUnk18C = TFALSE;

			rRacer.SetSpline( &m_oRaceTrack.oSpline );
		}

		// The car following the track
		if ( m_apCars[ CAR_FOLLOWER ] )
		{
			if ( m_eTrack == TRACK_STAR )
				TStringManager::String8Format( szLocator, sizeof( szLocator ), "track01_start%d", m_iNumPlayers + 1 );
			else
				TStringManager::String8Format( szLocator, sizeof( szLocator ), "track0%d_start%d", m_eTrack + 1, m_iNumPlayers + 1 );

			TVector4 vPosition;
			TVector4 vDirection;
			GetStartTransform( szLocator, m_oRaceTrack.oSpline, m_eTrack == TRACK_2 || m_eTrack == TRACK_4, vPosition, vDirection );

			if ( m_iNumSplineFollowers <= 0 )
			{
				ResetCarSteer( m_apSteers[ CAR_FOLLOWER ], m_vParkingPos, vDirection );
				m_vParkingPos.z -= 10.0f;
			}
			else
			{
				ResetCarSteer( m_apSteers[ CAR_FOLLOWER ], vPosition, vDirection );
			}

			APhysicsCar* pPhysics              = m_apSteers[ CAR_FOLLOWER ]->GetCar();
			pPhysics->m_oParams.fUnk00         = 12000.0f;
			pPhysics->m_oGearBox.bAllowReverse = TFALSE;

			AUnitCarChaseCar* pCar = m_apCars[ CAR_FOLLOWER ];
			pCar->Reset();
			pCar->m_iEngineLoopSound = 384;
			pCar->m_iState           = AUnitCarChaseCar::STATE_RACING;
			pCar->m_bUnk18C          = TFALSE;

			m_aSplineFollowers[ 0 ].Init( &m_oRaceTrack.oSpline );
		}

		// NOTE: the original sets up m_iNumSplineFollowers spline followers, but there's only one and m_iNumSplineFollowers is at most 1
		// (the others would write into m_oRaceTrack)
		if ( m_iNumSplineFollowers > 0 ) m_aSplineFollowers[ 0 ].SetUnk28( 2 );
		TASSERT( m_iNumSplineFollowers <= 1 );
	}

	m_oTimer.SetDuration( m_fTimeLimit );
	m_oTimer.SetTime( m_fTimeLimit );

	// The minimaps
	TVector4    vMapBottomLeft;
	TVector4    vMapTopRight;
	TQuaternion quatUnused;
	ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( "map_bottom_left", vMapBottomLeft, quatUnused );
	ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( "map_top_right", vMapTopRight, quatUnused );

	for ( TINT i = m_iNumPlayers - 1; i >= 0; i-- )
	{
		AGUI2MiniMap&                 rMap       = m_aMiniMaps[ i ];
		const AGUI2MiniMap::Viewport& rcViewport = *TREINTERPRETCAST( const AGUI2MiniMap::Viewport*, m_aiMapRect + i * 4 );

		rMap.Reset();
		rMap.Create( "Chase_Map_512", 0.3f, 0.3f, rcViewport, vMapBottomLeft, TVector2( 49.0f, 253.0f ), vMapTopRight, TVector2( 347.0f, 14.0f ), TTRUE, TNULL, TFALSE, ( m_eTrack == TRACK_COINS ) ? 129 : 50 );
		rMap.SetZoom( 0.25f );
		rMap.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_BOTTOMLEFT );
		rMap.SetTransform( TFLOAT( rcViewport.iX ), TFLOAT( rcViewport.iY ) );
		m_GUIElement.AddChildTail( rMap );

		if ( m_eTrack != TRACK_COINS )
			rMap.AddIcon( TVector4::VEC_ZERO, TVector4::VEC_POSY, AGUI2MiniMap::ICONTYPE_EDGE, AGUI2MiniMap::ICON_CHALLENGE, 0.1f );

		rMap.AddIcon( TVector4::VEC_ZERO, TVector4::VEC_POSY, AGUI2MiniMap::ICONTYPE_PLAYER, AGUI2MiniMap::ICON_PLAYER, 0.1f );

		if ( m_eVariant != 6 )
		{
			// Icons of the racers
			for ( TINT k = CAR_RACER2; k >= CAR_RACER1; k-- )
			{
				if ( !m_apCars[ k ] )
					continue;

				ARacingCarSteer* pSteer  = m_apSteers[ k ];
				m_apCars[ k ]->m_iUnk168 = rMap.AddIcon( *pSteer->GetPosition(), *pSteer->GetDirection(), AGUI2MiniMap::ICONTYPE_EDGE, AGUI2MiniMap::ICON_ENEMY, 0.1f );

				if ( !m_apCars[ k ]->IsVisible() )
					m_aMiniMaps[ 0 ].SetIconVisible( m_apCars[ k ]->m_iUnk168, TFALSE );
			}

			// Icons of the spline followers
			for ( TINT k = m_iNumSplineFollowers - 1; k >= 0; k-- )
			{
				AUnitCarChaseCar* pCar = m_apCars[ CAR_FOLLOWER + k ];

				if ( !pCar )
					continue;

				// NOTE: the original takes the direction from the steer of the first racer instead of the follower's
				// and hides the icon of the first racer instead of the follower's
				pCar->m_iUnk168 = rMap.AddIcon( *m_apSteers[ CAR_FOLLOWER + k ]->GetPosition(), *m_apSteers[ CAR_RACER1 + k ]->GetDirection(), AGUI2MiniMap::ICONTYPE_EDGE, AGUI2MiniMap::ICON_ITEM, 0.1f );

				if ( !pCar->IsVisible() )
					m_aMiniMaps[ 0 ].SetIconVisible( m_apCars[ CAR_RACER1 + k ]->m_iUnk168, TFALSE );
			}
		}

		rMap.Hide();
	}

	if ( m_eTrack == TRACK_COINS )
		CreateCoins();

	m_oDirectionArrow.HideArrow();
	m_oFadingText.Hide();
	m_oTimer.SetVisible( TFALSE );
	m_oSpeedometer.SetVisible( TFALSE );

	// Load the terrain section around the player car
	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
	ATerrainVIS*       pVIS     = pTerrain->GetVIS();

	TVector4 vPlayerPos = *m_apSteers[ CAR_PLAYER ]->GetPosition();
	vPlayerPos.y -= 20.0f;

	{
		TPString8 strPicture;
		g_oLoadScreen.SetPicture( AMiniGameManager::GetSingleton()->GetLoadingScreenName( strPicture, AMiniGameManager::GetSingleton()->GetNextMiniGameIndex() ), TFALSE );
	}

	pTerrain->SetIsLoaded( TFALSE );

	for ( TINT i = 0; i < pVIS->GetNumSections(); i++ )
	{
		if ( pVIS->GetSections()[ i ].IsPointInside( vPlayerPos ) == TTRUE )
		{
			pTerrain->ForceSection( i, TFALSE );
			break;
		}
	}

	pTerrain->Update();
	pTerrain->SetIsLoaded( TTRUE );

	if ( !pTerrain->IsLoaded() && !g_oLoadScreen.IsLoadingScreen() )
		g_oLoadScreen.SetLoadingState( TTRUE, TTRUE );

	m_oStartHudElement.Setup( 1, 0 );

	m_fRespawnTimer = 10.0f;
	m_bStartEngines = TTRUE;
	ACollisionManager::GetSingleton()->SetVehiclesUpdateEnabled( TTRUE );
	m_iNumFinishedCars     = 0;
	m_iNumCoinsCollected   = 0;
	m_iLastGear            = 0;
	m_fGearShiftCameraTime = 0.0f;
	m_iCue3824             = -1;
	m_fDistanceWarningTime = 0.0f;
	APlayerProfileManager::GetSingleton()->GetMainProfile()->SetMiniGamePickups( 0 );
	m_oPickUpHudElement.SetNumber( 0 );

	// The camera
	ACameraManager::GetSingleton()->SetCameraHelper( ACameraManager::CAMERAHELPER_BIKE );
	ACameraManager::GetSingleton()->GetCurrentCameraHelper()->ResetPosition( TTRUE );

	m_apCameraHelpers[ 0 ]->SetTargetUnit( m_apCars[ CAR_PLAYER ] );

	ABikePhysicsCameraHelper* pCameraHelper = TSTATICCAST( ABikePhysicsCameraHelper, m_apCameraHelpers[ 0 ] );
	pCameraHelper->SetCameraOffset( CAMERA_OFFSET );
	pCameraHelper->SetLookAtOffset( CAMERA_LOOKAT_OFFSET );
	pCameraHelper->SetPivotOffset( CAMERA_PIVOT_OFFSET );

	AGameLoader::Terrain_Load();

	s_bPreStartCameraSaved = TFALSE;

	m_iLives = 5;
	m_oLifeHud.SetLives( 5 );

	m_fDriverVoiceTimer = 0.0f;
	m_fBikeVoiceTimer   = 0.0f;
	m_fOutroTime        = 0.0f;
	m_oFadingText.SetText( L"", 0xFFFFFFFF );
	m_fSpeedScale = 1.0f;

	m_bSubmitHighScore = TTRUE;
	m_bRaceRunning     = TTRUE;

	g_oLoadScreen.Reset();
	g_oLoadScreen.SetLoadingState( TFALSE, TTRUE );

	UpdateHUD( 0.0f );
	UpdatePreStart( 0.0f );

	pTerrain->SetGetCurrentVISGroupCallback( ATerrainInterface::GetSectionPlayerIsIn );
}

// $Barnyard: FUNCTION 004f4130
void ACarChaseMiniGameState::OnInsertion()
{
	// NOTE: inlined AGameState::SaveCameraFOV, saves the FOV of the current camera
	if ( ACameraManager::IsSingletonCreated() )
	{
		if ( ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera() )
			m_fFOV = pCamera->GetFOV();
	}

	LoadData();

	m_bShowPlayerFloats = TFALSE;
	AMiniGameState::OnInsertion();

	AGameLoader::Terrain_Set( 34, TFALSE, TFALSE, 10, 0x500000, 0x600000, -1 );
	AGameLoader::Terrain_Load();
	ARegrowthManager::GetSingleton()->NewDayFruits2();

	m_iNumPlayers = 1;
	TMath::Clip( m_iNumPlayers, 1, 1 );

	m_eTrack = AMiniGameManager::GetSingleton()->GetMiniGameVariant();
	TMath::Clip( m_eTrack, 0, 6 );

	m_bNoTimeLimit = TFALSE;

	ACameraManager::GetSingleton()->SetCameraHelper( ACameraManager::CAMERAHELPER_BIKE );
	m_apCameraHelpers[ 0 ] = ACameraManager::GetSingleton()->GetCurrentCameraHelper();

	m_eVariant = AMiniGameManager::GetSingleton()->GetMiniGameVariant();

	if ( m_eVariant == 7 )
		m_eTrack = TRACK_FOLLOW_KIDS;

	// The cars of the players: the player in the truck, or Beady's car in the variants 4-7
	TINT iPlayerCarType = ( m_eVariant >= 4 && m_eVariant <= 7 ) ? AUnitCarChaseCar::CARTYPE_BEADY : AUnitCarChaseCar::CARTYPE_PLAYER;

	for ( TINT i = m_iNumPlayers - 1; i >= 0; i-- )
	{
		AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_NEGZ, 0, iPlayerCarType, 0, TTRUE );

		AUnitCarChaseCar* pCar = TSTATICCAST( AUnitCarChaseCar, AUnitManager::GetSingleton()->SpawnUnit( TPS8( car_chase_car1 ), &oTransform ) );
		m_apCars[ i ]          = pCar;
		pCar->CreateDrivers( m_eVariant );

		m_apSteers[ i ]                      = TSTATICCAST( ARacingCarSteer, pCar->GetSteer() );
		m_apSteers[ i ]->GetCar()->m_bUnkCD2 = TTRUE;

		SetupCarModel( pCar, TFALSE );

		APlayerManager::GetSingleton()->GetCurrentPlayerSet()->SetPlayerUnit( pCar, m_iNumPlayers - 1 - i );

		m_apCameraHelpers[ i ]->SetTargetUnit( m_apCars[ i ] );

		ABikePhysicsCameraHelper* pCameraHelper = TSTATICCAST( ABikePhysicsCameraHelper, m_apCameraHelpers[ i ] );
		pCameraHelper->SetCameraOffset( CAMERA_OFFSET );
		pCameraHelper->SetLookAtOffset( CAMERA_LOOKAT_OFFSET );
		pCameraHelper->SetPivotOffset( CAMERA_PIVOT_OFFSET );
	}

	// The racers (cop cars)
	if ( m_eVariant != 4 && m_eVariant != 7 && m_eVariant != 6 )
	{
		for ( TINT i = NUM_RACERS; i > 0; i-- )
		{
			AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_NEGZ, 0, AUnitCarChaseCar::CARTYPE_COP, 0, TTRUE );

			AUnitCarChaseCar* pCar = TSTATICCAST( AUnitCarChaseCar, AUnitManager::GetSingleton()->SpawnUnit( TPS8( car_chase_car1 ), &oTransform ) );
			m_apCars[ i ]          = pCar;
			pCar->CreateDrivers( m_eVariant );

			m_apSteers[ i ]                      = TSTATICCAST( ARacingCarSteer, pCar->GetSteer() );
			m_apSteers[ i ]->GetCar()->m_bUnkCD2 = TFALSE;

			SetupCarModel( pCar, TTRUE );

			// NOTE: m_fUnkc64 is only set (to 3.0) further down, the original passes the uninitialised value
			m_aRacers[ i - 1 ].Setup( m_apSteers[ i ], 20.0f, 15.0f, m_fUnkc64, 5.0f );
		}
	}

	// The car following the track and the finish line
	if ( m_eVariant < 4 )
	{
		AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_NEGZ, 0, AUnitCarChaseCar::CARTYPE_BEADY, 0, TTRUE );

		AUnitCarChaseCar* pCar   = TSTATICCAST( AUnitCarChaseCar, AUnitManager::GetSingleton()->SpawnUnit( TPS8( car_chase_car1 ), &oTransform ) );
		m_apCars[ CAR_FOLLOWER ] = pCar;
		pCar->CreateDrivers( m_eVariant );

		m_apSteers[ CAR_FOLLOWER ]                      = TSTATICCAST( ARacingCarSteer, pCar->GetSteer() );
		m_apSteers[ CAR_FOLLOWER ]->GetCar()->m_bUnkCD2 = TFALSE;

		SetupCarModel( pCar, TTRUE );

		m_aSplineFollowers[ 0 ].pSteer = m_apSteers[ CAR_FOLLOWER ];

		AAnimatableObjectType* pFinishLineType = AAnimatableObjectManager::GetSingleton()->FindType( TPS8( car_finishline ) );
		m_pFinishLine                          = AAnimatableObjectManager::GetSingleton()->CreateAnimatableObject( pFinishLineType, TNULL, 0 );
		m_pFinishLine->SetOpacity( 1.0f, TTRUE );

		// NOTE: ATerrainInterface::GetLocatorList returns the locator list of the terrain
		m_pFinishLine->GetModelInstance()->SetTransform( ATerrainInterface::GetSingleton()->GetVIS()->GetLocatorList()->GetWorldTransform().GetMatrix() );

		m_pFinishLine->SetSkeletonUpdating( TTRUE, TTRUE );

		m_pFinishLineCollision = new ACollisionObjectMesh();
		m_pFinishLineCollision->Create( m_pFinishLine->GetModelInstance()->GetSceneObject() );
	}

	m_fUnkc58     = 6.0f;
	m_fUnkc60     = 6.0f;
	m_fUnkc5c     = 35.0f;
	m_fUnkc64     = 3.0f;
	m_vParkingPos = TVector4( 1000.0f, -1000.0f, 0.0f, 1.0f );
	m_fUnkc68     = 20.0f;
	m_fUnkd9c     = 10.0f;

	// The kids of the follow the kids variants
	if ( m_eVariant == 4 || m_eVariant == 7 )
	{
		static const TCHAR* s_apKidBikes[ NUM_KID_BIKES ] = { "car_kid_bike", "car_kid_bike2", "car_kid_bike3" };

		AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_NEGZ, 0, 0, -1, TTRUE );

		for ( TINT i = 0; i < NUM_KID_BIKES; i++ )
		{
			AUnit* pKid = AUnitManager::GetSingleton()->CreateUnit( AUnitManager::GetSingleton()->FindUnit( s_apKidBikes[ i ] ), &oTransform );

			pKid->ChangeSteer( &TGetClass( AKidBikeSteer ) );

			m_apKidBikes[ i ]      = pKid;
			m_apKidBikeSteers[ i ] = TSTATICCAST( AKidBikeSteer, pKid->GetSteer() );
			TSTATICCAST( AVisibleUnit, pKid )->SetVisible( TFALSE );
		}

		m_apKidBikeSteers[ 1 ]->SetSideOffset( -1.5f );
		m_apKidBikeSteers[ 2 ]->SetSideOffset( 1.0f );
	}

	// The road blocks
	AAnimatableObjectType* pRoadBlockType = AAnimatableObjectManager::GetSingleton()->FindType( TPS8( road_block1 ) );

	for ( TINT i = MAX_ROAD_BLOCKS - 1; i >= 0; i-- )
	{
		m_apRoadBlocks[ i ] = AAnimatableObjectManager::GetSingleton()->CreateAnimatableObject( pRoadBlockType, TNULL, 0 );
		m_apRoadBlocks[ i ]->GetModelInstance()->SetTransform( TMatrix44::IDENTITY );
		m_apRoadBlockCollisions[ i ] = new ACollisionObjectMesh();
	}

	m_HUDParams.SetFlags( AHudElement::MASK_MINIGAME | AHudElement::MASK_UNK4 | AHudElement::MASK_UNK23 | AHudElement::MASK_UNK24 | AHudElement::MASK_UNK25 );

	// The HUD
	TFLOAT fScreenWidth;
	TFLOAT fScreenHeight;
	AGUI2::GetSingleton()->GetDimensions( fScreenWidth, fScreenHeight );

	m_aiMapRect[ 0 ] = 0;
	m_aiMapRect[ 1 ] = 0;
	m_aiMapRect[ 3 ] = TINT( fScreenHeight );
	m_aiMapRect[ 2 ] = TINT( fScreenWidth );

	m_oTimer.Create( 0.0f, 0.0f );
	m_oTimer.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );

	T2GUITransform oScale;
	oScale.Set( TVector2( 0.75f, 0.0f ), TVector2( 0.0f, 0.75f ), TVector2::VEC_ZERO );
	m_oTimer.GetTransform().PreMultiply( oScale );
	m_GUIElement.AddChildTail( m_oTimer );

	m_oSpeedometer.Create( TTRUE, &m_GUIElement );

	m_GUIElement.AddChildTail( m_oDirectionArrow );
	m_oDirectionArrow.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oDirectionArrow.Create( 150.0f, 75.0f );

	AHudManager::GetSingleton()->AddElement( &m_oRacePlacingHud, TFALSE, TTRUE, TFALSE );
	m_oRacePlacingHud.SetForceHide( TTRUE );

	T2GUIFontManager::FindFont( "Rekord26" );
	m_oIconRectangle.SetTextureSection( T2GUITextureSectionManager::GetTextureSection( "Chase_Icon_Light" ) );
	m_oIconElement.SetDimensions( m_oIconRectangle.GetWidth(), m_oIconRectangle.GetHeight() );
	m_oIconElement.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oIconElement.SetTransform( 0.0f, 60.0f );
	m_oIconElement.AddChildTail( m_oIconRectangle );
	m_GUIElement.AddChildTail( m_oIconElement );
	m_oIconElement.Hide();

	// Night time with the fog close to the car
	AGameTimeFXManager* pGameTimeFX = AGameTimeFXManager::GetSingleton();
	AGameTimeFX         oCarLightFX = *pGameTimeFX->FindFX( "CARLIGHT" );
	oCarLightFX.m_fFogStart         = 60.0f;
	oCarLightFX.m_fFogEnd           = 250.0f;
	pGameTimeFX->SetForcedFX( &oCarLightFX );

	ARenderer* pRenderer = ARenderer::GetSingleton();
	m_fSavedFarClip      = pRenderer->GetFar();
	pRenderer->SetFar( m_fSavedFarClip + 500.0f );
	m_fSavedNearClip = pRenderer->GetNear();
	pRenderer->SetNear( 0.5f );

	AInstanceManager::GetSingleton()->GetVisibilityDistances( m_afSavedVisibilityDistances[ 0 ], m_afSavedVisibilityDistances[ 1 ], m_afSavedVisibilityDistances[ 2 ] );
	AInstanceManager::GetSingleton()->SetVisibilityDistances( m_afSavedVisibilityDistances[ 0 ] + 100.0f, m_afSavedVisibilityDistances[ 1 ] + 100.0f, m_afSavedVisibilityDistances[ 2 ] + 100.0f );

	ATreeManager::GetSingleton()->GetDistances( m_afSavedTreeDistances[ 0 ], m_afSavedTreeDistances[ 1 ], m_afSavedTreeDistances[ 2 ] );
	ATreeManager::GetSingleton()->SetDistances( m_afSavedTreeDistances[ 0 ] + 100.0f, m_afSavedTreeDistances[ 1 ] + 100.0f, m_afSavedTreeDistances[ 2 ] + 100.0f );

	// The barriers
	static const TCHAR* s_apBarrierLocators[ NUM_BARRIERS ] = {
		"spline3barrier1",
		"spline3barrier2",
		"spline3barrier3",
		"spline4barrier1",
		"spline5barrier",
		"roadblock_10_00",
	};

	AAnimatableObjectType* pBarrierType = AAnimatableObjectManager::GetSingleton()->FindType( TPS8( road_block2 ) );

	for ( TINT i = 0; i < NUM_BARRIERS; i++ )
	{
		TVector4    vPosition;
		TQuaternion quatRotation;
		ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( s_apBarrierLocators[ i ], vPosition, quatRotation );
		vPosition.y = ATerrainInterface::GetSingleton()->GetGroundHeight( vPosition, 0 );

		m_apBarriers[ i ] = AAnimatableObjectManager::GetSingleton()->CreateAnimatableObject( pBarrierType, TNULL, 0 );
		m_apBarriers[ i ]->GetModelInstance()->SetTransform( vPosition, *TREINTERPRETCAST( TVector4*, &quatRotation ) );
		m_apBarriers[ i ]->GetModelInstance()->SetSkeletonUpdating( TTRUE );

		if ( i == NUM_BARRIERS - 1 )
		{
			// The last one is turned around
			TMatrix44 matTransform;
			m_apBarriers[ i ]->GetModelInstance()->GetLocalTransform( matTransform );
			matTransform.RotateZ( 2.5307274f );
			m_apBarriers[ i ]->GetModelInstance()->SetTransform( matTransform );
		}

		m_apBarrierCollisions[ i ] = new ACollisionObjectMesh();
		m_apBarrierCollisions[ i ]->Create( m_apBarriers[ i ]->GetModelInstance()->GetSceneObject() );
	}

	SetupLevel();

	// The star at the finish line
	AAnimatableObjectType* pStarType = AAnimatableObjectManager::GetSingleton()->FindType( TPS8( carstar ) );
	m_pCarStar                       = AAnimatableObjectManager::GetSingleton()->CreateAnimatableObject( pStarType, TNULL, 0 );
	m_pCarStar->SetGlow( TTRUE );

	TVector4 vStarOffset;
	vStarOffset.Multiply( TVector4::VEC_NEGY, 2.0f );

	TVector4 vStarPos( vStarOffset.x + m_vFinishPosition.x, vStarOffset.y + m_vFinishPosition.y, vStarOffset.z + m_vFinishPosition.z, m_vFinishPosition.w );
	m_pCarStar->GetModelInstance()->SetTransform( vStarPos, TVector4::VEC_POSX );

	TMatrix44 matStar;
	m_pCarStar->GetModelInstance()->GetLocalTransform( matStar );
	matStar.Scale( 3.0f, 3.0f, 3.0f );
	m_pCarStar->GetModelInstance()->SetTransform( matStar );
	m_pCarStar->SetSkeletonUpdating( TFALSE, TTRUE );

	if ( m_eTrack == TRACK_STAR )
		m_pCarStar->SetSkeletonUpdating( TTRUE, TTRUE );

	if ( m_eTrack == TRACK_COINS )
		m_oPickUpHudElement.SetForceHide( TFALSE );

	if ( m_eVariant == 4 )
	{
		ASSeqController::GetSingleton()->LoadScript( "SSFTK_FollowTheKids.tso", TVector4::VEC_ZERO );
		ASSeq* pScript = ASSeqController::GetSingleton()->FindScript( "SSFTK_FollowTheKids", TNULL );

		TMatrix44 matScript                                   = TMatrix44::IDENTITY;
		matScript.AsBasisVector4( BASISVECTOR_TRANSLATION ).x = -700.0f;
		matScript.AsBasisVector4( BASISVECTOR_TRANSLATION ).y = 1400.0f;
		matScript.AsBasisVector4( BASISVECTOR_TRANSLATION ).z = 0.0f;
		pScript->SetTransform( matScript );
	}

	m_oLifeHud.Create( &m_GUIElement, TNULL, 8, "Score_Wood_Thin", T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT, 1.0f, TFALSE );
	m_oLifeHud.SetLives( m_iLives );
	m_oLifeHud.SetTranslation( 0.0f, m_oTimer.GetHeight() * 0.8f );
	m_oLifeHud.SetVisible( TFALSE );

	AVehicleSkidManager::GetSingleton()->HideAllTracks();
	m_oFadingText.Create( TNULL, &m_GUIElement, T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );

	AVehicleManager::SetSlideOnObjects( TTRUE );
	AVehicleManager::SetUseTriangleNormal( TTRUE );

	AMusicManager::GetSingleton()->SetEnabled( TFALSE, TTRUE );
	AMultiplexStream::GetSingleton()->Open( "Joyride" );
}

// $Barnyard: FUNCTION 004f0d20
void ACarChaseMiniGameState::OnRemoval()
{
	// NOTE: inlined AGameState::RestoreCameraFOV, restores the FOV of the current camera
	if ( ACameraManager::IsSingletonCreated() )
	{
		if ( ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera() )
			pCamera->SetFOV( m_fFOV );
	}

	AMiniGameState::OnRemoval();
	AVehicleSkidManager::GetSingleton()->HideAllTracks();

	// The cars
	for ( TINT i = m_iNumPlayers - 1; i >= 0; i-- )
	{
		if ( m_apCars[ i ] )
		{
			DestroyCarChaseUnit( m_apCars[ i ] );
			m_apCars[ i ]   = TNULL;
			m_apSteers[ i ] = TNULL;
		}
	}

	for ( TINT i = CAR_RACER2; i >= CAR_RACER1; i-- )
	{
		if ( m_apCars[ i ] )
		{
			DestroyCarChaseUnit( m_apCars[ i ] );
			m_apCars[ i ]   = TNULL;
			m_apSteers[ i ] = TNULL;
		}
	}

	if ( m_apCars[ CAR_FOLLOWER ] )
	{
		DestroyCarChaseUnit( m_apCars[ CAR_FOLLOWER ] );
		m_apCars[ CAR_FOLLOWER ]   = TNULL;
		m_apSteers[ CAR_FOLLOWER ] = TNULL;
	}

	// The coins
	if ( m_eTrack == TRACK_COINS )
	{
		for ( TINT i = 0; i < NUM_COINS; i++ )
		{
			if ( m_aCoins[ i ].pObject )
			{
				AShadowManager::GetSingleton()->RemoveShadow( m_aCoins[ i ].pObject->GetModelInstance() );
				AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_aCoins[ i ].pObject );
				m_aCoins[ i ].pObject  = TNULL;
				m_aCoins[ i ].iMapIcon = -1;
			}
		}
	}

	// The kids
	for ( TINT i = 0; i < NUM_KID_BIKES; i++ )
	{
		if ( m_apKidBikes[ i ] )
		{
			DestroyCarChaseUnit( m_apKidBikes[ i ] );
			m_apKidBikes[ i ]      = TNULL;
			m_apKidBikeSteers[ i ] = TNULL;
		}
	}

	// The road blocks and the barriers
	for ( TINT i = MAX_ROAD_BLOCKS - 1; i >= 0; i-- )
	{
		if ( m_apRoadBlocks[ i ] )
		{
			AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_apRoadBlocks[ i ] );
			m_apRoadBlocks[ i ] = TNULL;
		}

		if ( m_apRoadBlockCollisions[ i ] )
		{
			delete m_apRoadBlockCollisions[ i ];
			m_apRoadBlockCollisions[ i ] = TNULL;
		}
	}

	for ( TINT i = 0; i < NUM_BARRIERS; i++ )
	{
		if ( m_apBarriers[ i ] )
		{
			AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_apBarriers[ i ] );
			m_apBarriers[ i ] = TNULL;
		}

		if ( m_apBarrierCollisions[ i ] )
		{
			delete m_apBarrierCollisions[ i ];
			m_apBarrierCollisions[ i ] = TNULL;
		}
	}

	if ( m_pFinishLine )
	{
		AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pFinishLine );
		m_pFinishLine = TNULL;

		if ( m_pFinishLineCollision )
			delete m_pFinishLineCollision;

		m_pFinishLineCollision = TNULL;
	}

	if ( m_iNumPlayers <= 1 )
	{
		m_apCameraHelpers[ 0 ] = TNULL;
	}
	else
	{
		ACameraManager::GetSingleton()->ResetHelperTargets();

		for ( TINT i = m_iNumPlayers - 1; i >= 0; i-- )
			m_apCameraHelpers[ i ] = TNULL;
	}

	AGameLoader::DestroyLevel();

	m_oTimer.RemoveSelf();
	m_oIconElement.RemoveSelf();
	m_oIconRectangle.RemoveSelf();
	m_oDirectionArrow.RemoveSelf();
	m_oLifeHud.RemoveSelf();
	m_oRacePlacingHud.Detach();

	AUnitManager::GetSingleton()->RemoveAllUnits();
	AUnitManager::GetSingleton()->DeleteDyingUnits();
	TRenderInterface::GetSingleton()->FlushDyingResources();
	AGameTimeFXManager::GetSingleton()->ClearForcedFX();

	ARenderer::GetSingleton()->SetFar( m_fSavedFarClip );
	ARenderer::GetSingleton()->SetNear( m_fSavedNearClip );
	AInstanceManager::GetSingleton()->SetVisibilityDistances( m_afSavedVisibilityDistances[ 0 ], m_afSavedVisibilityDistances[ 1 ], m_afSavedVisibilityDistances[ 2 ] );
	ATreeManager::GetSingleton()->SetDistances( m_afSavedTreeDistances[ 0 ], m_afSavedTreeDistances[ 1 ], m_afSavedTreeDistances[ 2 ] );

	AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pCarStar );

	ACollisionManager::GetSingleton()->SetVehiclesUpdateEnabled( TTRUE );

	m_oTrackDataTRB.Close();
	m_bTrackLoaded = TFALSE;

	ACameraManager::GetSingleton()->GetCurrentCamera()->SetFOV( ACamera::sm_fCurrentFOV );

	AVehicleManager::SetSlideOnObjects( TFALSE );
	AVehicleManager::SetUseTriangleNormal( TFALSE );

	if ( AMultiplexStream::IsPaused() )
		AMultiplexStream::GetSingleton()->Pause( TFALSE );

	AMultiplexStream::GetSingleton()->Close( TTRUE );
}
