#pragma once

extern Toshi::TApplication* Toshi::CreateApplication(int argc, char** argv);

#ifdef TOSHI_PLATFORM_WINDOWS
#include <Windows.h>

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, INT cmdShow)
{
	Toshi::TApplication* app = Toshi::CreateApplication(0, nullptr);
	app->Create();

	delete app;

	return WM_QUIT;
}

//int main(int argc, char** argv)
//{
//	Toshi::TLog::Init();
//
//	Toshi::TApplication* app = Toshi::CreateApplication(argc, argv);
//	app->Create();
//
//	delete app;
//}
#else
#error "Unknown platform", 0
#endif