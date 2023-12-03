#pragma once
#include <Toshi2/T2SList.h>
#include <Windows.h>
#include <detours.h>

class AHookManager
{
public:
	struct hook_t : Toshi::T2SList<hook_t>::Node
	{
		PVOID* ppPointer;
		PVOID pDetour;
	};

public:
	static void AddHook();

private:
	inline static Toshi::T2SList<hook_t> ms_Hooks;
};
