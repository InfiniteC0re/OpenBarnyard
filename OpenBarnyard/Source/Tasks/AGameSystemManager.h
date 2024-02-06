#pragma once
#include <Toshi/Core/TTask.h>

class AGameSystemManager : public Toshi::TTask
{
public:
	TDECLARE_CLASS(Toshi::TTask);

public:
	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate(TFLOAT a_fDeltaTime) override;
};
