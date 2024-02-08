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
#include "BYardSDK/THookedRenderD3DInterface.h"

#include <Toshi/THPTimer.h>
#include <Toshi/TUtil.h>

#include <stdio.h>
#include <windows.h>

HMODULE hModuleCore;

const char* GetModsDirectory()
{
	return "Mods\\";
}

DWORD WINAPI MainThread(HMODULE hModule)
{
	TINFO("BYModCore thread has been started!\n");
	TINFO("Waiting for Toshi systems to be loaded...\n");

	// Wait until AGUI2 is ready to use
	while (!AGUI2::IsSingletonCreated()) { Sleep(50); }
	while (!AGUI2::GetSingleton()->GetRootElement()) { Sleep(50); }

	// Log info about AGUI2
	TFLOAT fWidth, fHeight;
	AGUI2::GetSingleton()->GetDimensions(fWidth, fHeight);
	TINFO("AGUI2 is ready! (Dimensions: {%f}x{%f})\n", fWidth, fHeight);
	
	Toshi::TUtil::CreateTPStringPool();
	Toshi::TUtil::SetTPStringPool(**(Toshi::TPString8Pool***)0x007ce230);
	
	AGlobalModLoaderTask::Get()->OnAGUI2Ready();
	AImGUI::CreateSingleton();

	return TTRUE;
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
		Toshi::TMemory::Initialise(4 * 1024 * 1024, 0);
		
#ifndef TOSHI_NO_LOGS
		AllocConsole();
		FILE* fDummy;
		freopen_s(&fDummy, "CONOUT$", "w", stdout);
		hModuleCore = hModule;
#endif

		Toshi::TUtil::Create();

		// Initialise hooks
		AHooks::Initialise();

		SetConsoleCtrlHandler(exit_handler, TRUE);

		TINFO("Log system was successfully initialised!\n");
		TINFO("Starting BYModCore thread...\n");

		if (AGlobalModLoaderTask::CreateSingleton()->Create())
		{
			CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, 0));
		}

		return TTRUE;
	}
	default:
		return TFALSE;
	}
}