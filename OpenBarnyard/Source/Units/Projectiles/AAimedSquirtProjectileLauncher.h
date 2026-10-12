#pragma once
#include "AMilkSquirtProjectileLauncher.h"
#include "GUI/ATargetReticle.h"
#include "Units/AUnitProxy.h"

#include <Toshi/TRefCounted.h>
#include <Math/TVector2.h>
#include <Math/TVector4.h>

class ACameraHelper;

// The player's aimed milk squirt: switches to an over the shoulder aim camera, moves an
// aim cursor (ATargetReticle) around the screen and can lock on to targets.
class AAimedSquirtProjectileLauncher : public AMilkSquirtProjectileLauncher
{
public:
	TDECLARE_CLASS( AAimedSquirtProjectileLauncher, AMilkSquirtProjectileLauncher );

public:
	AAimedSquirtProjectileLauncher();
	~AAimedSquirtProjectileLauncher();

	//-----------------------------------------------------------------------------
	// AProjectileLauncher
	//-----------------------------------------------------------------------------
	virtual void            Activate() OVERRIDE;                                                                                                                 // +0x0c, enters aim mode
	virtual void            Deactivate() OVERRIDE;                                                                                                               // +0x10, leaves aim mode
	virtual TBOOL           ProcessInput( AInputCommand a_eCommand, const Toshi::TInputInterface::InputEvent* a_pEvent, TBOOL a_bUnk1, TBOOL a_bUnk2 ) OVERRIDE; // +0x14, moves the aim cursor
	virtual TBOOL           Update( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                              // +0x18
	virtual Toshi::TVector4 GetLaunchPosition() OVERRIDE;                                                                                                        // +0x28, owner's cb_udder bone
	virtual Toshi::TVector4 GetLaunchDirection() OVERRIDE;                                                                                                       // +0x2c
	virtual TFLOAT          GetThrowSpeed( AUnit* a_pObject ) OVERRIDE;                                                                                          // +0x3c, m_fLaunchSpeed
	virtual Toshi::TVector4 GetThrowVelocity( AUnit* a_pObject ) OVERRIDE;                                                                                       // +0x40

	// $Barnyard: FUNCTION 006636c0
	ATargetReticle& GetReticle() { return m_oReticle; }

	void SetDisableInteractions( TBOOL a_bDisable ) { m_bDisableInteractions = a_bDisable; }

	// $Barnyard: FUNCTION 00663410
	void SetNPCControlled( TBOOL a_bValue )
	{
		m_bNPCControlled = a_bValue;
		if ( a_bValue ) m_oReticle.SetActive( TFALSE, TFALSE, 0.0f );
	}

	// $Barnyard: FUNCTION 006636d0
	void SetCursorInput( TBOOL a_bCursorInput ) { m_bCursorInput = a_bCursorInput; }

	// $Barnyard: FUNCTION 006636e0
	void SetMoveAimsCursor( TBOOL a_bMoveAimsCursor ) { m_bMoveAimsCursor = a_bMoveAimsCursor; }

	// $Barnyard: FUNCTION 00663440
	Toshi::TVector4 GetAimPoint() const { return m_vAimPoint; }

	// $Barnyard: FUNCTION 00663470
	Toshi::TVector4 GetTracePoint() const { return m_vTracePoint; }

	// The unit the launcher is locked on to (TNULL if it's gone)
	AUnit* GetLockedTarget();

	// The unit the trajectory of the milk hits
	AUnit* GetUnitUnderCursor() const;

	// Locks on to a_pUnit (TNULL unlocks)
	void LockOn( AUnit* a_pUnit );

	// Moves the aim cursor to the target the launcher is locked on to
	void UpdateLockedCursor();

	// Whether the milk can hit a_pUnit (nothing blocks the trajectory)
	TBOOL CanHit( AUnit* a_pUnit );

	// Fires (or throws) at a_rcTarget
	TBOOL FireAt( const Toshi::TVector4& a_rcTarget, TBOOL a_bThrow );

	// Fires (or throws) at a_rcTarget, launched at a_fAngle (radians) from the horizontal plane
	TBOOL FireAt( const Toshi::TVector4& a_rcTarget, TFLOAT a_fAngle, TBOOL a_bThrow );

	// The direction to launch at with a_fSpeed from a_rcFrom to hit a_rcTo
	static void GetBallisticDirection( const Toshi::TVector4& a_rcFrom, const Toshi::TVector4& a_rcTo, TFLOAT a_fSpeed, Toshi::TVector4& a_rOutDirection );

	// The velocity to launch at a_fAngle (radians) from a_rcFrom to hit a_rcTo
	static void GetBallisticVelocity( TFLOAT a_fAngle, const Toshi::TVector4& a_rcFrom, const Toshi::TVector4& a_rcTo, Toshi::TVector4& a_rOutVelocity );

	// Follows the trajectory from a_rPosition (moved to where it ends) in steps of a_fStep, up to a_fMaxDistance. Returns TTRUE if it hits something.
	// With a_bFindUnit, a_rpOutUnit is set to the visible unit that's hit.
	static TBOOL TraceTrajectory( AUnit* a_pIgnoreUnit, Toshi::TVector4& a_rPosition, Toshi::TVector4 a_vDirection, TFLOAT a_fSpeed, TFLOAT a_fMaxDistance, TFLOAT a_fStep, TBOOL a_bFindUnit, AUnit** a_ppOutUnit );

private:
	// Adds the aim input to the aim cursor (mouse or stick), breaking the target lock if it's big enough.
	void AddAimCursorInput( TBOOL& a_rHandled, TBOOL a_bMouse, TFLOAT a_fX, TFLOAT a_fY );

	// Aims the launch directions at a_rcTarget.
	void AimAt( const Toshi::TVector4& a_rcTarget );

	// Aims at where a_rTarget moving with a_rcVelocity will be when the milk gets there (a_rTarget is moved there).
	void AimAt( Toshi::TVector4& a_rTarget, const Toshi::TVector4& a_rcVelocity );

	// Turns the owner towards a_rcTarget.
	void FaceTarget( const Toshi::TVector4& a_rcTarget );

	// Keeps the aim cursor within the screen and sets the edge flags that turn the aim camera.
	void UpdateCursorEdges();

	// Moves the aim camera behind the owner, turned by the edge flags.
	// BARNYARD_LATEST with the Wii Style camera: orbits the owner, turned by m_vAimCameraTurn (the aim cursor or the look buttons).
	void UpdateAimCamera( TFLOAT a_fDeltaTime );

	// Detaches the current camera helper (kept in m_pPrevCameraHelper) and starts the aim camera.
	void EnterAimCamera();

	// Gives the camera back to m_pPrevCameraHelper.
	void LeaveAimCamera();

private:
	TFLOAT                  m_fScreenWidth;              // 0x100, AGUI2 root element width
	TFLOAT                  m_fScreenHeight;             // 0x104, AGUI2 root element height
	TFLOAT                  m_fTargetHalfHeight;         // 0x108, of m_oTarget without a pelvis bone
	TBOOL                   m_bCursorAtLeft;             // 0x10c, the aim cursor is at the edges of the screen (turns the aim camera)
	TBOOL                   m_bCursorAtRight;            // 0x10d
	TBOOL                   m_bCursorAtTop;              // 0x10e
	TBOOL                   m_bCursorAtBottom;           // 0x10f
	TBOOL                   m_bNPCControlled;            // 0x110, no aim camera / aim input, fires along m_vAimDirection
	TBOOL                   m_bLockedOnTarget;           // 0x111
	TBOOL                   m_bAimCameraActive;          // 0x112
	TBOOL                   m_bCursorInput;              // 0x113, ProcessInput only moves the cursor while set, it's centred otherwise
	TBOOL                   m_bInteractionsWereDisabled; // 0x114, owner's AInteractionComponent state before aiming
	TBOOL                   m_bMoveAimsCursor;           // 0x115, AInputCommand_Move also moves the cursor while set
	TINT                    m_iTargetBone;               // 0x118, c_pelvis or c_pip_pelvis of m_oTarget
	Toshi::TRef<AUnitProxy> m_oTarget;                   // 0x11c
	Toshi::TRef<AUnitProxy> m_oUnitUnderCursor;          // 0x120
	Toshi::TVector4         m_vPrevTargetPosition;       // 0x124, to get the velocity of m_oTarget
	TBOOL                   m_bDisableInteractions;      // 0x134, disables the owner's interactions while aiming
	Toshi::TVector2         m_vAimCursor;                // 0x138, AGUI2 space
	ATargetReticle          m_oReticle;                  // 0x140
	Toshi::TVector4         m_vAimDirection;             // 0x4a8, owner's direction when aiming started
	Toshi::TVector4         m_vLaunchDirection;          // 0x4b8
	Toshi::TVector4         m_vCameraDirection;          // 0x4c8
	Toshi::TVector4         m_vTracePoint;               // 0x4d8, where the trajectory of the milk ends
	Toshi::TVector4         m_vAimPoint;                 // 0x4e8, the point under the aim cursor
	TFLOAT                  m_fCursorEdgeX;              // 0x4f8, how far the aim cursor is in the edge of the screen (0-1)
	TFLOAT                  m_fCursorEdgeY;              // 0x4fc
	TINT                    m_iUnk500;                   // 0x500
	ACameraHelper*          m_pPrevCameraHelper;         // 0x504
#ifdef BARNYARD_LATEST
	// NOTE: the latest version (Wii) orbits the aim camera around the owner. These are the fields it adds
	// (Wii offsets in the comments), appended here so the PC layout stays the same.
	Toshi::TVector2 m_vAimCameraTurn;         // 0xcf0 (Wii), yaw and pitch speeds (-1 to 1, -2 to 2 with the look buttons)
	Toshi::TVector4 m_vAimCameraOffset;       // 0xd00 (Wii), wanted direction from the owner's head to the camera
	Toshi::TVector4 m_vAimCameraSmoothOffset; // 0xd10 (Wii), follows m_vAimCameraOffset
	TBOOL           m_bAimCameraLookInput;    // 0xd20 (Wii), m_vAimCameraTurn was set by the look buttons this frame

	// NOTE: not in the original. The Wii Style camera option when the aim mode started (EnterAimCamera)
	TBOOL m_bWiiAimCamera;

	// NOTE: not in the original. The PC input that stands in for the Wii Remote pointer when the aim cursor isn't used
	// (the main game): the right stick and the camera keys (m_vAimCameraPCTurn, in m_vAimCameraTurn units, summed
	// over the frame) and the mouse (m_vAimCameraMouseTurn, radians)
	Toshi::TVector2 m_vAimCameraPCTurn;
	Toshi::TVector2 m_vAimCameraMouseTurn;

	// NOTE: not in the original. The mouse set the pitch, it's held instead of going back to the default
	TBOOL m_bAimCameraMousePitch;
#endif
};

#ifndef BARNYARD_LATEST
TSTATICASSERT( sizeof( AAimedSquirtProjectileLauncher ) == 0x508 );
#endif
