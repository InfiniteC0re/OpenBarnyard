#include "pch.h"
#include "AEntityLoader.h"
#include "Toshi/Core/THPTimer.h"
#include "AAssetStreaming.h"


void AEntityLoader::Update(float a_flt)
{
	Toshi::THPTimer timer = Toshi::THPTimer();

	switch (m_iState)
	{
	case STATE_Loading:
		SetState(STATE_Creating);
		m_pParamList = TSTATICCAST(ParamList*, m_pTRB->GetSymbolAddress("EntitiesMain"));
		if (m_pParamList->m_iEntityCount == 0)
		{
			SetState(STATE_IOLoading);
		}
		break;
	case STATE_Creating:
		timer.Reset();
		timer.Update();
		do
		{
			if (AAssetStreaming::GetSingleton()->HasActiveJobs()) break;

			TASSERT(m_iCurEnt < m_pParamList->m_iEntityCount);


			m_iCurEnt++;
		} while (TTRUE);
		
		break;
	default:
		break;
	}
}
