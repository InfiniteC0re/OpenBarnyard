#include "pch.h"
#include "ASimState.h"
#include "Helpers/ALadderBeansGame.h"
#include "MiniGame/ACoinPickupMicroGame.h"
#include "Sound/AMultiplexStream.h"
#include "Sound/AMusicManager.h"
#include "SoundBank/music.h"
#include "SoundBank/barnyard.h"
#include "Locale/ALocaleManager.h"
#include "Locale/ALocale.h"
#include "ARandom.h"
#include "MiniGame/AGopherShopInterface.h"
#include "Helpers/AHighScoreManager.h"
#include <Render/TCameraObject.h> // before the Physics headers (Opcode #defines PI)
#include "Helpers/ABikeManager.h"
#include "Helpers/AGateManager.h"
#include "Helpers/AInstanceManager.h"
#include "Helpers/ASimPersistData.h"
#include "Helpers/ARegrowthManager.h"
#include "AGameStateController.h"
#include "ASSeqState.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/ASimAnimModelHelperManager.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "Cameras/AGTAStyleCameraHelper.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "GameInterface/AInventoryState2.h"
#include "GameInterface/AMapState.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitNameManager.h"
#include "Units/AUnitPlayer.h"
#include "Units/AInteractionManager.h"
#include "Units/AAnimalPopulationManager.h"
#include "Units/AUnitBehaviour.h"
#include "Units/Projectiles/ASimpleProjectileManager.h"
#include "Units/Projectiles/AProjectileLauncher.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "Animation/AModel.h"
#include "Animation/AModelRepos.h"
#include "Animation/AAnimatableObject.h"
#include "Terrain/ATerrainInterface.h"
#include "Helpers/AWeatherManager.h"
#include "FX/AGameTimeFXManager.h"
#include "Input/AInputHandler.h"
#include "Physics/AVehicleSkidManager.h"
#include "Input/AInputMapManager.h"
#include "Render/ARenderer.h"
#include "GUI/AGUI2.h"
#include "GUI/AGUI2ModelRendererScreen.h"
#include "Helpers/AHintManager.h"
#include "Units/APlayerControlComponent.h"
#include "Units/AInteractionSequence.h"
#include "Units/AInteraction.h"
#include "MiniGame/AMiniGameManager.h"
#include "FSM/AMiscFunctions.h"
#include "Units/ASpecialEventRequirement.h"
#include "HUD/AHudManager.h"
#include "HUD/ATimeLeftHudElement.h"
#include "FX/AMilkManager.h"
#include "FX/AParticleEffectManager.h"
#include "Sound/ASoundManager.h"
#include "Quests/AQuestManager.h"
#include "Quests/AQuestElement.h"
#include "Quests/AQuest.h"
#include "Quests/AQuestInstance.h"
#include "Helpers/AFarmerTruck.h"
#include "Helpers/AReplayManager.h"
#include "Sound/AVOStream.h"
#include "Units/ABarnOpenRequirement.h"

#include <Toshi/TScheduler.h>

// Opcode (pulled in by the Physics headers) #defines PI, which breaks TMath::PI below
#ifdef PI
#  undef PI
#endif

#include <Plugins/PPropertyParser/PPropertyValue.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0043cde0, CreateObject 0043ee50, CreateObjectInPlace 0043eeb0
TDEFINE_CLASS( ASimState );

TPSTRING8_DECLARE( NormalBarnyard );
TPSTRING8_DECLARE( pip );
TPSTRING8_DECLARE( c_head );
TPSTRING8_DECLARE( joey );
TPSTRING8_DECLARE( barn );
TPSTRING8_DECLARE( park2 );
TPSTRING8_DECLARE( player_startgame );

// $Barnyard: FUNCTION 0043d750
ASimState::ASimState()
    : m_bPendingAutoSave( TFALSE )
    , m_pGateModel( TNULL )
    , m_pScarecrowDayModel( TNULL )
    , m_pScarecrowNightModel( TNULL )
    , m_pLadderBeansGame( TNULL )
    , m_fSavedNear( 1.0f )
    , m_fSavedFar( 280.0f )
    , m_bSpawnedPip( TFALSE )
    , m_pPip( TNULL )
    , m_pNbBull( TNULL )
    , m_iNightAmbienceCue( -1 )
{
	m_pInputMap = AInputMapManager::GetSingleton()->FindMap( TPS8( NormalBarnyard ) );
	m_fUnkFD50  = 0.1f;
	m_bUnkFD68  = TTRUE;
	m_eFlags    = FLAGS_NONE;

	if ( AGameLoader::IsMainGameLoaded() )
		AFarmerTruck::Create( AGameLoader::IsStoryMode() || AGameLoader::IsLoadingSavedGame() );

	m_bIsActive = TFALSE;
}

// $Barnyard: FUNCTION 0043d920
// $Barnyard: FUNCTION 0043eef0
ASimState::~ASimState()
{
	if ( AGameLoader::IsMainGameLoaded() )
		AFarmerTruck::Destroy();

	if ( m_pLadderBeansGame )
	{
		delete m_pLadderBeansGame;
		m_pLadderBeansGame = TNULL;
	}

	m_strBarnExitTrigger = TPString8();

	m_oDateChangeListener.Disconnect();
}

// $Barnyard: FUNCTION 0043cea0
TBOOL ASimState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	ACameraHelper* pCameraHelper = ACameraManager::GetSingleton()->GetCurrentCameraHelper();

	if ( pCameraHelper && pCameraHelper->ProcessInputCommand( (void*)a_eInputCommand, (void*)a_pInputEvent ) )
		return TFALSE;

	switch ( a_eInputCommand )
	{
		case AInputCommand_Cancel:
		case AInputCommand_Quit:
			if ( AInputHandler::GetSingleton()->IsValidDevice( a_pInputEvent->GetSource() ) == TTRUE )
			{
				// Open the start menu unless the inventory is open
				if ( !FindChildState( &TGetClass( AInventoryState2 ) ) )
					AGameStateController::GetSingleton()->StartMenuIfPossible();
				return TTRUE;
			}

			break;
		case AInputCommand_Select:
			// Open the map
			if ( !AGameLoader::IsBarnExploreMiniGame() )
			{
				if ( !AGameStateController::IsBusy() )
					AGameStateController::GetSingleton()->PushState( new AMapState() );
			}

			return TTRUE;
		case AInputCommand_Icons:
			if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
			{
				a_pInputEvent->StartRepeat( 0.0f, 0.0f );
				return TTRUE;
			}

			if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
				return TTRUE;

			break;
	}

	// NOTE: the original calls RETURNS_FALSE (006528c0) here
	return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $Barnyard: FUNCTION 0043d430
void ASimState::OnRender( TFLOAT a_fDeltaTime )
{
	if ( m_bIsActive && !AGameLoader::IsBarnExploreMiniGame() )
		m_oMilkReticles.UpdateSim( a_fDeltaTime );

	// NOTE: the original doesn't check whether the singleton exists
	if ( AVehicleSkidManager::IsSingletonCreated() )
		AVehicleSkidManager::GetSingleton()->Render();

	if ( AMilkManager::IsSingletonCreated() )
		AMilkManager::GetSingleton()->Render();

	if ( !AGameLoader::IsBarnExploreMiniGame() )
	{
		AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

		// The milk bottles are only shown while the player can pick them up
		APlayerControlComponent* pControlComponent = TSTATICCAST( APlayerControlComponent, pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) );

		if ( pControlComponent && pControlComponent->CanUseHand() )
			m_oMilkBottles.Render();
	}

	if ( m_pLadderBeansGame )
		m_pLadderBeansGame->Render();
}

