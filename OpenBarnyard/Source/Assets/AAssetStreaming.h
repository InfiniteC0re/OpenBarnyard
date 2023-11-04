#pragma once
#include "AMainThreadJob2.h"

class AAssetStreaming :
	public Toshi::TSingleton<AAssetStreaming>
{
public:
	AAssetStreaming();

	void Update();

	TBOOL HasActiveJobs() const
	{
		return !m_Jobs.IsEmpty() || m_pCurrentJob != TNULL;
	}

	void AddMainThreadJob2(AMainThreadJob2* a_pJob)
	{
		TASSERT(m_pCurrentJob != a_pJob);
		a_pJob->m_bIsRunning = TFALSE;
		m_Jobs.PushFront(a_pJob);
	}

	Toshi::TFileStream& GetFileStream()
	{
		return m_FileStream;
	}

private:
	Toshi::T2DList<AMainThreadJob2> m_Jobs;     // 0x0
	AMainThreadJob2* m_pCurrentJob;             // 0x8
	Toshi::TFileStream m_FileStream;            // 0x0C
	void* m_Unk;                                // 0xD8
	TBOOL m_bFlag;                              // 0xDC
};