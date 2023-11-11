#include "pch.h"
#include "AFadeManager.h"
#include "Tasks/ARootTask.h"

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
				TUINT8 aRGBA[4];
				it->GetCurrentColor(aRGBA);
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
