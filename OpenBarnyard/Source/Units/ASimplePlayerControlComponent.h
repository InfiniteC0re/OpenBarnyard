#pragma once
#include "AControlComponent.h"

#include <Math/TVector2.h>
#include <Math/TVector4.h>

// Drives the player (AUnitPlayer) from input: camera relative movement, sprinting (uses
// stamina of the main player profile), jumping and the projectile launchers.
//
// TClass 0x0078a338 (version 0.1, size 0x3c, runtime creatable), vtable 0x00728c48.
class ASimplePlayerControlComponent : public AControlComponent
{
public:
	TDECLARE_CLASS( ASimplePlayerControlComponent, AControlComponent );

public:
	ASimplePlayerControlComponent();
	~ASimplePlayerControlComponent();

	//-----------------------------------------------------------------------------
	// AUnitComponent
	//-----------------------------------------------------------------------------
	virtual void UpdateAI( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x2c

	//-----------------------------------------------------------------------------
	// AControlComponent
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eCommand, const Toshi::TInputInterface::InputEvent* a_pEvent ) OVERRIDE; // +0x3c
	virtual void  OnDisable() OVERRIDE;                                                                                    // +0x40
	virtual void  OnEnable() OVERRIDE;                                                                                     // +0x44

	//-----------------------------------------------------------------------------
	// Own virtuals
	//-----------------------------------------------------------------------------
	// Forward direction of the current camera projected onto the ground plane
	virtual void GetCameraForward( Toshi::TVector4& a_rOutForward ); // +0x48

	// Sprinting is only possible while a_bCanSprint is set (stops sprinting otherwise)
	void SetCanSprint( TBOOL a_bCanSprint );

	// Stops the player when disabled
	void  SetMovementEnabled( TBOOL a_bEnabled );
	TBOOL IsMovementEnabled() const { return m_bMovementEnabled; }

	void SetCanJump( TBOOL a_bCanJump ) { m_bCanJump = a_bCanJump; }

	// Sets the movement input (x: right, y: forward) while the movement is enabled
	// $Barnyard: FUNCTION 00508060
	void SetMovementInput( TFLOAT a_fX, TFLOAT a_fY )
	{
		if ( m_bMovementEnabled )
		{
			m_vMovement.x = a_fX;
			m_vMovement.y = a_fY;
		}
	}

	const Toshi::TVector2& GetMovementInput() const { return m_vMovement; }

	// Seconds without input
	TFLOAT GetIdleTime() const { return m_fIdleTime; }

	// Used by AQuestManager to lock the launchers during the quest sequences
	TBOOL GetUnk32() const { return m_bUnk32; }
	TBOOL GetSecondaryLauncherInput() const { return m_bSecondaryLauncherInput; }
	void  SetLauncherInput( TBOOL a_bValue ) { m_bLauncherInput = a_bValue; }
	void  SetUnk32( TBOOL a_bValue ) { m_bUnk32 = a_bValue; }
	void  SetSecondaryLauncherInput( TBOOL a_bValue ) { m_bSecondaryLauncherInput = a_bValue; }
	void  SetCameraInput( TBOOL a_bValue ) { m_bCameraInput = a_bValue; }

	// Set together by AUnitPlayer::SetControlFlags
	void SetLauncherFlags( TBOOL a_bValue )
	{
		m_bLauncherInput          = a_bValue;
		m_bUnk32                  = a_bValue;
		m_bSecondaryLauncherInput = a_bValue;
	}

private:
	// Moves the owner along m_vMovement (relative to the camera), stops it with
	// a_fStopSpeed once there's no movement input
	void UpdateMovement( TFLOAT a_fDeltaTime, TFLOAT a_fStopSpeed );

	// TTRUE if there's something in the way of a jump
	TBOOL IsJumpBlocked();

private:
	Toshi::TVector2 m_vMovement;               // 0x1c, x: right, y: forward, reset every update
	TFLOAT          m_afUnk24[ 2 ];            // 0x24, not initialised, unused
	TFLOAT          m_fIdleTime;               // 0x2c, seconds without input
	TBOOL           m_bSprinting;              // 0x30
	TBOOL           m_bLauncherInput;          // 0x31, passed to AProjectileLauncher::ProcessInput
	TBOOL           m_bUnk32;                  // 0x32
	TBOOL           m_bSecondaryLauncherInput; // 0x33, AInputCommand_SelectPrevious selects the secondary launcher
	TBOOL           m_bCanSprint;              // 0x34
	TBOOL           m_bCanJump;                // 0x35
	TBOOL           m_bMovementEnabled;        // 0x36
	TBOOL           m_bCameraInput;            // 0x37, the current camera helper gets the commands first
	TBOOL           m_bMoving;                 // 0x38, written as a dword
};

TSTATICASSERT( sizeof( ASimplePlayerControlComponent ) == 0x3c );
