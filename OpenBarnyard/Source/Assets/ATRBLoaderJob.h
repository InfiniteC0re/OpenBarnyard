#pragma once
#include "AMainThreadJob.h"

#include <File/TFileStream.h>

class ATRBLoaderJob : public AMainThreadJob
{
public:
	virtual void  BeginJob() override;
	virtual TBOOL RunJob() override;

	void InitJob( Toshi::TTRB* a_pTRB, const TCHAR* a_szFileName );

	Toshi::TTRBStreamJob& GetStream() { return m_oStreamJob; }

protected:
	Toshi::TTRBStreamJob m_oStreamJob;
};
