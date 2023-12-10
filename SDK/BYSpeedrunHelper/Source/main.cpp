#include "pch.h"
#include "ModLoader.h"
#include "BYardSDK/AGUI2.h"
#include "AGUITimer.h"

#include <Toshi/Core/THPTimer.h>

AGUITimer g_Timer;

t_AddHook ModCore_AddHook;

void NewGameStarted()
{
	g_Timer.Reset();
	g_Timer.Start();
}

void AGUI2_MainPostRenderCallback()
{
	g_Timer.Render();
}

extern "C"
{
	__declspec(dllexport) void GetModInfo(ModInfo_t& a_rModInfo)
	{
		a_rModInfo.szModName = "BYSpeedrunHelper";
	}

	__declspec(dllexport) TBOOL InitialiseMod()
	{
		TBOOL bLinked = LinkModCoreAtRuntime(ModCore_AddHook);
		
		if (bLinked)
		{
			Toshi::TMemory::Initialise(32 * 1024 * 1024, 0);
			Toshi::TLog::Create("BYSpeedrunHelper");

			if (ModCore_AddHook(Hook_NewGameStarted, HookType_Before, NewGameStarted) && 
				ModCore_AddHook(Hook_AGUI2_MainPostRenderCallback, HookType_Before, AGUI2_MainPostRenderCallback))
			{
				g_Timer.Create();
				return TTRUE;
			}
		}

		return TFALSE;
	}

	__declspec(dllexport) void DeinitialiseMod()
	{
		// TODO
	}

	__declspec(dllexport) TBOOL UpdateMod(TFLOAT a_fDeltaTime)
	{
		g_Timer.Update(a_fDeltaTime);
		return TTRUE;
	}
}
