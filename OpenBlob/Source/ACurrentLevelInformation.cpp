#include "pch.h"
#include "ACurrentLevelInformation.h"


void ACurrentLevelInformation::Create(DataBlock a_type)
{
	auto assetStreaming = AAssetStreaming::GetSingleton();

	char* szFilename = Toshi::TStringManager::GetTempString8();

	for (size_t i = 0; i < DataBlockCount; i++)
	{
		TASSERT(a_type >= 0 && a_type < DataBlockCount);

		if (sm_dataBlockFilenames[i].first == 0)
		{
			Toshi::TStringManager::String8Format(szFilename, sizeof(szFilename), "data/%s/%s", m_szLevelName, sm_dataBlockFilenames[i].second);
		}
		else
		{
			Toshi::TStringManager::String8Format(szFilename, sizeof(szFilename), "data/%s", sm_dataBlockFilenames[i].second);
		}

		TASSERT(!m_streamingJobs[i]);
		TASSERT(!m_datablocks[i]);
		m_datablocks[i] = new Toshi::TTRB();
		m_streamingJobs[i] = (ATRBLoaderJob*)assetStreaming->GetAvaiableJob();
		m_streamingJobs[i]->Init(m_datablocks[i], szFilename);
		assetStreaming->AddMainThreadJob2(m_streamingJobs[i]);
	}
}

Toshi::TTRB* ACurrentLevelInformation::GetDataBlock(DataBlock a_type)
{
	
	return nullptr;
}

void ACurrentLevelInformation::Stream()
{
	for (size_t i = 0; i < DataBlockCount; i++)
	{
		
	}
}
