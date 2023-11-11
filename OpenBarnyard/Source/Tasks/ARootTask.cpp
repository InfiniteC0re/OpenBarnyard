#include "pch.h"
#include "ARandom.h"
#include "ARootTask.h"
#include "Assets/AMaterialLibraryManager.h"
#include "Assets/AAssetLoader.h"
#include "Memory/AMemory.h"
#include "Input/AInputHandler.h"
#include "AGUI/AGUI2.h"
#include "AGUI/AGUISystem.h"
#include "AGUI/AGUI2TextureSectionManager.h"

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

	// Test T2ObjectPool #1
	T2ObjectPool<TFLOAT, 4> pool1;
	auto pFloat1 = pool1.NewObject(5.0f);
	auto pFloat2 = pool1.NewObject(9.0f);
	auto pFloat3 = pool1.NewObject(16.0f);
	auto pFloat4 = pool1.NewObject(21.0f);

	// Test T2ObjectPool #2
	struct MyCustomClass
	{
		MyCustomClass()
		{
			TOSHI_INFO("Created MyCustomClass");
		}

		virtual ~MyCustomClass()
		{
			TOSHI_INFO("Destroyed MyCustomClass");
		}
	};

	T2ObjectPool<MyCustomClass, 4> pool2;
	auto pObject1 = pool2.NewObject();
	pool2.DeleteObject(pObject1);

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

void ARootTask::LoadStartupData()
{
	TIMPLEMENT();

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
}

TBOOL ARootTask::IsPaused()
{
	return m_bPaused || Toshi::TSystemManager::GetSingleton()->IsPaused();
}
