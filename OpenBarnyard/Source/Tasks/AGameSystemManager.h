#pragma once
#include <Toshi/Core/TTask.h>

TOBJECT(AGameSystemManager, Toshi::TTask, TTRUE)
{
public:
	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate(TFLOAT a_fDeltaTime) override;
};
