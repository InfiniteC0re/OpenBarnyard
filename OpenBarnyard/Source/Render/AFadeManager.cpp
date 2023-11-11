#include "pch.h"
#include "AFadeManager.h"
#include "Tasks/ARootTask.h"
#include "AGUI/AGUI2.h"

TBOOL AFadeManager::OnUpdate(TFLOAT a_fDeltaTime)
{
	if (!ARootTask::GetSingleton()->IsPaused())
	{
		for (auto it = m_ActiveFades.Begin(); it != m_ActiveFades.End(); it++)
		{
			it->m_fCurrentTime += a_fDeltaTime;
		}

		for (auto it = m_ActiveFades.Begin(); it != m_ActiveFades.End(); )
		{
			auto pNext = it->Next();

			if (it->IsStillFading(TTRUE))
			{
				AFade::Color color;
				it->GetCurrentColor(color);
				TTODO("Change color of m_Overlay");
			}
			else
			{
				m_ActiveFades.Remove(it);
				m_Fades.DeleteObject(it);
			}

			it = pNext;
		}

		return TTRUE;
	}

	return TFALSE;
}

void AFadeManager::StopAllFades()
{
	while (TTRUE)
	{
		auto it = m_ActiveFades.Begin();
		if (it == m_ActiveFades.End()) break;

		m_ActiveFades.Remove(it);
		m_Fades.DeleteObject(it);
	}
}

AFade* AFadeManager::CreateFade(const AFade::Color& a_rFadeFrom, const AFade::Color& a_rFadeTo, TFLOAT a_fFadeTime)
{
	TFLOAT fWidth;
	TFLOAT fHeight;
	AGUI2::GetSingleton()->GetDimensions(fWidth, fHeight);

	if (m_Fades.GetUsedSize() == m_Fades.GetCapacity())
	{
		auto pFade = m_ActiveFades.Begin();

		if (pFade != m_ActiveFades.End())
		{
			m_ActiveFades.Remove(pFade);
			m_Fades.DeleteObject(pFade);
		}
	}

	TASSERT(m_Fades.GetUsedSize() != m_Fades.GetCapacity());

	auto pFade = m_Fades.NewObject();
	pFade->SetFadeTime(a_fFadeTime);
	pFade->SetFadeFromColor(a_rFadeFrom);
	pFade->SetFadeToColor(a_rFadeTo);

	TTODO("Setup m_Overlay when AGUI2Element and AGUI2Rectangle are implemented");

	return pFade;
}
