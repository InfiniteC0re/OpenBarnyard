#pragma once
#include "AMainThreadJob.h"
#include "THookedSingleton.h"

#include <File/TFileStream.h>

class AAssetStreaming :
	public THookedSingleton<AAssetStreaming, 0x0079b114>
{
public:
	void Update();
	void CancelAllJobs();

	TBOOL HasActiveJobs() const
	{
		return !m_Jobs.IsEmpty() || m_pCurrentJob != TNULL;
	}

	void AddMainThreadJob( AMainThreadJob* a_pJob )
	{
		TASSERT( m_pCurrentJob != a_pJob );
		a_pJob->m_bIsRunning = TFALSE;
		m_Jobs.PushBack( a_pJob );
	}

	Toshi::TFileStream& GetFileStream()
	{
		return m_FileStream;
	}

	void SetFlag( TBOOL a_bFlag )
	{
		m_bFlag = a_bFlag;
	}

private:
	Toshi::T2DList<AMainThreadJob> m_Jobs;		  // 0x0
	AMainThreadJob*				   m_pCurrentJob; // 0x8
	Toshi::TFileStream			   m_FileStream;  // 0x0C
	void*						   m_Unk;		  // 0xD8
	TBOOL						   m_bFlag;		  // 0xDC
};
