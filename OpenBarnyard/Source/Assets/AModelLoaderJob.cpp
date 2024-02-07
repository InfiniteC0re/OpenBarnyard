#include "pch.h"
#include "AModelLoaderJob.h"

#include <Render/TRenderInterface.h>

void AModelLoaderJob::BeginJob()
{
	m_pModel = Toshi::TRenderInterface::GetSingleton()->CreateModel(m_oStreamJob.GetFileName(), TFALSE);
	ATRBLoaderJob::BeginJob();
}

TBOOL AModelLoaderJob::RunJob()
{
	if (m_oStreamJob.IsProcessed())
	{
		m_pModel->LoadTRB(m_oStreamJob.GetTRB());
		m_pModelRef->SetModel(m_pModel);

		return TTRUE;
	}

	return TFALSE;
}

TBOOL AModelLoaderJob::CancelJob()
{
	if (m_bCanBeCanceled)
	{
		if (m_oStreamJob.GetTRB())
		{
			delete m_oStreamJob.GetTRB();
		}

		return TTRUE;
	}

	return TFALSE;
}

void AModelLoaderJob::InitJob(Toshi::TModelPtr* a_pModelRef, Toshi::TTRB* a_pTRB, const char* a_szFileName, TBOOL a_bCanBeCanceled)
{
	m_pModelRef = a_pModelRef;
	m_pModel = TNULL;
	ATRBLoaderJob::InitJob(a_pTRB, a_szFileName);
	m_bCanBeCanceled = a_bCanBeCanceled;
}