// $Barnyard: FUNCTION 0043ed70
// $BarnyardWiiJP: FUNCTION 8014e628
void ASimState::OnLoaded()
{
	// Count the pick ups collected while the sim wasn't shown
	APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();

	if ( pProfile->GetMiniGamePickups() != 0 )
	{
		pProfile->AddPickups( pProfile->GetMiniGamePickups() );
		pProfile->SetMiniGamePickups( 0 );
	}

	AUnit* pNbBull = AUnitManager::GetSingleton()->FindUnitByTypeName( TPS8D( "nb_bull" ) );

	if ( !AMiniGameManager::ms_bHasFade )
	{
		if ( AGameLoader::IsBarnExploreMiniGame() || !AGameLoader::WasBarnExploreMiniGame() ||
		     !AGameTimeManager::GetSingleton()->IsItNight() || ( !pNbBull && !AGameLoader::IsBarnExploreMiniGame() ) )
		{
#ifdef BARNYARD_LATEST
			// Latest version: no autosave while the player is brought back to the Pig with the apple pie
			if ( !AGameLoader::IsPigApplePieReturn() )
#endif
				m_bPendingAutoSave = TTRUE;
		}
	}
}

// $Barnyard: FUNCTION 0043d510
ASoundId ASimState::GetSound()
{
	if ( ABikeManager::GetSingleton()->IsPlayerOnBike() )
	{
		// The music of the bike ride
		AMusicManager::GetSingleton()->SetForcedSound( ABikeManager::GetRideMusic() );
		AMusicManager::GetSingleton()->ForceUpdate();
		return ASOUNDID_INVALID;
	}

	const TINT iWeather = AWeatherManager::GetSingleton()->GetCurrentWeather();

	if ( AGameTimeManager::GetSingleton()->IsItNight() )
	{
		if ( iWeather == AWeatherManager::WEATHERTYPE_HEAVYRAIN || iWeather == AWeatherManager::WEATHERTYPE_LIGHTNINGSTORM )
			return soundbank::MUSIC_BY_NIGHT_RAIN_HEAVY;

		return ( iWeather != AWeatherManager::WEATHERTYPE_LIGHTRAIN ) ? soundbank::MUSIC_BY_NIGHT : soundbank::MUSIC_BY_NIGHT_RAIN_LIGHT;
	}

	if ( iWeather == AWeatherManager::WEATHERTYPE_HEAVYRAIN || iWeather == AWeatherManager::WEATHERTYPE_LIGHTNINGSTORM )
		return soundbank::MUSIC_RAIN_HEAVY;

	return ( iWeather == AWeatherManager::WEATHERTYPE_LIGHTRAIN ) ? soundbank::MUSIC_RAIN_LIGHT : soundbank::MUSIC_BY_DAY;
}

// $Barnyard: FUNCTION 0043d5a0
void ASimState::OnCollisionModelLoaded( void* a_pCollisionModel, void* a_pGroupId )
{
	AReplayManager::GetSingleton()->UpdateReplays();
}

#ifdef BARNYARD_COMMUNITY_PATCH

// Development, tests the day/night cycle of the sim:
//  "-devdayphase <N>":  changes the phase of the day to N (0 dawn, 1 morning, 2 afternoon, 3 dusk, 4 night) once the sim runs
//  "-devtimescale <N>": the game time runs N times faster while the sim is the current state
static void Dev_UpdateGameTime( TFLOAT a_fDeltaTime )
{
	static TBOOL s_bDayPhaseSet = TFALSE;

	AGameTimeManager* pGameTimeManager = AGameTimeManager::GetSingleton();

	// Wait until the player's animation controller has started its idle animation: changing the phase on the first
	// sim frame fires the farmer sequence before any movement animation played, which leaves the player in the bind pose.
	AUnit* pPlayer         = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
	TBOOL  bPlayerAnimates = pPlayer && pPlayer->IsA( &TGetClass( AAnimatedUnit ) ) &&
	    !TSTATICCAST( AAnimatedUnit, pPlayer )->GetAnimatableObject()->GetToshiAnimInterface().GetAnimRefs().IsEmpty();

	static TFLOAT s_fDayPhaseWait = 0.0f;
	s_fDayPhaseWait += a_fDeltaTime;

	if ( !s_bDayPhaseSet && ( bPlayerAnimates || s_fDayPhaseWait > 5.0f ) )
	{
		s_bDayPhaseSet = TTRUE;

		if ( const TCHAR* szArg = TStringManager::String8FindString( AGameLoader::Dev_GetCommandLine(), "-devdayphase " ) )
		{
			TINT iDayPhase = atoi( szArg + TStringManager::String8Length( "-devdayphase " ) );

			if ( iDayPhase >= 0 && iDayPhase < AGameTimeManager::DAYPHASE_NUMOF )
			{
				TINFO( "[devtime] changing the phase of the day to %d\n", iDayPhase );
				pGameTimeManager->ChangePhaseOfDay( iDayPhase, TTRUE, TTRUE );
			}
		}
	}

	static TFLOAT s_fTimeScale = -1.0f;

	if ( s_fTimeScale < 0.0f )
	{
		s_fTimeScale = 1.0f;

		if ( const TCHAR* szArg = TStringManager::String8FindString( GetCommandLineA(), "-devtimescale " ) )
		{
			s_fTimeScale = TFLOAT( atof( szArg + TStringManager::String8Length( "-devtimescale " ) ) );
			TINFO( "[devtime] game time scale %.2f\n", s_fTimeScale );
		}
	}

	static TBOOL s_bWasStopped = TFALSE;

	if ( s_bWasStopped != pGameTimeManager->IsUpdateStopped() )
	{
		s_bWasStopped = pGameTimeManager->IsUpdateStopped();
		TINFO( "[devtime] the game time is %s\n", s_bWasStopped ? "stopped" : "running" );
	}

	if ( s_fTimeScale > 1.0f && !pGameTimeManager->IsUpdateStopped() )
	{
		static TINT s_iLastDayPhase = -1;

		// The game time manager has already been updated once this frame
		pGameTimeManager->OnUpdate( a_fDeltaTime * ( s_fTimeScale - 1.0f ) );

		static TFLOAT s_fLogTimer = 0.0f;
		s_fLogTimer += a_fDeltaTime;

		if ( s_iLastDayPhase != pGameTimeManager->GetCurrentDayPhase() || s_fLogTimer >= 5.0f )
		{
			s_fLogTimer     = 0.0f;
			s_iLastDayPhase = pGameTimeManager->GetCurrentDayPhase();
			TINFO( "[devtime] %02d:%02d phase %d date %d/%d\n", pGameTimeManager->GetCurrentHour(), pGameTimeManager->GetCurrentMinute(), pGameTimeManager->GetCurrentDayPhase(), pGameTimeManager->GetDay(), pGameTimeManager->GetMonth() );
		}
	}
}

#endif // BARNYARD_COMMUNITY_PATCH

// $Barnyard: FUNCTION 0043e510
TBOOL ASimState::OnUpdate( TFLOAT a_fDeltaTime )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	if ( GetClass() == &TGetClass( ASimState ) )
		Dev_UpdateGameTime( a_fDeltaTime );
