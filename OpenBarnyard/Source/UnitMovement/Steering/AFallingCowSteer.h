#pragma once
#include "ASteer.h"
#include "Physics/ARigidBody.h"

class AParticleEffectHandle;

//-----------------------------------------------------------------------------
// Purpose: Steer of the cow that crashed off the bike: a rigid body sphere
// thrown in the air that falls under the gravity until it lies on the ground
// (IsFinished) or sinks in the water (IsSunk). See ABikeManager::OnUpdate.
// TClass 0x00788d2c, vtable 0x007257c8, size 0x2fc.
//-----------------------------------------------------------------------------
class AFallingCowSteer : public ASteer
{
public:
	TDECLARE_CLASS( AFallingCowSteer, ASteer );

	enum EFFECT_ : TINT
	{
		EFFECT_WATERSPLASH2A,
		EFFECT_WATERSPLASH2B,
		EFFECT_WATERRIPPLE,
		EFFECT_NUMOF
	};

public:
	AFallingCowSteer();
	~AFallingCowSteer();

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

	// NOTE: folded with AThrowItemSteer::CopyDirection (0055fd50)
	virtual void CopyDirection( Toshi::TVector4& a_rOutDirection ) OVERRIDE { a_rOutDirection = m_vDirection; } // +0xb0

	// NOTE: folded with other getters of a vector at +0x18 (005772b0)
	virtual const Toshi::TVector4* GetDirection() OVERRIDE { return &m_vDirection; } // +0xb8

	virtual TFLOAT GetSpeed() OVERRIDE; // +0xc4

	// NOTE: folded with other float getters returning 0.0f (0056ce20)
	virtual TFLOAT GetDesiredSpeed() OVERRIDE { return 0.0f; }  // +0xc8
	virtual TFLOAT GetMeasuredSpeed() OVERRIDE { return 0.0f; } // +0xd4

	// DOES_NOTHING (00628f40): there's no ASteerKinematics to forward the speed to
	virtual void SetSpeed( TFLOAT a_fSpeed ) OVERRIDE {} // +0x100

	virtual void SetVelocity( const Toshi::TVector4& a_rcVelocity ) OVERRIDE; // +0x108

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// The cow touched the ground for the first time
	TBOOL HasLanded() const;

	// The cow has been in the water for more than a second
	TBOOL IsSunk() const;

	// The cow lies on the ground
	TBOOL IsFinished() const;

	ARigidBody& GetBody() { return m_oBody; }
	TFLOAT      GetTime() const { return m_fTime; }
	TBOOL       IsInWater() const { return m_bInWater; }

private:
	void SetupBody( const Toshi::TVector4& a_rcPosition, const Toshi::TQuaternion& a_rcOrientation );

	// Checks if the cow fell into deep water between a_rcPosition and its current position
	void UpdateWater( const Toshi::TVector4& a_rcPosition, TFLOAT a_fDeltaTime );

private:
	Toshi::TVector4 m_vDirection;                // 0x18
	TBOOL           m_bFinished;                 // 0x28
	TBOOL           m_bLanded;                   // 0x29
	TBOOL           m_bInWater;                  // 0x2a
	TFLOAT          m_fInWaterTime;              // 0x2c
	TFLOAT          m_fTime;                     // 0x30
	TFLOAT          m_fNoContactTime;            // 0x34
	Toshi::TVector4 m_vOffset;                   // 0x38, from the position of the unit to the centre of the body
	ARigidBody      m_oBody;                     // 0x48
	TINT            m_aiEffects[ EFFECT_NUMOF ]; // 0x2ec
	TINT            m_iLandCue;                  // 0x2f8
};

TSTATICASSERT( sizeof( AFallingCowSteer ) == 0x2fc );
