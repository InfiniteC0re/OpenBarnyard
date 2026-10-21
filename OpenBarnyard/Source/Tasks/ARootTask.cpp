#include "pch.h"
#include "ARandom.h"
#include "ARootTask.h"
#include "AGameSystemManager.h"
#include "Assets/AMaterialLibraryManager.h"
#include "Assets/AAssetLoader.h"
#include "Memory/AMemory.h"
#include "Input/AInputHandler.h"
#include "Input/AInputMapManager.h"
#include "GUI/AGUI2.h"
#include "GUI/AGUISystem.h"
#include "GUI/T2GUITextureSectionManager.h"
#include "GUI/AFadeManager.h"
#include "GUI/AGUIPicture.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/ASlideshowState.h"
#include "GameInterface/AMovieState.h"
#include "GameInterface/SaveLoadSKU.h"
#include "Helpers/AGameLoader.h"
#include "GameInterface/AMessagePopupState.h"
#include "GameInterface/AFrontEndMainMenuState2.h"
#include "GameInterface/AFrontEndLanguageState.h"
#include "Locale/ALocaleManager.h"
#include "Sound/ASoundManager.h"
#include "Movie/ABINKMoviePlayer.h"
#include "ALoadScreen.h"
#include "FSM/AFSMManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Physics/ACollisionManager.h"
#include "Helpers/APlayerProgress.h"
#include "ACheatActivator.h"
#include "Helpers/AInstanceManager.h"
#include "Helpers/AGameTimeManager.h"
#include "FX/AGameTimeFXManager.h"
#include "Units/AUnitManager.h"
#include "Units/AInteractionManager.h"

#include "SoundBank/music.h"
#include "SoundBank/ui.h"

#include <Plugins/PPropertyParser/PBProperties.h>
#include <Toshi/TScheduler.h>
#include <Toshi/T2ObjectPool.h>

#include <Render/TBackend.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00402b10, CreateObject 004039e0, CreateObjectInPlace 00403a40
TDEFINE_CLASS( ARootTask );

// Set when the engine starts shutting down (written only)
static TBOOL s_bShuttingDown = TFALSE;

// $Barnyard: FUNCTION 004031e0
ARootTask::ARootTask()
{
	m_pOptions             = TNULL;
	m_pGUISystem           = TNULL;
	m_pGUI2                = TNULL;
	m_pInputHandler        = TNULL;
	m_pRenderer            = TNULL;
	m_pGameStateController = TNULL;
	m_pUnkTask             = TNULL;
	m_pMoviePlayer         = TNULL;
	m_pGameSystemManager   = TNULL;
	m_pSoundManager        = TNULL;
	m_bUnk1                = TFALSE;
	m_bUnk2                = TFALSE;
	m_iUnk3                = 0;
	m_iUnk4                = 0;
	m_bStartedGame         = TFALSE;
	m_bRenderWorld         = TFALSE;
	m_bPaused              = TFALSE;
	m_bIsTransitioning     = TFALSE;
	m_bGameSystemCreated   = TFALSE;
	m_pOptions             = AOptions::CreateSingleton();
	AMemory::CreatePool( AMemory::POOL_Sound );

	auto pScheduler = g_oSystemManager.GetScheduler();

	m_pGUISystem = pScheduler->CreateTask<AGUISystem>( this );
	m_pGUI2      = pScheduler->CreateTask<AGUI2>( this );
	m_pRenderer  = pScheduler->CreateTask<ARenderer>( this );
	pScheduler->CreateTask<AFSMManager>( this );
	m_pInputHandler        = pScheduler->CreateTask<AInputHandler>( this );
	m_pGameStateController = pScheduler->CreateTask<AGameStateController>( this );
	m_pSoundManager        = pScheduler->CreateTask<ASoundManager>( this );
	m_pMoviePlayer         = pScheduler->CreateTask<ABINKMoviePlayer>( this );

	// NOTE: the wave bank info isn't loaded yet, SetCategoryVolume registers Music/VO/Ingame with the next free indices
	AOptions::GetSingleton()->ApplySettings();
}

// $Barnyard: FUNCTION 00403380
// $Barnyard: FUNCTION 00402bd0
ARootTask::~ARootTask()
{
	if ( m_pOptions )
		delete m_pOptions;
}

// $Barnyard: FUNCTION 00402dd0
TBOOL ARootTask::Reset()
{
	AUnitManager::GetSingleton()->RemoveAllUnits();
	ASoundManager::GetSingleton()->Reset();
	m_pGameSystemManager->Reset();

	return TTRUE;
}

