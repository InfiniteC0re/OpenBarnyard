#include "pch.h"

// Predefining Toshi initialization settings
#define TOSHI_TMEMORY_SIZE 640 * 1024 * 1024
#define TOSHI_TMEMORY_FLAGS Toshi::TMemory::Flags_Standard

// Including the entrypoint
#include <Toshi.h>

// Including everything else
#include "AAssetStreaming.h"
#include "AExampleClass.h"
#include "GameInterface/AAppInitState.h"
#include "Movie/AMoviePlayer.h"
#include "Locale/ALocaleManager.h"
#include "Input/AInputManager2.h"
#include "ALevelInformation.h"

#include <Toshi/Sound/TSound.h>
#include <Toshi/Render/TRender.h>
#include TOSHI_MULTIRENDER(TRender)

AApplication AApplication::g_oTheApp;
static Toshi::TSound m_soundSystem = Toshi::TSound();

bool AApplication::OnCreate(int argc, char** argv)
{
	TOSHI_INFO("Starting Blob...");
	AMemory::CreatePools();

	ARenderer::CreateSingleton();
	AAssetStreaming::CreateSingleton();
	ALocaleManager::Create();

	m_Renderer = ARenderer::GetSingleton();
	bool interfaceCreated = m_Renderer->CreateInterface();
	
	if (interfaceCreated)
	{
		AInputManager2::CreateSingleton();

		size_t poolSize = 128 * 1024 * 1024;
		void* mempool = malloc(poolSize);

		bool bResult = m_soundSystem.Create(mempool, poolSize, -1, -1, Toshi::TSound::SpeakerType_7POINT1);
		TASSERT(TTRUE == bResult);

		m_Renderer->Create();
		SetRenderWorld(true);
		m_pGameStateController = AGameStateController::CreateSingleton();
		m_pGameStateController->Create();
		m_pGameStateController->PushState(new AAppInitState);

		TApplication::OnCreate(argc, argv);
		//ALevelInformation info = ALevelInformation();
		//info.Create("C:\\Program Files (x86)\\Steam\\steamapps\\common\\de Blob\\Data\\Levels.trb");

		AExampleClass* exampleClass = new AExampleClass();
		exampleClass->Delete();
	}
	
	return true;
}

bool AApplication::OnUpdate(float deltaTime)
{
	AMoviePlayer* pMoviePlayer = AMoviePlayer::GetSingletonWeak();

	if (pMoviePlayer != TNULL)
	{
		pMoviePlayer->OnUpdate(deltaTime);
	}

	m_pGameStateController->Update(deltaTime);
	ARenderer::GetSingletonWeak()->Update(deltaTime);
	return true;
}