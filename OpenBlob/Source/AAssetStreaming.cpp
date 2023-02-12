#include "pch.h"
#include "AAssetStreaming.h"

void AAssetStreaming::Update()
{
	if (m_pCurrentJob == TNULL)
	{
		if (!m_Jobs.IsEmpty())
		{
			RunJob();
		}
	}
	else
	{
		m_pCurrentJob->RunJob();
	}
}
