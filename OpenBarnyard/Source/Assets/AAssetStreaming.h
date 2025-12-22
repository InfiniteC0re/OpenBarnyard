#pragma once
#include "AMainThreadJob.h"

#include <File/TFileStream.h>

class AAssetStreaming : public Toshi::TSingleton<AAssetStreaming>
{
public:
	AAssetStreaming();
	~AAssetStreaming();

	void Update();
	void CancelAllJobs();

	TBOOL HasActiveJobs() const;

	void AddMainThreadJob( AMainThreadJob* a_pJob );

	Toshi::TFileStream& GetFileStream()
	{
		return m_FileStream;
	}

	void SetFlag( TBOOL a_bFlag )
	{
		m_bFlag = a_bFlag;
	}

private:
	Toshi::T2DList<AMainThreadJob> m_Jobs;        // 0x0

#ifdef BARNYARD_COMMUNITY_PATCH
	AMainThreadJob* m_apCurrentJobs[ 16 ];
	TINT            m_iNumStartedJobs;
#else // BARNYARD_COMMUNITY_PATCH
	m_pCurrentJob = TNULL;                        // 0x8
#endif // !BARNYARD_COMMUNITY_PATCH

	Toshi::TFileStream             m_FileStream;  // 0x0C
	void*                          m_Unk;         // 0xD8
	TBOOL                          m_bFlag;       // 0xDC
};