// $Barnyard: FUNCTION 00402c40
void ARootTask::OnDestroy()
{
	AAssetLoader::Close( AAssetType_Startup );

	if ( AInputMapManager::IsSingletonCreated() )
		AInputMapManager::GetSingleton()->Delete();

	AMemory::DestroyPool( AMemory::POOL_Viewport );
	AMemory::DestroyPool( AMemory::POOL_Misc );
	AMemory::DestroyPool( AMemory::POOL_Collision );
	AMemory::DestroyPool( AMemory::POOL_Sound );

	if ( ARandom::IsSingletonCreated() )
		delete ARandom::GetSingleton();

	if ( APlayerProgress::IsSingletonCreated() )
		delete APlayerProgress::GetSingleton();
}

// Moves the task to the end of its parent's children
static void ReattachTask( TTask* a_pTask )
{
	if ( a_pTask )
		a_pTask->SetParent( a_pTask->Parent() );
}

// $Barnyard: FUNCTION 004033a0
void ARootTask::Shutdown()
{
	s_bShuttingDown = TTRUE;

	ReattachTask( m_pRenderer );
	ReattachTask( AFSMManager::IsSingletonCreated() ? AFSMManager::GetSingleton() : TNULL );
	ReattachTask( m_pInputHandler );
	ReattachTask( m_pMoviePlayer );
	ReattachTask( m_pGUISystem );
	ReattachTask( m_pGUI2 );
	ReattachTask( m_pGameStateController );
	ReattachTask( m_pGameSystemManager );
	ReattachTask( m_pUnkTask );

	if ( AGameStateController::IsSingletonCreated() )
		AGameStateController::GetSingleton()->ResetStack();

	if ( AUnitManager::IsSingletonCreated() )
		AUnitManager::GetSingleton()->RemoveAllUnits();

	if ( AInteractionManager::IsSingletonCreated() )
		AInteractionManager::GetSingleton()->Reset();

	if ( AGUISystem::IsSingletonCreated() )
		AGUISystem::GetSingleton()->ResetPicture();

	AGameLoader::DestroyLevel();
}

// $Barnyard: FUNCTION 00402b20
static TBOOL InitialiseClassCallback( TClass* a_pClass, void* a_pUserData )
{
	if ( !a_pClass->IsInitialized() )
		a_pClass->Initialize();

	return TTRUE;
}

// $Barnyard: FUNCTION 00403cf0
TBOOL ARootTask::OnCreate()
{
	AMemory::CreatePool( AMemory::POOL_Viewport );
	AMemory::CreatePool( AMemory::POOL_Misc );
	AMemory::CreatePool( AMemory::POOL_Collision );

	AAssetLoader::Load( "data\\assets\\lib_startup.trb", AAssetType_Startup, TTRUE );

	// Initialise all classes
	TGetClass( TObject ).RecurseTree( TNULL, TNULL, InitialiseClassCallback, TNULL );

	if ( m_pRenderer )
	{
		if ( !m_pRenderer->Create() )
		{
			m_pRenderer = TNULL;
		}
	}

	if ( !m_pRenderer )
	{
		return TFALSE;
	}

	m_pInputHandler->Create();
	m_pSoundManager->Create();

	ARandom::CreateSingleton();

	if ( m_pMoviePlayer )
	{
		m_pMoviePlayer->Create();
	}

	TGetClass( AInputMapManager ).CreateObject();
	AInputMapManager::GetSingleton()->ReadControlsData();

	LoadStartupData();
	m_pGameStateController->Create();

	AGUISystem::AllowBackgroundClear( TTRUE );
	ARenderer::GetSingleton()->ForceUpdate30FPS();
	ARenderer::GetSingleton()->ForceUpdate30FPS();
	AGUISystem::AllowBackgroundClear( TFALSE );

	return TTRUE;
}

// $Barnyard: FUNCTION 00403a80
TBOOL ARootTask::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( !IsGameSystemCreated() )
	{
		if ( !AGameStateController::GetSingleton()->IsCurrentState( &TGetClass( ASlideshowState ) ) &&
		     !AGameStateController::GetSingleton()->IsCurrentState( &TGetClass( SaveLoadSKU ) ) &&
		     !AGameStateController::GetSingleton()->IsCurrentState( &TGetClass( AMovieState ) ) &&
		     !AGameStateController::GetSingleton()->IsCurrentState( &TGetClass( AMessagePopupState ) )
#ifdef BARNYARD_COMMUNITY_PATCH
		     // NOTE: the EU PS2 builds wait for the language select screen too (see CreateStartupGameStates)
		     && !AGameStateController::GetSingleton()->IsCurrentState( &TGetClass( AFrontEndLanguageState ) )
#endif
		)
		{
			if ( !m_bStartedGame )
			{
				CreateStartupGameStates();
				return TTRUE;
			}
			else
			{
				g_oLoadScreen.Create();
				CreateGameSystem();

				AGameLoader::LoadFrontEnd();
				return TTRUE;
			}
		}
	}

