#pragma once

extern Toshi::TApplication* Toshi::CreateApplication(int argc, char** argv);

#ifdef TOSHI_PLATFORM_WINDOWS
#include <Windows.h>

int main(int argc, char** argv)
{
	Toshi::TLog::Init();

	Toshi::TApplication* app = Toshi::CreateApplication(argc, argv);
	app->Create();

	delete app;
}
#else
#error "Unknown platform", 0
#endif