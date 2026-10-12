#pragma once
#include "ASteer.h"

//-----------------------------------------------------------------------------
// Purpose: Steer of the gopher of the Gopher Golf minigame (AGolfGopherUnit).
// The golf state pushes it around with AddImpulse every frame. It slides along
// the terrain collision, keeps out of an obstacle circle (SetObstacle) and sticks
// to the ground. Underground it plays the "tracks" sound and the "inground_*"
// animations; on a TOSHICGROUP_TIMBER surface it comes out ("aboveground_surface",
// animation set "above") and dives back on TOSHICGROUP_GRASS ("aboveground_dive",
// animation set "default"). The shadow of the gopher is hidden underground.
// TClass "AGopherMoundSteer" (parent ASteer, size 0x80, runtime creatable),
// vtable 0x00725958.
// NOTE: original names of the members and own methods are unknown.
//-----------------------------------------------------------------------------
class AGopherMoundSteer : public ASteer
{
public:
	TDECLARE_CLASS( AGopherMoundSteer, ASteer );

	enum STATE : TINT
	{
		STATE_UNDERGROUND, // 0
		STATE_SURFACING,   // 1, playing "aboveground_surface"
		STATE_ABOVEGROUND, // 2, runs/idles with the "above" animation set
		STATE_DIVING,      // 3, playing "aboveground_dive"
	};

public:
	AGopherMoundSteer();
	~AGopherMoundSteer();

	virtual TBOOL CreateFrom( ASteer* a_pOther, TINT a_iLOD ) OVERRIDE;                                                                       // +0x20
	virtual TBOOL Create( AUnit* a_pOwner, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, TINT a_iLOD ) OVERRIDE; // +0x24
	virtual TBOOL Update( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                     // +0x30
	virtual TBOOL SetPosition( const Toshi::TVector4& a_rcPosition ) OVERRIDE;                                                                // +0x84
	virtual TBOOL SetPosition3( const Toshi::TVector3& a_rcPosition ) OVERRIDE;                                                               // +0x88
	virtual void  SetDirection( const Toshi::TVector4& a_rcDirection ) OVERRIDE;                                                              // +0x8c

	virtual void                   CopyPosition( Toshi::TVector4& a_rOutPosition ) OVERRIDE;    // +0xa4
	virtual void                   CopyPosition3( Toshi::TVector3& a_rOutPosition ) OVERRIDE;   // +0xa8
	virtual const Toshi::TVector4* GetPosition() OVERRIDE;                                      // +0xac
	virtual void                   CopyDirection( Toshi::TVector4& a_rOutDirection ) OVERRIDE;  // +0xb0
	virtual void                   CopyDirection3( Toshi::TVector3& a_rOutDirection ) OVERRIDE; // +0xb4
	virtual const Toshi::TVector4* GetDirection() OVERRIDE;                                     // +0xb8
	virtual void                   CopyVelocity( Toshi::TVector4& a_rOutVelocity ) OVERRIDE;    // +0xbc
	virtual Toshi::TVector4        GetVelocity() OVERRIDE;                                      // +0xc0
	virtual TFLOAT                 GetSpeed() OVERRIDE;                                         // +0xc4
	virtual TFLOAT                 GetMeasuredSpeed() OVERRIDE { return 0.0f; }                 // +0xd4, 0056ce20 (shared)

	virtual void SetVelocity( const Toshi::TVector4& a_rcVelocity ) OVERRIDE; // +0x108

	// Adds a_rcImpulse (xyz) to the impulse of this frame. Update moves the
	// gopher by 0.75 * impulse * deltatime and clears it.
	void AddImpulse( const Toshi::TVector4& a_rcImpulse );

	// The gopher keeps a_fRadius + 0.3 away from a_rcCentre (on the XZ plane).
	// a_fRadius is also the radius of the sphere swept against the terrain.
	void SetObstacle( const Toshi::TVector4& a_rcCentre, TFLOAT a_fRadius );

	// TTRUE while the gopher is under the ground (STATE_UNDERGROUND)
	TBOOL IsUnderground();

	STATE GetState() const { return m_eState; }

	// While set, the idle gopher doesn't peek out ("inground_surface") and dives back in
	void SetStayHidden( TBOOL a_bStayHidden ) { m_bStayHidden = a_bStayHidden; }

private:
	// Moves the gopher with the impulse and updates the animations and the state
	TBOOL UpdateMovement( TFLOAT a_fDeltaTime );

	// Plays "run" (speed a_fImpulse / 4) or "idle" above the ground
	void UpdateAboveGroundAnimation( TFLOAT a_fImpulse );

	// Moves a_rPosition towards a_rNewPosition, sliding along the terrain, keeps
	// a_rNewPosition out of the obstacle and puts it on the ground
	void MoveWithCollision( Toshi::TVector4& a_rPosition, Toshi::TVector4& a_rNewPosition, TFLOAT a_fDeltaTime );

	// Collision group of the terrain right above (or else below) a_rcPosition, -1 if none
	static TINT GetSurfaceGroup( const Toshi::TVector4& a_rcPosition );

	// Sets the animation set of the gopher model (doesn't kill the animations)
	void SetAnimationSet( const Toshi::TPString8& a_rcName );

private:
	Toshi::TVector4 m_vPosition;   // 0x18
	Toshi::TVector4 m_vVelocity;   // 0x28, only set by SetVelocity/CreateFrom
	Toshi::TVector4 m_vImpulse;    // 0x38, cleared every update
	Toshi::TVector4 m_vDirection;  // 0x48, the last impulse direction
	Toshi::TVector4 m_vObstacle;   // 0x58
	TFLOAT          m_fSpeed;      // 0x68
	TFLOAT          m_fRadius;     // 0x6c
	TFLOAT          m_fIdleTime;   // 0x70, the underground idle gopher peeks out after 1s
	TINT            m_iTracksCue;  // 0x74, GOLF_GOPHER_TRACKS while moving underground
	STATE           m_eState;      // 0x78
	TBOOL           m_bStayHidden; // 0x7c
};

TSTATICASSERT( sizeof( AGopherMoundSteer ) == 0x80 );
