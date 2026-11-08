#include "ToshiPCH.h"
#include "TFileStream.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006d7ef0
TFileStream::TFileStream()
{
}

// $Barnyard: FUNCTION 006d7f20
// $Barnyard: FUNCTION 006d7fb0
TFileStream::~TFileStream()
{
}

// $Barnyard: FUNCTION 006d7f30
void TFileStream::Main()
{
	TFileStreamJob* pJob;

	while ( TTRUE )
	{
		do
		{
			m_Jobs.Pop( pJob );

#ifdef BARNYARD_COMMUNITY_PATCH
			// A null job is pushed by Stop()
			if ( pJob == TNULL )
				return;
#endif

			pJob->Process();
			pJob->m_bIsProcessed = TTRUE;
		} while ( pJob->m_pSemaphore == TNULL );

		pJob->m_pSemaphore->Signal();
	}
}

// $Barnyard: FUNCTION 006d7f70
void TFileStream::AddStream( TFileStreamJob* job )
{
	TPROFILER_SCOPE();
	job->m_bIsProcessed = TFALSE;
	m_Jobs.Push( &job );
}

void TFileStream::Stop()
{
	if ( m_hThreadHnd == TNULL )
		return;

#ifdef TOSHI_SKU_WASM
	TFileStreamJob* pStopJob = TNULL;
	m_Jobs.Push( &pStopJob );
	return;
#else
	// TThread::Exit closes m_hThreadHnd on the thread itself, so wait on a copy of it
	HANDLE hThread = NULL;
	if ( !DuplicateHandle( GetCurrentProcess(), m_hThreadHnd, GetCurrentProcess(), &hThread, SYNCHRONIZE, FALSE, 0 ) )
		return;

	TFileStreamJob* pStopJob = TNULL;
	m_Jobs.Push( &pStopJob );

	DWORD uiResult = WaitForSingleObject( hThread, 10000 );
	TASSERT( uiResult == WAIT_OBJECT_0, "TFileStream::Stop: the thread didn't finish its jobs" );
	CloseHandle( hThread );
#endif
}

//-----------------------------------------------------------------------------
// TTRBStreamJob
//-----------------------------------------------------------------------------

const TCHAR* TTRBStreamJob::GetFileName() const
{
	return m_fileName;
}

TTRB* TTRBStreamJob::GetTRB() const
{
	return m_trb;
}

// $Barnyard: FUNCTION 006d7ec0
void TTRBStreamJob::Init( TTRB* trb, const TCHAR* fileName )
{
	m_trb = trb;
	T2String8::Copy( m_fileName, fileName, -1 );
}

// $Barnyard: FUNCTION 006d7ee0
void TTRBStreamJob::Process()
{
	TPROFILER_SCOPE();
	m_trb->Load( m_fileName );
}

// $Barnyard: FUNCTION 006d7ea0
TTRBStreamJob::TTRBStreamJob()
    : TFileStreamJob( TNULL )
{
	m_trb           = TNULL;
	m_fileName[ 0 ] = '\0';
}

TTRBStreamJob::~TTRBStreamJob()
{
}

TOSHI_NAMESPACE_END
