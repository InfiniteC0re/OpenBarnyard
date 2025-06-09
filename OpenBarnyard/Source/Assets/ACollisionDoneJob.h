#pragma once
#include "ATRBLoaderJob.h"
#include "Terrain/ATerrainSection.h"

class ACollisionDoneJob : public AMainThreadJob
{
public:
	virtual void  BeginJob() override;
	virtual TBOOL RunJob() override;
	virtual TBOOL CancelJob() override;

	void InitJob( ATerrainSection* a_pVISGroup, TBOOL a_bFlag );

private:
	ATerrainSection* m_pVISGroup;
	TBOOL            m_bNoModelInstance;
};
