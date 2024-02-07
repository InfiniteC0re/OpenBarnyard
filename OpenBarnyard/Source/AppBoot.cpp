#include "pch.h"

#include <Toshi/Toshi.h>
#include <Toshi/TArray.h>
#include <Toshi/TScheduler.h>
#include <Toshi/TSystem.h>
#include <Toshi/T2SimpleArray.h>
#include <Toshi/TPString8.h>

#include <Core/TMemoryInitialiser.h>

#include <Render/TRenderInterface.h>
#include <Render/TModelRegistry.h>

#include <Input/TInputInterface.h>

#ifdef TOSHI_SKU_WINDOWS
#include <Platform/DX8/TTextureResourceHAL_DX8.h>
#include <Platform/DX8/TRenderInterface_DX8.h>
#include <Platform/DX8/TVertexBlockResource_DX8.h>
#include <Platform/DX8/TIndexBlockResource_DX8.h>
#endif // TOSHI_SKU_WINDOWS

#include "AppBoot.h"
#include "AClassHelper.h"
#include "Tasks/ADummyTask.h"
#include "Locale/ALocaleManager.h"
#include "GUI/AGUI2.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

AApplication g_oTheApp;

TOSHI_NAMESPACE_USING

#ifndef TOSHI_DIST
#define TOSHI_ENTRY int main(int argc, char** argv)
#else
#define TOSHI_ENTRY int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, INT cmdShow)
#endif

static const char* GetOSName(OSVERSIONINFOEX& osVersionInfo)
{
	TBOOL isWorkstation = osVersionInfo.wProductType == VER_NT_WORKSTATION;

	if (osVersionInfo.dwMajorVersion == 10)
	{
		return isWorkstation ? "Windows 10" : "Windows Server 2016";
	}

	if (osVersionInfo.dwMajorVersion == 6)
	{
		if (osVersionInfo.dwMinorVersion == 0)
		{
			return isWorkstation ? "Windows Vista" : "Windows Server 2008";
		}

		if (osVersionInfo.dwMinorVersion == 1)
		{
			return isWorkstation ? "Windows 7" : "Windows Server 2008 R2";
		}

		if (osVersionInfo.dwMinorVersion == 2)
		{
			return isWorkstation ? "Windows 8" : "Windows Server 2012";
		}

		if (osVersionInfo.dwMinorVersion == 3)
		{
			return isWorkstation ? "Windows 8.1" : "Windows Server 2012 R2";
		}
	}

	return "unknown";
}

TOSHI_ENTRY
{
	AClassHelper::ReferenceTClasses();

	static TMemoryInitialiser s_MemoryInitialiser;
	LPSTR cmd = GetCommandLineA();

	TUtil::ToshiCreate(cmd, 0, 0);
	TMemory::StartDebugPipe();

	g_oSystemManager.SetQuitCallback([]() {
		TUtil::ToshiDestroy();
		exit(0);
	});
	AMemory::CreatePool(AMemory::POOL_StringPool);
	TUtil::Log("Build Version %s", "0.28");

	OSVERSIONINFOEX osVersionInfo = { };
	osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);

	const char* osName = "unknown";
	HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");

	if (ntdll != NULL)
	{
		typedef void (WINAPI* t_RtlGetVersion) (OSVERSIONINFOEX*);
		auto RtlGetVersion = reinterpret_cast<t_RtlGetVersion>(GetProcAddress(ntdll, "RtlGetVersion"));

		if (RtlGetVersion != NULL)
		{
			RtlGetVersion(&osVersionInfo);
			osName = GetOSName(osVersionInfo);
		}
	}

	TUtil::Log("Command Line: %s", cmd);
	TUtil::Log("OS Name: %s", osName);
	TUtil::Log("OS Version: %d.%d Build:%d %s", osVersionInfo.dwMajorVersion, osVersionInfo.dwMinorVersion, osVersionInfo.dwBuildNumber, osVersionInfo.szCSDVersion);

	TUtil::SetGlobalMutex(
		CreateMutexA(NULL, TTRUE, "BARNYARD")
	);

	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBoxA(NULL, "Game is already running", "Barnyard", MB_OK);
		return 0;
	}

	TBOOL bCreated = g_oTheApp.Create("The Barnyard - (c) Blue Tongue Entertainment", 0, 0);

	if (bCreated)
	{
		g_oTheApp.Execute();
	}

	TUtil::ToshiDestroy();
	return 0;
}

TBOOL AApplication::OnCreate(int argc, char** argv)
{
	TOSHI_INFO("Starting Barnyard...");

	if (!CreateStringPool())
		return TFALSE;

	TTextureResourceHAL::ms_oFreeList.SetCapacity(690);
	TTextureResourceHAL::ms_oFreeList.SetGrowSize(0);

	TVertexBlockResource::ms_oFreeList.SetCapacity(500);
	TVertexBlockResource::ms_oFreeList.SetGrowSize(0);

	TIndexBlockResource::ms_oFreeList.SetCapacity(350);
	TIndexBlockResource::ms_oFreeList.SetGrowSize(0);

	ALocaleManager* pLocaleManager = T2Locale::CreateSingleton<ALocaleManager>();
	TTODO("Create some instances of some classes");

	TModelRegistry::Initialise();
	pLocaleManager->SetLanguage(ALocaleManager::Lang_EnglishUK);

	m_pRenderTask = TSTATICCAST(ADummyTask*, g_oSystemManager.GetScheduler()->CreateTask(&TGetClass(ADummyTask)));
	m_pRenderTask->Create();
	m_pRenderTask->Activate(TTRUE);
	m_pRenderTask->SetName("RenderTask");

	m_pUpdate3Task = TSTATICCAST(ADummyTask*, g_oSystemManager.GetScheduler()->CreateTask(&TGetClass(ADummyTask)));
	m_pUpdate3Task->Create();
	m_pUpdate3Task->Activate(TTRUE);
	m_pUpdate3Task->SetName("Update3");

	m_pUpdate2Task = TSTATICCAST(ADummyTask*, g_oSystemManager.GetScheduler()->CreateTask(&TGetClass(ADummyTask)));
	m_pUpdate2Task->Create();
	m_pUpdate2Task->Activate(TTRUE);
	m_pUpdate2Task->SetName("Update2");

	m_pUpdate1Task = TSTATICCAST(ADummyTask*, g_oSystemManager.GetScheduler()->CreateTask(&TGetClass(ADummyTask)));
	m_pUpdate1Task->Create();
	m_pUpdate1Task->Activate(TTRUE);
	m_pUpdate1Task->SetName("Update1");

	m_pInputTask = TSTATICCAST(ADummyTask*, g_oSystemManager.GetScheduler()->CreateTask(&TGetClass(ADummyTask)));
	m_pInputTask->Create();
	m_pInputTask->Activate(TTRUE);
	m_pInputTask->SetName("InputTask");

	m_pRootTask = TSTATICCAST(ARootTask*, g_oSystemManager.GetScheduler()->CreateTask(&TGetClass(ARootTask)));

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

	TUtil::SetTPStringPool(pStringPool);
	pStringPool->InitialiseStatic();

	return TTRUE;
}
