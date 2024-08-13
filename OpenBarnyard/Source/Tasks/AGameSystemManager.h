#pragma once
#include <Toshi/TTask.h>

class AGameSystemManager : public Toshi::TTask
{
public:
	TDECLARE_CLASS( AGameSystemManager, Toshi::TTask );

public:
	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;
};