#ifdef BARNYARD_COMMUNITY_PATCH
	if ( IsGameSystemCreated() )
	{
		AGameLoader::Dev_UpdateMiniGameRequest();
		AGameLoader::Dev_UpdateSimStateRequest();
		AGameLoader::Dev_UpdateDevTierSequence();
		ACheatActivator::Dev_Update();
	}

#  if defined( TOSHI_SKU_WINDOWS )
	// The one-shot dev switches keep counting while the window isn't active (retail pauses the whole game then)
	TMSWindow::Dev_SetKeepRunning( AGameLoader::Dev_IsRequestPending() );
#  endif // TOSHI_SKU_WINDOWS

#endif

	// Keep updating the transition until it reports it's done
	if ( IsTransitioning() )
	{
		SetTransitioning(
		    !AGameLoader::UpdateTransitionTask()
		);
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00402b40
void ARootTask::OnChildDied( const TClass* a_pClass, TTask* a_pDeletedTask )
{
	if ( a_pDeletedTask == m_pGUISystem ) m_pGUISystem = TNULL;
	else if ( a_pDeletedTask == m_pGUI2 )
		m_pGUI2 = TNULL;
	else if ( a_pDeletedTask == m_pInputHandler )
		m_pInputHandler = TNULL;
	else if ( a_pDeletedTask == m_pRenderer )
		m_pRenderer = TNULL;
	else if ( a_pDeletedTask == m_pGameStateController )
		m_pGameStateController = TNULL;
	else if ( a_pDeletedTask == m_pMoviePlayer )
		m_pMoviePlayer = TNULL;
}

// $Barnyard: FUNCTION 00402cb0
void ARootTask::OnActivate()
{
	if ( m_pGUISystem ) m_pGUISystem->Activate( TTRUE );
	if ( m_pGUI2 ) m_pGUI2->Activate( TTRUE );
	if ( m_pRenderer ) m_pRenderer->Activate( TTRUE );
	if ( m_pInputHandler ) m_pInputHandler->Activate( TTRUE );
	if ( m_pGameStateController ) m_pGameStateController->Activate( TTRUE );
	if ( m_pMoviePlayer ) m_pMoviePlayer->Activate( TTRUE );

	if ( m_bGameSystemCreated )
	{
		if ( m_pUnkTask ) m_pUnkTask->Activate( TTRUE );
		if ( AFSMManager::IsSingletonCreated() ) AFSMManager::GetSingleton()->Activate( TTRUE );
		if ( m_pGameSystemManager ) m_pGameSystemManager->Activate( TTRUE );
	}
}

// $Barnyard: FUNCTION 00402d50
void ARootTask::OnDeactivate()
{
	if ( m_pGUISystem ) m_pGUISystem->Activate( TFALSE );
	if ( m_pGUI2 ) m_pGUI2->Activate( TFALSE );
	if ( m_pRenderer ) m_pRenderer->Activate( TFALSE );
	if ( m_pInputHandler ) m_pInputHandler->Activate( TFALSE );
	if ( m_pGameStateController ) m_pGameStateController->Activate( TFALSE );
	if ( m_pUnkTask ) m_pUnkTask->Activate( TFALSE );
	if ( m_pMoviePlayer ) m_pMoviePlayer->Activate( TFALSE );
	if ( AFSMManager::IsSingletonCreated() ) AFSMManager::GetSingleton()->Activate( TFALSE );
}

TPSTRING8_DECLARE( bkg_by_legal1 );
TPSTRING8_DECLARE( bkg_Bink_Big );
TPSTRING8_DECLARE( bkg_bluetongue );
TPSTRING8_DECLARE( bkg_fmod );

// $Barnyard: FUNCTION 00402e70
void ARootTask::CreateStartupGameStates()
{
	// Create main object for storing info about the current story progress
	APlayerProgress::CreateSingleton( 1, 4, 0, 0 );

#ifdef BARNYARD_COMMUNITY_PATCH
	// Development: skip the startup screens when a minigame or the sim is requested on the command line
#  ifndef TOSHI_FINAL
	if ( AGameLoader::Dev_IsMiniGameRequested() || AGameLoader::Dev_IsSimStateRequested() ||
	     TStringManager::String8FindString( GetCommandLineA(), "-devkeys " ) )
#  else
	if ( AGameLoader::Dev_IsMiniGameRequested() || AGameLoader::Dev_IsSimStateRequested() )
#  endif
	{
		m_bStartedGame = TTRUE;

		// Load the dev save file like the startup does, so saving (leaving the options with changes writes the file
		// too) keeps the slots that are in it
		if ( AGameLoader::Dev_SetupSaveFileAtBoot() )
		{
			SaveLoadSKU* pSaveLoad = new SaveLoadSKU( APlayerProgress::GetSingleton(), TFALSE, TNULL );
			pSaveLoad->StartLoad();
			pSaveLoad->SetUseSavedLanguage( TTRUE );
			AGameStateController::GetSingleton()->PushState( pSaveLoad );
		}

		return;
	}
#endif

	auto attractMovie    = new AMovieState( "Attract", TFALSE, TNULL, TFALSE );
	auto bluetongueMovie = new AMovieState( "BTE", TFALSE, attractMovie, TFALSE );
	auto thqMovie        = new AMovieState( "THQ", TFALSE, bluetongueMovie, TFALSE );
	auto nickmovsMovie   = new AMovieState( "NickMovs", TFALSE, thqMovie, TFALSE );

	AGUISlideshow::Params slideshowSettings = {
		.iUnk1 = 0,
#ifdef BARNYARD_COMMUNITY_PATCH
		.bSlideSkippable = TTRUE,
#else
		.bSlideSkippable = TFALSE,
#endif
		.bCancelSkippable = TFALSE,
		.bInstaSkippable  = TFALSE,
		.bHasFadeIn       = TTRUE,
		.bHasFadeOut      = TTRUE,
		.bRepeat          = TFALSE,
		.bDelayAppear     = TFALSE,
		.fDuration        = 2.0f,
		.fFadeInTime      = 1.0f,
		.fFadeOutTime     = 0.5f,
		.fMaxTotalTime    = -1.0f,
	};

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail has no FMOD slide (bkg_fmod isn't in BYard.exe) and its slides can't be skipped
	auto fmodSlide = new ASlideshowState( slideshowSettings, nickmovsMovie, TFALSE );
	fmodSlide->AddSlide( TPS8( bkg_fmod ) );

	auto binkSlide = new ASlideshowState( slideshowSettings, fmodSlide, TFALSE );
#else
	auto binkSlide = new ASlideshowState( slideshowSettings, nickmovsMovie, TFALSE );
#endif
	binkSlide->AddSlide( TPS8( bkg_Bink_Big ) );

	slideshowSettings.fDuration = 5.0f;
	auto legalSlide             = new ASlideshowState( slideshowSettings, binkSlide, TTRUE );
	legalSlide->AddSlide( TPS8( bkg_by_legal1 ) );

	// Load the save data first
#ifdef BARNYARD_COMMUNITY_PATCH
	AGameLoader::Dev_SetupSaveFileAtBoot();
#endif
	SaveLoadSKU* pSaveLoad = new SaveLoadSKU( APlayerProgress::GetSingleton(), TTRUE, legalSlide );
	pSaveLoad->StartLoad();

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: from the EU PS2 builds (SLES_543.78 0x00107cc8). Without a console language the disc has, they ask for
	// the language before the save is loaded, otherwise the save's language is used (retail PC keeps English). The
	// PC always has a default language, so here the language comes from the save (Options -> Language sets it)
	if ( ALocaleManager::GetDefaultLanguage() == ALocaleManager::Lang_UNKNOWN )
	{
		AGameStateController::GetSingleton()->PushState( new AFrontEndLanguageState( pSaveLoad, TTRUE ) );
	}
	else
	{
		// NOTE: the PS2 also starts a memory card check here
		pSaveLoad->SetUseSavedLanguage( TTRUE );
		AGameStateController::GetSingleton()->PushState( pSaveLoad );
	}
#else
	AGameStateController::GetSingleton()->PushState( pSaveLoad );
#endif

	m_bStartedGame = TTRUE;
}

// $Barnyard: FUNCTION 00403b90
void ARootTask::LoadStartupData()
{
	TString8 startupLibPath;
	startupLibPath.Format( "Data/%s.trb", "lib_startup" );

	TTRB trb;
	trb.Load( startupLibPath );

	auto properties     = PBProperties::LoadFromTRB( trb );
	auto matlibProperty = properties->GetProperty( "matlib" );

	if ( matlibProperty )
	{
		AMaterialLibraryManager::GetSingleton()->LoadLibrariesFromProperties(
		    matlibProperty,
		    AAssetLoader::GetAssetTRB( AAssetType_Startup ),
		    TTRUE
		);
	}

	trb.Close();

	if ( m_pGUI2 && !m_pGUI2->Create() )
	{
		m_pGUI2 = TNULL;
	}

	if ( m_pGUISystem && !m_pGUISystem->Create() )
	{
		m_pGUISystem = TNULL;
	}

	TRenderInterface::GetSingleton()->FlushDyingResources();
	TRenderInterface::GetSingleton()->FlushDyingResources();
	T2GUITextureSectionManager::UpdateMaterials();

	g_oSystemManager.GetScheduler()->CreateTask<AFadeManager>( this )->Create();
}

// $Barnyard: FUNCTION 00402ba0
TBOOL ARootTask::IsPaused() const
{
	return m_bPaused || g_oSystemManager.IsPaused();
}

// $Barnyard: FUNCTION 00402e00
void ARootTask::SetPaused( TBOOL a_bPaused )
{
	if ( a_bPaused == m_bPaused )
		return;

	m_bPaused = a_bPaused;

	if ( a_bPaused )
	{
		if ( AGameTimeManager::IsSingletonCreated() )
			AGameTimeManager::GetSingleton()->PushUpdateState();

		if ( AGameTimeFXManager::IsSingletonCreated() )
			AGameTimeFXManager::GetSingleton()->Pause( TFALSE );
	}
	else
	{
		if ( AGameTimeManager::IsSingletonCreated() )
			AGameTimeManager::GetSingleton()->ToggleUpdateState();

		if ( AGameTimeFXManager::IsSingletonCreated() )
			AGameTimeFXManager::GetSingleton()->RestorePause();
	}

	if ( AGameSystemManager::IsSingletonCreated() )
		AGameSystemManager::GetSingleton()->ActivateSystemTasks( !a_bPaused );
}

// $Barnyard: FUNCTION 00403690
void ARootTask::CreateGameSystem()
{

	AFadeManager::GetSingleton()->StopAllFades();
	g_oLoadScreen.StartLoading( 9, TTRUE );

	ASoundManager::GetSingleton()->LoadWaveBanksInfo( "soundmetadata" );

	// Load the FSM scripts
	AFSMManager::GetSingleton()->Create();
	g_oLoadScreen.Update( 1.0f, TTRUE );
	g_oLoadScreen.Update( 1.0f, TTRUE );

	m_pGameSystemManager = g_oSystemManager.GetScheduler()->CreateTask<AGameSystemManager>( this );
	m_pGameSystemManager->Create();

	g_oLoadScreen.Update( 1.0f, TTRUE );
	g_oLoadScreen.Update( 1.0f, TTRUE );
	g_oLoadScreen.Update( 1.0f, TTRUE );

	// Reorder tasks?
	if ( m_pInputHandler )
	{
		m_pInputHandler->SetParent( m_pInputHandler->Parent() );
	}

	if ( m_pMoviePlayer )
	{
		m_pMoviePlayer->SetParent( m_pMoviePlayer->Parent() );
	}

	if ( m_pGUISystem )
	{
		m_pGUISystem->SetParent( m_pGUISystem->Parent() );
	}

	if ( m_pGameStateController )
	{
		m_pGameStateController->SetParent( m_pGameStateController->Parent() );
	}

	// The world (AAnimatableObjectManager...) updates after the game states, the FSMs and the
	// sounds after the world. The game states see the animations that ended during the last
	// render (TAnimation::STATE_BLENDING_OUT) before AToshiAnimationInterface removes them.
	if ( m_pGameSystemManager )
	{
		m_pGameSystemManager->SetParent( m_pGameSystemManager->Parent() );
	}

	if ( AFSMManager::IsSingletonCreated() )
	{
		AFSMManager::GetSingleton()->SetParent( AFSMManager::GetSingleton()->Parent() );
	}

	if ( m_pSoundManager )
	{
		m_pSoundManager->SetParent( m_pSoundManager->Parent() );
	}

	if ( m_pRenderer )
	{
		m_pRenderer->SetParent( m_pRenderer->Parent() );
	}

	m_bGameSystemCreated = TTRUE;
	OnActivate();
}
