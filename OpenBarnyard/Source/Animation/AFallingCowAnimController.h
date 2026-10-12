#pragma once
#include "AAnimController.h"

//-----------------------------------------------------------------------------
// Purpose: Animation controller of the characters crashing off a bike
// (AFallingCowSteer): plays the fall loop, the landing and the getting up.
// Created disabled, enabled by the code that throws the character off.
// NOTE: AFallingCowSteer isn't reconstructed yet, so OnUpdate can't be
// translated.
//-----------------------------------------------------------------------------
class AFallingCowAnimController
    : public AAnimController
{
public:
	TDECLARE_CLASS( AFallingCowAnimController, AAnimController );

	enum STATE : TINT
	{
		STATE_FALLING    = 1,
		STATE_LAND       = 2,
		STATE_LANDING    = 3,
		STATE_GETUP      = 4,
		STATE_GETTING_UP = 5,
		STATE_DONE       = 6,
	};

public:
	AFallingCowAnimController();
	~AFallingCowAnimController();

	//-----------------------------------------------------------------------------
	// AAnimController
	//-----------------------------------------------------------------------------
	virtual TBOOL OnCreate() OVERRIDE;                      // +0x0c
	virtual void  OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x10

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Places the model at the owner's steer
	void UpdateTransform( TFLOAT a_fDeltaTime );

	TINT GetState() const { return m_eState; }
	void SetDirection( TINT a_iDirection ) { m_iDirection = a_iDirection; }

private:
	TINT m_eState;     // 0x1c, STATE
	TINT m_iDirection; // 0x20, 1 to use the bk_crashoffbike_back animations
};
