#include "pch.h"

#include <stdio.h>
#include <windows.h>
#include <detours.h>

#define TOSHI_TMEMORY_SIZE 128 * 1024 * 1024
#define TOSHI_TMEMORY_FLAGS Toshi::TMemory::Flags_NativeMethods

#define thisCallHook(fnName, addr, thisType, retType, ...) \
    typedef retType (__fastcall fnName)(thisType _this, DWORD _edx, __VA_ARGS__); \
    fnName* real##fnName = (fnName*)addr; \
    retType __fastcall _##fnName(thisType _this, DWORD _edx, __VA_ARGS__)

#define Attach(fnName) DetourAttach(&(PVOID&)real##fnName, _##fnName);
#define Dettach(fnName) DetourDetach(&(PVOID&)real##fnName, _##fnName);

HMODULE hModuleCore;

//// 0x006c6da0 - ~TRenderD3DInterface()
//thisCallHook(TRenderD3DInterfaceDestructor, 0x006c6da0, void*, void*, char flag)
//{
//	std::cout << "[Modloader] Unloading " << plugins.size() << " mods" << std::endl;
//
//	for (auto& plugin : plugins) plugin->Unload();
//	FreeLibrary(hModuleCore);
//
//	return realTRenderD3DInterfaceDestructor(_this, _edx, flag);
//}

DWORD APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
	{
		Toshi::TMemory(TOSHI_TMEMORY_FLAGS, TOSHI_TMEMORY_SIZE).Init();

		AllocConsole();
		FILE* fDummy;
		freopen_s(&fDummy, "CONOUT$", "w", stdout);
		hModuleCore = hModule;

		Toshi::TLog::Create();
		TOSHI_INFO("Log system was successfully initialised!");
		/*AGUI2::SetSingletonExplicit(*(TUINT32*)0x007b4ff4);

		TFLOAT fWidth, fHeight;
		AGUI2::GetSingleton()->GetDimensions(fWidth, fHeight);

		using t_AGUI2RectangleCTOR = AGUI2Rectangle*(__thiscall*)(AGUI2Rectangle* pThis);
		t_AGUI2RectangleCTOR AGUI2RectangleCTOR = (t_AGUI2RectangleCTOR)0x006c3ae0;
		AGUI2Rectangle* rectangle = new AGUI2Rectangle;
		AGUI2RectangleCTOR(rectangle);

		rectangle->Create(200, 300);
		rectangle->SetAttachment(AGUI2Element::Anchor_BottomLeft, AGUI2Element::Pivot_BottomLeft);
		rectangle->Show();
		rectangle->SetInFront();
		rectangle->SetColour(TCOLOR(60, 255, 60));
		AGUI2::GetRootElement()->AddChildTail(rectangle);*/

		return TTRUE;
	}
	default:
		return TFALSE;
	}
}