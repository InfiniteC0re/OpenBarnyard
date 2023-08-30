#include "pch.h"

// Predefining Toshi initialization settings
#define TOSHI_TMEMORY_SIZE 128 * 1024 * 1024
#define TOSHI_TMEMORY_FLAGS Toshi::TMemory::Flags_Standard

// Including the entrypoint
#include <Toshi.h>

#include <Toshi/Core/TArray.h>
#include <Toshi/Core/TScheduler.h>
#include <Toshi/Core/TSystem.h>
#include <Toshi2/T2GUI/T2GUI.h>
#include <Toshi2/T2SimpleArray.h>
#include <Toshi/Sound/TSound.h>
#include <Toshi/Render/TRender.h>
#include <Toshi/Strings/TPString8.h>
#include <Toshi/Input/TInputInterface.h>

#include <Platform/Windows/TSound_Win.h>

#include TOSHI_MULTIRENDER(TRender)

#include "Tasks/ADummyTask.h"
#include "Locale/ALocaleManager.h"
#include "AGUI2/AGUI2TextureSection.h"

AApplication AApplication::g_oTheApp;

TOSHI_NAMESPACE_USING

TBOOL AApplication::OnCreate(int argc, char** argv)
{
	TOSHI_INFO("Starting Barnyard...");
	
	// FIXME: Move this line to EntryPoint's code
	AMemory::CreatePool(AMemory::POOL_StringPool);

	if (!CreateStringPool())
		return TFALSE;

	ALocaleManager* pLocaleManager = T2Locale::CreateSingleton<ALocaleManager>();
	TTODO("Create some instances of some classes");

	pLocaleManager->SetLanguage(ALocaleManager::Lang_EnglishUK);
	
	auto pSystemManager = TSystemManager::GetSingleton();

	m_pRenderTask = TSTATICCAST(ADummyTask*, pSystemManager->GetScheduler()->CreateTask(TGetClass(ADummyTask)));
	m_pRenderTask->Create();
	m_pRenderTask->Activate(TTRUE);
	m_pRenderTask->SetName("RenderTask");

	m_pUpdate3Task = TSTATICCAST(ADummyTask*, pSystemManager->GetScheduler()->CreateTask(TGetClass(ADummyTask)));
	m_pUpdate3Task->Create();
	m_pUpdate3Task->Activate(TTRUE);
	m_pUpdate3Task->SetName("Update3");

	m_pUpdate2Task = TSTATICCAST(ADummyTask*, pSystemManager->GetScheduler()->CreateTask(TGetClass(ADummyTask)));
	m_pUpdate2Task->Create();
	m_pUpdate2Task->Activate(TTRUE);
	m_pUpdate2Task->SetName("Update2");

	m_pUpdate1Task = TSTATICCAST(ADummyTask*, pSystemManager->GetScheduler()->CreateTask(TGetClass(ADummyTask)));
	m_pUpdate1Task->Create();
	m_pUpdate1Task->Activate(TTRUE);
	m_pUpdate1Task->SetName("Update1");

	m_pInputTask = TSTATICCAST(ADummyTask*, pSystemManager->GetScheduler()->CreateTask(TGetClass(ADummyTask)));
	m_pInputTask->Create();
	m_pInputTask->Activate(TTRUE);
	m_pInputTask->SetName("InputTask");

	m_pRootTask = TSTATICCAST(ARootTask*, pSystemManager->GetScheduler()->CreateTask(TGetClass(ARootTask)));

	if (m_pRootTask != TNULL)
	{
		if (TFALSE == m_pRootTask->Create())
			return TFALSE;

		m_pRootTask->Activate(TTRUE);
	}

	AGUI2TextureSectionManager::Open("data/gui/texsec.trb", (TTRB*)1);

#if 1

	for (int i = 0; i < pLocaleManager->GetNumStrings(); i++)
	{
		TOSHI_INFO(L"{}: {}", i, pLocaleManager->GetString(i));
	}

#endif

	return TApplication::OnCreate(argc, argv);
}

TBOOL AApplication::OnUpdate(float deltaTime)
{

	return TTRUE;
}

TBOOL AApplication::CreateStringPool()
{
	// FIXME: use AMemory's allocator
	auto pStringPool = new TPString8Pool(1024, 0, &AMemory::s_aAllocators[0].Allocator, TNULL);

	TSystemManager::GetSingleton()->SetStringPool(pStringPool);
	pStringPool->InitialiseStatic();

	return TTRUE;
}
