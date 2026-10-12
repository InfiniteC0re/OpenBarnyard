#pragma once
#include "ACameraHelper.h"
#include "Units/AUnitHash.h"

#include <Math/TVector2.h>

class ABarnStairCamera;

//-----------------------------------------------------------------------------
// Purpose: The third person camera that follows the player around in the sim
// (ACameraManager::CAMERAHELPER_GTA and SPLITSCREEN_CAMERAHELPER_GTA_1..4).
// It orbits the target at m_fDistance, is rotated with the camera commands/mouse,
// pushes itself in front of walls, fades the target out when too close and fades
// out characters that stand between the camera and the target.
//
// TClass 0x007824b0 (size 0xe0), vtable 0x0071ced8.
//-----------------------------------------------------------------------------
class AGTAStyleCameraHelper
    : public ACameraHelper
{
public:
	TDECLARE_CLASS( AGTAStyleCameraHelper, ACameraHelper );

public:
	AGTAStyleCameraHelper();
	~AGTAStyleCameraHelper();

	//-----------------------------------------------------------------------------
	// ACameraHelper
	//-----------------------------------------------------------------------------
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;                         // +0x10
	virtual TBOOL Unk2( void* ) override { return TFALSE; }                         // +0x14, RETURNS_FALSE 006ce000
	virtual TBOOL ProcessInputCommand( void* a_pCommand, void* a_pEvent ) override; // +0x18, processes the camera input commands
	virtual void  OnCreate() override;                                              // +0x1c
	virtual void  OnDestroy() override;                                             // +0x20
	virtual void  OnAttach() override;                                              // +0x24
	virtual void  ResetPosition( TBOOL a_bSnapCamera ) override;                    // +0x2c
	virtual void  SetTarget( Toshi::TRef<AUnitProxy> a_pTarget ) override;          // +0x30
	virtual void  ResetTarget() override;                                           // +0x38
	virtual void  OnDetach() override;                                              // +0x3c

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Places the camera at a_rcPosition looking at a_rcTarget and resets the orbit around it.
	// Unless a_bSnapCamera is set, the camera itself keeps its current position, direction and FOV.
	void SetPosition( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcTarget, TBOOL a_bSnapCamera );

	// Enables or disables the spline path (see ABarnStairCamera)
	void SetUsePath( TBOOL a_bUsePath );

	// Pauses the spline path updates
	void SetPathPaused( TBOOL a_bPaused );

	// Makes the next update recompute the pitch (a_bForce makes it ignore the collision checks)
	void SetForceUpdate( TBOOL a_bForce );

	// Limits the yaw of the camera around the target (radians)
	void SetYawLimits( TBOOL a_bEnabled, TFLOAT a_fMin, TFLOAT a_fMax );

	// Adds/removes a bit of the collision mask used for the camera collision checks
	TBOOL AddCollisionFlag( TINT a_iBit );
	TBOOL RemoveCollisionFlag( TINT a_iBit );

	void SetTargetPositionOverride( const Toshi::TVector4& a_rcPosition ) { m_vTargetPosOverride = a_rcPosition; }

	// $Barnyard: FUNCTION 0048bf10
	void                   SetTargetPositionOverride2( const Toshi::TVector4& a_rcPosition ) { m_vTargetPosOverride = a_rcPosition; }
	const Toshi::TVector4& GetTargetPositionOverride() const { return m_vTargetPosOverride; }

	// Moves the camera (without the smoothing of SetPosition)
	void SetCameraPosition( const Toshi::TVector4& a_rcPosition ) { m_vPosition = a_rcPosition; }
	void SetTargetDirectionOverride( const Toshi::TVector4& a_rcDirection ) { m_vTargetDirOverride = a_rcDirection; }

	void  SetMouseInput( TBOOL a_bEnabled ) { m_bMouseInput = a_bEnabled; }
	TBOOL IsMouseInputEnabled() const { return m_bMouseInput; }

	TFLOAT GetIdleTime() const { return m_fIdleTime; }

	const Toshi::TVector4& GetPosition() const { return m_vPosition; }
	const Toshi::TVector4& GetPivot() const { return m_vPivot; }

	TFLOAT GetDistance() const { return m_fDistance; }
	TFLOAT GetDefaultDistance() const { return m_fDefaultDistance; }
	void   SetDefaultDistance( TFLOAT a_fDistance ) { m_fDefaultDistance = a_fDistance; }

	TFLOAT GetPitch() const { return m_fPitch; }
	void   SetPitch( TFLOAT a_fPitch ) { m_fPitch = a_fPitch; }
	TFLOAT GetYaw() const { return m_fYaw; }
	void   SetYaw( TFLOAT a_fYaw ) { m_fYaw = a_fYaw; }
	void   SetTargetPitch( TFLOAT a_fPitch ) { m_fTargetPitch = a_fPitch; }
	TFLOAT GetDefaultPitch() const { return m_fDefaultPitch; }
	void   SetDefaultPitch( TFLOAT a_fPitch ) { m_fDefaultPitch = a_fPitch; }

	TFLOAT GetHeightOffset() const { return m_fHeightOffset; }
	void   SetHeightOffset( TFLOAT a_fHeightOffset ) { m_fHeightOffset = a_fHeightOffset; }

	TUINT GetCollisionMask() const { return m_uiCollisionMask; }

	void SetDefaultMaxPitch( TFLOAT a_fPitch ) { m_fDefaultMaxPitch = a_fPitch; }
	void SetMaxPitch( TFLOAT a_fPitch ) { m_fMaxPitch = a_fPitch; }
	void SetCollisionRadius( TFLOAT a_fRadius ) { m_fCollisionRadius = a_fRadius; }
	void SetSmallCollisionRadius( TFLOAT a_fRadius ) { m_fSmallCollisionRadius = a_fRadius; }

	void SetMaxPitch( TFLOAT a_fDefaultMax, TFLOAT a_fMax )
	{
		m_fDefaultMaxPitch = a_fDefaultMax;
		m_fMaxPitch        = a_fMax;
	}
	void SetCollisionRadius( TFLOAT a_fRadius, TFLOAT a_fSmallRadius )
	{
		m_fCollisionRadius      = a_fRadius;
		m_fSmallCollisionRadius = a_fSmallRadius;
	}

	ABarnStairCamera* GetPath() const { return m_pPath; }

#ifdef BARNYARD_LATEST
	// Keeps the camera height for a moment (while the target stays on screen), used when the player jumps
	// $BarnyardWiiJP: FUNCTION 80180710
	void HoldHeight() { m_fHoldHeightTime = 0.75f; }

	// $BarnyardWiiJP: FUNCTION 8018071c
	void ReleaseHeight() { m_fHoldHeightTime = -1.0f; }
#endif

private:
	// Returns TTRUE if a character (other than a_pIgnoreUnit) of the query result is closer than
	// a_fRadius to the segment a_rcFrom-a_rcTo (on the XZ plane). Characters too close to a_rcCameraPos are faded out.
	static TBOOL IsCharacterInTheWay( AUnitHash::QueryResult* a_pResults, TUINT a_uiNumResults, const Toshi::TVector4& a_rcFrom, const Toshi::TVector4& a_rcTo, const Toshi::TVector4& a_rcCameraPos, AUnit* a_pIgnoreUnit, TFLOAT a_fRadius );

private:
	TUINT           m_uiCollisionMask;    // 0x10
	Toshi::TVector2 m_vRotateInput;       // 0x14, x - yaw, y - pitch
	TFLOAT          m_fYawSpeed;          // 0x1c
	TBOOL           m_bRotating;          // 0x20
	TBOOL           m_bPitchInput;        // 0x21
	Toshi::TVector4 m_vLastTargetPos;     // 0x24
	Toshi::TVector4 m_vPosition;          // 0x34
	Toshi::TVector4 m_vPivot;             // 0x44, the target's position raised by m_fHeightOffset
	Toshi::TVector4 m_vLookAt;            // 0x54, smoothed m_vPivot the camera looks at
	Toshi::TVector4 m_vTargetPosOverride; // 0x64, used instead of the target's position unless it's VEC_ZERO
	Toshi::TVector4 m_vTargetDirOverride; // 0x74, used instead of the target's direction unless it's VEC_ZERO
	TFLOAT          m_fDistance;          // 0x84
	TFLOAT          m_fTargetDistance;    // 0x88
	TFLOAT          m_fDefaultDistance;   // 0x8c
	TFLOAT          m_fDistanceSpeed;     // 0x90
	TFLOAT          m_fHeightOffset;      // 0x94
	TFLOAT          m_fPitch;             // 0x98, degrees
	TFLOAT          m_fYaw;               // 0x9c, degrees, rotation to apply this frame
	TFLOAT          m_fTargetPitch;       // 0xa0
	TFLOAT          m_fDefaultPitch;      // 0xa4
	TFLOAT          m_fDefaultMaxPitch;   // 0xa8
	TFLOAT          m_fMaxPitch;          // 0xac
	TFLOAT          m_fMinPitch;          // 0xb0
	TFLOAT          m_fTargetOpacity;     // 0xb4
#ifdef BARNYARD_LATEST
	TFLOAT m_fCameraClearTime; // Wii 0xc4, time the camera hasn't been inside anything (more collision types are checked after 1.5s)
#endif
	TBOOL             m_bUsePath;              // 0xb8
	TBOOL             m_bPathPaused;           // 0xb9
	ABarnStairCamera* m_pPath;                 // 0xbc
	TFLOAT            m_fCollisionRadius;      // 0xc0
	TFLOAT            m_fSmallCollisionRadius; // 0xc4
	TBOOL             m_bForceUpdate;          // 0xc8
	TFLOAT            m_fIdleTime;             // 0xcc, time since the last camera input
#ifdef BARNYARD_LATEST
	TFLOAT m_fHoldHeightTime; // Wii 0xe0, see HoldHeight
#endif
	TBOOL  m_bYawLimits;         // 0xd0
	TFLOAT m_fYawMin;            // 0xd4
	TFLOAT m_fYawMax;            // 0xd8
	TBOOL  m_bCharacterInTheWay; // 0xdc
	TBOOL  m_bMouseInput;        // 0xdd
};

#ifndef BARNYARD_LATEST
TSTATICASSERT( sizeof( AGTAStyleCameraHelper ) == 0xe0 );
#endif
