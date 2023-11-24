#pragma once

TOBJECT(AGameSystemManager, Toshi::TTask, TTRUE)
{
public:
	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate(TFLOAT a_fDeltaTime) override;
};
