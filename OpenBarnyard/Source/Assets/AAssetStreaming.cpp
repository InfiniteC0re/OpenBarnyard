#include "pch.h"
#include "AAssetStreaming.h"
#include "ATRBLoaderJob.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

// $Barnyard: FUNCTION 006066e0
AAssetStreaming::AAssetStreaming()
{
	m_pCurrentJob = TNULL;
	m_Unk         = TNULL;
	m_bFlag       = TTRUE;
	m_FileStream.Create( 0, Toshi::TThread::THREAD_PRIORITY_NORMAL, 0 );
}

AAssetStreaming::~AAssetStreaming()
{
}

// $Barnyard: FUNCTION 00606a90
void AAssetStreaming::Update()
{
	if ( m_pCurrentJob == TNULL )
	{
		if ( !m_Jobs.IsEmpty() )
		{
			m_pCurrentJob = m_Jobs.PopFront();
			m_pCurrentJob->BeginJob();
		}
	}
	else if ( m_pCurrentJob->RunJob() )
	{
		m_pCurrentJob->m_bIsFinished = TTRUE;
		m_pCurrentJob                = TNULL;
	}
}

// $Barnyard: FUNCTION 00606b20
void AAssetStreaming::CancelAllJobs()
{
	for ( auto it = m_Jobs.Begin(); it != m_Jobs.End(); )
	{
		auto next = it->Next();

		if ( it->CancelJob() )
		{
			it->m_bIsFinished = TTRUE;
			it->Remove();
		}

		it = next;
	}
}

// $Barnyard: FUNCTION 006067d0
TBOOL AAssetStreaming::HasActiveJobs() const
{
	return !m_Jobs.IsEmpty() || m_pCurrentJob != TNULL;
}

// $Barnyard: FUNCTION 00606ae0
void AAssetStreaming::AddMainThreadJob( AMainThreadJob* a_pJob )
{
	TASSERT( m_pCurrentJob != a_pJob );
	a_pJob->m_bIsFinished = TFALSE;
	m_Jobs.PushBack( a_pJob );
}
