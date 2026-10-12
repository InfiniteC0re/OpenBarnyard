#pragma once
#include "ASteer.h"
#include <Render/TCollisionCommon.h>

//-----------------------------------------------------------------------------
// Purpose: Ballistic steer of a thrown/fired item. Once launched (SetVelocity)
// it falls with gravity (m_fGravity) along its velocity until it hits the
// terrain or its lifetime runs out. It has no ASteerKinematics.
// NOTE: the position is at 0x1c and the direction at 0x3c. The direction
// getters are folded with ACarHeliSteer's position getters (its position is at
// 0x3c), the position getters with other getters of 0x1c.
// NOTE: vtable 0x007268b0, TClass parent ASteer, version 3.0, size 0xa0.
//-----------------------------------------------------------------------------
class AProjectileItemSteer : public ASteer
{
public:
	TDECLARE_CLASS( AProjectileItemSteer, ASteer );

	enum STATE : TINT
	{
		STATE_FLYING,
		STATE_LANDED,
		STATE_IDLE,
		STATE_TIMEDOUT,
	};

public:
	AProjectileItemSteer();
	~AProjectileItemSteer();

	//-----------------------------------------------------------------------------
	// ASteer
	//-----------------------------------------------------------------------------
	virtual TBOOL                  CreateFrom( ASteer* a_pOther, TINT a_iLOD ) OVERRIDE;                                                                       // +0x20
	virtual TBOOL                  Create( AUnit* a_pOwner, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, TINT a_iLOD ) OVERRIDE; // +0x24
	virtual TBOOL                  Update( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                     // +0x30
	virtual TBOOL                  SetPosition( const Toshi::TVector4& a_rcPosition ) OVERRIDE;                                                                // +0x84
	virtual TBOOL                  SetPosition3( const Toshi::TVector3& a_rcPosition ) OVERRIDE;                                                               // +0x88
	virtual void                   SetDirection( const Toshi::TVector4& a_rcDirection ) OVERRIDE;                                                              // +0x8c
	virtual void                   CopyPosition( Toshi::TVector4& a_rOutPosition ) OVERRIDE;                                                                   // +0xa4
	virtual void                   CopyPosition3( Toshi::TVector3& a_rOutPosition ) OVERRIDE;                                                                  // +0xa8
	virtual const Toshi::TVector4* GetPosition() OVERRIDE;                                                                                                     // +0xac
	virtual void                   CopyDirection( Toshi::TVector4& a_rOutDirection ) OVERRIDE;                                                                 // +0xb0
	virtual void                   CopyDirection3( Toshi::TVector3& a_rOutDirection ) OVERRIDE;                                                                // +0xb4
	virtual const Toshi::TVector4* GetDirection() OVERRIDE;                                                                                                    // +0xb8
	virtual void                   SetVelocity( const Toshi::TVector4& a_rcVelocity ) OVERRIDE;                                                                // +0x108, launches the item

	STATE                  GetState() const { return m_eState; }
	const Toshi::TVector4& GetHitNormal() const { return m_vecHitNormal; }

private:
	// Averages the normals of the terrain triangles around a_vecPosition that face a_vecDirection (VEC_NEGY if there are none)
	static Toshi::TVector4 GetLandingNormal( Toshi::TVector4 a_vecPosition, Toshi::TVector4 a_vecDirection );

private:
	STATE                                    m_eState;          // 0x18
	Toshi::TVector4                          m_vecPosition;     // 0x1c
	Toshi::TVector4                          m_vecVelocity;     // 0x2c
	Toshi::TVector4                          m_vecDirection;    // 0x3c
	Toshi::TVector4                          m_vecUp;           // 0x4c
	Toshi::TVector4                          m_vecRight;        // 0x5c, m_vecDirection x m_vecUp (Create)
	Toshi::TVector4                          m_vecFallVelocity; // 0x6c, velocity gained from gravity
	Toshi::TVector4                          m_vecHitNormal;    // 0x7c
	TFLOAT                                   m_fGravity;        // 0x8c
	TFLOAT                                   m_fUnk90;          // 0x90
	TFLOAT                                   m_fLifeTime;       // 0x94
	TFLOAT                                   m_fTime;           // 0x98
	Toshi::TCollisionCommon::TOSHICGROUPFLAG m_eIgnoreGroups;   // 0x9c
};

TSTATICASSERT( sizeof( AProjectileItemSteer ) == 0xa0 );
