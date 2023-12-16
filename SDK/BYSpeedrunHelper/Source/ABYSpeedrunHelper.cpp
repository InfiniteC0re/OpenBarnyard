#include "pch.h"
#include "AGUITimer.h"
#include "ACollisionInspector.h"

#include <AImGUI.h>
#include <ModLoader.h>
#include <AHooks.h>

#include <BYardSDK/AGUI2.h>

#include <Toshi/Core/THPTimer.h>

AGUITimer g_Timer;

void NewGameStarted()
{
	g_Timer.Reset();
	g_Timer.Start();
}

void AGUI2_MainPostRenderCallback()
{
	g_Timer.Render();
}

class ABYSpeedrunHelper : public AModInstance
{
public:
	TBOOL OnLoad() override
	{
		if (AHooks::AddHook(Hook_NewGameStarted, HookType_Before, NewGameStarted) &&
			AHooks::AddHook(Hook_AGUI2_MainPostRenderCallback, HookType_Before, AGUI2_MainPostRenderCallback))
		{
			ACollisionInspector::CreateSingleton();
			g_Timer.Create();
			return TTRUE;
		}

		return TFALSE;
	}

	TBOOL OnUpdate(TFLOAT a_fDeltaTime) override
	{
		g_Timer.Update(a_fDeltaTime);
		return TTRUE;
	}

	void OnUnload() override
	{
		
	}

	void OnImGuiRender() override
	{
		ImGui::Checkbox("Show Timer", &g_Timer.IsVisible());
		ImGui::Checkbox("Show Collision", &ACollisionInspector::GetSingleton()->IsCollisionVisible());
	}
	
	TBOOL HasSettingsUI() override
	{
		return TTRUE;
	}

	const char* GetName() override
	{
		return "BYSpeedrunHelper";
	}
};

extern "C"
{
	MODLOADER_EXPORT AModInstance* CreateModInstance()
	{
		Toshi::TMemory::Initialise(8 * 1024 * 1024, 0);
		Toshi::TLog::Create("BYSpeedrunHelper");

		return new ABYSpeedrunHelper();
	}
}