#endif

	RefreshPlayerProxy();

	if ( m_bPendingAutoSave && !AQuestManager::GetSingleton()->IsTimerPaused() )
		AGameStateController::GetSingleton()->SetFlags( AGameStateController::FLAGS_FADING_IN );

	m_bPendingAutoSave = TFALSE;

	if ( AGameLoader::WasBarnExploreMiniGame() )
	{
		if ( !AGameTimeFXManager::GetSingleton()->IsInBarn() )
			AGameTimeFXManager::GetSingleton()->SetInBarn( TTRUE );

		if ( !AGameLoader::IsBarnExploreMiniGame() && AGameTimeManager::GetSingleton()->IsItNight() )
		{
			// The player left the barn at night, the bull takes care of them
			AUnit* pNbBull = AUnitManager::GetSingleton()->FindUnitByTypeName( TPS8D( "nb_bull" ) );

			if ( pNbBull )
			{
				AUnit* apUnits[ 2 ] = { pNbBull, m_pPlayerProxy->GetUnit() };
				AInteractionManager::GetSingleton()->AssignInteractionToUnits( apUnits, 2, m_strBarnExitTrigger, AInteractionManager::ASSIGNFLAGS( 5 ) );
			}
		}

		AGameLoader::ResetWasBarnExploreMiniGame();
	}

	if ( !AGameLoader::IsBarnExploreMiniGame() )
	{
		// Ben's grave: the music gets louder close to it
		if ( AMusicManager::GetSingleton()->IsNearBenGrave() )
		{
			TVector4 vecPosition = *m_pPlayerProxy->GetUnit()->GetSteer()->GetPosition();
			TFLOAT   fVolume     = TVector4::DistanceSq( AMusicManager::GetSingleton()->GetBenGravePosition(), vecPosition );
			TMath::Clip( fVolume, 0.0f, 4900.0f );

			fVolume = 1.0f - fVolume * ( 1.0f / 4900.0f );
			fVolume = TMath::Pow( fVolume, 4.0f );

			AMultiplexStream::GetSingleton()->SetVolume( fVolume, 0, TTRUE );
			AMultiplexStream::GetSingleton()->SetVolume( fVolume, 1, TTRUE );
		}
	}

	if ( !ABikeManager::GetSingleton()->IsPlayerOnBike() )
	{
		if ( !m_pNbBull && AGameTimeManager::GetSingleton()->IsItNight() )
		{
			if ( AUnit* pNbBull = AUnitManager::GetSingleton()->FindUnitByTypeName( TPS8D( "nb_bull" ) ) )
				m_pNbBull = pNbBull;
		}

		if ( m_pNbBull && !m_pNbBull->HasInteraction() )
		{
			AAnimalPopulationManager::GetSingleton()->SetMarker( m_pNbBull, AAnimalPopulationManager::MARKERTYPE_QUESTION, TTRUE );
		}

		if ( !m_pPip )
		{
			if ( AUnit* pPip = AUnitManager::GetSingleton()->FindUnitByTypeName( TPS8( pip ) ) )
				m_pPip = pPip;
		}

		if ( m_pPip && !TSTATICCAST( AAnimatedUnit, m_pPip )->GetAnimatableObject()->GetParentObject() )
		{
			// Pip isn't carried, end the interaction the player has with him
			APlayerControlComponent* pPlayerControl = TSTATICCAST( APlayerControlComponent, m_pPlayerProxy->GetUnit()->FindComponent( &TGetClass( APlayerControlComponent ) ) );

			if ( pPlayerControl )
			{
				AInteractionSequence* pSequence = pPlayerControl->FindUnitInteraction( m_pPip, TTRUE );

				if ( pSequence )
				{
					if ( pSequence->GetInteraction()->GetFlags() & AInteraction::FLAGS_MISSION_OBJECTIVE )
						AAnimalPopulationManager::GetSingleton()->SetMarker( m_pPip, AAnimalPopulationManager::MARKERTYPE_QUESTION, TTRUE );

					pSequence->SetFlags( pSequence->GetFlags() | AInteractionSequence::FLAGS_ENDED );
					pPlayerControl->ReleaseInteraction( &pSequence );
				}
			}
		}
	}

	m_oMiniMap.SetPlayer( m_pPlayerProxy );

	ABikeManager::GetSingleton()->RespawnBikes( a_fDeltaTime, TFALSE );

	UpdateSim( a_fDeltaTime );

	if ( AGameStateController::GetSingleton()->GetCurrentState()->GetClass() == &TGetClass( ASimState ) )
	{
		// Swap the scarecrow models at the beginning of the day and of the night
		if ( m_pScarecrowDayModel && m_pScarecrowNightModel && !m_pScarecrowDayModel->IsVisible() )
		{
			if ( AGameTimeManager::GetSingleton()->IsDaytime() )
			{
				if ( !m_pScarecrowDayModel->IsUpdatingSkeleton() || m_pScarecrowNightModel->IsUpdatingSkeleton() )
				{
					m_pScarecrowDayModel->SetSkeletonUpdating( TTRUE );
					m_pScarecrowNightModel->SetSkeletonUpdating( TFALSE );
				}
			}
			else if ( AGameTimeManager::GetSingleton()->IsItNight() )
			{
				if ( !m_pScarecrowNightModel->IsUpdatingSkeleton() || m_pScarecrowDayModel->IsUpdatingSkeleton() )
				{
					m_pScarecrowDayModel->SetSkeletonUpdating( TFALSE );
					m_pScarecrowNightModel->SetSkeletonUpdating( TTRUE );
				}
			}
		}
	}

	if ( m_pLadderBeansGame )
		m_pLadderBeansGame->Update( a_fDeltaTime );

	// Not during the quest sequences
	if ( !AQuestManager::GetSingleton()->IsTimerPaused() )
	{
		if ( HASANYFLAG( m_eFlags, FLAGS_DAILY_EVENTS ) )
			UpdateDailyEvents();
		else if ( HASANYFLAG( m_eFlags, FLAGS_FARMER_TRUCK ) )
			UpdateFarmerTruck();
		else if ( HASANYFLAG( m_eFlags, FLAGS_SHOW_WEEKDAY ) )
			UpdateWeekdayBanner();
	}

	if ( !AGameLoader::IsBarnExploreMiniGame() )
	{
		// Night ambience of the barn while it's open at night (unless the quest music plays)
		if ( AGameTimeManager::GetSingleton()->IsItNight() == TTRUE && ABarnOpenRequirement::IsBarnOpenAtNight( -1 ) == TTRUE &&
		     !HASANYFLAG( m_eFlags, FLAGS_RESTART_MUSIC ) && AQuestManager::GetSingleton()->GetQuestMusicCue() == -1 )
		{
			if ( m_iNightAmbienceCue == -1 )
			{
				AGateManager::Gate* pNightBarnGate = AGateManager::GetSingleton()->FindGateOfType( AGateManager::GATETYPE_NIGHTBARN );
				TVector4            vecPosition    = pNightBarnGate->GetPosition();

				m_iNightAmbienceCue = ASoundManager::GetSingleton()->PlayCueAtPositionEx( soundbank::BARNYARD_BARNNIGHTCLUB, vecPosition, 0.0f, TFALSE, 0.0f, -1 );
				ASoundManager::GetSingleton()->FadeCue( m_iNightAmbienceCue, 1.0f, 1.0f, 3.0f );
			}
		}
		else if ( m_iNightAmbienceCue != -1 )
		{
			ASoundManager::GetSingleton()->FadeOutCue( m_iNightAmbienceCue, 1.0f, TTRUE, 3.0f );
			m_iNightAmbienceCue = -1;
		}
	}

	AGameState::OnUpdate( a_fDeltaTime );

	if ( AGameLoader::GetLastScriptedSequence() != -1 )
	{
		if ( AGameLoader::GetLastScriptedSequence() == 9 )
			AGameStateController::GetSingleton()->SaveCurrentGameState( TFALSE, TFALSE, TFALSE );

		AGameLoader::ResetLastScriptedSequence();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0043f080
void ASimState::OnInsertion()
{
	m_strBarnExitTrigger = TPS8D( "barn_exit_trigger" );

	// NOTE: inlined AGameState::SaveCameraFOV, saves the FOV of the current camera
	if ( ACameraManager::IsSingletonCreated() )
	{
		if ( ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera() )
			m_fFOV = pCamera->GetFOV();
	}

	RefreshPlayerProxy();
	m_HUDParams.SetFlags( AHudElement::MASK_SIM );

	if ( !AGameLoader::IsBarnExploreMiniGame() )
	{
		AMiscFunctions::SetSuperSquirtParticlesEnabled( ASpecialEventRequirement::IsSuperSquirtAvailable() );
		AMilkManager::Load( "Data/MilkParticles/SimMilk.trb" );
	}

	AHudManager::GetSingleton()->AddElement( &m_oItemCountHud, TFALSE, TTRUE, TFALSE );
	m_oItemCountHud.SetVisible( TFALSE );

	// The coin spawners of the gold rush
	ACoinPickupMicroGame::SpawnCoinSpawners();

	// The bike racks and the bike
	ABikeManager::GetSingleton()->CreateRacks();
	ABikeManager::GetSingleton()->SpawnBikesAtRacks( 1 );

	if ( GetClass() == &TGetClass( ASimState ) )
	{
		AGameLoader::RestoreSimBike();
	}

	// Pip was following the player when the sim was left
	if ( ASimPersistData::GetSingleton()->IsPipFollowingPlayer() )
	{
		AUnit* pPip = AUnitNameManager::GetSingleton()->RetrieveByName( TPS8( pip ) );

		if ( !pPip )
		{
			pPip = AUnitManager::GetSingleton()->SpawnUnit( TPS8( pip ), TVector4::VEC_ZERO, TVector4::VEC_POSZ );
			AUnitNameManager::GetSingleton()->SetUnitName( TPS8( pip ), pPip->GetId() );
			m_bSpawnedPip = TTRUE;
		}

		pPip->SetMovement( TFALSE );
		TSTATICCAST( AAnimatedUnit, APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) )->AttachUnit( pPip, TPS8( c_head ) );
		pPip->SetNoQuery( TTRUE );
		ASimPersistData::GetSingleton()->SetPipFollowingPlayer( TFALSE );
		AAnimalPopulationManager::GetSingleton()->RemoveMarker( pPip );
	}

	if ( AGameStateController::GetSingleton()->GetCurrentState()->GetClass() == &TGetClass( ASimState ) )
	{
		if ( AUnit* pPip = AUnitNameManager::GetSingleton()->RetrieveByName( TPS8( pip ) ) )
		{
			AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, pPip->GetSteer() );
			pSteer->Update( 0.25f );

			pSteer->SetFlags( pSteer->GetFlags() | AInteriorSteer::FLAGS_KINEMATICS_NO_PUSH | AInteriorSteer::FLAGS_KINEMATICS_NO_AVOID );

			pSteer->EnableKinematicsObstructionStop( TTRUE );

			pSteer->SetMode( AInteriorSteer::MODE_STILL );
		}
	}

	m_oSimHudGroup.Create();

	if ( !AGameLoader::IsBarnExploreMiniGame() )
	{
		m_oMiniMap.Create();
		AAnimalPopulationManager::GetSingleton()->AddMarkersToMiniMap( &m_oMiniMap );

		AQuestManager::GetSingleton()->AddElementMarkers();
	}

	m_oDayOfWeek.Create();

	// Renders objects on top of the GUI
	new AGUI2ModelRendererScreen();
	AGUI2ModelRendererScreen::GetSingleton()->SetActive( TTRUE );

	m_oDateChangeListener.Connect( *AGameTimeManager::GetSingleton()->GetDateChangeEmitter(), this, OnDateChangeCallback, 0 );
	AGameState::OnInsertion();

	if ( AGameStateController::GetSingleton()->GetCurrentState()->GetClass() == &TGetClass( ASimState ) )
	{
		ATerrainInterface* pTerrain    = ATerrainInterface::GetSingleton();
		AModelRepos*       pModelRepos = AModelRepos::GetSingleton();

		// The scarecrow, one model for the day and one for the night
		if ( pTerrain->FindLocator( "scarecrow" ) )
		{
			TVector4    vecPosition;
			TQuaternion quatRotation;
			pTerrain->GetLocatorPosAndRot( "scarecrow", vecPosition, quatRotation );

			TMatrix44 mTransform;
			pTerrain->GetLocatorTransform( "scarecrow", mTransform );

			m_pScarecrowDayModel = pModelRepos->InstantiateNewModel( TPS8D( "data/models/scarecrow_day" ), TNULL );

			if ( m_pScarecrowDayModel )
			{
				m_pScarecrowDayModel->SetTransform( mTransform );
				m_pScarecrowDayModel->SetSkeletonUpdating( TTRUE );
				m_oScarecrowDayCollision.Create( m_pScarecrowDayModel->GetSceneObject() );
			}

			m_pScarecrowNightModel = pModelRepos->InstantiateNewModel( TPS8D( "data/models/scarecrow_night" ), TNULL );

			if ( m_pScarecrowNightModel )
			{
				m_pScarecrowNightModel->SetTransform( mTransform );
				m_pScarecrowNightModel->SetSkeletonUpdating( TFALSE );
				m_oScarecrowNightCollision.Create( m_pScarecrowNightModel->GetSceneObject() );
			}
		}

		// The gate of the vegetable garden
		m_pGateModel = pModelRepos->InstantiateNewModel( TPS8D( "data/models/fncveggateopen" ), TNULL );

		if ( m_pGateModel )
		{
			m_pGateModel->SetSkeletonUpdating( TTRUE );
			m_pGateModel->SetUnlit();

			TMatrix44 mTransform;
			pTerrain->GetLocatorTransform( "fncveggate", mTransform );
			m_pGateModel->SetTransform( mTransform );

			TFLOAT fLOD0Distance, fBlendEnd, fBlendStart;
			AInstanceManager::GetSingleton()->GetVisibilityDistances( fLOD0Distance, fBlendEnd, fBlendStart );

			m_pGateModel->GetSceneObject()->GetInstance()->GetModel()->m_aLODDistances[ 0 ] = fLOD0Distance;

			m_oGateCollision.Create( m_pGateModel->GetSceneObject() );
		}

		m_oWindmillHelper.CreateFarmObjects();

		// Only the English barn sign is animated
		T2Locale::Lang eLanguage = g_pLocaleManager->GetLanguage();
		m_oWindmillHelper.SetBarnSignAnimated( eLanguage == ALocaleManager::Lang_English || eLanguage == ALocaleManager::Lang_EnglishUK );

		ASimAnimModelHelperManager::GetSingleton()->AddModelHelper( &m_oWindmillHelper );
	}

	ARenderer* pRenderer = ARenderer::GetSingleton();
	m_fSavedNear         = pRenderer->GetNear();
	m_fSavedFar          = pRenderer->GetFar();
	pRenderer->SetNear( 0.1f );

	AAnimalPopulationManager::GetSingleton()->SetMarkersEnabled( TTRUE );

	// Place the camera behind the player
	{
		ASteer*                pSteer     = m_pPlayerProxy->GetUnit()->GetSteer();
		AGTAStyleCameraHelper* pGTAHelper = TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 ) );

		TVector4 vecPosition;
		TVector4 vecDirection;
		pSteer->CopyPosition( vecPosition );
		pSteer->CopyDirection( vecDirection );

		// NOTE: the original doesn't check this, the helpers aren't all created yet
		if ( pGTAHelper )
		{
			TVector4 vecTarget( vecPosition.x, vecPosition.y - pGTAHelper->GetHeightOffset(), vecPosition.z, 1.0f );

			TVector4 vecOffset;
			vecOffset.Multiply( vecDirection, -5.0f );

			TVector4 vecCamera( vecOffset.x + vecTarget.x, vecOffset.y + vecTarget.y, vecOffset.z + vecTarget.z, 1.0f );
			pGTAHelper->SetPosition( vecCamera, vecTarget, TTRUE );
		}
	}

	// The markers of the elements that still run
	AQuestManager* pQuestManager = AQuestManager::GetSingleton();

	for ( TINT i = 0; i < pQuestManager->GetNumAssignedQuests(); i++ )
	{
		AQuestInstance* pInstance = pQuestManager->GetAssignedQuest( i );
		AQuest*         pQuest    = pInstance->GetQuest();

		for ( TINT k = 0; k < pQuest->GetNumElements(); k++ )
		{
			if ( !pInstance->IsElementDone( k ) )
				pQuest->GetElement( k )->GetLocator().AddMarker();
		}
	}

	// Regrow everything when the sim is entered before the main game has been loaded
	if ( GetClass() == &TGetClass( ASimState ) && !ASimPersistData::GetSingleton()->HasFlags( ASimPersistData::FLAGS_MAINGAME_LOADED ) )
		ARegrowthManager::GetSingleton()->NewDayAll();

	if ( AGameLoader::IsMainGameLoaded() )
	{
		AReplayManager::GetSingleton()->OnEnterFarm();

		m_pLadderBeansGame = new ALadderBeansGame();
		m_pLadderBeansGame->Create();
	}
	else if ( AGameLoader::IsBarnExploreMiniGame() )
	{
		AReplayManager::GetSingleton()->OnEnterBarn();
	}

	APlayerControlComponent* pControlComponent = TSTATICCAST( APlayerControlComponent, APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->FindComponent( &TGetClass( APlayerControlComponent ) ) );
	pControlComponent->SetCanSprint( TTRUE );
	APlayerProfileManager::GetSingleton()->GetMainProfile()->SetUpdateStamina( TTRUE );
	AHintManager::GetSingleton()->Load();

	if ( !AGameLoader::IsBarnExploreMiniGame() )
	{
		AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

		m_oMilkReticles.Create( pPlayer, 50.0f, TNULL );
	}

	if ( AGameStateController::GetSingleton()->GetCurrentState()->GetClass() == &TGetClass( ASimState ) )
	{
		AInteriorNetworkManager::GetSingleton()->FindNetwork( TPS8( park2 ) );

		m_oMilkBottles.LoadFromLocators( ATerrainInterface::GetSingleton()->GetLocatorList() );
	}

	if ( !AGameLoader::IsBarnExploreMiniGame() )
	{
		if ( !AGameTimeManager::GetSingleton()->IsItNight() )
			m_pPip = AUnitManager::GetSingleton()->FindUnitByTypeName( TPS8( pip ) );

		if ( !AGameLoader::IsBarnExploreMiniGame() && AGameTimeManager::GetSingleton()->IsItNight() )
		{
			AUnit* pNbBull = AUnitManager::GetSingleton()->FindUnitByTypeName( TPS8D( "nb_bull" ) );
			m_pNbBull      = TNULL;

			if ( pNbBull )
				m_pNbBull = pNbBull;
		}
	}

	// NOTE: inlined AGameTimeManager::GetCurrentMinute
	m_iCachedMinute = AGameTimeManager::GetSingleton()->GetCurrentMinute();

	if ( !AGameLoader::IsBarnExploreMiniGame() )
	{
		// The farmer is at home at night
		if ( AGameTimeManager::GetSingleton()->IsItNight() )
		{
			AFarmerTruck::MovePlayerToSafePos( TTRUE );
			AFarmerTruck::Park();
		}
		else
		{
			AFarmerTruck::Hide();
		}

		m_eFlags &= ~FLAGS_FARMER_TRUCK;

		AReplayManager::GetSingleton()->UpdateReplays();
	}

	if ( ASimPersistData::GetSingleton()->GetItemCountHudState().bSaved )
		m_oItemCountHud.LoadState( &ASimPersistData::GetSingleton()->GetItemCountHudState() );

	AGameStateController::GetSingleton()->ShowOverlay( TTRUE, TFALSE, 1.0f, TFALSE );

	// Inside the barn
	ATerrainInterface* pTerrain  = ATerrainInterface::GetSingleton();
	const TCHAR*       szSection = pTerrain->GetSection( pTerrain->GetSectionID() )->GetName();

	if ( TStringManager::String8CompareNoCase( szSection, "Barn", 4 ) == 0 || TStringManager::String8CompareNoCase( szSection, "barnnorth", 9 ) == 0 )
		AGameTimeFXManager::GetSingleton()->SetInBarn( TTRUE );
}

