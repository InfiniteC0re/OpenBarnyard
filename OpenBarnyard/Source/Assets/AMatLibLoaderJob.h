#pragma once
#include "ATRBLoaderJob.h"
#include "AMaterialLibrary.h"

class AMatLibLoaderJob :
	public ATRBLoaderJob
{
public:
	virtual void BeginJob() override;
	virtual TBOOL RunJob() override;
	virtual TBOOL CancelJob() override;

	void InitJob(const char* a_szFileName, Toshi::TTRB* a_pTRB, AMaterialLibrary*& a_rpOutMatLib, Toshi::TMemoryHeap* a_pMemoryHeap);

private:
	Toshi::TString8 m_FileName;
	Toshi::TTRB* m_pTRB = TNULL;
	AMaterialLibrary* m_pMatLib = TNULL;
	AMaterialLibrary** m_ppOutMatLib;
	Toshi::TMemoryHeap* m_pMemoryHeap = TNULL;
};
