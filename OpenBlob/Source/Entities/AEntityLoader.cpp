#include "pch.h"
#include "AEntityLoader.h"

void AEntityLoader::Update(float a_flt)
{
	Toshi::THPTimer timer = Toshi::THPTimer();
	switch (m_iState)
	{
	case Loading:
		break;
	case Creating:
		
		timer.Reset();
		timer.Update();
		do
		{
			//TBOOL bRes = AAssetStreaming::GetSingleton()->HasActiveJobs();
			//if (bRes) break;
			TASSERT(m_iCurEnt < m_pParamList->m_iEntityCount);
			m_iCurEnt++;
		} while (TTRUE);
		
		break;
	default:
		break;
	}
}
