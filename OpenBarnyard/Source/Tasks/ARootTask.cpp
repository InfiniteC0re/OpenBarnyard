#include "pch.h"
#include "ARandom.h"
#include "ARootTask.h"
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
#include "Sound/ASoundManager.h"
#include "ALoadScreen.h"

#include <Plugins/PPropertyParser/PProperties.h>
#include <Toshi/Core/TScheduler.h>
#include <Toshi2/T2ObjectPool.h>

TOSHI_NAMESPACE_USING

ARootTask::ARootTask()
{
	TIMPLEMENT();

	m_bLoadedTerrain = TFALSE;
	m_bPaused = TFALSE;
	m_bStopRenderingWorld = TFALSE;
	m_bGameSystemCreated = TFALSE;
	m_pOptions = AOptions::CreateSingleton();
	AMemory::CreatePool(AMemory::POOL_Sound);

	auto pSystemManager = TSystemManager::GetSingleton();

	m_pGUISystem = pSystemManager->GetScheduler()->CreateTask(&TGetClass(AGUISystem), this);
	m_pGUI2 = pSystemManager->GetScheduler()->CreateTask(&TGetClass(AGUI2), this);
	m_pRenderer = TSTATICCAST(ARenderer*, pSystemManager->GetScheduler()->CreateTask(&TGetClass(ARenderer), this));
	m_pInputHandler = pSystemManager->GetScheduler()->CreateTask(&TGetClass(AInputHandler), this);
	m_pGameStateController = pSystemManager->GetScheduler()->CreateTask(&TGetClass(AGameStateController), this);
	m_pSoundManager = pSystemManager->GetScheduler()->CreateTask(&TGetClass(ASoundManager), this);

	m_bGameSystemCreated = TFALSE;
}

TBOOL ARootTask::OnCreate()
{
	TIMPLEMENT();

	AMemory::CreatePool(AMemory::POOL_Viewport);
	AMemory::CreatePool(AMemory::POOL_Misc);
	AMemory::CreatePool(AMemory::POOL_Collision);

	TGetClass(TObject).RecurseTree(
		TNULL,
		TNULL,
		[](TClass* a_pClass, void*)
		{
			if (!a_pClass->IsInitialized())
			{
				a_pClass->Initialize();
			}

			return TTRUE;
		},
		TNULL
	);

	AAssetLoader::Load("data\\assets\\lib_startup.trb", AAssetLoader::AssetType_Startup, TTRUE);

	if (m_pRenderer)
	{
		if (!m_pRenderer->Create())
		{
			m_pRenderer = TNULL;
		}
	}

	if (!m_pRenderer)
	{
		return TFALSE;
	}

	m_pInputHandler->Create();
	m_pSoundManager->Create();

	ARandom::CreateSingleton();

	TGetClass(AInputMapManager).CreateObject();
	AInputMapManager::GetSingleton()->ReadControlsData();

	LoadStartupData();
	m_pGameStateController->Create();

	AGUISystem::AllowBackgroundClear(TTRUE);
	ARenderer::GetSingleton()->ForceUpdate30FPS();
	ARenderer::GetSingleton()->ForceUpdate30FPS();
	AGUISystem::AllowBackgroundClear(TFALSE);
	
	return TTRUE;
}

TBOOL ARootTask::OnUpdate(TFLOAT a_fDeltaTime)
{
	TIMPLEMENT();

	if (!m_bStartedGame)
	{
		CreateStartupGameStates();
		return TTRUE;
	}

	//ALoadScreen::GetGlobalInstance()->Update(1.0f, TTRUE);

	return TTRUE;
}

TPSTRING8_DECLARE(bkg_by_legal1);
TPSTRING8_DECLARE(bkg_Bink_Big);
TPSTRING8_DECLARE(bkg_bluetongue);

void ARootTask::CreateStartupGameStates()
{
	TIMPLEMENT();

	AGUISlideshow::Params params;
	params.iUnk1 = 0;
	params.bSlideSkippable = TFALSE;
	params.bUnk2 = TFALSE;
	params.bInstaSkippable = TFALSE;
	params.bHasFadeIn = TTRUE;
	params.bHasFadeOut = TTRUE;
	params.bRepeat = TFALSE;
	params.bDelayAppear = TFALSE;
	params.fDuration = 2.0f;
	params.fUnk5 = -1.0f;
	params.fFadeInTime = 1.0f;
	params.fFadeOutTime = 0.5f;

	auto bluetongueSlide = new ASlideshowState(params, TNULL, TFALSE);
	bluetongueSlide->AddSlide(*TPS8(bkg_bluetongue));

	auto binkSlide = new ASlideshowState(params, bluetongueSlide, TFALSE);
	binkSlide->AddSlide(*TPS8(bkg_Bink_Big));

	params.fDuration = 5.0f;
	auto legalSlide = new ASlideshowState(params, binkSlide, TTRUE);
	legalSlide->AddSlide(*TPS8(bkg_by_legal1));

	AGameStateController::GetSingleton()->PushState(legalSlide);

	m_bStartedGame = TTRUE;
}

void ARootTask::LoadStartupData()
{
	TTRB trb;
	trb.Load(TString8::Format("Data/%s.trb", "lib_startup"));

	auto properties = PProperties::LoadFromTRB(trb);
	auto matlibProperty = properties->GetOptionalProperty("matlib");

	if (matlibProperty)
	{
		AMaterialLibraryManager::GetSingleton()->LoadFromProperties(
			matlibProperty,
			AAssetLoader::GetAssetTRB(AAssetLoader::AssetType_Startup),
			TTRUE
		);
	}

	trb.Close();

	if (m_pGUI2)
	{
		if (!m_pGUI2->Create())
		{
			m_pGUI2 = TNULL;
		}
	}

	if (m_pGUISystem)
	{
		if (!m_pGUISystem->Create())
		{
			m_pGUISystem = TNULL;
		}
	}

	Toshi::TRenderInterface::GetSingleton()->FlushDyingResources();
	Toshi::TRenderInterface::GetSingleton()->FlushDyingResources();
	AGUI2TextureSectionManager::UpdateMaterials();

	auto pSystemManager = TSystemManager::GetSingleton();
	auto pFadeManager = TSTATICCAST(AFadeManager*, pSystemManager->GetScheduler()->CreateTask(&TGetClass(AFadeManager), this));
	
#if 1
	// Code for tests
	pFadeManager->Create();
	ALoadScreen::GetGlobalInstance()->Create();
	//ALoadScreen::GetGlobalInstance()->StartLoading(0, TTRUE);
#endif
}

TBOOL ARootTask::IsPaused() const
{
	return m_bPaused || Toshi::TSystemManager::GetSingleton()->IsPaused();
}
