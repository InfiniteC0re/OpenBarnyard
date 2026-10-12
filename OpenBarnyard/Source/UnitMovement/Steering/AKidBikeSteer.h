#pragma once
#include "ASteer.h"

#include <Math/TMatrix44.h>

class ASpline;

//-----------------------------------------------------------------------------
// Purpose: Steer of the kids riding their bikes in Joy Ride (ACarChaseMiniGameState).
// The bike follows a spline (the race track) at m_fSideOffset to the side of it,
// sits on the ground and is pitched along the line between its two wheels.
// TClass 0x007892a0, vtable 0x007261d0, size 0xf0.
//-----------------------------------------------------------------------------
class AKidBikeSteer : public ASteer
{
public:
	TDECLARE_CLASS( AKidBikeSteer, ASteer );

	enum WHEEL : TINT
	{
		WHEEL_FRONT,
		WHEEL_REAR,
		WHEEL_NUMOF
	};

public:
	AKidBikeSteer();
	~AKidBikeSteer();

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
	virtual TFLOAT                 GetSpeed() OVERRIDE;                                                                                                        // +0xc4
	virtual void                   SetSpeed( TFLOAT a_fSpeed ) OVERRIDE;                                                                                       // +0x100
	virtual void                   SetVelocity( const Toshi::TVector4& a_rcVelocity ) OVERRIDE;                                                                // +0x108
	virtual TFLOAT                 GetRadius() OVERRIDE;                                                                                                       // +0x160, the distance between the wheels

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Copies the velocity (direction * speed)
	virtual void CopyVelocity2( Toshi::TVector4& a_rOutVelocity ); // +0x174

	// Returns the velocity (direction * speed)
	virtual Toshi::TVector4 GetVelocity2(); // +0x178

	void SetSpline( const ASpline* a_pSpline );

	// Places the bike at a_fProgress (0..1) along the spline
	void SetProgress( TFLOAT a_fProgress );

	// NOTE: shared folded getter of a float at +0xdc in retail (004d7ef0)
	TFLOAT GetProgress() const { return m_fProgress; }

	void   SetSideOffset( TFLOAT a_fOffset ) { m_fSideOffset = a_fOffset; }
	TFLOAT GetSideOffset() const { return m_fSideOffset; }

private:
	// Reads the positions of the "frontwheel" and "rearwheel" bones of the owner's model
	void InitialiseWheels();

	// Moves along the spline
	void UpdateProgress( TFLOAT a_fDeltaTime );

	// Sets the direction, applies the side offset, puts the bike and its wheels on the ground
	// and builds m_oTransform
	void UpdateTransform( Toshi::TVector4 a_vDirection );

private:
	const ASpline*   m_pSpline;                       // 0x18
	Toshi::TMatrix44 m_oTransform;                    // 0x1c
	Toshi::TVector4  m_vPosition;                     // 0x5c
	Toshi::TVector4  m_vDirection;                    // 0x6c, flat (y = 0)
	Toshi::TVector4  m_vUp;                           // 0x7c
	Toshi::TVector4  m_vRight;                        // 0x8c
	Toshi::TVector4  m_avWheels[ WHEEL_NUMOF ];       // 0x9c, world positions on the ground
	Toshi::TVector4  m_avWheelOffsets[ WHEEL_NUMOF ]; // 0xbc, bone positions in the model
	TFLOAT           m_fProgress;                     // 0xdc
	TFLOAT           m_fWheelBase;                    // 0xe0
	TFLOAT           m_fSpeed;                        // 0xe4
	TFLOAT           m_fDesiredSpeed;                 // 0xe8
	TFLOAT           m_fSideOffset;                   // 0xec, to the right of the spline
};

TSTATICASSERT( sizeof( AKidBikeSteer ) == 0xf0 );
