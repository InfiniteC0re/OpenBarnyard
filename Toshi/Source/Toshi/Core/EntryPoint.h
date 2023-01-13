#pragma once

extern Toshi::TApplication* Toshi::CreateApplication(int argc, char** argv);

#ifdef TOSHI_PLATFORM_WINDOWS
#include <Windows.h>
#include "../Utils/TRegion.h"

#ifndef TOSHI_DIST
#define TOSHI_ENTRY int main(int argc, char** argv)
#else
#define TOSHI_ENTRY int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, INT cmdShow)
#endif

#ifndef TOSHI_CONSOLE

#ifndef TOSHI_APP
#define TOSHI_APP \
Toshi::TApplication* app = Toshi::CreateApplication(0, nullptr); \
app->Create("Blob07 - (c) Blue Tongue Software", 0, 0); \
app->Execute(); \
delete app;
#endif

#else

#define TOSHI_ENTRY int main(int argc, char** argv)

#ifndef TOSHI_APP
#define TOSHI_APP \
extern int TMain(int argc, char** argv); \
return TMain(__argc, __argv);
#endif

#endif

#ifndef TOSHI_TMEMORY_FLAGS
#define TOSHI_TMEMORY_FLAGS Toshi::TMemory::Flags_Standard
#endif

#ifndef TOSHI_TMEMORY_SIZE
#define TOSHI_TMEMORY_SIZE 64 * 1024 * 1024 // deblob says 0x28000000
#endif

const char* s_cOsNames[11] =
{
	"Windows 10",
	"Windows Server 2016",
	"Windows 8.1",
	"Windows Server 2012 R2",
	"Windows 8",
	"Windows Server 2012",
	"Windows 7",
	"Windows Server 2008 R2",
	"Windows Server 2008",
	"Windows Vista",
	"unknown"
};

typedef void (WINAPI* RtlGetVersion_FUNC) (OSVERSIONINFOEX*);

//const char* GetOSName()
//{
//	OSVERSIONINFOEX osVersionInfo;
//	::ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
//	osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
//
//	RtlGetVersion_FUNC func = (RtlGetVersion_FUNC)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion");
//	if (func != 0) {
//		func(&osVersionInfo);
//	}
//
//	int index = 0;
//
//	//while (osVersionInfo.dwMajorVersion != 0xA || osVersionInfo.dwMinorVersion != 0 || )
//	return s_cOsNames[0];
//}


TOSHI_ENTRY
{
	Toshi::TRegion::SetRegion(0);
	Toshi::TMemory memorySettings(TOSHI_TMEMORY_FLAGS, TOSHI_TMEMORY_SIZE);
	Toshi::TUtil::ToshiCreate(0, 0, memorySettings);
	TOSHI_INFO("Build Version {0}", 0.28);

	OSVERSIONINFOEX osVersionInfo;
	::ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
	osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);

	RtlGetVersion_FUNC func = (RtlGetVersion_FUNC)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion");
	if (func != 0) {
		func(&osVersionInfo);
	}

	LPTSTR cmd = GetCommandLine();

	TOSHI_INFO(L"Command Line: {}", cmd);
	TOSHI_INFO("OS Name: {}", "Windows 10");
	TOSHI_INFO(L"OS Version: {}.{} Build:{} {}", osVersionInfo.dwMajorVersion, osVersionInfo.dwMinorVersion, osVersionInfo.dwBuildNumber, osVersionInfo.szCSDVersion);

	HANDLE hMutex = CreateMutexA(NULL, true, "BLOB07");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBoxA(NULL, "Game is already running", "de Blob", MB_OK);
		return 0;
	}

	if (IsDebuggerPresent()) TIMPLEMENT_D("SetUnhandledExceptionFilter(FUN_00576760);");
	// de blob does steam init

	TOSHI_APP
		ReleaseMutex(hMutex);
	// spdlog needs to be replaced with own Log system
	// because it doesn't work fine with custom allocators
	// Toshi::TUtil::ToshiDestroy();

	return 0;
}

#else
#error "Unknown platform", 0
#endif