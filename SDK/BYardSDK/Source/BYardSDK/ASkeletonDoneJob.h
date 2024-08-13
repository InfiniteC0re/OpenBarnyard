#pragma once
#include "ATRBLoaderJob.h"
#include "ATerrainVIS.h"

class ASkeletonDoneJob :
	public AMainThreadJob
{
public:
	virtual void  BeginJob() override;
	virtual TBOOL RunJob() override;

	void InitJob( ATerrainVIS* a_pVIS, ATRBLoaderJob* a_pTRBJob );

private:
	ATerrainVIS*   m_pVIS;
	ATRBLoaderJob* m_pTRBLoaderJob;
};
