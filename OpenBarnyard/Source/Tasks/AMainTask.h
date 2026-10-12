#pragma once
#include <Toshi/TTask.h>

// NOTE: nothing creates it in the Windows build, only its TClass is left
class AMainTask : public Toshi::TTask
{
public:
	TDECLARE_CLASS( AMainTask, Toshi::TTask );

public:
	AMainTask();
	~AMainTask();

	virtual TBOOL OnCreate() override;                      // +0x18
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override; // +0x1C

private:
	TFLOAT m_fTotalTime; // 0x24
};
