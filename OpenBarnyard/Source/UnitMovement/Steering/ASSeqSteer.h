#pragma once
#include "ACommonSteer.h"

//-----------------------------------------------------------------------------
// Purpose: Steer of units driven by scripted sequences (SSeq). Has no kinematics:
// the position and facing are set directly, Seek only stores a target that
// HasArrived checks against the position.
// TClass 0x007893e4, vtable 0x00726bf0, size 0x80.
//-----------------------------------------------------------------------------
class ASSeqSteer : public ACommonSteer
{
public:
	TDECLARE_CLASS( ASSeqSteer, ACommonSteer );

public:
	ASSeqSteer();
	~ASSeqSteer();

	virtual TBOOL Update( TFLOAT a_fDeltaTime ) OVERRIDE { return TTRUE; } // +0x30, RETURNS_TRUE 00413920
	virtual TBOOL HasArrived() OVERRIDE;                                   // +0x34

	virtual TBOOL SetPosition( const Toshi::TVector4& a_rcPosition ) OVERRIDE;                    // +0x84
	virtual TBOOL SetPosition3( const Toshi::TVector3& a_rcPosition ) OVERRIDE { return TFALSE; } // +0x88, RETURNS_FALSE 006ce000
	virtual void  SetDirection( const Toshi::TVector4& a_rcDirection ) OVERRIDE;                  // +0x8c

	// NOTE: folded with AStationarySteer::CopyPosition (00570250)
	virtual void CopyPosition( Toshi::TVector4& a_rOutPosition ) OVERRIDE { a_rOutPosition = m_vPosition; } // +0xa4
	virtual void CopyPosition3( Toshi::TVector3& a_rOutPosition ) OVERRIDE {}                               // +0xa8, DOES_NOTHING 00628f40

	// NOTE: folded with other getters of a vector at +0x20 (0061c770)
	virtual const Toshi::TVector4* GetPosition() OVERRIDE { return &m_vPosition; } // +0xac

	virtual void CopyDirection( Toshi::TVector4& a_rOutDirection ) OVERRIDE;    // +0xb0
	virtual void CopyDirection3( Toshi::TVector3& a_rOutDirection ) OVERRIDE {} // +0xb4, DOES_NOTHING 00628f40

	// NOTE: folded with other getters of a vector at +0x40 (005748d0)
	virtual const Toshi::TVector4* GetDirection() OVERRIDE { return &m_vDirection; } // +0xb8

	virtual TFLOAT GetSpeed() OVERRIDE;         // +0xc4
	virtual TFLOAT GetMeasuredSpeed() OVERRIDE; // +0xd4, same function as GetSpeed

	// NOTE: folded with other getters of a float at +0x70 (0061d670)
	virtual TFLOAT GetTurnSpeed() OVERRIDE { return m_fTurnSpeed; } // +0xd8

	virtual void  SetSpeed( TFLOAT a_fSpeed ) OVERRIDE;                                                 // +0x100
	virtual TBOOL Seek( const Toshi::TVector4& a_rcTarget, TFLOAT a_fSeekOffset ) OVERRIDE;             // +0x144
	virtual TBOOL HaltFacingDesired( const Toshi::TVector3& a_rcFacing ) OVERRIDE;                      // +0x14c
	virtual TBOOL SetDesiredVelocity( const Toshi::TVector4& a_rcVelocity ) OVERRIDE { return TFALSE; } // +0x174, RETURNS_FALSE 006ce000

private:
	Toshi::TVector4 m_vPosition;         // 0x20
	Toshi::TVector4 m_vTarget;           // 0x30, see Seek
	Toshi::TVector4 m_vDirection;        // 0x40
	Toshi::TVector4 m_vDesiredDirection; // 0x50
	Toshi::TVector4 m_vUnk60;            // 0x60
	TFLOAT          m_fTurnSpeed;        // 0x70
	TFLOAT          m_fSpeed;            // 0x74
	TBOOL           m_bHasDesiredFacing; // 0x78, set by HaltFacingDesired
	TBOOL           m_bSeeking;          // 0x79, set by Seek
	TFLOAT          m_fSeekOffset;       // 0x7c
};

TSTATICASSERT( sizeof( ASSeqSteer ) == 0x80 );
