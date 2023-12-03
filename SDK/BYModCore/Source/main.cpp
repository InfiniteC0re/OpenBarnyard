#include "pch.h"
#include "BYardSDK/AGUI2.h"
#include "BYardSDK/AGUI2Element.h"
#include "BYardSDK/AGUI2Rectangle.h"
#include "BYardSDK/AGUI2TextBox.h"
#include "BYardSDK/AGUI2FontManager.h"

#include <stdio.h>
#include <windows.h>
#include <detours.h>

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

AGUI2TextBox* g_pTextBox = TNULL;

DWORD WINAPI MainThread(HMODULE hModule)
{
	TOSHI_INFO("ModLoader thread has been started!");
	TOSHI_INFO("Waiting for Toshi systems to be loaded...");

	while (!AGUI2::IsSingletonCreated()) { Sleep(50); }
	while (!AGUI2::GetSingleton()->GetRootElement()) { Sleep(50); }

	auto pGUI = AGUI2::GetSingleton();
	
	TFLOAT fWidth, fHeight;
	pGUI->GetDimensions(fWidth, fHeight);
	TOSHI_INFO("AGUI2 was initialised ({0}x{1})", fWidth, fHeight);

	auto pFont = AGUI2FontManager::FindFont("Rekord18");

	g_pTextBox = AGUI2TextBox::CreateFromEngine();
	g_pTextBox->SetAttachment(AGUI2Element::Anchor_TopCenter, AGUI2Element::Pivot_TopCenter);
	g_pTextBox->Create(pFont, 200.0f);
	g_pTextBox->SetTransform(0, 16.0f);
	g_pTextBox->SetText(L"ModLoader works!");
	g_pTextBox->SetInFront();

	AGUI2::GetRootElement()->AddChildTail(*g_pTextBox);

	return 0;
}

DWORD APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
	{
		Toshi::TMemory(Toshi::TMemory::Flags_NativeMethods, 0).Init();

		AllocConsole();
		FILE* fDummy;
		freopen_s(&fDummy, "CONOUT$", "w", stdout);
		hModuleCore = hModule;

		Toshi::TLog::Create("BYModLoader");
		TOSHI_INFO("Log system was successfully initialised!");
		TOSHI_INFO("Starting ModLoader thread...");

		CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, 0));

		/*AGUI2::SetSingletonExplicit(*(TUINT32*)0x007b4ff4);

		TFLOAT fWidth, fHeight;
		AGUI2::GetSingleton()->GetDimensions(fWidth, fHeight);

		using t_AGUI2RectangleCTOR = AGUI2Rectangle*(__thiscall*)(AGUI2Rectangle* pThis);
		t_AGUI2RectangleCTOR AGUI2RectangleCTOR = (t_AGUI2RectangleCTOR)0x006c3ae0;
		AGUI2Rectangle* rectangle = new AGUI2Rectangle;
		AGUI2RectangleCTOR(rectangle);

		*/

		return TTRUE;
	}
	default:
		return TFALSE;
	}
}