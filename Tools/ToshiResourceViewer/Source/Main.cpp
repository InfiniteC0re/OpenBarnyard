#include "pch.h"

// TOSHI_TMEMORY_FLAGS allows you to change the mode TMemory initializes with
// Flags_NativeMethods stands for native memory management methods instead of using dlmalloc
#define TOSHI_TMEMORY_FLAGS Toshi::TMemory::Flags_Standard

// TOSHI_TMEMORY_SIZE allows you to change size of the global memory block
// The value is not used when TMemory initialized with native methods
#define TOSHI_TMEMORY_SIZE 128 * 1024 * 1024

// This file includes the entrypoint so set all the settings before including it
#include "Toshi.h"

#include "AImGuiRenderer.h"
#include "Resources/AResourceViewLoader.h"

class AApplication : public Toshi::TApplication
{
public:
	virtual TBOOL OnCreate(int argc, char** argv) override
	{
		AResourceViewLoader::CreateSingleton();
		AResourceViewLoader::GetSingleton()->CreateFile("C:\\Stuff\\Barnyard\\Game\\Data\\Units\\AllUnitTypes.trz");

		return AImGuiRenderer::CreateSingleton()->CreateTRender() && TApplication::OnCreate(argc, argv);
	}

	virtual TBOOL OnDestroy() override
	{
		AResourceViewLoader::DestroySingleton();
		return TTRUE;
	}

	virtual TBOOL OnUpdate(float a_fDeltaTime) override
	{
		AResourceViewLoader::GetSingleton()->Update(a_fDeltaTime);
		AImGuiRenderer::GetSingleton()->Update(a_fDeltaTime);

		return TTRUE;
	}
};

static AApplication* g_pTheApp;

int TMain(int argc, char** argv)
{
	g_pTheApp = new AApplication;
	g_pTheApp->Create("Toshi Resource Viewer", argc, argv);
	g_pTheApp->Execute();
	delete g_pTheApp;

	return 0;
}