// $Barnyard: FUNCTION 0043d040
void ASimState::OnRemoval()
{
	AInputHandler* pInputHandler = AInputHandler::GetSingleton();
	pInputHandler->StopAllRumbles();

	// NOTE: the original calls DOES_NOTHING (nullsub_2) with 0 here

	pInputHandler->StopRumble( pInputHandler->GetMainController() );

	// NOTE: inlined AGameState::RestoreCameraFOV, restores the FOV of the current camera
	if ( ACameraManager::IsSingletonCreated() )
	{
		if ( ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera() )
			pCamera->SetFOV( m_fFOV );
	}

	if ( GetClass() == &TGetClass( ASimState ) )
	{
		AGameLoader::StoreSimBike();
	}

	m_pPip = AUnitManager::GetSingleton()->FindUnitByTypeName( TPS8( pip ) );

	ASimPersistData::GetSingleton()->SetPipFollowingPlayer( TFALSE );

	if ( m_pPip && TSTATICCAST( AAnimatedUnit, m_pPip )->GetAnimatableObject()->GetParentObject() )
	{
		// Was pip following the player?
		AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerSet( APlayerManager::PLAYERSET_SIM )->GetPlayerUnit( 0 );

		if ( pPlayer && TSTATICCAST( AAnimatedUnit, m_pPip )->GetAnimatableObject()->GetParentObject() == TSTATICCAST( AAnimatedUnit, pPlayer )->GetAnimatableObject() )
		{
			ASimPersistData::GetSingleton()->SetPipFollowingPlayer( TTRUE );

			if ( m_bSpawnedPip == TTRUE )
				AUnitManager::GetSingleton()->DestroyUnit( m_pPip );
		}
	}

	AQuestManager::GetSingleton()->HideTimeLeft();

	ACoinPickupMicroGame::DestroyCoinSpawners();
	AReplayManager::GetSingleton()->DestroyStars();

	if ( ABikeManager::GetSingleton()->IsPlayerOnBike() == TTRUE )
	{
		ABikeManager::GetSingleton()->DestroyPlayerBike( TFALSE );
		ABikeManager::GetSingleton()->EndRide( TFALSE );
	}

	ACameraManager::GetSingleton()->DetachCameraHelpers();

	ARenderer* pRenderer = ARenderer::GetSingleton();
	pRenderer->SetNear( m_fSavedNear );
	m_fSavedNear = 1.0f;
	pRenderer->SetFar( m_fSavedFar );

	m_oDateChangeListener.Disconnect();

	ASimAnimModelHelperManager::GetSingleton()->RemoveModelHelper( &m_oWindmillHelper );
	m_oWindmillHelper.OnDestroy();

	ACameraManager::GetSingleton()->ResetHelperTargets();

	ACameraManager::GetSingleton()->DetachSplitscreenCameraHelpers();

	ASimpleProjectileManager::GetSingleton()->DestroyAllPools();

	if ( AMilkManager::IsSingletonCreated() )
		delete AMilkManager::GetSingleton();

	m_oSimHudGroup.Destroy();

	// Remove the markers of the elements
	for ( TINT i = 0; i < AQuestManager::GetSingleton()->GetNumAssignedQuests(); i++ )
		AQuestManager::GetSingleton()->GetAssignedQuest( i )->RemoveMarkers();

	m_oMiniMap.Destroy();
	m_oDayOfWeek.Destroy();

	if ( AGUI2ModelRendererScreen::GetSingleton() )
		delete AGUI2ModelRendererScreen::GetSingleton();

	m_oItemCountHud.Detach();

	AModelRepos* pModelRepos = AModelRepos::GetSingleton();

	if ( m_pScarecrowDayModel )
	{
		m_oScarecrowDayCollision.UnlinkCollisionObject();
		pModelRepos->DestroyModelInstance( m_pScarecrowDayModel );
		m_pScarecrowDayModel = TNULL;
	}

	if ( m_pScarecrowNightModel )
	{
		m_oScarecrowNightCollision.UnlinkCollisionObject();
		pModelRepos->DestroyModelInstance( m_pScarecrowNightModel );
		m_pScarecrowNightModel = TNULL;
	}

	if ( m_pGateModel )
	{
		m_oGateCollision.UnlinkCollisionObject();
		pModelRepos->DestroyModelInstance( m_pGateModel );
		m_pGateModel = TNULL;
	}

	AUnit* pUnitToDestroy = m_pUnitToDestroy ? m_pUnitToDestroy->GetUnit() : TNULL;

	if ( pUnitToDestroy )
		AUnitManager::GetSingleton()->DestroyUnit( pUnitToDestroy );

	AAnimalPopulationManager::GetSingleton()->SetMarkersEnabled( TFALSE );

	AHintManager::GetSingleton()->Unload();

	if ( !AGameLoader::IsBarnExploreMiniGame() )
		m_oMilkBottles.Clear();

	m_oItemCountHud.SaveState( &ASimPersistData::GetSingleton()->GetItemCountHudState() );

	if ( m_iNightAmbienceCue != -1 )
	{
		ASoundManager::GetSingleton()->StopCue( m_iNightAmbienceCue );
		m_iNightAmbienceCue = -1;
	}

	AParticleEffectManager::GetSingleton()->DetachAllEffects();
	AGameState::OnRemoval();

	TRenderInterface::GetSingleton()->FlushDyingResources();
	TRenderInterface::GetSingleton()->FlushDyingResources();
}

