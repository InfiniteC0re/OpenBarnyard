#include "Toshi.h"
#include "AExampleClass.h"
#include "AApplication.h"

using Toshi::TCString;

Toshi::TApplication* Toshi::CreateApplication(int argc, char** argv)
{
	// allocate 64 mbytes for the Toshi memory block
	Toshi::TMemory::TMemManager::Initialize(64 * 1024 * 1024);

	auto application = new AApplication;
	TOSHI_INFO("Created application with {0} arguments", argc);

	return application;
}

bool AApplication::OnCreate()
{
	TOSHI_INFO("The application has been created");

	return true;
}

bool AApplication::OnUpdate()
{
	TOSHI_INFO("AApplication::OnUpdate");

	AExampleClass* exampleClass = new AExampleClass();
	exampleClass->Destroy();

	return true;
}