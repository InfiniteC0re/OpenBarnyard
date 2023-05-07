#pragma once

#include <Toshi/File/TFileStream.h>
#include "AAssetStreaming.h"

using namespace Toshi;

class ATRBLoaderJob : public AMainThreadJob2
{
public:
	ATRBLoaderJob()
	{
		
	}

	virtual ~ATRBLoaderJob() = default;

	virtual JOBTYPE GetJobType()
	{
		return JOBTYPE::TRB;
	}

	virtual void Init(TTRB* trb, const char* a_szFilename)
	{
		m_StreamJob.Init(trb, a_szFilename);
		TASSERT(TStringManager::String8Length(a_szFilename) > 0);
	}

	virtual void BeginJob()
	{
		m_StreamJob.Init(m_StreamJob.m_trb, m_StreamJob.m_fileName);
		AAssetStreaming::GetSingletonWeak()->GetFileStream().AddStream(&m_StreamJob);
	}

	virtual bool RunJob()
	{
		return m_IsRunning;
	}
};

