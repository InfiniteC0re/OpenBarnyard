#pragma once
#include "ASteer.h"

#include <Math/TMatrix44.h>

//-----------------------------------------------------------------------------
// Purpose: Steer of a helicopter that follows a target unit: it flies at
// m_fHeight above the target (bobbing up and down), speeds up and turns towards
// the target while it's further than sqrt( m_fRadiusSq ), slows down when it's
// close, and banks with its speed. It has no ASteerKinematics.
// NOTE: nothing in retail sets m_pTarget or m_bFollowing, so Update only
// updates the orientation.
// NOTE: vtable 0x007252b0, TClass parent ASteer, version 3.0, size 0xcc.
//-----------------------------------------------------------------------------
class ACarHeliSteer : public ASteer
{
public:
	TDECLARE_CLASS( ACarHeliSteer, ASteer );

public:
	ACarHeliSteer();
	~ACarHeliSteer();

	//-----------------------------------------------------------------------------
	// ASteer
	//-----------------------------------------------------------------------------
	virtual AFunctionTable*        GetFunctionTable() OVERRIDE;                                                                                                // +0x0c
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
	virtual void                   CopyVelocity( Toshi::TVector4& a_rOutVelocity ) OVERRIDE;                                                                   // +0xbc
	virtual Toshi::TVector4        GetVelocity() OVERRIDE;                                                                                                     // +0xc0
	virtual TFLOAT                 GetSpeed() OVERRIDE;                                                                                                        // +0xc4
	virtual void                   SetSpeed( TFLOAT a_fSpeed ) OVERRIDE;                                                                                       // +0x100
	virtual void                   SetVelocity( const Toshi::TVector4& a_rcVelocity ) OVERRIDE;                                                                // +0x108

public:
	static AFunctionTable ms_oFunctionTable;

private:
	AUnit*           m_pTarget;    // 0x18, never set
	TFLOAT           m_fHeight;    // 0x1c, above the target
	TFLOAT           m_fBobHeight; // 0x20
	TFLOAT           m_fBobAngle;  // 0x24, degrees
	TFLOAT           m_fRadiusSq;  // 0x28
	TFLOAT           m_fSpeed;     // 0x2c
	TFLOAT           m_fMaxSpeed;  // 0x30
	TFLOAT           m_fMinSpeed;  // 0x34
	TBOOL            m_bFollowing; // 0x38, never set
	Toshi::TVector4  m_vPosition;  // 0x3c
	Toshi::TVector4  m_vVelocity;  // 0x4c
	Toshi::TVector4  m_vDirection; // 0x5c
	Toshi::TVector4  m_vUp;        // 0x6c
	Toshi::TVector4  m_vRight;     // 0x7c
	Toshi::TMatrix44 m_oTransform; // 0x8c, of the model
};

TSTATICASSERT( sizeof( ACarHeliSteer ) == 0xcc );
