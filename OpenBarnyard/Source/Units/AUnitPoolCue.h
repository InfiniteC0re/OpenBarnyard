#pragma once
#include "AAnimatedUnit.h"

//-----------------------------------------------------------------------------
// Purpose: The cue of the Barn Pool minigame. While aiming it's placed behind
// the cue ball (m_vecCueBallPosition) pointing at m_vecTarget and pulled back
// by the power of the shot. Shoot starts the animation of the shot (pushes the
// cue forward, then fades it out).
// TClass 0x00783238 (version 1.0, size 0x10c, runtime creatable), vtable 0x00720798,
// secondary (AGoalCollisionUpdateInterface) vtable 0x00720788.
// NOTE: original names of the members and own methods are unknown.
//-----------------------------------------------------------------------------
class AUnitPoolCue : public AAnimatedUnit
{
public:
	TDECLARE_CLASS( AUnitPoolCue, AAnimatedUnit );

	enum State : TINT
	{
		STATE_AIMING,     // follows the aim and the power, fades in
		STATE_SHOOTING,   // moves forward along m_vecShotDirection (see Shoot)
		STATE_SHOT,       // stays for a moment
		STATE_FADING_OUT, // moves forward and fades out
		STATE_HIDDEN,     // faded out
	};

public:
	AUnitPoolCue();
	~AUnitPoolCue();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AUnitTransform* a_pTransform ) override; // +0x58
	virtual void  UpdateAI( TFLOAT a_fDeltaTime ) override;        // +0x68
	virtual TBOOL Think( TFLOAT a_fDeltaTime ) override;           // +0x6c

	//-----------------------------------------------------------------------------
	// AVisibleUnit
	//-----------------------------------------------------------------------------
	virtual void SetVisible( TBOOL a_bVisible ) override; // +0x90

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	// Points the cue from the cue ball (SetCueBallPosition) at a_rcTarget.
	void SetTarget( const Toshi::TVector4& a_rcTarget );

	// Position of the cue ball.
	void SetCueBallPosition( const Toshi::TVector4& a_rcPosition );

	// Position of the model of the cue.
	Toshi::TVector4 GetCuePosition();

	// Makes the power go up and down between 0.01 and m_fMaxPower.
	void UpdatePowerOscillation( TFLOAT a_fDeltaTime );

	// Lowers the power (and moves the cue back) by a_fDeltaTime, m_fPullBackSpeed is the change of the pull back over a_fSpeedDeltaTime.
	void UpdatePullBack( TFLOAT a_fDeltaTime, TFLOAT a_fSpeedDeltaTime );

	// Returns the power and goes back to aiming with no power.
	TFLOAT ResetPower();

	// The direction of the shot scaled by the power.
	Toshi::TVector4 GetShotVelocity();

	// Starts the animation of the shot.
	void Shoot();

	// $Barnyard: FUNCTION 004d7ee0
	void SetMaxPower( TFLOAT a_fMaxPower ) { m_fMaxPower = a_fMaxPower; }

	// $Barnyard: FUNCTION 004d7ef0
	TFLOAT GetMaxPower() { return m_fMaxPower; }

	// $Barnyard: FUNCTION 004d7f00
	TFLOAT GetPower() { return m_fPower; }

	// TTRUE while the cue is aiming or hidden (the shot animation is over)
	// $Barnyard: FUNCTION 004d1c10
	TBOOL IsReady() const { return m_eState == STATE_HIDDEN || m_eState == STATE_AIMING; }

public:
	Toshi::TVector4 m_vecTarget;          // 0x98, see SetTarget
	Toshi::TVector4 m_vecDirection;       // 0xa8, from the cue ball to m_vecTarget
	Toshi::TVector4 m_vecCueBallPosition; // 0xb8
	Toshi::TVector4 m_vecCuePosition;     // 0xc8, position of the model
	TFLOAT          m_fPower;             // 0xd8
	TFLOAT          m_fMaxPower;          // 0xdc
	TFLOAT          m_fCueLength;         // 0xe0, distance of the model from the cue ball
	TFLOAT          m_fPullBack;          // 0xe4, see UpdatePullBack
	TFLOAT          m_fPullBackSpeed;     // 0xe8, then the speed of the cue while shooting
	TFLOAT          m_fTimer;             // 0xec, fade in/out (opacity is half of it) and state timer
	TFLOAT          m_fUnkF0;             // 0xf0
	State           m_eState;             // 0xf4
	Toshi::TVector4 m_vecShotDirection;   // 0xf8, m_vecDirection when it was shot
	TBOOL           m_bPowerDecreasing;   // 0x108, see UpdatePowerOscillation
};

TSTATICASSERT( sizeof( AUnitPoolCue ) == 0x10c );
