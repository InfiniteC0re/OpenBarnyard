#include "pch.h"
#include "AHooks.h"
#include "AModLoaderTask.h"

#include <BYardSDK/THookedRenderD3DInterface.h>
#include <BYardSDK/AGUI2FontManager.h>

#include <filesystem>

void AModLoaderTask::AGUI2MainPostRenderCallback()
{
	if (AModLoaderTask::GetSingleton()->m_pTextBox)
	{
		AModLoaderTask::GetSingleton()->m_pTextBox->PreRender();
		AModLoaderTask::GetSingleton()->m_pTextBox->Render();
		AModLoaderTask::GetSingleton()->m_pTextBox->PostRender();
	}
}

AModLoaderTask::AModLoaderTask()
{
	m_fTotalTime = 0.0f;
	m_bLoaded = TFALSE;
	m_pTextBox = TNULL;

	AHooks::AddHook(Hook_AGUI2_MainPostRenderCallback, HookType_Before, AGUI2MainPostRenderCallback);
}

AModLoaderTask::~AModLoaderTask()
{
	m_LoadedMods.DeleteAll();
	// TODO: remove hook
}

TBOOL AModLoaderTask::OnUpdate(TFLOAT a_fDeltaTime)
{
	if (m_pTextBox && m_pTextBox->IsVisible())
	{
		TFLOAT fAlpha = (m_fTotalTime - 3.0f) / 0.8f;
		Toshi::TMath::Clip(fAlpha, 0.0f, 1.0f);

		fAlpha *= fAlpha;
		m_pTextBox->SetAlpha(1.0f - fAlpha);

		if (fAlpha == 1.0f)
		{
			m_pTextBox->Hide();
		}
	}

	for (auto it = m_LoadedMods.Begin(); it != m_LoadedMods.End(); it++)
	{
		TBOOL bResult = it->OnUpdate(a_fDeltaTime);
		TASSERT(bResult != TFALSE);
	}

	m_fTotalTime += a_fDeltaTime;
	return TTRUE;
}

TBOOL AModLoaderTask::OnCreate()
{
	LoadMods();

	return TTRUE;
}

void AModLoaderTask::LoadMods()
{
	for (const auto& entry : std::filesystem::directory_iterator(L"Mods"))
	{
		if (entry.path().extension().compare(L".dll") == 0)
		{
			const wchar_t* dll = entry.path().native().c_str();

			char dllPath[MAX_PATH];
			Toshi::TStringManager::StringUnicodeToChar(dllPath, entry.path().native().c_str(), -1);

			HMODULE hModModule = LoadLibraryW(dll);
			auto fnCreateModInstance = TREINTERPRETCAST(t_CreateModInstance, GetProcAddress(hModModule, "CreateModInstance"));

			if (fnCreateModInstance)
			{
				AModInstance* pModInstance = fnCreateModInstance();

				if (pModInstance)
				{
					TOSHI_INFO("Trying to initialise '{0}'", pModInstance->GetName());

					if (pModInstance->OnLoad())
					{
						TOSHI_INFO("  Successfully initialised!");
						AddModInstance(pModInstance);
					}
					else
					{
						TOSHI_ERROR("  Couldn't initialise!");
						delete pModInstance;
						FreeLibrary(hModModule);
					}
				}
				else
				{

					TOSHI_ERROR("{0}: CreateModInstance returned TFALSE!", dllPath);
				}
			}
			else
			{
				TOSHI_ERROR("{0}: CreateModInstance is not found!", dllPath);
				FreeLibrary(hModModule);
			}
		}
	}

	m_bLoaded = TTRUE;
}

void AModLoaderTask::OnAGUI2Ready()
{
	auto pFont = AGUI2FontManager::FindFont("Rekord18");
	m_pTextBox = AGUI2TextBox::CreateFromEngine();
	m_pTextBox->SetAttachment(AGUI2Element::Anchor_MiddleCenter, AGUI2Element::Pivot_TopCenter);
	m_pTextBox->Create(pFont, 300.0f);
	m_pTextBox->SetTransform(0, -285.0f);
	m_pTextBox->SetText(L"ModLoader works!");
	m_pTextBox->SetInFront();

	if (m_bLoaded)
	{
		static wchar_t s_wcsBuffer[64];
		const wchar_t* wcsFormat = (m_uiNumMods != 1) ? L"Loaded %d mods!\n%ls" : L"Loaded %d mod!\n%ls";
		Toshi::TStringManager::String16Format(s_wcsBuffer, 64, wcsFormat, m_uiNumMods, L"Press ~ to open menu.");
		m_pTextBox->SetText(s_wcsBuffer);
	}

	for (auto it = m_LoadedMods.Begin(); it != m_LoadedMods.End(); it++)
	{
		it->OnAGUI2Ready();
	}
}

void AModLoaderTask::OnRenderInterfaceReady()
{
	auto pRender = THookedRenderD3DInterface::GetSingleton();
	TASSERT(TNULL != pRender);

	for (auto it = m_LoadedMods.Begin(); it != m_LoadedMods.End(); it++)
	{
		it->OnRenderInterfaceReady(pRender);
	}
}
