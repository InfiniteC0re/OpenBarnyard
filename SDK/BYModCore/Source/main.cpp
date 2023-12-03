#include "pch.h"
#include "AHooks.h"
#include "ModLoader.h"
#include "AModLoaderTask.h"
#include "BYardSDK/SDKHooks.h"

#include "BYardSDK/AGUI2.h"
#include "BYardSDK/AGUI2Element.h"
#include "BYardSDK/AGUI2Rectangle.h"
#include "BYardSDK/AGUI2TextBox.h"
#include "BYardSDK/AGUI2FontManager.h"
#include "BYardSDK/AGUISlideshow.h"

#include <Toshi/Core/THPTimer.h>

#include <stdio.h>
#include <windows.h>
#include <filesystem>

HMODULE hModuleCore;
AGUI2TextBox* g_pTextBox = TNULL;
AModLoaderTask* g_pModLoaderTask = TNULL;

DWORD WINAPI MainThread(HMODULE hModule)
{
	TOSHI_INFO("ModLoader thread has been started!");
	TOSHI_INFO("Waiting for Toshi systems to be loaded...");

	// Wait until AGUI2 is ready to use
	while (!AGUI2::IsSingletonCreated()) { Sleep(50); }
	while (!AGUI2::GetSingleton()->GetRootElement()) { Sleep(50); }

	// Create AModLoaderTask
	Toshi::TTask* pRootTask = *(Toshi::TTask**)0x0077de78;
	auto pScheduler = CALL_THIS(0x006bbc10, Toshi::TSystemManager*, Toshi::TScheduler*, (Toshi::TSystemManager*)0x007ce640);
	g_pModLoaderTask = CALL_THIS(0x006bcbf0, Toshi::TScheduler*, AModLoaderTask*, pScheduler, Toshi::TClass*, AModLoaderTask::GetClassStatic(), Toshi::TTask*, pRootTask);

	// Initialise hooks
	AHooks::Initialise();

	auto pGUI = AGUI2::GetSingleton();
	
	TFLOAT fWidth, fHeight;
	pGUI->GetDimensions(fWidth, fHeight);
	TOSHI_INFO("AGUI2 is ready! (Dimensions: {0}x{1})", fWidth, fHeight);

	auto pFont = AGUI2FontManager::FindFont("Rekord18");
	g_pTextBox = AGUI2TextBox::CreateFromEngine();
	g_pTextBox->SetAttachment(AGUI2Element::Anchor_TopCenter, AGUI2Element::Pivot_TopCenter);
	g_pTextBox->Create(pFont, 200.0f);
	g_pTextBox->SetTransform(0, 16.0f);
	g_pTextBox->SetText(L"ModLoader works!");
	g_pTextBox->SetInFront();

	AGUI2::GetRootElement()->AddChildTail(*g_pTextBox);
	TINT uiNumLoaded = 0;

	for (const auto& entry : std::filesystem::directory_iterator(L"Mods"))
	{
		if (entry.path().extension().compare(L".dll") == 0)
		{
			const wchar_t* dll = entry.path().native().c_str();
			HMODULE hModModule = LoadLibraryW(dll);

			auto fnGetModInfo = TREINTERPRETCAST(t_GetModInfo, GetProcAddress(hModModule, "GetModInfo"));
			auto fnInitialiseMod = TREINTERPRETCAST(t_InitialiseMod, GetProcAddress(hModModule, "InitialiseMod"));
			auto fnDeinitialiseMod = TREINTERPRETCAST(t_DeinitialiseMod, GetProcAddress(hModModule, "DeinitialiseMod"));
			auto fnUpdateMod = TREINTERPRETCAST(AModInstance::t_UpdateMod, GetProcAddress(hModModule, "UpdateMod"));
			
			if (fnGetModInfo && fnInitialiseMod && fnDeinitialiseMod)
			{
				ModInfo_t modInfo;
				fnGetModInfo(modInfo);

				TOSHI_INFO("Trying to initialise '{0}'", modInfo.szModName);
				
				if (fnInitialiseMod())
				{
					TOSHI_INFO("Successfully loaded '{0}' mod", modInfo.szModName);
					g_pModLoaderTask->AddModInstance(new AModInstance(fnUpdateMod));
					uiNumLoaded++;
				}
				else
				{
					TOSHI_ERROR("Unable to load '{0}' mod", modInfo.szModName);
				}
			}
			else
			{
				char path[MAX_PATH];
				Toshi::TStringManager::StringUnicodeToChar(path, entry.path().filename().native().c_str(), -1);

				TOSHI_ERROR("Tried to load unsupported mod: {0}", path);
				FreeLibrary(hModModule);
			}
		}
	}

	g_pModLoaderTask->Create();

	static wchar_t s_buffer[256];
	Toshi::TStringManager::String16Format(s_buffer, sizeof(s_buffer), L"Loaded %d mods!", uiNumLoaded);
	g_pTextBox->SetText(s_buffer);

	Toshi::THPTimer timer;
	TFLOAT fTimeFromStart = 0.0f;

	while (true)
	{
		timer.Update();
		fTimeFromStart += timer.GetDelta();

		TFLOAT fAlpha = (fTimeFromStart - 3.0f) / 0.8f;
		Toshi::TMath::Clip(fAlpha, 0.0f, 1.0f);

		fAlpha *= fAlpha;
		g_pTextBox->SetAlpha(1.0f - fAlpha);

		if (fAlpha == 1.0f)
		{
			g_pTextBox->Hide();
			break;
		}

		Sleep(25);
	}

	return 0;
}

BOOL WINAPI exit_handler(DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
	case CTRL_C_EVENT:
		return TRUE;
	case CTRL_BREAK_EVENT:
		return TRUE;
	case CTRL_CLOSE_EVENT:
		return TRUE;
	default:
		return FALSE;
	}

	return TRUE;
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

		SetConsoleCtrlHandler(exit_handler, TRUE);

		Toshi::TLog::Create("BYModLoader");

		TOSHI_INFO("Log system was successfully initialised!");
		TOSHI_INFO("Starting ModLoader thread...");

		CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, 0));

		return TTRUE;
	}
	default:
		return TFALSE;
	}
}