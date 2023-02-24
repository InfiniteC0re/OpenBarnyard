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
#include "A2GUI/A2GUIRenderer_DX11.h"

#include <Toshi/Xui/TXUI.h>
#include <Toshi2/T2GUI/T2GUI.h>
#include <Toshi/Sound/TSound.h>
#include <Toshi/Render/TRender.h>
#include <Platform/Windows/TSound_Win.h>
#include TOSHI_MULTIRENDER(TRender)

AApplication AApplication::g_oTheApp;

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
		auto pGUIRenderer = new A2GUIRenderer;
		auto pGUI = Toshi::T2GUI::Open(AMemory::ms_apMemoryBlocks[AMemory::POOL_FrequentAllocations]);
		pGUI->SetRenderer(pGUIRenderer);

		AInputManager2::CreateSingleton();

		// Temp solution
		Toshi::TXUI::ms_pXUIMemoryBlock = Toshi::TMemory::CreateHeap(0x10000, 4, "xui pile");
		Toshi::TXUI::CreateSingleton();

		size_t poolSize = 128 * 1024 * 1024;
		void* mempool = malloc(poolSize);

		bool bResult = Toshi::TSound::CreateSingleton()->Create(mempool, poolSize, -1, -1, Toshi::TSound::SpeakerType_7POINT1);
		TASSERT(TTRUE == bResult);

		FMOD::System* system = Toshi::TSound::GetSingletonWeak()->GetSystem();
		system->setFileSystem(NULL, NULL, NULL, NULL, NULL, NULL, 0);

		m_Renderer->Create();
		AGameState::SetupLoadIcon();
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
	Toshi::T2GUI* pGUI = Toshi::T2GUI::GetSingletonWeak();
	
	if (pGUI != TNULL)
	{
		pGUI->Tick(deltaTime);
	}
	
	AMoviePlayer* pMoviePlayer = AMoviePlayer::GetSingletonWeak();

	if (pMoviePlayer != TNULL)
	{
		pMoviePlayer->OnUpdate(deltaTime);
	}

	UpdateSound(deltaTime);
	m_pGameStateController->Update(deltaTime);
	ARenderer::GetSingletonWeak()->Update(deltaTime);
	return true;
}

bool AApplication::UpdateSound(float deltaTime)
{
	Toshi::TSound* sound = Toshi::TSound::GetSingleton();
	sound->Update();
	return false;
}
