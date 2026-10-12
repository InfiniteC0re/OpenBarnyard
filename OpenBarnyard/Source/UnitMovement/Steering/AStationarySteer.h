#pragma once
#include "ACommonSteer.h"

//-----------------------------------------------------------------------------
// Purpose: Default steer of units whose type doesn't name one (AUnit::GetSteerClass).
// Has no kinematics: the unit never moves on its own, it only keeps a position
// and a facing direction that can be set directly. Every movement query returns
// zero and every command finishes immediately.
// NOTE: the constructor is inlined into CreateObject/CreateObjectInPlace (00570500/00570560).
//-----------------------------------------------------------------------------
class AStationarySteer : public ACommonSteer
{
public:
	TDECLARE_CLASS( AStationarySteer, ACommonSteer );

public:
	AStationarySteer();
	~AStationarySteer();

	virtual TBOOL CreateFrom( ASteer* a_pOther, TINT a_iLOD ) OVERRIDE;                                                                       // +0x20
	virtual TBOOL Create( AUnit* a_pOwner, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, TINT a_iLOD ) OVERRIDE; // +0x24
	virtual TBOOL Update( TFLOAT a_fDeltaTime ) OVERRIDE { return TTRUE; }                                                                    // +0x30, RETURNS_TRUE 00413920
	virtual TBOOL IsCommandFinished() OVERRIDE { return TTRUE; }                                                                              // +0x44, RETURNS_TRUE 0046cde0

	// NOTE: folded with other float getters returning 0.0f (0056ce20)
	virtual TFLOAT GetCollisionRadius() OVERRIDE { return 0.0f; } // +0x80

	virtual TBOOL SetPosition( const Toshi::TVector4& a_rcPosition ) OVERRIDE;   // +0x84
	virtual TBOOL SetPosition3( const Toshi::TVector3& a_rcPosition ) OVERRIDE;  // +0x88
	virtual void  SetDirection( const Toshi::TVector4& a_rcDirection ) OVERRIDE; // +0x8c
	virtual void  CopyPosition( Toshi::TVector4& a_rOutPosition ) OVERRIDE;      // +0xa4
	virtual void  CopyPosition3( Toshi::TVector3& a_rOutPosition ) OVERRIDE;     // +0xa8

	// NOTE: folded with other getters of a vector at +0x20 (0061c770)
	virtual const Toshi::TVector4* GetPosition() OVERRIDE { return &m_vPosition; } // +0xac

	virtual void CopyDirection3( Toshi::TVector3& a_rOutDirection ) OVERRIDE; // +0xb4

	// NOTE: folded with other getters of a vector at +0x30 (00577260)
	virtual const Toshi::TVector4* GetDirection() OVERRIDE { return &m_vDirection; } // +0xb8

	virtual void CopyVelocity( Toshi::TVector4& a_rOutVelocity ) OVERRIDE; // +0xbc

	// NOTE: folded with other functions returning TVector4::VEC_ZERO (0061d680)
	virtual Toshi::TVector4 GetVelocity() OVERRIDE { return Toshi::TVector4::VEC_ZERO; } // +0xc0

	// NOTE: folded with other float getters returning 0.0f (0056ce20)
	virtual TFLOAT GetSpeed() OVERRIDE { return 0.0f; }              // +0xc4
	virtual TFLOAT GetDesiredSpeed() OVERRIDE { return 0.0f; }       // +0xc8
	virtual TFLOAT GetSpeedRatio() OVERRIDE { return 0.0f; }         // +0xcc
	virtual TFLOAT GetMeasuredSpeedRatio() OVERRIDE { return 0.0f; } // +0xd0
	virtual TFLOAT GetMeasuredSpeed() OVERRIDE { return 0.0f; }      // +0xd4
	virtual TFLOAT GetTurnSpeed() OVERRIDE { return 0.0f; }          // +0xd8

	virtual TFLOAT           GetSpeedFromModifier( TFLOAT a_fModifier ) OVERRIDE;                                  // +0xdc
	virtual TFLOAT           GetGroundY() OVERRIDE;                                                                // +0xe4
	virtual Toshi::TVector4  GetGroundPosition() OVERRIDE;                                                         // +0xe8
	virtual Toshi::TVector4& PredictPosition( TINT a_iUnk, Toshi::TVector4& a_rOut ) OVERRIDE;                     // +0xec
	virtual void             SetSpeed( TFLOAT a_fSpeed ) OVERRIDE {}                                               // +0x100, DOES_NOTHING 00628f40
	virtual void             SetDesiredSpeed( TFLOAT a_fSpeed ) OVERRIDE {}                                        // +0x104, DOES_NOTHING 00628f40
	virtual TBOOL            Halt() OVERRIDE { return TTRUE; }                                                     // +0x138, RETURNS_TRUE 0046cde0
	virtual TBOOL            HaltAtPosition( const Toshi::TVector3& a_rcPosition ) OVERRIDE { return TTRUE; }      // +0x13c, RETURNS_TRUE 00413920
	virtual TBOOL            HaltFacing( const Toshi::TVector3& a_rcFacing ) OVERRIDE { return TTRUE; }            // +0x140, RETURNS_TRUE 00413920
	virtual TBOOL            HaltFacingDesired( const Toshi::TVector3& a_rcFacing ) OVERRIDE;                      // +0x14c
	virtual TBOOL            IsSeekingConstant() OVERRIDE { return TFALSE; }                                       // +0x150, RETURNS_FALSE 00660960
	virtual TBOOL            IsFacingAtDesiredFacing() OVERRIDE { return TTRUE; }                                  // +0x154, RETURNS_TRUE 0046cde0
	virtual TBOOL            SetDesiredVelocity( const Toshi::TVector4& a_rcVelocity ) OVERRIDE { return TFALSE; } // +0x174, RETURNS_FALSE 006ce000

	// AStationarySteer's own slots (+0x198..+0x19c)
	virtual void  Unk_0x198( Toshi::TVector3& a_rOutVector );                  // +0x198, always outputs zero
	virtual void  SetPosition3NoReturn( const Toshi::TVector3& a_rcPosition ); // +0x19c, same as SetPosition3 without a result
	virtual TBOOL Unk_0x1a0( void*, void* ) { return TTRUE; }                  // +0x1a0, RETURNS_TRUE 00558f80
	virtual TBOOL Unk_0x1a4( void*, void* ) { return TTRUE; }                  // +0x1a4, RETURNS_TRUE 00558f80

private:
	Toshi::TVector4 m_vPosition;  // 0x20
	Toshi::TVector4 m_vDirection; // 0x30
};

TSTATICASSERT( sizeof( AStationarySteer ) == 0x40 );
