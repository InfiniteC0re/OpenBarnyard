#include "pch.h"

// Predefining Toshi initialization settings
#define TOSHI_TMEMORY_SIZE 128 * 1024 * 1024
#define TOSHI_TMEMORY_FLAGS Toshi::TMemory::Flags_Standard

// Including the entrypoint
#include <Toshi.h>

#include <Toshi/Core/TArray.h>
#include <Toshi/Core/TScheduler.h>
#include <Toshi/Core/TSystem.h>
#include <Toshi2/T2SimpleArray.h>
#include <Toshi/Render/TRenderInterface.h>
#include <Toshi/Render/TModelRegistry.h>
#include <Toshi/Strings/TPString8.h>
#include <Toshi/Input/TInputInterface.h>

#include <Platform/DX8/TTextureResourceHAL_DX8.h>

#include TOSHI_MULTIRENDER(TRenderInterface)

#include "Tasks/ADummyTask.h"
#include "Locale/ALocaleManager.h"
#include "GUI/AGUI2.h"

AApplication g_oTheApp;

TOSHI_NAMESPACE_USING

TBOOL AApplication::OnCreate(int argc, char** argv)
{
	TOSHI_INFO("Starting Barnyard...");

	// FIXME: Move this line to EntryPoint's code
	AMemory::CreatePool(AMemory::POOL_StringPool);

	if (!CreateStringPool())
		return TFALSE;

	TTextureResourceHAL::ms_oFreeList.SetCapacity(690);
	TTextureResourceHAL::ms_oFreeList.SetGrowSize(0);

	ALocaleManager* pLocaleManager = T2Locale::CreateSingleton<ALocaleManager>();
	TTODO("Create some instances of some classes");

	TModelRegistry::Initialise();
	pLocaleManager->SetLanguage(ALocaleManager::Lang_EnglishUK);
	
	auto pSystemManager = TSystemManager::GetSingleton();

	m_pRenderTask = TSTATICCAST(ADummyTask*, pSystemManager->GetScheduler()->CreateTask(&TGetClass(ADummyTask)));
	m_pRenderTask->Create();
	m_pRenderTask->Activate(TTRUE);
	m_pRenderTask->SetName("RenderTask");

	m_pUpdate3Task = TSTATICCAST(ADummyTask*, pSystemManager->GetScheduler()->CreateTask(&TGetClass(ADummyTask)));
	m_pUpdate3Task->Create();
	m_pUpdate3Task->Activate(TTRUE);
	m_pUpdate3Task->SetName("Update3");

	m_pUpdate2Task = TSTATICCAST(ADummyTask*, pSystemManager->GetScheduler()->CreateTask(&TGetClass(ADummyTask)));
	m_pUpdate2Task->Create();
	m_pUpdate2Task->Activate(TTRUE);
	m_pUpdate2Task->SetName("Update2");

	m_pUpdate1Task = TSTATICCAST(ADummyTask*, pSystemManager->GetScheduler()->CreateTask(&TGetClass(ADummyTask)));
	m_pUpdate1Task->Create();
	m_pUpdate1Task->Activate(TTRUE);
	m_pUpdate1Task->SetName("Update1");

	m_pInputTask = TSTATICCAST(ADummyTask*, pSystemManager->GetScheduler()->CreateTask(&TGetClass(ADummyTask)));
	m_pInputTask->Create();
	m_pInputTask->Activate(TTRUE);
	m_pInputTask->SetName("InputTask");

	m_pRootTask = TSTATICCAST(ARootTask*, pSystemManager->GetScheduler()->CreateTask(&TGetClass(ARootTask)));

	if (m_pRootTask != TNULL)
	{
		if (TFALSE == m_pRootTask->Create())
			return TFALSE;

		m_pRootTask->Activate(TTRUE);
	}

	return TApplication::OnCreate(argc, argv);
}

TBOOL AApplication::OnUpdate(float deltaTime)
{

	return TTRUE;
}

TBOOL AApplication::CreateStringPool()
{
	auto pStringPool = new TPString8Pool(1024, 0, AMemory::GetAllocator(AMemory::POOL_StringPool), TNULL);

	TSystemManager::GetSingleton()->SetStringPool(pStringPool);
	pStringPool->InitialiseStatic();

	return TTRUE;
}
