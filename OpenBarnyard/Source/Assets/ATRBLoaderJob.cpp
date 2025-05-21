#include "pch.h"
#include "ATRBLoaderJob.h"
#include "AAssetStreaming.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

// $Barnyard: FUNCTION 00606a20
void ATRBLoaderJob::BeginJob()
{
	AAssetStreaming::GetSingleton()->GetFileStream().AddStream(
	    &m_oStreamJob
	);
}

// $Barnyard: FUNCTION 006066c0
TBOOL ATRBLoaderJob::RunJob()
{
	return m_oStreamJob.IsProcessed() != TFALSE;
}

// $Barnyard: FUNCTION 006066b0
void ATRBLoaderJob::InitJob( Toshi::TTRB* a_pTRB, const TCHAR* a_szFileName )
{
	m_oStreamJob.Init( a_pTRB, a_szFileName );
}
