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

typedef void (WINAPI* RtlGetVersion_FUNC) (OSVERSIONINFOEX*);

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

	TOSHI_INFO(L"Command Line: {0}", cmd);
	//TOSHI_INFO("OS Name: {0}", osVersionInfo.);

	TOSHI_APP

	// spdlog needs to be replaced with own Log system
	// because it doesn't work fine with custom allocators
	// Toshi::TUtil::ToshiDestroy();

	return 0;
}

#else
#error "Unknown platform", 0
#endif