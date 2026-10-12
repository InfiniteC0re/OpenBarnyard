#pragma once
#include "ACameraHelper.h"

#include <Math/TMatrix44.h>

//-----------------------------------------------------------------------------
// Purpose: Chase camera of the bikes (ACameraManager::CAMERAHELPER_BIKE).
//
// TClass 0x0078234c (size 0xe0), vtable 0x0071cbd0.
//-----------------------------------------------------------------------------
class ABikePhysicsCameraHelper
    : public ACameraHelper
{
public:
	TDECLARE_CLASS( ABikePhysicsCameraHelper, ACameraHelper );

	// Damped spring pulling fValue towards fTarget (0x1c bytes)
	// NOTE: the name is invented, its methods are only used by this class
	struct Spring
	{
		// $Barnyard: FUNCTION 0065e710
		Spring( TFLOAT a_fValue, TFLOAT a_fStiffness, TFLOAT a_fDamping )
		    : fValue( a_fValue )
		    , fTarget( 0.0f )
		    , fVelocity( 0.0f )
		    , fStiffness( a_fStiffness )
		    , fDamping( a_fDamping )
		    , fMass( 1.0f )
		    , fInvMass( 1.0f )
		{}

		// Moves fValue towards fTarget
		void Update( TFLOAT a_fDeltaTime );

		// Moves the angle fValue towards the angle fTarget (both wrapped to -PI..PI)
		void UpdateAngle( TFLOAT a_fDeltaTime );

		void SetMass( TFLOAT a_fMass );

		// Sets the damping relative to the critical damping
		void SetDampingRatio( TFLOAT a_fRatio );

		// Sets the critical damping
		void SetCriticalDamping();

		TFLOAT fValue;     // 0x00
		TFLOAT fTarget;    // 0x04
		TFLOAT fVelocity;  // 0x08
		TFLOAT fStiffness; // 0x0c
		TFLOAT fDamping;   // 0x10
		TFLOAT fMass;      // 0x14
		TFLOAT fInvMass;   // 0x18
	};

	enum MODE : TINT
	{
		MODE_FOLLOW,    // the camera position is a spring towards the offset behind the bike
		MODE_CAR_CHASE, // the camera yaw follows the bike (ACarChaseMiniGameState)
		MODE_DEFAULT,   // keeps the distance and height to the bike, rotated by the input
		MODE_LOOK_BACK, // looks behind the bike (Trick1 held in the car chase)
	};

public:
	ABikePhysicsCameraHelper();
	~ABikePhysicsCameraHelper();

	//-----------------------------------------------------------------------------
	// ACameraHelper
	//-----------------------------------------------------------------------------
	virtual TBOOL  OnUpdate( TFLOAT a_fDeltaTime ) override;                         // +0x10
	virtual TBOOL  Unk2( void* ) override { return TFALSE; }                         // +0x14, RETURNS_FALSE 006ce000
	virtual TBOOL  ProcessInputCommand( void* a_pCommand, void* a_pEvent ) override; // +0x18
	virtual void   OnCreate() override;                                              // +0x1c
	virtual void   OnDestroy() override;                                             // +0x20
	virtual void   OnAttach() override;                                              // +0x24
	virtual void   ResetPosition( TBOOL a_bSnapCamera ) override;                    // +0x2c
	virtual void   SetTarget( Toshi::TRef<AUnitProxy> a_pTarget ) override;          // +0x30
	virtual void   SetTargetUnit( AUnit* a_pUnit ) override;                         // +0x34
	virtual void   OnDetach() override;                                              // +0x3c
	virtual TFLOAT GetFOV() const override;                                          // +0x40

	// Starts blending the FOV of the camera from GetFOV() to a_fTargetFOV in a_fDuration seconds
	void BlendFOV( TFLOAT a_fTargetFOV, TFLOAT a_fDuration );

	// Set the offsets (in vehicle space) and snap the distance/height springs (used by ACarChaseMiniGameState)
	void SetLookAtOffset( const Toshi::TVector4& a_rcOffset );
	void SetCameraOffset( const Toshi::TVector4& a_rcOffset );
	void SetPivotOffset( const Toshi::TVector4& a_rcOffset );

	const Toshi::TVector4& GetCameraOffset() const { return m_vCameraOffset; }

#ifdef BARNYARD_LATEST
	// Sets the mode and places the camera. In the car chase MODE_CAR_CHASE and MODE_DEFAULT become MODE_FOLLOW,
	// elsewhere MODE_FOLLOW and MODE_CAR_CHASE become MODE_DEFAULT.
	void SetMode( MODE a_eMode );
#endif

private:
	// Sets the targets of the distance and height springs from the look at and camera offsets
	void UpdateSpringTargets();

	// Snaps the distance and height springs to their targets
	void SnapSprings();

#ifdef BARNYARD_LATEST
	// Values of the springs that are updated at a fixed rate (m_vSpringPosition, distance and height)
	struct SpringState
	{
		Toshi::TVector4 vPosition;
		TFLOAT          fDistance;
		TFLOAT          fHeight;
	};

	void GetSpringState( SpringState& a_rOutState ) const;

	// Call before each fixed step (keeps the state before the last one)
	void BeforeSpringStep() { GetSpringState( m_oPrevSpringState ); }

	// Call after the fixed steps: keeps what the last step changed (m_oLastSpringStep) and returns how much of it
	// the camera that is drawn hasn't reached yet, from -1 to 0 (the time left in the accumulator / step - 1)
	TFLOAT EndSpringSteps( TINT a_iNumSteps );

	// Forgets the last step, for the cuts and the resets (the camera is never interpolated across them)
	void ResetSpringInterpolation();

	// Moves the camera from where it was before the collisions to where they pushed it, over m_fCollisionTime
	void SmoothCollisionPush( Toshi::TVector4& a_rCameraPos, const Toshi::TVector4& a_rcUnpushedPos, const Toshi::TVector4& a_rcLookAt, TFLOAT& a_rfPushSpeed, TFLOAT a_fDeltaTime );
#endif

	// Places the camera behind the bike
	void PlaceCamera();

	// Transform of the vehicle of the target (identity if it isn't a vehicle)
	void GetVehicleTransform( Toshi::TMatrix44& a_rOutTransform );

	// Moves a_rCameraPos out of the ground and the obstacles between it and a_rcPivot.
	// Returns TTRUE if it has been moved.
	TBOOL CheckCollision( Toshi::TVector4& a_rCameraPos, const Toshi::TVector4& a_rcPivot );

	// Widens the FOV of the camera with the speed of the vehicle (or blends it, see BlendFOV)
	void UpdateFOV( TFLOAT a_fDeltaTime );

	// Sets the near clip distance from the horizontal distance between the camera and the vehicle
	void UpdateNearClip( const Toshi::TVector4& a_rcVehiclePos, const Toshi::TVector4& a_rcCameraPos );

	// Fades the bikes of ABikeRaceMicroGame which are close to the camera
	static void FadeCloseRaceBikes( const Toshi::TVector4& a_rcCameraPos, const Toshi::TVector4& a_rcLookAt );

	// Sets the opacity of a bike and its rider
	static void SetBikeOpacity( AUnit* a_pUnit, TFLOAT a_fOpacity );

private:
	TFLOAT          m_fFOV;              // 0x10
	Toshi::TVector4 m_vSpringPosition;   // 0x14, camera position relative to the vehicle (MODE_FOLLOW)
	Toshi::TVector4 m_vSpringVelocity;   // 0x24
	TFLOAT          m_fTimeAccumulator;  // 0x34, the springs are updated at 30 Hz
	MODE            m_eMode;             // 0x38
	TINT            m_iCarChase;         // 0x3c, 1 while in ACarChaseMiniGameState
	Spring          m_oYawSpring;        // 0x40, MODE_CAR_CHASE
	Spring          m_oDistanceSpring;   // 0x5c, horizontal distance to the look at point
	Spring          m_oHeightSpring;     // 0x78, height above the look at point
	TFLOAT          m_fRotateInput;      // 0x94, MODE_DEFAULT
	Toshi::TVector4 m_vLookAtOffset;     // 0x98, in vehicle space
	Toshi::TVector4 m_vCameraOffset;     // 0xa8, in vehicle space
	Toshi::TVector4 m_vPivotOffset;      // 0xb8, in vehicle space, start of the collision sweep towards the camera
	TUINT           m_uiCollisionMask;   // 0xc8
	TBOOL           m_bBlendingFOV;      // 0xcc
	TFLOAT          m_fFOVBlendTime;     // 0xd0
	TFLOAT          m_fFOVBlendDuration; // 0xd4
	TFLOAT          m_fFOVBlendStart;    // 0xd8
	TFLOAT          m_fFOVBlendTarget;   // 0xdc
#ifdef BARNYARD_LATEST
	TFLOAT m_fCollisionTime; // Wii 0xf8, time the camera has been pushed by the collisions, smooths the push
	TBOOL  m_bCollided;      // Wii 0xfc

	// NOTE: not in the original. The springs are stepped at a fixed rate, the camera that is drawn interpolates
	// between the last two steps (render interpolation). The stepped values stay as they are.
	SpringState m_oPrevSpringState = { Toshi::TVector4::VEC_ZERO, 0.0f, 0.0f };
	SpringState m_oLastSpringStep  = { Toshi::TVector4::VEC_ZERO, 0.0f, 0.0f };
#endif
};

TSTATICASSERT( sizeof( ABikePhysicsCameraHelper::Spring ) == 0x1c );
#ifndef BARNYARD_LATEST
TSTATICASSERT( sizeof( ABikePhysicsCameraHelper ) == 0xe0 );
#endif
