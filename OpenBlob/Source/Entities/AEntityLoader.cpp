#include "pch.h"
#include "AEntityLoader.h"

void AEntityLoader::Update(float a_flt)
{
	switch (m_iState)
	{
	case Loading:
		break;
	case Creating:
		Toshi::THPTimer timer = Toshi::THPTimer();
		timer.Reset();
		timer.Update();
		do
		{
			//bool bRes = AAssetStreaming::GetSingleton()->HasActiveJobs();
			//if (bRes) break;
			TASSERT(m_iCurEnt < m_pParamList->m_iEntityCount, "");
			m_iCurEnt++;
		} while (true);
		
		break;
	default:
		break;
	}
}
