#pragma once
#include "AHooks.h"
#include <Windows.h>

struct ModInfo_t
{
	const char* szModName;
};

using t_GetModInfo = void(*)(ModInfo_t& a_rModInfo);
using t_InitialiseMod = TBOOL(*)();
using t_DeinitialiseMod = void(*)();
using t_AddHook = TBOOL(*)(Hook a_eHook, HookType a_eHookType, void* a_pCallback);

TBOOL LinkModCoreAtRuntime(t_AddHook& a_rAddHook)
{
	auto hModCore = GetModuleHandleA("BYModCore.dll");

	if (hModCore)
	{
		a_rAddHook = (t_AddHook)GetProcAddress(hModCore, "AddHook");
		return a_rAddHook != TNULL;
	}

	return TFALSE;
}
