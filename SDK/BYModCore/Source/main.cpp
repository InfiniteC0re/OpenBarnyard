#include "pch.h"
#include "AHooks.h"
#include "ModLoader.h"
#include "AModLoaderTask.h"
#include "AImGUI.h"

#include "BYardSDK/SDKHooks.h"

#include "BYardSDK/AGUI2.h"
#include "BYardSDK/AGUI2Element.h"
#include "BYardSDK/AGUI2Rectangle.h"
#include "BYardSDK/AGUI2TextBox.h"
#include "BYardSDK/AGUI2FontManager.h"
#include "BYardSDK/AGUISlideshow.h"

#include <Toshi/Core/THPTimer.h>

#include <stdio.h>
#include <windows.h>

HMODULE hModuleCore;
AModLoaderTask* g_pModLoaderTask = TNULL;

DWORD WINAPI MainThread(HMODULE hModule)
{
	TOSHI_INFO("ModLoader thread has been started!");
	TOSHI_INFO("Waiting for Toshi systems to be loaded...");

	// Wait until AGUI2 is ready to use
	while (!AGUI2::IsSingletonCreated()) { Sleep(50); }
	while (!AGUI2::GetSingleton()->GetRootElement()) { Sleep(50); }

	// Log info about AGUI2
	TFLOAT fWidth, fHeight;
	AGUI2::GetSingleton()->GetDimensions(fWidth, fHeight);
	TOSHI_INFO("AGUI2 is ready! (Dimensions: {0}x{1})", fWidth, fHeight);

	// Create AModLoaderTask
	Toshi::TTask* pRootTask = *(Toshi::TTask**)0x0077de78;
	auto pScheduler = CALL_THIS(0x006bbc10, Toshi::TSystemManager*, Toshi::TScheduler*, (Toshi::TSystemManager*)0x007ce640);
	g_pModLoaderTask = CALL_THIS(0x006bcbf0, Toshi::TScheduler*, AModLoaderTask*, pScheduler, Toshi::TClass*, &TGetClass(AModLoaderTask), Toshi::TTask*, pRootTask);
	
	return g_pModLoaderTask->Create() && AImGUI::CreateSingleton() != TNULL;
}

BOOL WINAPI exit_handler(DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
	case CTRL_C_EVENT:
		return TRUE;
	case CTRL_BREAK_EVENT:
		return TRUE;
	case CTRL_CLOSE_EVENT:
		return TRUE;
	default:
		return FALSE;
	}

	return TRUE;
}

DWORD APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
	{
		Toshi::TMemory::Initialise(8 * 1024 * 1024, 0);
		
		AllocConsole();
		FILE* fDummy;
		freopen_s(&fDummy, "CONOUT$", "w", stdout);
		hModuleCore = hModule;
		Toshi::TLog::Create("BYModCore");

		// Initialise hooks
		AHooks::Initialise();

		SetConsoleCtrlHandler(exit_handler, TRUE);

		TOSHI_INFO("Log system was successfully initialised!");
		TOSHI_INFO("Starting main thread...");

		CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, 0));

		return TTRUE;
	}
	default:
		return TFALSE;
	}
}