// $Barnyard: FUNCTION 0043dae0
void ASimState::OnActivate()
{
	RefreshPlayerProxy();

	AHighScoreManager::UpdateChampionChart();

	AHintManager::GetSingleton()->OnActivate();

	AQuestManager::GetSingleton()->OnActivate();

	AGUI2::GetSingleton()->SetCursorVisible( TFALSE );

	// Hide the HUD while transitioning or while a message is shown
	const TUINT16 uiControllerFlags = AGameStateController::GetSingleton()->GetFlags();
	TBOOL         bHideHUD          = HASANYFLAG( uiControllerFlags, AGameStateController::FLAGS_IGNORE_INPUT | AGameStateController::FLAGS_LEAVE_IMMEDIATELY ) ||
	    HASANYFLAG( uiControllerFlags, AGameStateController::FLAGS_UNK10 );

	if ( !bHideHUD )
	{
		// A minigame is about to start
		bHideHUD = !AMiniGameManager::GetSingleton()->GetNextMiniGameName().IsEmpty();
	}

	m_HUDParams.SetFlags( bHideHUD ? 0 : AHudElement::MASK_SIM );

	ACameraManager* pCameraManager = ACameraManager::GetSingleton();

	if ( !ABikeManager::GetSingleton()->IsPlayerOnBike() )
	{
		// Follow the player with the GTA camera
		ACameraHelper* pGTAHelper = pCameraManager->GetSplitscreenCameraHelper( 1 );

		// NOTE: the original doesn't check this, the helpers aren't all created yet
		if ( pGTAHelper )
		{
			pGTAHelper->SetTarget( m_pPlayerProxy );
			pCameraManager->SetSmoothTransition( pGTAHelper, 0.6f );
		}
	}

	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
	m_pPlayerProxy = pPlayer->GetSelfProxy();

	if ( !AGameLoader::IsStoryMode() && !ABikeManager::GetSingleton()->IsPlayerOnBike() )
	{
		// Place the camera behind the player
		AGTAStyleCameraHelper* pGTAHelper = TSTATICCAST( AGTAStyleCameraHelper, pCameraManager->GetSplitscreenCameraHelper( 1 ) );
		ASteer*                pSteer     = pPlayer->GetSteer();

		TVector4 vecPosition;
		TVector4 vecDirection;
		pSteer->CopyPosition( vecPosition );
		pSteer->CopyDirection( vecDirection );

		// NOTE: the original doesn't check this, the helpers aren't all created yet
		if ( pGTAHelper )
		{
			TVector4 vecTarget( vecPosition.x, vecPosition.y - pGTAHelper->GetHeightOffset(), vecPosition.z, 1.0f );

			TVector4 vecOffset;
			vecOffset.Multiply( vecDirection, 10.0f );

			TVector4 vecCamera;
			vecCamera.Add( vecTarget, vecOffset );
			vecCamera.AsVector3().RotateY( TMath::PI );

			pGTAHelper->SetPosition( vecCamera, vecTarget, TTRUE );
		}
	}

	if ( AGameLoader::IsLoadingSavedGame() )
	{
		AUnitBehaviour* pBehaviour = TSTATICCAST( AUnitBehaviour, pPlayer->FindComponentExact( &TGetClass( AUnitBehaviour ) ) );

#ifdef BARNYARD_COMMUNITY_PATCH
		// Retail expects the player to have a behaviour
		if ( pBehaviour )
#endif
			pBehaviour->SetCommand( TPS8( player_startgame ), TNULL, TTRUE );
	}

	m_oMiniMap.SetPlayer( m_pPlayerProxy );
	AGUI2::GetSingleton()->SetCursorVisible( TFALSE );

	if ( !AGameLoader::IsBarnExploreMiniGame() )
		m_oMilkReticles.SetActive( TFALSE );

	// NOTE: the original calls DOES_NOTHING (005acd80) on the sim HUD group here

	m_bIsActive = TTRUE;
	AGameState::OnActivate();

	// NOTE: inlined AGameTimeManager::GetCurrentMinute
	m_iCachedMinute = AGameTimeManager::GetSingleton()->GetCurrentMinute();

	if ( !AGameStateController::GetSingleton()->GetCurrentState()->IsA( &TGetClass( ASSeqState ) ) && HASANYFLAG( m_eFlags, FLAGS_RESTART_MUSIC ) )
	{
		AVOStream::GetSingleton()->Close( TTRUE );
		AMusicManager::GetSingleton()->SetEnabled( TTRUE, TTRUE );
		m_eFlags &= ~FLAGS_RESTART_MUSIC;
	}

	if ( AGameStateController::GetSingleton()->GetCurrentState()->GetClass() == &TGetClass( ASimState ) )
	{
		// Show the characters again once the farmer's sequence is over
		if ( HASANYFLAG( m_eFlags, FLAGS_FARMER_SEQUENCE | FLAGS_FARMER_UNK1 ) )
			AFarmerTruck::SetSequenceMode( HASANYFLAG( m_eFlags, FLAGS_FARMER_UNK1 ) );

		if ( HASANYFLAG( m_eFlags, FLAGS_CLOSE_BARN_GATE ) )
		{
			m_eFlags &= ~FLAGS_CLOSE_BARN_GATE;

			AGateManager::GetSingleton()->SetGate( TPS8( barn ), TFALSE );
		}
	}

	// Select the primary launcher
	AUnitPlayer* pUnitPlayer = TSTATICCAST( AUnitPlayer, APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) );

	if ( pUnitPlayer && pUnitPlayer->GetActiveLauncher() && pUnitPlayer->GetActiveLauncher() == pUnitPlayer->GetSecondaryLauncher() )
		pUnitPlayer->SelectLauncher( TFALSE );
}

