#pragma once
#include "ATRBLoaderJob.h"

class ACollisionDoneJob :
	public AMainThreadJob
{
public:
	virtual void BeginJob() override;
	virtual TBOOL RunJob() override;
	virtual TBOOL CancelJob() override;
};
