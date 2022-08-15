#include "Toshi.h"
#include "AExampleClass.h"

class AApplication : public Toshi::TApplication
{
public:
	void Start() override
	{
		TOSHI_INFO("The application has started");
	}

	void Run() override
	{
		TOSHI_INFO("Running the application...");

		AExampleClass* exampleClass = new AExampleClass();
		exampleClass->Destroy();
	}
};

Toshi::TApplication* Toshi::CreateApplication(int argc, char** argv)
{
	Toshi::TMemory::TMemManager::Initialize(64 * 1024 * 1024);

	auto application = new AApplication();
	TOSHI_INFO("Created application with {0} arguments", argc);
	
	return application;
}