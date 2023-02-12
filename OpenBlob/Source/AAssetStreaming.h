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
	virtual void Init() = 0;
	virtual void BeginJob() = 0;
	virtual bool RunJob() = 0;

	virtual bool CancelJob() { return false; };
};

class AAssetStreaming : public Toshi::TSingleton<AAssetStreaming>
{

	static constexpr int JOBCOUNT = 256;

public:

	AAssetStreaming()
	{
		// Create thread and run it
		m_FileStream.Create(0, Toshi::TThread::THREAD_PRIORITY_ABOVE_NORMAL, 0);
		TTODO("The whole function");
	}

	AMainThreadJob2* GetAvaiableJob()
	{
		return TNULL;
	}

	void RunJob()
	{
		TASSERT(!m_pCurrentJob->IsLinked());
		m_Jobs.PopFront()->BeginJob();
		TASSERT(!m_pCurrentJob->IsLinked());
	}

	bool HasActiveJobs() const
	{
		return m_Jobs.IsEmpty() || m_pCurrentJob != TNULL;
	}

	void AddMainThreadJob2(AMainThreadJob2* a_pJob)
	{
		TASSERT(m_pCurrentJob != a_pJob);
		m_Jobs.PushBack(a_pJob);
	}

	void Update();

	Toshi::T2DList<AMainThreadJob2> m_Jobs;
	AMainThreadJob2* m_pCurrentJob;
	Toshi::TFileStream m_FileStream; // 0x0C
	Toshi::T2DList<AMainThreadJob2> m_FreeList; // 0xDC
};