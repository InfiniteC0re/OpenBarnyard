#pragma once
#include "AMainThreadJob2.h"

class ATRBLoaderJob : public AMainThreadJob2
{
public:
	virtual void BeginJob() override;
	virtual TBOOL RunJob() override;

	void InitJob(Toshi::TTRB* a_pTRB, const char* a_szFileName);
};
