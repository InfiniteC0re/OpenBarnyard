#pragma once
#include "AAnimController.h"

//-----------------------------------------------------------------------------
// Purpose: Simplest animation controller (the minigame characters, props...).
// Only places the model at the owner unit's steer every frame, the animations
// are played by the scripts.
//-----------------------------------------------------------------------------
class ABasicAnimController
    : public AAnimController
{
public:
	TDECLARE_CLASS( ABasicAnimController, AAnimController );

public:
	ABasicAnimController();
	~ABasicAnimController();

	//-----------------------------------------------------------------------------
	// AAnimController
	//-----------------------------------------------------------------------------
	virtual TBOOL OnCreate() OVERRIDE;                      // +0x0c
	virtual void  OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x10
};
