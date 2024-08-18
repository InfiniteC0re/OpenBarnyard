#pragma once
#include "ATRBLoaderJob.h"
#include "ATerrainSection.h"

class ASectionDoneJob :
    public AMainThreadJob
{
public:
	virtual void  BeginJob() override;
	virtual TBOOL RunJob() override;
	virtual TBOOL CancelJob() override;

	void InitJob( ATerrainSection* a_pVISGroup, ATerrainLODType a_eLODType );

private:
	ATerrainSection* m_pSection;
	TUINT            m_eLODType;
	TBOOL            m_bIsCanceled;
};
