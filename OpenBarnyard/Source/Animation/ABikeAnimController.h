#pragma once
#include "AAnimController.h"

#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Purpose: Animation controller of the characters riding a bike (ABikeSteer).
// Places the rider on the bike, plays the riding, trick and jump animations
// and drives the animations of the bike attached to body_loc (wheels, pedals,
// steering, flapping).
//-----------------------------------------------------------------------------
class ABikeAnimController
    : public AAnimController
{
public:
	TDECLARE_CLASS( ABikeAnimController, AAnimController );

public:
	ABikeAnimController();
	~ABikeAnimController();

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

	// Returns TTRUE if one of the riding animations the controller handles itself is playing
	TBOOL IsMovementAnimPlaying();

	// Places the rider on the bike
	void UpdateTransform( TFLOAT a_fDeltaTime );

	// Animates the wheels, pedals, steering and flapping of the bike attached to body_loc
	void UpdateBikeAnimations( TFLOAT a_fDeltaTime );

	// Plays the flapping animations of the rider while coasting
	void UpdateFlappingAnimation( TFLOAT a_fDeltaTime );

	// Plays the riding, trick, jump and crash animations of the rider
	void UpdateRiderAnimation( TFLOAT a_fDeltaTime );

private:
	Toshi::TVector4 m_vecLastPosition;      // 0x1c, position of the steer last frame
	TFLOAT          m_fStoppedTime;         // 0x2c
	TBOOL           m_bPlayingCustomAnim;   // 0x30
	TBOOL           m_bRestoreFastProgress; // 0x31, start bk_fast at m_fSlowProgress
	TFLOAT          m_fSlowProgress;        // 0x34, progress of bk_slow when switching to bk_fast
	TFLOAT          m_fWheelTurn;           // 0x38, smoothed steering of the front wheel
};
