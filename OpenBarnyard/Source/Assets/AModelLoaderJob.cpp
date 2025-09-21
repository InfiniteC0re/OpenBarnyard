#include "pch.h"
#include "AModelLoaderJob.h"

#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

// $Barnyard: FUNCTION 00606850
void AModelLoaderJob::BeginJob()
{
	m_pModel = Toshi::TRenderInterface::GetSingleton()->CreateModelTMDFile( m_oStreamJob.GetFileName(), TFALSE );
	ATRBLoaderJob::BeginJob();
}

// $Barnyard: FUNCTION 00606630
TBOOL AModelLoaderJob::RunJob()
{
	if ( m_oStreamJob.IsProcessed() )
	{
		m_pModel->LoadTRB( m_oStreamJob.GetTRB() );
		m_pModelRef->SetModel( m_pModel );

		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00606880
TBOOL AModelLoaderJob::CancelJob()
{
	if ( m_bCanBeCanceled )
	{
		if ( m_oStreamJob.GetTRB() )
		{
			delete m_oStreamJob.GetTRB();
		}

		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00606600
void AModelLoaderJob::InitJob( Toshi::TManagedModel* a_pModelRef, Toshi::TTRB* a_pTRB, const TCHAR* a_szFileName, TBOOL a_bCanBeCanceled )
{
	m_pModelRef = a_pModelRef;
	m_pModel    = TNULL;
	ATRBLoaderJob::InitJob( a_pTRB, a_szFileName );
	m_bCanBeCanceled = a_bCanBeCanceled;
}
