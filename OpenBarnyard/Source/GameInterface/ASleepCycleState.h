#pragma once
#include "AGameState.h"

#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Purpose: The player sleeps until the night (or the next dawn when it's
// already night). Fades to black, moves the camera to the bed, fades back in
// and fast forwards the time (the scheduler runs 8 times faster) phase by
// phase, then fades out again, restores the camera, refills the stamina and
// the ammo and the player wakes up with a yawn.
// Pushed by ASleepCycleChooseState.
// TClass 00781b3c, vtable 0071b130, size 0xe8.
// NOTE: ProcessInput (+0x0c) only calls AGameState::ProcessInput, the same function
// (0043fc30) is used by ASleepCycleChooseState and AShakeTreeMicroGame. Member names are invented.
//-----------------------------------------------------------------------------
class ASleepCycleState
    : public AGameState
{
public:
	TDECLARE_CLASS( ASleepCycleState, AGameState );

	using STATE = TINT;
	enum STATE_ : STATE
	{
		STATE_NONE,
		STATE_START,     // starts the fade out
		STATE_FADE_OUT,  // fades to black, then moves the camera to the bed
		STATE_FADE_IN,   // fades back in
		STATE_SLEEPING,  // fast forwards the day phases
		STATE_WAKE_FADE, // fades to black, then restores the camera and wakes the player up
		STATE_WAKE_IN,   // fades back in
		STATE_YAWNING,   // waits for the yawn animation
		STATE_FINISHED,  // removes the state
	};

	// Seconds a day phase takes while sleeping (on the 8 times faster clock) and the length of the blend of the time of day effects
	static constexpr TFLOAT PHASE_DURATION = 24.0f; // flt_00781b9c
	static constexpr TFLOAT FADE_SPEED     = 1.0f;  // flt_00781ba0
	static constexpr TFLOAT TIME_SCALE     = 8.0f;  // debug delta time multiplier while sleeping

public:
	ASleepCycleState();
	~ASleepCycleState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE;                                                        // +0x0c
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x4c
	virtual void  OnInsertion() OVERRIDE;                                                                                                                  // +0x50
	virtual void  OnRemoval() OVERRIDE;                                                                                                                    // +0x54

private:
	// Remembers the current camera (m_vecCameraPos/m_vecCameraTarget)
	void SaveCamera();

	// Puts the cameras back behind the player (at the "camera_save" goal) and the player at "player_save"
	void RestoreCamera();

	// Refills the stamina of the player and the ammo of the active launcher
	static void RefillPlayer();

	// Freezes the bike closest to the player
	static void FreezeClosestBike();

	static void SetFadeAlpha( TFLOAT a_fAlpha );

private:
	STATE           m_eState;          // 0xa4
	TFLOAT          m_fFade;           // 0xa8, alpha of the black overlay
	TFLOAT          m_fPhaseTime;      // 0xac, time spent in the current day phase
	TFLOAT          m_fPhaseDuration;  // 0xb0
	TBOOL           m_bCameraSaved;    // 0xb4
	Toshi::TVector4 m_vecCameraPos;    // 0xb8
	Toshi::TVector4 m_vecCameraTarget; // 0xc8
	TFLOAT          m_fSleptTime;      // 0xd8
	TFLOAT          m_fSleepDuration;  // 0xdc
	TFLOAT          m_fStartTime;      // 0xe0, game time (0-1200) the player went to bed
	TFLOAT          m_fEndTime;        // 0xe4, game time the player wakes up
};
