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
#include "GUI/AGUI2TextureSectionManager.h"
#include "GUI/AFadeManager.h"
#include "GUI/AGUIPicture.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/ASlideshowState.h"
#include "GameInterface/AMovieState.h"
#include "GameInterface/SaveLoadSKU.h"
#include "GameInterface/AMessagePopupState.h"
#include "GameInterface/AFrontEndMainMenuState2.h"
#include "Sound/ASoundManager.h"
#include "Movie/ABINKMoviePlayer.h"
#include "ALoadScreen.h"
#include "Terrain/ATerrainInterface.h"
#include "Terrain/ABYardTerrainManager.h"

#include "SoundBank/music.h"
#include "SoundBank/ui.h"

#include <Plugins/PPropertyParser/PBProperties.h>
#include <Toshi/TScheduler.h>
#include <Toshi/T2ObjectPool.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( ARootTask );

ARootTask::ARootTask()
{
	TIMPLEMENT();

	m_bStartedGame        = TFALSE;
	m_bRenderWorld        = TFALSE;
	m_bPaused             = TFALSE;
	m_bStopRenderingScene = TFALSE;
	m_bGameSystemCreated  = TFALSE;
	m_pOptions            = AOptions::CreateSingleton();
	AMemory::CreatePool( AMemory::POOL_Sound );

	auto pScheduler = g_oSystemManager.GetScheduler();

	m_pGUISystem           = pScheduler->CreateTask<AGUISystem>( this );
	m_pGUI2                = pScheduler->CreateTask<AGUI2>( this );
	m_pRenderer            = pScheduler->CreateTask<ARenderer>( this );
	m_pInputHandler        = pScheduler->CreateTask<AInputHandler>( this );
	m_pGameStateController = pScheduler->CreateTask<AGameStateController>( this );
	m_pSoundManager        = pScheduler->CreateTask<ASoundManager>( this );
	m_pMoviePlayer         = pScheduler->CreateTask<ABINKMoviePlayer>( this );
	m_bGameSystemCreated   = TFALSE;
}

TBOOL ARootTask::OnCreate()
{
	TIMPLEMENT();

	AMemory::CreatePool( AMemory::POOL_Viewport );
	AMemory::CreatePool( AMemory::POOL_Misc );
	AMemory::CreatePool( AMemory::POOL_Collision );

	// Initialise all classes
	TGetClass( TObject ).RecurseTree(
	    TNULL,
	    TNULL,
	    []( TClass* a_pClass, void* ) {
		    if ( !a_pClass->IsInitialized() )
		    {
			    a_pClass->Initialize();
		    }

		    return TTRUE;
	    },
	    TNULL
	);

	AAssetLoader::Load( "data\\assets\\lib_startup.trb", AAssetType_Startup, TTRUE );

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

TBOOL ARootTask::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( !IsGameSystemCreated() )
	{
		if ( !AGameStateController::GetSingleton()->IsCurrentState( &TGetClass( ASlideshowState ) ) &&
		     !AGameStateController::GetSingleton()->IsCurrentState( &TGetClass( SaveLoadSKU ) ) &&
		     !AGameStateController::GetSingleton()->IsCurrentState( &TGetClass( AMovieState ) ) &&
		     !AGameStateController::GetSingleton()->IsCurrentState( &TGetClass( AMessagePopupState ) ) )
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
				LoadFrontEnd();
				return TTRUE;
			}
		}
	}

	TTODO( "FUN_00427e70" );
	g_oLoadScreen.Update( 1.0f, TTRUE );

	return TTRUE;
}

void ARootTask::OnChildDied( TClass* a_pClass, TTask* a_pDeletedTask )
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
		TTODO( "Activate three more tasks" );
	}
}

void ARootTask::OnDeactivate()
{
	if ( m_pGUISystem ) m_pGUISystem->Activate( TFALSE );
	if ( m_pGUI2 ) m_pGUI2->Activate( TFALSE );
	if ( m_pRenderer ) m_pRenderer->Activate( TFALSE );
	if ( m_pInputHandler ) m_pInputHandler->Activate( TFALSE );
	if ( m_pGameStateController ) m_pGameStateController->Activate( TFALSE );
	if ( m_pMoviePlayer ) m_pMoviePlayer->Activate( TFALSE );

	TTODO( "Deactivate two more tasks" );
}

TPSTRING8_DECLARE( bkg_by_legal1 );
TPSTRING8_DECLARE( bkg_Bink_Big );
TPSTRING8_DECLARE( bkg_bluetongue );
TPSTRING8_DECLARE( bkg_fmod );

