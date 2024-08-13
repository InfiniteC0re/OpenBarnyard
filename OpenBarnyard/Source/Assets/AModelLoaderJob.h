#pragma once
#include "ATRBLoaderJob.h"

#include <Render/TModelManager.h>

class AModelLoaderJob :
	public ATRBLoaderJob
{
public:
	virtual void  BeginJob() override;
	virtual TBOOL RunJob() override;
	virtual TBOOL CancelJob() override;

	void InitJob( Toshi::TModelPtr* a_pModelRef, Toshi::TTRB* a_pTRB, const TCHAR* a_szFileName, TBOOL a_bCanBeCanceled );

private:
	Toshi::TModelPtr* m_pModelRef = TNULL;
	Toshi::TModel*	  m_pModel	  = TNULL;
	TBOOL			  m_bCanBeCanceled;
};
