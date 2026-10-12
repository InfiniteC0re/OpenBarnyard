#pragma once
#include "ACharacterAnimController.h"

//-----------------------------------------------------------------------------
// Purpose: ACharacterAnimController of the characters that can ride on the
// head of another character (Pip). While attached to the c_head bone of the
// parent unit, plays the onhead animations matching the parent's movement.
// NOTE: retail derives it from ACharacterAnimController but registers
// AAnimController as the TClass parent.
//-----------------------------------------------------------------------------
class AAttachedAnimController
    : public ACharacterAnimController
{
public:
	TDECLARE_CLASS( AAttachedAnimController, AAnimController );

public:
	AAttachedAnimController();
	~AAttachedAnimController();

	//-----------------------------------------------------------------------------
	// AAnimController
	//-----------------------------------------------------------------------------
	virtual TBOOL OnCreate() OVERRIDE;                      // +0x0c
	virtual void  OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x10

	//-----------------------------------------------------------------------------
	// ACharacterAnimController
	//-----------------------------------------------------------------------------
	virtual TBOOL IsMovementAnimPlaying() OVERRIDE; // +0x1c

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Plays the onhead animation matching the movement of the character it sits on
	virtual void UpdateOnHeadAnimation( TFLOAT a_fDeltaTime ); // +0x24
};
