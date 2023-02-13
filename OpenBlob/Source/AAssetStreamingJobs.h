#pragma once

#include <Toshi/File/TFileStream.h>
#include "AAssetStreaming.h"

using namespace Toshi;

class ATRBLoaderJob : public AMainThreadJob2
{
public:
	ATRBLoaderJob()
	{
		m_streamJob = TTRBStreamJob();
	}

	virtual ~ATRBLoaderJob() = default;

	virtual JOBTYPE GetJobType()
	{
		return JOBTYPE::TRB;
	}

	virtual void Init(TTRB* trb, const char* a_szFilename)
	{
		m_streamJob.Init(trb, a_szFilename);
		TASSERT(TStringManager::String8Length(a_szFilename) > 0);
	}

	virtual void BeginJob()
	{
		m_streamJob.Init(m_streamJob.m_trb, m_streamJob.m_fileName);
		AAssetStreaming::GetSingletonWeak()->m_FileStream.AddStream(&m_streamJob);
	}

	virtual bool RunJob()
	{
		return true;
	}

};

