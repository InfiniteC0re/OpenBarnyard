#pragma once
#include "Toshi/Core.h"
#include "Toshi/TApplication.h"

#include <iostream>

#ifdef TOSHI_PLATFORM_WINDOWS
#include <Windows.h>

extern Toshi::TApplication* Toshi::CreateApplication(int argc, char** argv);

int main(int argc, char** argv)
{
	Toshi::TApplication* app = Toshi::CreateApplication(argc, argv);

	app->Start();
	app->Run();

	std::cout << "Exiting the application..." << std::endl << std::endl;
	std::cout << "Press any key to exit" << std::endl;

	std::cin.get();

	delete app;
}
#else
#error "Unknown platform", 0
#endif