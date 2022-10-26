#pragma once

extern Toshi::TApplication* Toshi::CreateApplication(int argc, char** argv);

#ifdef TOSHI_PLATFORM_WINDOWS
#include <Windows.h>

#ifndef TOSHI_DIST
#define TOSHI_ENTRY int main(int argc, char** argv)
#else
#define TOSHI_ENTRY int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, INT cmdShow)
#endif

TOSHI_ENTRY
{
	Toshi::TMemory memorySettings(Toshi::TMemory::Flags_Standard, 64 * 1024 * 1024);
	Toshi::TUtil::ToshiCreate(0, 0, memorySettings);
	
	Toshi::TApplication* app = Toshi::CreateApplication(0, nullptr);
	
	app->Create("Blob07 - (c) Blue Tongue Software", 0, 0);
	delete app;

	// spdlog needs to be replaced with own Log system
	// because it doesn't work fine with custom allocators
	// Toshi::TUtil::ToshiDestroy();

	return 0;
}

#else
#error "Unknown platform", 0
#endif