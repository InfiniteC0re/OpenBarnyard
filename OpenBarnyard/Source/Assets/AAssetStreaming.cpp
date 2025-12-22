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
#ifdef BARNYARD_COMMUNITY_PATCH
	T2_FOREACH_ARRAY( m_apCurrentJobs, it )
		m_apCurrentJobs[ it ] = TNULL;

	m_iNumStartedJobs = 0;
#else // BARNYARD_COMMUNITY_PATCH
	m_pCurrentJob = TNULL;
#endif // !BARNYARD_COMMUNITY_PATCH

	m_Unk   = TNULL;
	m_bFlag = TTRUE;
	m_FileStream.Create( 0, Toshi::TThread::THREAD_PRIORITY_NORMAL, 0 );
}

AAssetStreaming::~AAssetStreaming()
{
}

// $Barnyard: FUNCTION 00606a90
void AAssetStreaming::Update()
{
	TPROFILER_SCOPE();

#ifdef BARNYARD_COMMUNITY_PATCH

	{
		TPROFILER_NAMED( "Assigning" );
		for ( TINT i = 0; i < TARRAYSIZE( m_apCurrentJobs ); i++ )
		{
			if ( !m_apCurrentJobs[ i ] )
			{
				if ( !m_Jobs.IsEmpty() )
				{
					AMainThreadJob* pJob = m_Jobs.PopFront();
					pJob->BeginJob();

					m_apCurrentJobs[ i ] = pJob;
					m_iNumStartedJobs++;
				}
			}
		}
	}

	{
		TPROFILER_NAMED( "Running" );

		for ( TINT i = 0; i < TARRAYSIZE( m_apCurrentJobs ); i++ )
		{
			AMainThreadJob* pJob = m_apCurrentJobs[ i ];

			if ( pJob )
			{
				if ( !pJob->RunJob() ) break;
				pJob->m_bIsFinished = TTRUE;
				m_apCurrentJobs[ i ] = TNULL;
				m_iNumStartedJobs--;
			}
		}
	}

#else // BARNYARD_COMMUNITY_PATCH

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

#endif // !BARNYARD_COMMUNITY_PATCH
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
#ifdef BARNYARD_COMMUNITY_PATCH
	return !m_Jobs.IsEmpty() || m_iNumStartedJobs > 0;
#else // BARNYARD_COMMUNITY_PATCH
	return !m_Jobs.IsEmpty() || m_pCurrentJob != TNULL;
#endif // !BARNYARD_COMMUNITY_PATCH
}

// $Barnyard: FUNCTION 00606ae0
void AAssetStreaming::AddMainThreadJob( AMainThreadJob* a_pJob )
{
#ifndef BARNYARD_COMMUNITY_PATCH
	TASSERT( m_pCurrentJob != a_pJob );
#endif // !BARNYARD_COMMUNITY_PATCH

	a_pJob->m_bIsFinished = TFALSE;
	m_Jobs.PushBack( a_pJob );
}
