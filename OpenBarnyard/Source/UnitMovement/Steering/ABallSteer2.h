#pragma once
#include "ASteer.h"

//-----------------------------------------------------------------------------
// Purpose: Steer of a unit that bounces around like a ball (the coins, put down
// hand items, the pieces of a broken unit). It falls with gravity, slows down
// with the damping, bounces off the collision objects and the terrain (by the
// ASteerTraits "Bounce" of the unit) and stops once it stays on the ground for
// a few updates. Then it changes the steer of the unit to m_pNextSteerClass.
// The position of the unit is the position of the ball moved by m_vecOffset.
// TClass 0x00788b24 (version 1.0, size 0x7c, runtime creatable), vtable 0x00724ed0.
// NOTE: original names of the members and own methods are unknown.
//-----------------------------------------------------------------------------
class ABallSteer2 : public ASteer
{
public:
	TDECLARE_CLASS( ABallSteer2, ASteer );

	// m_iNumStillUpdates needed to stop moving
	static constexpr TINT NUM_STILL_UPDATES_TO_STOP = 6;

public:
	ABallSteer2();
	~ABallSteer2();

	virtual TBOOL CreateFrom( ASteer* a_pOther, TINT a_iLOD ) OVERRIDE;                                                                       // +0x20
	virtual TBOOL Create( AUnit* a_pOwner, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, TINT a_iLOD ) OVERRIDE; // +0x24
	virtual TBOOL Update( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                     // +0x30
	virtual TBOOL SetPosition( const Toshi::TVector4& a_rcPosition ) OVERRIDE;                                                                // +0x84
	virtual TBOOL SetPosition3( const Toshi::TVector3& a_rcPosition ) OVERRIDE;                                                               // +0x88
	virtual void  SetDirection( const Toshi::TVector4& a_rcDirection ) OVERRIDE;                                                              // +0x8c

	virtual void                   CopyPosition( Toshi::TVector4& a_rOutPosition ) OVERRIDE;   // +0xa4
	virtual void                   CopyPosition3( Toshi::TVector3& a_rOutPosition ) OVERRIDE;  // +0xa8
	virtual const Toshi::TVector4* GetPosition() OVERRIDE;                                     // +0xac
	virtual void                   CopyDirection( Toshi::TVector4& a_rOutDirection ) OVERRIDE; // +0xb0
	virtual const Toshi::TVector4* GetDirection() OVERRIDE;                                    // +0xb8
	virtual TFLOAT                 GetSpeed() OVERRIDE;                                        // +0xc4
	virtual TFLOAT                 GetDesiredSpeed() OVERRIDE { return 0.0f; }                 // +0xc8, 0056ce20 (shared)
	virtual TFLOAT                 GetMeasuredSpeed() OVERRIDE { return 0.0f; }                // +0xd4, 0056ce20 (shared)

	virtual void SetSpeed( TFLOAT a_fSpeed ) OVERRIDE {}                      // +0x100, DOES_NOTHING 00628f40
	virtual void SetVelocity( const Toshi::TVector4& a_rcVelocity ) OVERRIDE; // +0x108

	// Own virtuals
	virtual void  SetDamping( TFLOAT a_fDamping );             // +0x174
	virtual void  SetBounceDistanceFactor( TFLOAT a_fFactor ); // +0x178
	virtual void  SetAlignToGround( TBOOL a_bAlign );          // +0x17c
	virtual TBOOL IsStopped();                                 // +0x180
	virtual void  SetUnk75( TBOOL a_bValue );                  // +0x184

	const Toshi::TVector4& GetOffset() const { return m_vecOffset; }
	void                   SetOffset( const Toshi::TVector4& a_rcOffset ) { m_vecOffset = a_rcOffset; }
	TUINT                  GetCollisionFlags() const { return m_uiCollisionFlags; }
	void                   SetGravity( TFLOAT a_fGravity ) { m_fGravity = a_fGravity; }
	void                   SetCollisionFlags( TUINT a_uiFlags ) { m_uiCollisionFlags = a_uiFlags; }
	void                   SetIgnoreHitsWhenLeaving( TBOOL a_bIgnore ) { m_bIgnoreHitsWhenLeaving = a_bIgnore; }
	void                   SetNextSteerClass( Toshi::TClass* a_pClass ) { m_pNextSteerClass = a_pClass; }

private:
	// Moves the ball a_fDistance along m_vecMoveDirection and bounces off what it hits, moving the rest
	// of the distance (scaled by m_fBounceDistanceFactor) in the new direction.
	// a_fGravity is the speed gained from the gravity in this update, a_bCheckStop is TTRUE for the first call.
	TBOOL Move( TFLOAT a_fDistance, TFLOAT a_fGravity, TBOOL a_bCheckStop );

private:
	Toshi::TVector4 m_vecPosition;            // 0x18
	Toshi::TVector4 m_vecMoveDirection;       // 0x28, normalised velocity
	Toshi::TVector4 m_vecDirection;           // 0x38, facing
	Toshi::TVector4 m_vecOffset;              // 0x48, the unit is at m_vecPosition + m_vecOffset
	TFLOAT          m_fGravity;               // 0x58
	TUINT           m_uiCollisionFlags;       // 0x5c, a_eFlags of ACollisionManager::CollideSweptSphere
	TFLOAT          m_fSpeed;                 // 0x60
	TINT            m_iNumStillUpdates;       // 0x64, stops at NUM_STILL_UPDATES_TO_STOP
	TFLOAT          m_fDamping;               // 0x68
	TFLOAT          m_fBounceDistanceFactor;  // 0x6c
	TFLOAT          m_fLandSoundTime;         // 0x70, the land sound plays again once it's 0
	TBOOL           m_bAlignToGround;         // 0x74, faces along the normal of the ground while stuck
	TBOOL           m_bUnk75;                 // 0x75
	TBOOL           m_bIgnoreHitsWhenLeaving; // 0x76, hits are ignored while moving away from their normal
	Toshi::TClass*  m_pNextSteerClass;        // 0x78, the steer of the unit once it stops
};

TSTATICASSERT( sizeof( ABallSteer2 ) == 0x7c );