// $Barnyard: FUNCTION 0043d3c0
void ASimState::OnDeactivate()
{
	m_bIsActive = TFALSE;

	AHintManager::GetSingleton()->OnDeactivate();

	AQuestManager::GetSingleton()->OnDeactivate();

	g_oSystemManager.GetScheduler()->SetDebugDeltaTimeMult( TFALSE, 1.0f );

	if ( !AGameLoader::IsBarnExploreMiniGame() )
	{
		m_oSimHudGroup.GetTimeLeftElement()->GetTimer().StopCue();

		m_oMilkReticles.SetActive( TFALSE );
	}

	AGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 0043d5b0
void ASimState::UpdateSim( TFLOAT a_fDeltaTime )
{
	if ( !AGameLoader::IsBarnExploreMiniGame() )
	{
		AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

		APlayerControlComponent* pControlComponent = TSTATICCAST( APlayerControlComponent, pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) );
		m_oMilkBottles.Update( a_fDeltaTime, pControlComponent && pControlComponent->CanUseHand() );
	}

	AMilkManager::GetSingleton()->Update( a_fDeltaTime );

	AUnitPlayer* pPlayer = TSTATICCAST( AUnitPlayer, APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) );

	if ( AProjectileLauncher* pLauncher = pPlayer->GetActiveLauncher() )
		pLauncher->Update( a_fDeltaTime );

	m_oMiniMap.Update( a_fDeltaTime );
	m_oSimHudGroup.Update( a_fDeltaTime );
}

// $Barnyard: FUNCTION 0043d6a0
void ASimState::RefreshPlayerProxy()
{
	m_pPlayerProxy = APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSelfProxy();
}

// $Barnyard: FUNCTION 0043d4d0
void ASimState::UpdateFarmerTruck()
{
	// The farmer is at home at night
	if ( AGameTimeManager::GetSingleton()->IsItNight() )
	{
		AFarmerTruck::MovePlayerToSafePos( TTRUE );
		AFarmerTruck::Park();
	}
	else
	{
		AFarmerTruck::Hide();
	}

	m_eFlags &= ~FLAGS_FARMER_TRUCK;
}

