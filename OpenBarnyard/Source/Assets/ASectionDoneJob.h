#pragma once
#include "ATRBLoaderJob.h"

class ASectionDoneJob :
	public AMainThreadJob
{
public:
	virtual void BeginJob() override;
	virtual TBOOL RunJob() override;
	virtual TBOOL CancelJob() override;
};
