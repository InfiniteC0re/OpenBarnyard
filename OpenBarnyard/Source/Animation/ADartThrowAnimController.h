#pragma once
#include "AAnimController.h"

//-----------------------------------------------------------------------------
// Purpose: Animation controller of the darts minigame players. Places the
// model at the steer and plays the raise/throw/idle animations requested by
// the minigame.
//-----------------------------------------------------------------------------
class ADartThrowAnimController
    : public AAnimController
{
public:
	TDECLARE_CLASS( ADartThrowAnimController, AAnimController );

public:
	ADartThrowAnimController();
	~ADartThrowAnimController();

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

	// Called by the darts minigame with a pair of values (aim?)
	void SetAim( TFLOAT a_fAimX, TFLOAT a_fAimY );

	// Plays darts_throw
	void Throw();

	// Plays darts_raise
	void Raise();

	void SetPlayIdle( TBOOL a_bPlayIdle ) { m_bPlayIdle = a_bPlayIdle; }

private:
	TBOOL  m_bPlayIdle; // 0x1c, play darts_idle when not throwing
	TBOOL  m_bRaised;   // 0x1d
	TFLOAT m_fAimX;     // 0x20
	TFLOAT m_fAimY;     // 0x24
};
