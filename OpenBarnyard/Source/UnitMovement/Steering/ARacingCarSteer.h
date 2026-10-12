#pragma once
#include "ACarSteer.h"

class APhysicsCar;

//-----------------------------------------------------------------------------
// Purpose: Steer of the cars of Joy Ride. The movement comes from an
// APhysicsCar (allocated from the car pool of AVehicleManager in CreatePhysics)
// driven by SetPedal/SetSteering. Puts the car back where it last stood on its
// wheels when it's stuck for 5 seconds.
// NOTE: derives from ACarSteer in C++, but its TClass has ASteer as the parent.
// TClass 0x007893b4, vtable 0x00726a48, size 0x278.
//-----------------------------------------------------------------------------
class ARacingCarSteer : public ACarSteer
{
public:
	TDECLARE_CLASS( ARacingCarSteer, ASteer );

public:
	ARacingCarSteer();
	~ARacingCarSteer();

	//-----------------------------------------------------------------------------
	// ASteer
	//-----------------------------------------------------------------------------
	virtual TBOOL                  CreateFrom( ASteer* a_pOther, TINT a_iLOD ) OVERRIDE;                                                                       // +0x20
	virtual TBOOL                  Create( AUnit* a_pOwner, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, TINT a_iLOD ) OVERRIDE; // +0x24
	virtual TBOOL                  Update( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                     // +0x30
	virtual TBOOL                  SetPosition( const Toshi::TVector4& a_rcPosition ) OVERRIDE;                                                                // +0x84
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
	virtual void                   SetVelocity( const Toshi::TVector4& a_rcVelocity ) OVERRIDE;                                                                // +0x108

	// NOTE: folded with other float getters returning 1.0f (006bf680)
	virtual TFLOAT GetRadius() OVERRIDE { return 1.0f; } // +0x160

	//-----------------------------------------------------------------------------
	// ACarSteer
	//-----------------------------------------------------------------------------
	virtual void   GetSeatMatrix( Toshi::TMatrix44& a_rOutMatrix ) OVERRIDE;                    // +0x174
	virtual void   Reset( Toshi::TVector4 a_vPosition, Toshi::TVector4 a_vDirection ) OVERRIDE; // +0x178
	virtual void   SetPedal( PEDAL a_ePedal, TBOOL a_bGradual ) OVERRIDE;                       // +0x17c
	virtual void   SetSteering( STEER a_eSteer, TFLOAT a_fSteering, TFLOAT a_fAngle ) OVERRIDE; // +0x180
	virtual TFLOAT GetMaxSpeed() OVERRIDE;                                                      // +0x184

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	APhysicsCar* GetCar() const { return m_pCar; }

private:
	// Allocates and sets up the physics of the car and puts it at the position
	void CreatePhysics( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection );

	// Remembers where the car last stood on its wheels, counts the time it's stuck otherwise
	void UpdateStuck( TFLOAT a_fDeltaTime );

	// Puts the car back where it last stood on its wheels after 5 seconds of being stuck
	void UpdateRespawn();

	// Particle effects and sounds of the bushes the car drives through
	void UpdateCollisions( TFLOAT a_fDeltaTime );

private:
	TINT               m_iUnk21C;              // 0x21C
	APhysicsCar*       m_pCar;                 // 0x220
	Toshi::TVector4    m_vSeatOffset;          // 0x224
	TFLOAT             m_fStuckTime;           // 0x234
	Toshi::TVector4    m_vLastGoodPosition;    // 0x238
	Toshi::TVector4    m_vLastPosition;        // 0x248
	Toshi::TQuaternion m_qLastGoodOrientation; // 0x258
	TINT               m_iGrassEffect;         // 0x268, grasswalk
	TINT               m_iBushEffect;          // 0x26C, bushhit
	TFLOAT             m_fBushEffectTime;      // 0x270, time left before the next bushhit effect
	TINT               m_iBushCue;             // 0x274, the sound of the last bush hit
};

TSTATICASSERT( sizeof( ARacingCarSteer ) == 0x278 );
