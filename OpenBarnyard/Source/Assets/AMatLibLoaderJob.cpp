#include "pch.h"
#include "AMatLibLoaderJob.h"
#include "AMaterialLibraryManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00606970
void AMatLibLoaderJob::BeginJob()
{
	ATRBLoaderJob::InitJob( m_pTRB, m_FileName );
	ATRBLoaderJob::BeginJob();
}

// $Barnyard: FUNCTION 006069a0
TBOOL AMatLibLoaderJob::RunJob()
{
	TPROFILER_SCOPE();

	if ( m_oStreamJob.IsProcessed() )
	{
		TASSERT( TNULL != m_pMemBlock );
		TMemory::MemBlock* pOldMemBlock = g_pMemory->SetGlobalBlock( m_pMemBlock );

		m_pMatLib = AMaterialLibraryManager::List::GetSingleton()->CreateLibraryFromTRB( m_oStreamJob.GetTRB(), m_FileName );
		AMaterialLibraryManager::GetSingleton()->CreateTextures( m_pMatLib );

		if ( m_ppOutMatLib )
		{
			*m_ppOutMatLib = m_pMatLib;
		}

		g_pMemory->SetGlobalBlock( pOldMemBlock );
		return TTRUE;
	}

	return TFALSE;
}

TBOOL AMatLibLoaderJob::CancelJob()
{
	return TTRUE;
}

// $Barnyard: FUNCTION 00606660
void AMatLibLoaderJob::InitJob( const TCHAR* a_szFileName, Toshi::TTRB* a_pTRB, AMaterialLibrary*& a_rpOutMatLib, Toshi::TMemory::MemBlock* a_pMemBlock )
{
	m_FileName    = a_szFileName;
	m_pTRB        = a_pTRB;
	m_pMemBlock   = a_pMemBlock;
	m_ppOutMatLib = &a_rpOutMatLib;
}
