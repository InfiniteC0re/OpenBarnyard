#include "pch.h"
#include "AHooks.h"
#include "ModLoader.h"
#include "HookHelpers.h"
#include "AModLoaderTask.h"
#include "ACoreSettings.h"
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
#include <Toshi/TApplication.h>
#include <Toshi/TUtil.h>

#include <stdio.h>
#include <windows.h>

TOSHI_NAMESPACE_USING

HMODULE hModuleCore;

static T2CommandLine g_CommandLine;
const T2CommandLine* g_pCommandLine = &g_CommandLine;

const char* GetModsDirectory()
{
	return "Mods\\";
}

MEMBER_HOOK( 0x006c17f0, TApplication, TApplication_Create, TBOOL, const TCHAR* a_pchName, TINT a_iArgc, TCHAR** a_ppArgv )
{
	// Create the modloader task
	AGlobalModLoaderTask::CreateSingleton()->Create();

	return CallOriginal( a_pchName, a_iArgc, a_ppArgv );
}

MEMBER_HOOK( 0x006c1760, TApplication, TApplication_Destroy, void )
{
	AGlobalModLoaderTask::Get()->UnloadMods();
	CallOriginal();
}

HOOK( 0x004042c0, ToshiDestroy, void )
{
	AGlobalModLoaderTask::Get()->UnloadMods();
	CallOriginal();

	// Temporary solution
	ExitProcess( 0 );
}

DWORD WINAPI MainThread( HMODULE hModule )
{
	TINFO( "BYModCore thread has been started!\n" );
	TINFO( "Waiting for Toshi systems to be loaded...\n" );

	// Wait until AGUI2 is ready to use
	while ( !AGUI2::IsSingletonCreated() ) { ThreadSleep( 50 ); }
	while ( !AGUI2::GetSingleton()->GetRootElement() ) { ThreadSleep( 50 ); }

	// Log info about AGUI2
	TFLOAT fWidth, fHeight;
	AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );
	TINFO( "AGUI2 is ready! (Dimensions: %fx%f)\n", fWidth, fHeight );

	TUtil::SetTPStringPool( **(TPString8Pool***)0x007ce230 );

	AGlobalModLoaderTask::Get()->OnAGUI2Ready();
	AImGUI::CreateSingleton();

	return TTRUE;
}

BOOL WINAPI exit_handler( DWORD dwCtrlType )
{
	switch ( dwCtrlType )
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

DWORD APIENTRY DllMain( HMODULE hModule, DWORD reason, LPVOID reserved )
{
	switch ( reason )
	{
		case DLL_PROCESS_ATTACH:
		{
			TMemory::Initialise( 4 * 1024 * 1024, 0 );

#ifdef TOSHI_DEBUG
			AllocConsole();
			FILE* fDummy;
			freopen_s( &fDummy, "CONOUT$", "w", stdout );
			hModuleCore = hModule;
#endif

			TUtil::TOSHIParams toshiParams;
			toshiParams.szCommandLine = "";
			toshiParams.szLogFileName = "modcore";
			toshiParams.szLogAppName  = "BYModCore";

			TUtil::ToshiCreate( toshiParams );

			g_oSettings.Load();
			g_CommandLine.Create( GetCommandLineA() );

			// Initialise hooks
			AHooks::Initialise();
			InstallHook<TApplication_Create>();
			InstallHook<TApplication_Destroy>();
			InstallHook<ToshiDestroy>();

			SetConsoleCtrlHandler( exit_handler, TRUE );

			TINFO( "Log system was successfully initialised!\n" );
			TINFO( "Starting BYModCore thread...\n" );

			CloseHandle( CreateThread( 0, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, 0 ) );

			return TTRUE;
		}
		default:
			return TFALSE;
	}
}
