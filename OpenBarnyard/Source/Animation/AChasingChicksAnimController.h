#pragma once
#include "AAnimController.h"
#include "ANamedAnimationSet.h"

//-----------------------------------------------------------------------------
// Purpose: Animation controller of the chasing chicks minigame characters.
// Works like ACharacterAnimController (idle/walk/run loops), and also plays the
// same loops on the chick held by the character (cc_chick_held).
//-----------------------------------------------------------------------------
class AChasingChicksAnimController
    : public AAnimController
{
public:
	TDECLARE_CLASS( AChasingChicksAnimController, AAnimController );

public:
	AChasingChicksAnimController();
	~AChasingChicksAnimController();

	//-----------------------------------------------------------------------------
	// AAnimController
	//-----------------------------------------------------------------------------
	virtual TBOOL OnCreate() OVERRIDE;                                               // +0x0c
	virtual void  OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                          // +0x10
	virtual TBOOL Unknown3( void* ) OVERRIDE;                                        // +0x14
	virtual TBOOL IsAnimationAllowed( ANamedAnimation* a_pNamedAnimation ) OVERRIDE; // +0x18

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Places the model at the owner's steer
	void UpdateTransform( TFLOAT a_fDeltaTime );

	// Plays the movement animation matching the steer's movement type
	void UpdateMovementAnimation( TFLOAT a_fDeltaTime );

	// Sets the animation set called *a_pName (or the default one if a_pName is TNULL)
	void SetAnimationSet( const Toshi::TPString8* a_pName );

private:
	TBOOL                 m_bPlayingCustomAnim; // 0x1c
	ANamedAnimationSetRef m_pAnimationSet;      // 0x20
};
