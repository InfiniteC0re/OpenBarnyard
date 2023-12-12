#pragma once
#include "ATRBLoaderJob.h"
#include "ATerrainVISGroup.h"

class ACollisionDoneJob :
	public AMainThreadJob
{
public:
	virtual void BeginJob() override;
	virtual TBOOL RunJob() override;
	virtual TBOOL CancelJob() override;

	void InitJob(ATerrainVISGroup* a_pVISGroup, TBOOL a_bFlag);

private:
	ATerrainVISGroup* m_pVISGroup;
	TBOOL m_bFlag;
};
