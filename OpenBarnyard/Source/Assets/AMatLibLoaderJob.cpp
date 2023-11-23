#include "pch.h"
#include "AMatLibLoaderJob.h"
#include "AMaterialLibraryManager.h"

void AMatLibLoaderJob::BeginJob()
{
	ATRBLoaderJob::InitJob(m_pTRB, m_FileName);
	ATRBLoaderJob::BeginJob();
}

TBOOL AMatLibLoaderJob::RunJob()
{
	if (m_oStreamJob.IsProcessed())
	{
		TASSERT(TNULL != m_pMemoryHeap);
		Toshi::TMemoryHeapSwap heapSwap(m_pMemoryHeap);

		m_pMatLib = AMaterialLibraryManager::List::GetSingleton()->CreateLibraryFromTRB(m_oStreamJob.GetTRB(), m_FileName);
		AMaterialLibraryManager::GetSingleton()->CreateTextures(m_pMatLib);

		if (m_ppOutMatLib)
		{
			*m_ppOutMatLib = m_pMatLib;
		}
		
		return TTRUE;
	}

	return TFALSE;
}

TBOOL AMatLibLoaderJob::CancelJob()
{
	return TTRUE;
}

void AMatLibLoaderJob::InitJob(const char* a_szFileName, Toshi::TTRB* a_pTRB, AMaterialLibrary*& a_rpOutMatLib, Toshi::TMemoryHeap* a_pMemoryHeap)
{
	m_FileName = a_szFileName;
	m_pTRB = a_pTRB;
	m_pMemoryHeap = a_pMemoryHeap;
	m_ppOutMatLib = &a_rpOutMatLib;
}
