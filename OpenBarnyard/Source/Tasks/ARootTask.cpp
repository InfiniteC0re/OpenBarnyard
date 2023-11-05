#include "pch.h"
#include "ARandom.h"
#include "ARootTask.h"
#include "Assets/AMaterialLibraryManager.h"
#include "Assets/AAssetLoader.h"
#include "Memory/AMemory.h"
#include "AGUI2/AGUI2.h"

#include <Plugins/PPropertyParser/PProperties.h>
#include <Toshi/Core/TScheduler.h>

TOSHI_NAMESPACE_USING

ARootTask::ARootTask()
{
	TIMPLEMENT();

	m_pOptions = AOptions::CreateSingleton();
	AMemory::CreatePool(AMemory::POOL_Sound);

	auto pSystemManager = TSystemManager::GetSingleton();

	m_pGUI2 = pSystemManager->GetScheduler()->CreateTask(&TGetClass(AGUI2));
	m_pRenderer = TSTATICCAST(ARenderer*, pSystemManager->GetScheduler()->CreateTask(&TGetClass(ARenderer)));

	m_bRenderScene = TFALSE;
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
			return TFALSE;
		}
	}

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
		m_pGUI2->Create();
	}
}
