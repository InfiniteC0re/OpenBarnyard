#pragma once
#include "ASteer.h"

//-----------------------------------------------------------------------------
// Purpose: Steer of the balls of the Barn Pool minigame (AUnitPoolBall). The
// ball rolls on the table (the y of the position stays at the height of
// m_vecTableCenter), slows down with m_fFriction and stops below m_fMinSpeed.
// It bounces off the cushions (terrain collision), the other pool balls (an
// elastic collision along the line between the centers) and falls into the
// pockets (m_pPockets), after which it's moved to m_vecPottedPosition.
// The state reads and clears the m_bHit* flags to play the sounds.
// TClass 0x00789354 (version 3.0, size 0x94, runtime creatable), vtable 0x00726728.
// NOTE: original names of the members and own methods are unknown.
//-----------------------------------------------------------------------------
class APoolBallSteer : public ASteer
{
public:
	TDECLARE_CLASS( APoolBallSteer, ASteer );

	enum State : TINT
	{
		STATE_STOPPED,        // not moving (SetPosition)
		STATE_MOVING,         // rolling on the table (Create, SetVelocity)
		STATE_FALLING_POCKET, // moving into the m_iPocket'th pocket
		STATE_POTTED,         // in m_vecPottedPosition
	};

	// Iterations of the collision loop in UpdateMoving
	static constexpr TINT MAX_NUM_COLLISIONS = 7;

public:
	APoolBallSteer();
	~APoolBallSteer();

	virtual AFunctionTable* GetFunctionTable() OVERRIDE;                                                                                                // +0x0c
	virtual TBOOL           CreateFrom( ASteer* a_pOther, TINT a_iLOD ) OVERRIDE;                                                                       // +0x20
	virtual TBOOL           Create( AUnit* a_pOwner, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, TINT a_iLOD ) OVERRIDE; // +0x24
	virtual TBOOL           Update( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                     // +0x30
	virtual TBOOL           SetPosition( const Toshi::TVector4& a_rcPosition ) OVERRIDE;                                                                // +0x84
	virtual TBOOL           SetPosition3( const Toshi::TVector3& a_rcPosition ) OVERRIDE;                                                               // +0x88
	virtual void            SetDirection( const Toshi::TVector4& a_rcDirection ) OVERRIDE;                                                              // +0x8c

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

	virtual void   SetVelocity( const Toshi::TVector4& a_rcVelocity ) OVERRIDE; // +0x108
	virtual TFLOAT GetRadius() OVERRIDE;                                        // +0x160, the radius of the ball

	// Sets the pockets (a_iNumPockets centers in a_pPockets, the array is referenced, not copied).
	void SetPockets( TINT a_iNumPockets, const Toshi::TVector4* a_pPockets, TFLOAT a_fPocketRadius );

	// Where the ball goes once it has fallen into a pocket.
	void SetPottedPosition( const Toshi::TVector4& a_rcPosition );

	// The y of this is the height the ball rolls at.
	void SetTableCenter( const Toshi::TVector4& a_rcCenter );

	// $Barnyard: FUNCTION 0056d160
	void SetState( State a_eState ) { m_eState = a_eState; }

	// $Barnyard: FUNCTION 0056d170
	State GetState() const { return m_eState; }

	// NOTE: folded with the other getters of offset 0x8c (00414ee0, also in the AUnit vtable) in retail
	AUnit* GetFirstHitUnit() const { return m_pFirstHitUnit; }

	// $Barnyard: FUNCTION 0056d420
	void ResetFirstHitUnit() { m_pFirstHitUnit = TNULL; }

	// $Barnyard: FUNCTION 0056d430
	TBOOL HasHitBall() const { return m_bHitBall; }

	// $Barnyard: FUNCTION 0056d440
	void ResetHitBall() { m_bHitBall = TFALSE; }

	// $Barnyard: FUNCTION 0056d450
	TBOOL HasHitCushion() const { return m_bHitCushion; }

	// $Barnyard: FUNCTION 0056d460
	void ResetHitCushion() { m_bHitCushion = TFALSE; }

	// $Barnyard: FUNCTION 0056d470
	TBOOL HasHitPocket() const { return m_bHitPocket; }

	// $Barnyard: FUNCTION 0056d480
	void ResetHitPocket() { m_bHitPocket = TFALSE; }

	// TTRUE if the ball touches any other pool ball.
	TBOOL IsTouchingOtherBall();

private:
	// STATE_MOVING: rolls the ball and handles the collisions.
	void UpdateMoving( TFLOAT a_fDeltaTime );

	// STATE_FALLING_POCKET: moves the ball to the pocket, then to m_vecPottedPosition.
	TBOOL UpdateFallingPocket( TFLOAT a_fDeltaTime );

public:
	// Sweeps the sphere along a_rcDirection (a_rfDistance at most) against the pockets, the last one first.
	// Also used by the aim guide of ABarnYardPoolMiniGameState.
	// Shortens a_rfDistance and outputs the index of the pocket when one is hit.
	static TBOOL CollidePockets( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, TFLOAT& a_rfDistance, TINT& a_riPocket, TINT a_iNumPockets, TFLOAT a_fRadius, const Toshi::TVector4* a_pPockets, TFLOAT a_fPocketRadius );

	// Sweeps the sphere along a_rcDirection (a_rfDistance at most) against the moving pool balls (but a_pIgnoreUnit).
	// Shortens a_rfDistance and outputs the unit of the ball when one is hit.
	static TBOOL CollideBalls( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, TFLOAT& a_rfDistance, AUnit*& a_rpHitUnit, AUnit* a_pIgnoreUnit, TFLOAT a_fRadius );

private:
	TFLOAT                 m_fMinSpeed;         // 0x18, the ball stops below this speed
	TFLOAT                 m_fFriction;         // 0x1c, velocity change per second (relative)
	TFLOAT                 m_fRadius;           // 0x20
	TINT                   m_iUnk24;            // 0x24
	Toshi::TVector4        m_vecPosition;       // 0x28
	Toshi::TVector4        m_vecVelocity;       // 0x38
	Toshi::TVector4        m_vecDirection;      // 0x48
	Toshi::TVector4        m_vecPottedPosition; // 0x58
	Toshi::TVector4        m_vecTableCenter;    // 0x68
	State                  m_eState;            // 0x78
	const Toshi::TVector4* m_pPockets;          // 0x7c
	TINT                   m_iNumPockets;       // 0x80
	TINT                   m_iPocket;           // 0x84, the pocket the ball fell into
	TFLOAT                 m_fPocketRadius;     // 0x88
	AUnit*                 m_pFirstHitUnit;     // 0x8c, the first ball this one hit (see ResetFirstHitUnit)
	TBOOL                  m_bHitBall;          // 0x90
	TBOOL                  m_bHitCushion;       // 0x91
	TBOOL                  m_bHitPocket;        // 0x92

public:
	static AFunctionTable ms_oFunctionTable;
};

TSTATICASSERT( sizeof( APoolBallSteer ) == 0x94 );
