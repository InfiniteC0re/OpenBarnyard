#pragma once
#include "AMainThreadJob.h"

#include <Toshi/File/TFileStream.h>

class ATRBLoaderJob : public AMainThreadJob
{
public:
	virtual void BeginJob() override;
	virtual TBOOL RunJob() override;

	void InitJob(Toshi::TTRB* a_pTRB, const char* a_szFileName);

	Toshi::TTRBStreamJob& GetStream() { return m_oStreamJob; }

protected:
	Toshi::TTRBStreamJob m_oStreamJob;
};