void ARootTask::CreateStartupGameStates()
{
	TIMPLEMENT();

	auto attractMovie    = new AMovieState( "Attract", TFALSE, TNULL, TFALSE );
	auto bluetongueMovie = new AMovieState( "BTE", TFALSE, attractMovie, TFALSE );
	auto thqMovie        = new AMovieState( "THQ", TFALSE, bluetongueMovie, TFALSE );
	auto nickmovsMovie   = new AMovieState( "NickMovs", TFALSE, thqMovie, TFALSE );

	AGUISlideshow::Params slideshowSettings = {
		.iUnk1           = 0,
		.bSlideSkippable = TTRUE,
		.bUnk2           = TFALSE,
		.bInstaSkippable = TFALSE,
		.bHasFadeIn      = TTRUE,
		.bHasFadeOut     = TTRUE,
		.bRepeat         = TFALSE,
		.bDelayAppear    = TFALSE,
		.fDuration       = 2.0f,
		.fFadeInTime     = 1.0f,
		.fFadeOutTime    = 0.5f,
		.fUnk5           = -1.0f,
	};

	auto fmodSlide = new ASlideshowState( slideshowSettings, nickmovsMovie, TFALSE );
	fmodSlide->AddSlide( TPS8( bkg_fmod ) );

	auto binkSlide = new ASlideshowState( slideshowSettings, fmodSlide, TFALSE );
	binkSlide->AddSlide( TPS8( bkg_Bink_Big ) );

	slideshowSettings.fDuration = 5.0f;
	auto legalSlide             = new ASlideshowState( slideshowSettings, binkSlide, TTRUE );
	legalSlide->AddSlide( TPS8( bkg_by_legal1 ) );

	AGameStateController::GetSingleton()->PushState( legalSlide );

	m_bStartedGame = TTRUE;
}

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
	AGUI2TextureSectionManager::UpdateMaterials();

	g_oSystemManager.GetScheduler()->CreateTask<AFadeManager>( this )->Create();
}

TBOOL ARootTask::IsPaused() const
{
	return m_bPaused || g_oSystemManager.IsPaused();
}

void ARootTask::CreateGameSystem()
{
	TIMPLEMENT();

	AFadeManager::GetSingleton()->StopAllFades();
	g_oLoadScreen.StartLoading( 9, TTRUE );

	ASoundManager::GetSingleton()->LoadWaveBanksInfo( "soundmetadata" );

	// ...
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

	// ...

	if ( m_pRenderer )
	{
		m_pRenderer->SetParent( m_pRenderer->Parent() );
	}

	m_bGameSystemCreated = TTRUE;
	OnActivate();
}

TPSTRING8_DECLARE( ui );
TPSTRING8_DECLARE( music );

void ARootTask::LoadFrontEnd()
{
	TIMPLEMENT();
	ASoundManager* pSoundManager = ASoundManager::GetSingleton();

	// Load ui & music soundbanks
	pSoundManager->LoadSoundBank( TPS8( ui ), TFALSE, TTRUE );
	pSoundManager->LoadSoundBank( TPS8( music ), TFALSE, TFALSE );

	AAssetLoader::LoadAssetPackFromLibrary( "lib_frontend", TTRUE );

	ABYardTerrainManager::SetTerrain( ABYardTerrainManager::Terrain_EnvMain, TTRUE, TTRUE, 0, 0, 0, 0 );
	ABYardTerrainManager::StartLoading();
	AAssetLoader::CreateAssetsFromLibrary( "lib_frontend" );

	// Load music wavebank samples
	ASoundManager::ms_bShouldUpdateLoadingScreen = TTRUE;
	pSoundManager->LoadWaveBankSamples( TPS8( music ), AWaveBank::LOADFLAGS_NONE, -1 );
	ASoundManager::ms_bShouldUpdateLoadingScreen = TFALSE;

	// Load music samples
	pSoundManager->LoadSoundBankSamples( TPS8( music ) );

	// Create front end state
	AFrontEndMainMenuState2* pFrontEndState = new AFrontEndMainMenuState2();
	ABYardTerrainManager::SetTerrain( ABYardTerrainManager::Terrain_EnvMain, TFALSE, TTRUE, 0, 0, 0, 0 );
	pFrontEndState->SetupCamera();

	ABYardTerrainManager::StartLoading();

	GetSingleton()->SetRenderWorld( TTRUE );

	// Push the state
	AGameStateController::GetSingleton()->PushState( pFrontEndState );

	// Hide loading screen
	g_oLoadScreen.Reset();
	g_oLoadScreen.SetLoadingState( TFALSE, TTRUE );

	// Fade screen from black
	AFadeManager::GetSingleton()->StartFade(
	    AFade::Color( 255, 0, 0, 0 ),
	    AFade::Color( 0, 0, 0, 0 ),
	    1.0f
	);
}
