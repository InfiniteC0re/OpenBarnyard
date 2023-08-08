#pragma once
#include "AppBoot.h"

#ifdef TOSHI_SKU_WINDOWS

#ifndef TOSHI_DIST
#define TOSHI_ENTRY int main(int argc, char** argv)
#else
#define TOSHI_ENTRY int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, INT cmdShow)
#endif

#ifndef TOSHI_CONSOLE

#ifndef TOSHI_APP
#define TOSHI_APP \
Toshi::TApplication& app = AApplication::g_oTheApp; \
app.Create("Blob07 - (c) Blue Tongue Software", 0, 0); \
app.Execute();
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
#define TOSHI_TMEMORY_SIZE 64 * 1024 * 1024
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
	Toshi::TRegion::SetRegion(0);
	Toshi::TMemory memorySettings(TOSHI_TMEMORY_FLAGS, TOSHI_TMEMORY_SIZE);
	Toshi::TUtil::ToshiCreate(0, 0, memorySettings);
	Toshi::TUtil::Log("Build Version %s", "0.28");

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

	LPSTR cmd = GetCommandLineA();
	Toshi::TUtil::Log("Command Line: %s", cmd);
	Toshi::TUtil::Log("OS Name: %s", osName);
	Toshi::TUtil::Log("OS Version: %d.%d Build:%d %s", osVersionInfo.dwMajorVersion, osVersionInfo.dwMinorVersion, osVersionInfo.dwBuildNumber, osVersionInfo.szCSDVersion);


	HANDLE hMutex = CreateMutexA(NULL, TTRUE, "BLOB07");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBoxA(NULL, "Game is already running", "de Blob", MB_OK);
		return 0;
	}

	if (IsDebuggerPresent())
	{
		SetUnhandledExceptionFilter(
			[](_EXCEPTION_POINTERS* ExceptionInfo) -> LONG
			{
				TOSHI_CORE_ERROR("Blob_UnhandledExceptionFilter");
				TBREAK();
				return EXCEPTION_EXECUTE_HANDLER;
			}
		);
	}

	// de blob does steam init

	TOSHI_APP;
	
	if (hMutex != NULL)
	{
		ReleaseMutex(hMutex);
	}

	// spdlog needs to be replaced with own Log system
	// because it doesn't work fine with custom allocators
	// Toshi::TUtil::ToshiDestroy();

	return 0;
}

#else
#error "Unknown platform", 0
#endif