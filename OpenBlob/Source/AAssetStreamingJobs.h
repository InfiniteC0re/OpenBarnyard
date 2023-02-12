#pragma once

#include <Toshi/File/TFileStream.h>
#include "AAssetStreaming.h"

using namespace Toshi;

class ATRBLoaderJob : public AMainThreadJob2
{
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
		TStringManager::String8Copy(m_fileName, a_szFilename, 0x80);
		m_trb = trb;
		TASSERT(TStringManager::String8Length(a_szFilename) > 0);
	}

	virtual void BeginJob()
	{
		m_streamJob.Init(m_trb, m_fileName);
		AAssetStreaming::GetSingletonWeak()->m_FileStream.AddStream(&m_streamJob);
	}

	virtual bool RunJob()
	{
		return true;
	}

	TTRBStreamJob m_streamJob;
	char* m_fileName;
	TTRB* m_trb;
};

