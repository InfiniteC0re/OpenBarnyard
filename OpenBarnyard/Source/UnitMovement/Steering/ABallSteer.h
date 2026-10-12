#pragma once
#include "ASteer.h"
#include "Sound/ASoundLegacy.h"

#include <Render/TCollisionCommon.h>

//-----------------------------------------------------------------------------
// Purpose: Steer of the Gopher Golf ball (unit type "golfball"). Runs a fixed
// step simulation (230 steps per second) with two states: in the air it falls
// with m_fGravity and bounces off the terrain (m_fBounce, m_fBounceFriction),
// on the ground it rolls down the slopes, slowed down by m_fRollingFriction.
// Only the terrain is collided with, the units are ignored.
// TClass 0x0070a110 (version 3.0, size 0x80, runtime creatable), vtable 0x00724d38.
// NOTE: original names of the members and own methods are unknown.
//-----------------------------------------------------------------------------
class ABallSteer : public ASteer
{
public:
	TDECLARE_CLASS( ABallSteer, ASteer );

	enum State : TINT
	{
		STATE_STOPPED,
		STATE_ROLLING, // on the ground (m_vecGroundNormal)
		STATE_FLYING,
	};

	// Time of one simulation step
	static constexpr TFLOAT STEP_TIME = 1.0f / 230.0f;

	// Ignored by every terrain collision of the ball
	static constexpr Toshi::TCollisionCommon::TOSHICGROUPFLAG IGNORE_GROUPS =
	    Toshi::TCollisionCommon::TOSHICGROUPFLAG_SUPPORT |
	    Toshi::TCollisionCommon::TOSHICGROUPFLAG_PLAYER |
	    Toshi::TCollisionCommon::TOSHICGROUPFLAG_LIQUID |
	    Toshi::TCollisionCommon::TOSHICGROUPFLAG_CAMERA;

public:
	ABallSteer();
	~ABallSteer();

	virtual TBOOL CreateFrom( ASteer* a_pOther, TINT a_iLOD ) OVERRIDE;                                                                       // +0x20
	virtual TBOOL Create( AUnit* a_pOwner, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, TINT a_iLOD ) OVERRIDE; // +0x24
	virtual TBOOL Update( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                     // +0x30
	virtual TBOOL SetPosition( const Toshi::TVector4& a_rcPosition ) OVERRIDE;                                                                // +0x84, also stops the ball
	virtual TBOOL SetPosition3( const Toshi::TVector3& a_rcPosition ) OVERRIDE;                                                               // +0x88, also stops the ball
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

	virtual void SetVelocity( const Toshi::TVector4& a_rcVelocity ) OVERRIDE; // +0x108, launches the ball (STATE_FLYING)

	State GetState() const { return m_eState; }
	TBOOL IsStopped() const { return m_eState == STATE_STOPPED; }

	TFLOAT GetBounce() const { return m_fBounce; }
	TFLOAT GetMinBounceRatio() const { return m_fMinBounceRatio; }
	TFLOAT GetRollingFriction() const { return m_fRollingFriction; }
	TFLOAT GetBounceFriction() const { return m_fBounceFriction; }
	TFLOAT GetGravity() const { return m_fGravity; }

	void SetBounce( TFLOAT a_fBounce ) { m_fBounce = a_fBounce; }
	void SetMinBounceRatio( TFLOAT a_fRatio ) { m_fMinBounceRatio = a_fRatio; }
	void SetRollingFriction( TFLOAT a_fFriction ) { m_fRollingFriction = a_fFriction; }
	void SetBounceFriction( TFLOAT a_fFriction ) { m_fBounceFriction = a_fFriction; }
	void SetGravity( TFLOAT a_fGravity ) { m_fGravity = a_fGravity; }
	void SetObstacleHitSound( ASoundId a_iSound ) { m_iObstacleHitSound = a_iSound; }
	void SetPlayHitSounds( TBOOL a_bPlay ) { m_bPlayHitSounds = a_bPlay; }
	void SetPushOutIteratively( TBOOL a_bIteratively ) { m_bPushOutIteratively = a_bIteratively; }

private:
	// Runs one simulation step of a_fDeltaTime
	TBOOL Step( TFLOAT a_fDeltaTime );

	// STATE_FLYING: moves the ball and bounces it off the terrain, changes to STATE_ROLLING once it stops bouncing
	TBOOL Fly( TFLOAT a_fDeltaTime );

	// STATE_ROLLING: rolls the ball along m_vecGroundNormal, changes to STATE_FLYING if it leaves the ground or hits something
	TBOOL Roll( TFLOAT a_fDeltaTime );

	// Pushes the ball out of the terrain
	void PushOutOfTerrain();

private:
	Toshi::TVector4 m_vecPosition;         // 0x18
	Toshi::TVector4 m_vecMoveDirection;    // 0x28, normalised velocity
	Toshi::TVector4 m_vecDirection;        // 0x38, facing
	Toshi::TVector4 m_vecGroundNormal;     // 0x48, normal of the ground it rolls on
	TFLOAT          m_fSpeed;              // 0x58
	TFLOAT          m_fBounce;             // 0x5c, part of the speed into the hit normal that bounces back
	TFLOAT          m_fMinBounceRatio;     // 0x60, stops bouncing once bounce speed / new speed is below this
	TFLOAT          m_fRollingFriction;    // 0x64, friction coefficient while rolling, can't stop if 0
	TFLOAT          m_fBounceFriction;     // 0x68, if above 0, the velocity along the hit surface is multiplied by it on every bounce
	TFLOAT          m_fGravity;            // 0x6c
	TFLOAT          m_fStepTime;           // 0x70, time not simulated yet
	State           m_eState;              // 0x74
	ASoundId        m_iObstacleHitSound;   // 0x78, played when bouncing off TOSHICGROUP_NOFRICTION surfaces
	TBOOL           m_bPlayedHitSound;     // 0x7c, one hit sound per Update
	TBOOL           m_bPlayHitSounds;      // 0x7d
	TBOOL           m_bPushOutIteratively; // 0x7e, see PushOutOfTerrain
};

TSTATICASSERT( sizeof( ABallSteer ) == 0x80 );
