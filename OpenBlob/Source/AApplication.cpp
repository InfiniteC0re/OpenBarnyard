#include "Toshi.h"
#include "AExampleClass.h"

class AApplication : public Toshi::TApplication
{
public:
	void Start() override
	{
		std::cout << "The application has started" << std::endl;
	}

	void Run() override
	{
		std::cout << "Running the application..." << std::endl;

		AExampleClass* exampleClass = new AExampleClass();
		
		// `exampleClass->Destroy()` is equal to `delete exampleClass` but can be overriden
		exampleClass->Destroy();
		// delete exampleClass;

		// while (true)
		// {
		// 	
		// }
	}
};

Toshi::TApplication* Toshi::CreateApplication(int argc, char** argv)
{
	Toshi::TMemory::TMemManager::Initialize(64 * 1024 * 1024);

	auto application = new AApplication();
	std::cout << "Created application with " << argc << " arguments" << std::endl << std::endl;
	
	return application;
}