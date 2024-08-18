#include "pch.h"
#include "AModelLoaderJob.h"

#include <Render/TRenderInterface.h>

void AModelLoaderJob::BeginJob()
{
}

TBOOL AModelLoaderJob::RunJob()
{
	return TFALSE;
}

TBOOL AModelLoaderJob::CancelJob()
{
	return TFALSE;
}

void AModelLoaderJob::InitJob( Toshi::TModelPtr* a_pModelRef, Toshi::TTRB* a_pTRB, const char* a_szFileName, TBOOL a_bCanBeCanceled )
{
	m_pModelRef = a_pModelRef;
	m_pModel    = TNULL;
	ATRBLoaderJob::InitJob( a_pTRB, a_szFileName );
	m_bCanBeCanceled = a_bCanBeCanceled;
}
