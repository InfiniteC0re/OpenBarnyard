#include "pch.h"
#include "ModLoader.h"
#include "BYardSDK/AGUI2.h"
#include "BYardSDK/AGUI2TextBox.h"
#include "BYardSDK/AGUI2FontManager.h"

#include <Toshi/Core/THPTimer.h>
#include <Windows.h>
#include <detours.h>

AGUI2TextBox* g_pTimerTextBox;
TFLOAT g_fTimerTime = 0.0f;
TBOOL g_bIsStarted = TFALSE;

t_AddHook ModCore_AddHook;

void ResetTimer()
{
	g_fTimerTime = 0.0f;
	g_bIsStarted = TTRUE;
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
			Toshi::TMemory(Toshi::TMemory::Flags_NativeMethods, 0).Init();
			Toshi::TLog::Create("BYSpeedrunHelper");

			if (ModCore_AddHook(Hook_NewGameStarted, HookType_Before, ResetTimer))
			{
				auto pFont = AGUI2FontManager::FindFont("Rekord18");
				g_pTimerTextBox = AGUI2TextBox::CreateFromEngine();
				g_pTimerTextBox->Create(pFont, 200.0f);
				g_pTimerTextBox->SetText(L"00:00:00.000");
				g_pTimerTextBox->SetColour(TCOLOR(253, 226, 1));
				g_pTimerTextBox->SetTransform(6.0f, 0.0f);
				g_pTimerTextBox->SetAlpha(1.0f);
				g_pTimerTextBox->SetInFront();
				g_pTimerTextBox->SetTextAlign(AGUI2Font::TextAlign_Left);
				g_pTimerTextBox->SetAttachment(AGUI2Element::Anchor_MiddleLeft, AGUI2Element::Pivot_MiddleLeft);

				AGUI2::GetRootElement()->AddChildTail(*g_pTimerTextBox);
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
		if (g_bIsStarted)
		{
			TINT iMilliseconds = Toshi::TMath::FloorToInt(fmod(g_fTimerTime, 1.0f) * 1000);
			TINT iSeconds = Toshi::TMath::FloorToInt(fmod(g_fTimerTime, 60.0f));
			TINT iMinutes = Toshi::TMath::FloorToInt(g_fTimerTime / 60.0f);
			TINT iHours = Toshi::TMath::FloorToInt(g_fTimerTime / 3600.0f);

			static wchar_t s_buffer[48];

			if (iHours != 0)
			{
				Toshi::TStringManager::String16Format(s_buffer, sizeof(s_buffer), L"%02d:%02d:%02d.%03d", iHours, iMinutes, iSeconds, iMilliseconds);
			}
			else if (iMinutes != 0)
			{
				Toshi::TStringManager::String16Format(s_buffer, sizeof(s_buffer), L"%02d:%02d.%03d", iMinutes, iSeconds, iMilliseconds);
			}
			else
			{
				Toshi::TStringManager::String16Format(s_buffer, sizeof(s_buffer), L"%0d.%03d", iSeconds, iMilliseconds);
			}

			g_pTimerTextBox->SetText(s_buffer);
			g_fTimerTime += a_fDeltaTime;
		}

		return TTRUE;
	}
}

DWORD APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
	{
		return TTRUE;
	}
	default:
		return TFALSE;
	}
}