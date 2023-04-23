#pragma once
#include <Toshi/File/TFileStream.h>
#include <Toshi/Utils/TSingleton.h>
#include <Toshi2/T2DList.h>

enum class JOBTYPE
{
	MODEL,
	TRB,
	KEYLIB
};

class AMainThreadJob2 : public Toshi::T2DList<AMainThreadJob2>::Node
{
public:

	virtual JOBTYPE GetJobType() = 0;
	virtual void Init(){};
	virtual void BeginJob() = 0;
	virtual bool RunJob() = 0;

	virtual bool CancelJob() { return false; };

	Toshi::TTRBStreamJob m_streamJob;
};

class AAssetStreaming : public Toshi::TSingleton<AAssetStreaming>
{

	static constexpr int JOB_COUNT = 256;

public:

	AAssetStreaming();

	AMainThreadJob2* GetAvaiableJob()
	{
		auto job = m_FreeList.PopBack();
		job->m_streamJob = Toshi::TTRBStreamJob();
		return job;
	}

	void RunJob()
	{
		TASSERT(!m_pCurrentJob->IsLinked());
		m_Jobs.PopFront()->BeginJob();
		TASSERT(!m_pCurrentJob->IsLinked());
	}

	bool HasActiveJobs() const
	{
		return !m_Jobs.IsEmpty() || m_pCurrentJob != TNULL;
	}

	void AddMainThreadJob2(AMainThreadJob2* a_pJob)
	{
		TASSERT(m_pCurrentJob != a_pJob);
		m_Jobs.PushBack(a_pJob);
	}

	AMainThreadJob2* CancelAllWaitungTerrainJobs()
	{
		TASSERT(!m_FreeList.IsEmpty(), "Job free list is empty!");
		auto job = m_FreeList.PopBack();
		TASSERT(!job->IsLinked());
		TASSERT(m_pCurrentJob != job);
		return job;
	}

	void Update();
	void ReleaseJob(AMainThreadJob2* a_pJob);

	Toshi::T2DList<AMainThreadJob2> m_Jobs;     // 0x0
	AMainThreadJob2* m_pCurrentJob;      // 0x8
	Toshi::TFileStream m_FileStream;            // 0x0C
	Toshi::T2DList<AMainThreadJob2> m_FreeList; // 0xDC
};