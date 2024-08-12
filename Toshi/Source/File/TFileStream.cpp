#include "ToshiPCH.h"
#include "TFileStream.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

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

void TFileStream::AddStream( TFileStreamJob* job )
{
    job->m_bIsProcessed = TFALSE;
    m_Jobs.Push( &job );
}

TOSHI_NAMESPACE_END