// $Barnyard: FUNCTION 0043d650
void ASimState::UpdateWeekdayBanner()
{
	// Not while the quest banner is shown
	if ( !AQuestManager::GetSingleton()->IsBannerShown() )
	{
		// Wait until the player is done with the current interaction
		if ( !m_pPlayerProxy || m_pPlayerProxy->GetUnit()->HasInteraction() != TTRUE )
		{
			m_oDayOfWeek.Show( AGameTimeManager::GetSingleton()->GetWeekDay() );
			m_eFlags &= ~FLAGS_SHOW_WEEKDAY;
		}
	}
}

// Daily events: the farmer leaving or arriving (the SSNBO/SSNBC/farmer_arive/farmer_depart sequences), the music and the barn
// $Barnyard: FUNCTION 0043dfa0
void ASimState::UpdateDailyEvents()
{
	if ( AQuestManager::GetSingleton()->IsBannerShown() )
		return;

	// Wait until the player is done with the current interaction
	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	if ( pPlayer && pPlayer->HasInteraction() )
		return;

	// Wait until the barn gate is open
	AGateManager::Gate* pBarnGate = AGateManager::GetSingleton()->FindGateOfType( AGateManager::GATETYPE_BARN );

	if ( pBarnGate && AGateManager::GetSingleton()->IsGateOpening( pBarnGate ) == TTRUE )
		return;

	if ( AGameLoader::IsBarnExploreMiniGame() )
		return;

	if ( AGameStateController::GetSingleton()->GetCurrentState()->GetClass() != &TGetClass( ASimState ) )
		return;

	AGameTimeManager*  pGameTimeManager = AGameTimeManager::GetSingleton();
	ATerrainInterface* pTerrain         = ATerrainInterface::GetSingleton();
	ASimPersistData*   pPersistData     = ASimPersistData::GetSingleton();

	// NOTE: inlined AGameTimeManager::GetCurrentMinute
	TINT iMinute = pGameTimeManager->GetCurrentMinute();
	TINT iDay    = pGameTimeManager->GetDay() + 30 * pGameTimeManager->GetMonth();

	// The farmer leaves in the morning
	if ( pPersistData->GetFarmerDepartDay() != iDay && pGameTimeManager->GetCurrentDayPhase() == AGameTimeManager::DAYPHASE_DAWN )
	{
		m_eFlags &= ~FLAGS_DAILY_EVENTS;

		TINT iBarnSection      = pTerrain->GetSectionIndex( "Barn" );
		TINT iBarnNorthSection = pTerrain->GetSectionIndex( "BarnNorth" );
		pPersistData->SetFarmerDepartDay( iDay );

		if ( pTerrain->GetSectionID() != iBarnSection && pTerrain->GetSectionID() != iBarnNorthSection )
		{
			// The player isn't in the barn, only the farmer's line is heard
			if ( AUnit* pPlayerUnit = APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) )
				AVOStream::GetSingleton()->PlayAtPosition( 1237, *pPlayerUnit->GetSteer()->GetPosition() );
		}
		else
		{
			AFarmerTruck::SetSequenceMode( TTRUE );
			m_eFlags |= FLAGS_FARMER_SEQUENCE;

			if ( !AGateManager::GetSingleton()->IsGateOpen( TPS8( barn ) ) )
			{
				m_eFlags |= FLAGS_CLOSE_BARN_GATE;
				AGateManager::GetSingleton()->SetGate( TPS8( barn ), TTRUE );
			}

			AFarmerTruck::Show();

			AMusicManager::GetSingleton()->SetEnabled( TFALSE, TTRUE );

			AVOStream::GetSingleton()->Open( "data/sound/FSB/ss.fsb", TTRUE, TTRUE );
			m_eFlags |= FLAGS_RESTART_MUSIC;

			// Select the primary launcher
			AUnitPlayer* pUnitPlayer = TSTATICCAST( AUnitPlayer, APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) );

			if ( pUnitPlayer && pUnitPlayer->GetActiveLauncher() && pUnitPlayer->GetActiveLauncher() == pUnitPlayer->GetSecondaryLauncher() )
				pUnitPlayer->SelectLauncher( TFALSE );

			TBOOL bBarnOpenAtNight = ABarnOpenRequirement::IsBarnOpenAtNight( -1 );

			// NOTE: the cue isn't stopped
			m_iNightAmbienceCue = -1;

			ASSeqState* pSSeqState;

			if ( bBarnOpenAtNight )
				pSSeqState = new ASSeqState( "SSNBC_main", "SSNBC_main", "SSNBC_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE );
			else
				pSSeqState = new ASSeqState( "farmer_depart", "farmer_depart", "ss_f_inout_camerab", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE );

			AGameStateController::GetSingleton()->PushState( pSSeqState );
		}
	}

	// The farmer comes back at night
	if ( pPersistData->GetFarmerArriveDay() != iDay && pGameTimeManager->GetCurrentDayPhase() == AGameTimeManager::DAYPHASE_NIGHT )
	{
		m_eFlags &= ~FLAGS_DAILY_EVENTS;
		pPersistData->SetFarmerArriveDay( iDay );

		TINT iBarnSection      = pTerrain->GetSectionIndex( "Barn" );
		TINT iBarnNorthSection = pTerrain->GetSectionIndex( "BarnNorth" );

		if ( pTerrain->GetSectionID() != iBarnSection && pTerrain->GetSectionID() != iBarnNorthSection )
		{
			// The player isn't in the barn, only the farmer's line is heard
			if ( AUnit* pPlayerUnit = APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) )
				AVOStream::GetSingleton()->PlayAtPosition( 1236, *pPlayerUnit->GetSteer()->GetPosition() );
		}
		else
		{
			m_eFlags |= FLAGS_FARMER_SEQUENCE | FLAGS_FARMER_ARRIVED;
			AFarmerTruck::SetSequenceMode( TTRUE );

			if ( !AGateManager::GetSingleton()->IsGateOpen( TPS8( barn ) ) )
			{
				m_eFlags |= FLAGS_CLOSE_BARN_GATE;
				AGateManager::GetSingleton()->SetGate( TPS8( barn ), TTRUE );
			}

			AFarmerTruck::Show();

			AMusicManager::GetSingleton()->SetEnabled( TFALSE, TTRUE );

			AVOStream::GetSingleton()->Open( "data/sound/FSB/ss.fsb", TTRUE, TTRUE );
			m_eFlags |= FLAGS_RESTART_MUSIC;

			// Select the primary launcher
			AUnitPlayer* pUnitPlayer = TSTATICCAST( AUnitPlayer, APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) );

			if ( pUnitPlayer && pUnitPlayer->GetActiveLauncher() && pUnitPlayer->GetActiveLauncher() == pUnitPlayer->GetSecondaryLauncher() )
				pUnitPlayer->SelectLauncher( TFALSE );

			// The barn stays closed during the last tiers of the story
			AQuestManager* pQuestManager = AQuestManager::GetSingleton();
			TBOOL          bFinalTiers   = TFALSE;
			TINT           iAbsTier      = pQuestManager->GetCurAbsQuestTier();

			if ( iAbsTier >= pQuestManager->GetAbsQuestTier( 3, 14 ) )
				bFinalTiers = iAbsTier <= pQuestManager->GetAbsQuestTier( 3, 15 );

			ASSeqState* pSSeqState;

			if ( !ABarnOpenRequirement::IsBarnOpenAtNight( -1 ) || bFinalTiers )
			{
				// NOTE: the cue isn't stopped
				m_iNightAmbienceCue = -1;
				pSSeqState          = new ASSeqState( "farmer_arive", "farmer_arive", "ss_f_inout_camerab", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE );
			}
			else
			{
				// NOTE: the cue isn't stopped
				m_iNightAmbienceCue = -1;
				pSSeqState          = new ASSeqState( "SSNBO_main", "SSNBO_main", "SSNBO_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE );
			}

			AGameStateController::GetSingleton()->PushState( pSSeqState );
		}
	}

	m_iCachedMinute = iMinute;
}

