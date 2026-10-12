#pragma once
#include "ASteer.h"

#include <Math/TMatrix44.h>
#include <Render/TCollisionCommon.h>

//-----------------------------------------------------------------------------
// Purpose: Steer of a thrown (or dropped) unit. It flies along a parabola,
// bounces off the terrain (losing speed by the AObjectTraits restitution of the
// unit) and stops once it lies still. The model spins around its "center" bone
// while flying.
// TClass 0x007894d8 (version 3.0, size 0xac, runtime creatable), vtable 0x00727380.
// NOTE: original names of the members and AThrowItemSteer's own method are unknown.
//-----------------------------------------------------------------------------
class AThrowItemSteer : public ASteer
{
public:
	TDECLARE_CLASS( AThrowItemSteer, ASteer );

public:
	AThrowItemSteer();
	~AThrowItemSteer();

	virtual TBOOL CreateFrom( ASteer* a_pOther, TINT a_iLOD ) OVERRIDE;                                                                       // +0x20
	virtual TBOOL Create( AUnit* a_pOwner, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, TINT a_iLOD ) OVERRIDE; // +0x24
	virtual TBOOL Update( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                     // +0x30
	virtual TBOOL SetPosition( const Toshi::TVector4& a_rcPosition ) OVERRIDE;                                                                // +0x84
	virtual TBOOL SetPosition3( const Toshi::TVector3& a_rcPosition ) OVERRIDE;                                                               // +0x88
	virtual void  SetDirection( const Toshi::TVector4& a_rcDirection ) OVERRIDE;                                                              // +0x8c

	// NOTE: folded with other getters of a vector at +0x18 (shared by several steers)
	virtual void                   CopyPosition( Toshi::TVector4& a_rOutPosition ) OVERRIDE;    // +0xa4
	virtual void                   CopyPosition3( Toshi::TVector3& a_rOutPosition ) OVERRIDE;   // +0xa8
	virtual const Toshi::TVector4* GetPosition() OVERRIDE;                                      // +0xac
	virtual void                   CopyDirection( Toshi::TVector4& a_rOutDirection ) OVERRIDE;  // +0xb0
	virtual void                   CopyDirection3( Toshi::TVector3& a_rOutDirection ) OVERRIDE; // +0xb4
	virtual const Toshi::TVector4* GetDirection() OVERRIDE;                                     // +0xb8
	virtual void                   CopyVelocity( Toshi::TVector4& a_rOutVelocity ) OVERRIDE;    // +0xbc
	virtual Toshi::TVector4        GetVelocity() OVERRIDE;                                      // +0xc0
	virtual TFLOAT                 GetSpeed() OVERRIDE;                                         // +0xc4
	virtual TFLOAT                 GetMeasuredSpeed() OVERRIDE;                                 // +0xd4, the smoothed speed

	// NOTE: retail returns TTRUE if the velocity is zero (unused by the callers)
	virtual void SetSpeed( TFLOAT a_fSpeed ) OVERRIDE;                        // +0x100
	virtual void SetVelocity( const Toshi::TVector4& a_rcVelocity ) OVERRIDE; // +0x108

	// Own virtuals
	virtual void GetTransform( Toshi::TMatrix44& a_rOutTransform ); // +0x174, transform of the model (position and spin)

	// The terrain triangles of the group a_iGroup don't stop the unit
	TBOOL IgnoreCollisionGroup( TINT a_iGroup );

	void  SetUpdateModelTransform( TBOOL a_bUpdate ) { m_bUpdateModelTransform = a_bUpdate; }
	void  SetFreezeSpeed( TBOOL a_bFreeze ) { m_bFreezeSpeed = a_bFreeze; }
	void  SetSmoothedSpeed( TFLOAT a_fSpeed ) { m_fSpeed = a_fSpeed; }
	void  SetRestitution( TFLOAT a_fRestitution ) { m_fRestitution = a_fRestitution; }
	TBOOL IsStopped() const { return m_bStopped; }
	TBOOL HasBounced() const { return m_bBounced; }

private:
	// Plays the sound of an egg landing (only eggs have one)
	void PlayLandSound();

private:
	Toshi::TVector4                          m_vecPosition;            // 0x18
	Toshi::TVector4                          m_vecVelocity;            // 0x28
	Toshi::TVector4                          m_vecDirection;           // 0x38
	TFLOAT                                   m_fSpinSpeed;             // 0x48
	TFLOAT                                   m_fSpinAngle;             // 0x4c
	TFLOAT                                   m_fRestitution;           // 0x50, AObjectTraits "Restitution"
	TFLOAT                                   m_fTangentialRestitution; // 0x54, AObjectTraits "TangentialRestitution"
	TBOOL                                    m_bStopped;               // 0x58
	TBOOL                                    m_bBounced;               // 0x59, bounced during the last update
	TBOOL                                    m_bUpdateModelTransform;  // 0x5a
	TFLOAT                                   m_fSpeed;                 // 0x5c, smoothed speed
	TBOOL                                    m_bFreezeSpeed;           // 0x60, m_fSpeed isn't updated
	Toshi::TMatrix44                         m_matCenterInv;           // 0x64
	TINT                                     m_iNumStillUpdates;       // 0xa4
	Toshi::TCollisionCommon::TOSHICGROUPFLAG m_eIgnoreGroups;          // 0xa8
};

TSTATICASSERT( sizeof( AThrowItemSteer ) == 0xac );
