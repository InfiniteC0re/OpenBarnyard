#pragma once
#include "ATRBLoaderJob.h"
#include <Toshi/Render/TModelRegistry.h>

class AModelLoaderJob :
	public ATRBLoaderJob
{
public:
	void BeginJob() override;
	TBOOL RunJob() override;
	TBOOL CancelJob() override;

	void InitJob(Toshi::TModelRef* a_pModelRef, Toshi::TTRB* a_pTRB, const char* a_szFileName, TBOOL a_bCanBeCanceled);

private:
	Toshi::TModelRef* m_pModelRef;
	Toshi::TModel* m_pModel;
	TBOOL m_bCanBeCanceled;
};