#ifdef BARNYARD_LATEST

// Ben's tomato launcher is shown to the player (SSUL_Main, Wii only), started by AQuestManager::OnUpdate once
// "Ben Tomato Launcher" is done. The tomato gate stays closed during it, ASSeqState::OnRemoval opens it.
// $BarnyardWiiJP: FUNCTION 8014e940
void ASimState::StartTomatoLauncherSequence()
{
	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
	pTerrain->ForceSection( pTerrain->GetSectionIndex( "Meadowsouth" ), TTRUE );
	AGameLoader::Terrain_Load();

	AGateManager::GetSingleton()->SetTomatoGateOpen( TFALSE );

	AMusicManager::GetSingleton()->SetEnabled( TFALSE, TTRUE );

	AVOStream::GetSingleton()->Open( "data/sound/FSB/ss.fsb", TTRUE, TTRUE );
	m_eFlags |= FLAGS_RESTART_MUSIC;

	// Select the primary launcher
	AUnitPlayer* pUnitPlayer = TSTATICCAST( AUnitPlayer, APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) );

	if ( pUnitPlayer && pUnitPlayer->GetActiveLauncher() && pUnitPlayer->GetActiveLauncher() == pUnitPlayer->GetSecondaryLauncher() )
		pUnitPlayer->SelectLauncher( TFALSE );

	ASSeqState* pSSeqState = new ASSeqState( "SSUL_Main", "SSUL_main", "SSUL_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE );
	AGameStateController::GetSingleton()->PushState( pSSeqState );
}

// The farmer comes home to Daisy's birthday party with the cake (SSBFF_Main, Wii only).
// Like the farmer's arrival at night, but the truck is parked straight away and it counts as today's arrival.
// $BarnyardWiiJP: FUNCTION 8014eab4
void ASimState::StartBirthdayCakeSequence()
{
	AFarmerTruck::SetSequenceMode( TTRUE );
	m_eFlags |= FLAGS_FARMER_SEQUENCE;

	AFarmerTruck::Show();
	AFarmerTruck::Park();

	AMusicManager::GetSingleton()->SetEnabled( TFALSE, TTRUE );

	AVOStream::GetSingleton()->Open( "data/sound/FSB/ss.fsb", TTRUE, TTRUE );
	m_eFlags |= FLAGS_RESTART_MUSIC;

	// Select the primary launcher
	AUnitPlayer* pUnitPlayer = TSTATICCAST( AUnitPlayer, APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) );

	if ( pUnitPlayer && pUnitPlayer->GetActiveLauncher() && pUnitPlayer->GetActiveLauncher() == pUnitPlayer->GetSecondaryLauncher() )
		pUnitPlayer->SelectLauncher( TFALSE );

	// The farmer doesn't arrive again tonight
	AGameTimeManager* pGameTimeManager = AGameTimeManager::GetSingleton();
	ASimPersistData::GetSingleton()->SetFarmerArriveDay( pGameTimeManager->GetDay() + 30 * pGameTimeManager->GetMonth() );

	// NOTE: the cue isn't stopped
	m_iNightAmbienceCue = -1;

	ATerrainInterface::GetSingleton()->SetLightOverride( TTRUE );

	ASSeqState* pSSeqState = new ASSeqState( "SSBFF_Main", "SSBFF_main", "SSBFF_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE );
	AGameStateController::GetSingleton()->PushState( pSSeqState );
}

#endif // BARNYARD_LATEST

// A new day has started
// $Barnyard: FUNCTION 0043eac0
void ASimState::OnNewDay()
{
	// Joey can only offer his things once his quest has been done
	AQuestManager* pQuestManager = AQuestManager::GetSingleton();
	TINT           iJoeyTier     = pQuestManager->GetAbsQuestTier( 1, 13 );

	// No offer of Joey today (the first byte of the shop's buffer)
	AHackedQuickSave::Buffer* pShopBuffer = AGopherShopInterface::GetShopBuffer();
	pShopBuffer->SetCursor( 0 );
	pShopBuffer->WriteUInt8( 0xff );
	pShopBuffer->SetSaved( TTRUE );

	// Not while Joey is busy with a quest of the tier
	AQuestManager::QuestTier* pTier = pQuestManager->GetCurrentQuestTier();

	for ( TINT i = 0; i < pTier->uiNumQuests; i++ )
	{
		if ( pQuestManager->GetQuest( pTier->auiQuests[ i ] )->HasUnit( TPS8( joey ) ) )
			return;
	}

	APlayerProfileManager::GetSingleton()->GetMainProfile()->ReleaseItem( TPS8( joey ) );

	if ( pQuestManager->GetCurAbsQuestTier() > iJoeyTier || pQuestManager->IsQuestComplete( TPS8D( "Joey Supply And Demand" ), 1 ) )
	{
		// Joey may have an offer today
		TRandom& rRandom = ARandom::GetSingleton()->m_oRandom;

		if ( TFLOAT( rRandom.RandRaw() >> 1 ) * ( 1.0f / 2147483648.0f ) <= 0.4f )
		{
			TUINT8                          uiOffer = TUINT8( rRandom.GetInt( AGopherShopInterface::GetNumJoeyOffers() ) );
			AGopherShopInterface::JoeyOffer oOffer  = AGopherShopInterface::GetJoeyOffer( uiOffer );

			APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();
			ALocale::FormatTags( AGopherShopInterface::GetJoeyOfferText(), 128, ALocaleManager::Interface()->GetString( oOffer.iTextId ), PPropertyValue( const_cast<TWCHAR*>( pProfile->GetName().GetString( 0 ) ), TFALSE ) );
			pProfile->AddPendingItem( AGopherShopInterface::GetJoeyOfferText(), TPS8( joey ), oOffer.iTextId );

			pShopBuffer->SetCursor( 0 );
			pShopBuffer->WriteUInt8( uiOffer );
			pShopBuffer->SetSaved( TTRUE );
		}
	}
}

// $Barnyard: FUNCTION 0043ef10
TBOOL ASimState::OnDateChange( AGameTimeManager::DateChangeEvent* a_pEvent )
{
	AReplayManager::GetSingleton()->OnDateChange( a_pEvent->m_eCurrDayPhase );
	AReplayManager::GetSingleton()->UpdateReplays();

	if ( a_pEvent->m_eCurrDayPhase == AGameTimeManager::DAYPHASE_DAWN )
		m_eFlags |= FLAGS_SHOW_WEEKDAY;

	if ( a_pEvent->m_eUpdateType == AGameTimeManager::DATECHANGETYPE_DAYPHASE && a_pEvent->m_eCurrDayPhase == AGameTimeManager::DAYPHASE_DAWN )
		OnNewDay();

	if ( a_pEvent->m_eUpdateType == AGameTimeManager::DATECHANGETYPE_DAYPHASE &&
	     ( a_pEvent->m_eCurrDayPhase == AGameTimeManager::DAYPHASE_NIGHT || a_pEvent->m_eCurrDayPhase == AGameTimeManager::DAYPHASE_DAWN ) )
	{
		if ( AGameStateController::GetSingleton()->GetCurrentState()->GetClass() == &TGetClass( ASimState ) )
			m_eFlags |= FLAGS_DAILY_EVENTS;

		m_eFlags |= FLAGS_FARMER_TRUCK;
	}

	if ( AGameLoader::IsBarnExploreMiniGame() || !AGameTimeManager::GetSingleton()->IsItNight() )
		return TTRUE;

	AUnit* pNbBull = AUnitManager::GetSingleton()->FindUnitByTypeName( TPS8D( "nb_bull" ) );
	m_pNbBull      = TNULL;

	if ( pNbBull )
		m_pNbBull = pNbBull;

	return TTRUE;
}

// $Barnyard: FUNCTION 0043f060
TBOOL __stdcall ASimState::OnDateChangeCallback( ASimState* a_pCaller, AGameTimeManager* a_pOwner, AGameTimeManager::DateChangeEvent* a_pEvent )
{
	return a_pCaller->OnDateChange( a_pEvent );
}
