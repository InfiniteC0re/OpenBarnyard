#pragma once
#include "ATRBLoaderJob.h"
#include "Terrain/ATerrainVIS.h"

class ASkeletonDoneJob :
	public AMainThreadJob
{
public:
	virtual void BeginJob() override;
	virtual TBOOL RunJob() override;

private:
	ATerrainVIS* m_pVIS;
	ATRBLoaderJob* m_pTRBLoaderJob;
};
