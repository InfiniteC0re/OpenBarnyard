#include "pch.h"
#include "AHooks.h"
#include "AModLoaderTask.h"

#include <BYardSDK/AGUI2FontManager.h>

void AModLoaderTask::AGUI2MainPostRenderCallback()
{
	AModLoaderTask::GetSingleton()->m_pTextBox->PreRender();
	AModLoaderTask::GetSingleton()->m_pTextBox->Render();
	AModLoaderTask::GetSingleton()->m_pTextBox->PostRender();
}

AModLoaderTask::AModLoaderTask()
{
	m_fTotalTime = 0.0f;

	auto pFont = AGUI2FontManager::FindFont("Rekord18");
	m_pTextBox = AGUI2TextBox::CreateFromEngine();
	m_pTextBox->SetAttachment(AGUI2Element::Anchor_MiddleCenter, AGUI2Element::Pivot_TopCenter);
	m_pTextBox->Create(pFont, 200.0f);
	m_pTextBox->SetTransform(0, -285.0f);
	m_pTextBox->SetText(L"ModLoader works!");
	m_pTextBox->SetInFront();

	AddHook(Hook_AGUI2_MainPostRenderCallback, HookType_Before, AGUI2MainPostRenderCallback);
}

AModLoaderTask::~AModLoaderTask()
{
	m_LoadedMods.DeleteAll();
	// TODO: remove hook
}

TBOOL AModLoaderTask::OnUpdate(TFLOAT a_fDeltaTime)
{
	if (m_pTextBox->IsVisible())
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
		TBOOL bResult = it->Update(a_fDeltaTime);
		TASSERT(bResult != TFALSE);
	}

	m_fTotalTime += a_fDeltaTime;
	return TTRUE;
}
