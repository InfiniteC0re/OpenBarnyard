#include "ToshiPCH.h"
#include "TFileStream.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $Barnyard FUNCTION 006d7ef0
TFileStream::TFileStream()
{
}

// $Barnyard FUNCTION 006d7fb0
TFileStream::~TFileStream()
{
}

// $Barnyard FUNCTION 006d7f30
void TFileStream::Main()
{
	TFileStreamJob* pJob;

	while ( TTRUE )
	{
		do
		{
			m_Jobs.Pop( pJob );
			pJob->Process();
			pJob->m_bIsProcessed = TTRUE;
		} while ( pJob->m_pSemaphore == TNULL );

		pJob->m_pSemaphore->Signal();
	}
}

// $Barnyard FUNCTION 006d7f70
void TFileStream::AddStream( TFileStreamJob* job )
{
	job->m_bIsProcessed = TFALSE;
	m_Jobs.Push( &job );
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

// $Barnyard FUNCTION 006d7ec0
void TTRBStreamJob::Init( TTRB* trb, const TCHAR* fileName )
{
	m_trb = trb;
	T2String8::Copy( m_fileName, fileName, -1 );
}

// $Barnyard FUNCTION 006d7ee0
void TTRBStreamJob::Process()
{
	m_trb->Load( m_fileName );
}

// $Barnyard FUNCTION 006d7ea0
TTRBStreamJob::TTRBStreamJob() :
    TFileStreamJob( TNULL )
{
	m_trb           = TNULL;
	m_fileName[ 0 ] = '\0';
}

TTRBStreamJob::~TTRBStreamJob()
{
}

TOSHI_NAMESPACE_END
