#pragma once
#include "AAnimController.h"

//-----------------------------------------------------------------------------
// Purpose: Animation controller of the bike models (bike1..bike4). Places the
// bike at the steer of its unit and animates the wheels, steering and flapping
// while it's ridden (ABikeSteer).
//-----------------------------------------------------------------------------
class ABikeCrashAnimController
    : public AAnimController
{
public:
	TDECLARE_CLASS( ABikeCrashAnimController, AAnimController );

public:
	ABikeCrashAnimController();
	~ABikeCrashAnimController();

	//-----------------------------------------------------------------------------
	// AAnimController
	//-----------------------------------------------------------------------------
	virtual TBOOL OnCreate() OVERRIDE;                      // +0x0c
	virtual void  OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x10

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Animates the wheels, steering and flapping of the bike
	void UpdateBikeAnimations( TFLOAT a_fDeltaTime );

	// Places the bike at the steer
	void UpdateTransform( TFLOAT a_fDeltaTime );

private:
	TFLOAT m_fWheelTurn; // 0x1c, smoothed steering of the front wheel
};
