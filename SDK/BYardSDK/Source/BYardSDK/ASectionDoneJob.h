#pragma once
#include "ATRBLoaderJob.h"
#include "ATerrainVISGroup.h"

class ASectionDoneJob :
	public AMainThreadJob
{
public:
	virtual void BeginJob() override;
	virtual TBOOL RunJob() override;
	virtual TBOOL CancelJob() override;

	void InitJob(ATerrainVISGroup* a_pVISGroup, ATerrainLODType a_eLODType);

private:
	ATerrainVISGroup* m_pVISGroup;
	TUINT m_eLODType;
	TBOOL m_bIsCanceled;
};
