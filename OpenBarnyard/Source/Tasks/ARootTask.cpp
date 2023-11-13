#include "pch.h"
#include "ARandom.h"
#include "ARootTask.h"
#include "Assets/AMaterialLibraryManager.h"
#include "Assets/AAssetLoader.h"
#include "Memory/AMemory.h"
#include "Input/AInputHandler.h"
#include "GUI/AGUI2.h"
#include "GUI/AGUISystem.h"
#include "GUI/AGUI2TextureSectionManager.h"
#include "GUI/AFadeManager.h"
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

	m_bGameSystemCreated = TFALSE;
}

TBOOL ARootTask::OnCreate()
{
	TIMPLEMENT();

	AMemory::CreatePool(AMemory::POOL_Misc);
	AMemory::CreatePool(AMemory::POOL_Collision);
	AMemory::CreatePool(AMemory::POOL_Viewport);

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

	ARandom::CreateSingleton();

	LoadStartupData();
	
	return TTRUE;
}

TBOOL ARootTask::OnUpdate(TFLOAT a_fDeltaTime)
{
	ALoadScreen::GetGlobalInstance()->Update(1.0f, TTRUE);

	return TTRUE;
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
	pFadeManager->StartFade(AFade::Color(255, 0, 0, 0), AFade::Color(0, 0, 0, 0), 2.0f);

	ALoadScreen::GetGlobalInstance()->Create();
	ALoadScreen::GetGlobalInstance()->StartLoading(0, TTRUE);
#endif
}

TBOOL ARootTask::IsPaused()
{
	return m_bPaused || Toshi::TSystemManager::GetSingleton()->IsPaused();
}
