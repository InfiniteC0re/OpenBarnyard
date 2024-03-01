#pragma once
#include "ATRBLoaderJob.h"
#include "Terrain/ATerrainSection.h"

class ASectionDoneJob :
	public AMainThreadJob
{
public:
	virtual void BeginJob() override;
	virtual TBOOL RunJob() override;
	virtual TBOOL CancelJob() override;

	void InitJob(ATerrainSection* a_pSection, ATerrainLODType a_eLODType);

private:
	ATerrainSection* m_pSection;
	TUINT m_eLODType;
	TBOOL m_bIsCanceled;
};
