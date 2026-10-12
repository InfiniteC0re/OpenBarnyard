#pragma once
#include "AAniEffect.h"
#include "Units/AUnitProxy.h"

#include <Math/TVector4.h>
#include <Math/TQuaternion.h>
#include <Toshi/TPString8.h>

//-----------------------------------------------------------------------------
// Moves the unit (or one of its bones) towards a bone of the target unit,
// accelerating up to a maximum speed. On the way the unit can be scaled and
// faded. Finishes once the unit is close enough to the bone.
// Used to move items taken out of the inventory screen into the hands of the player.
//
// TClass 0x007b4db0 (size 0x54), vtable 0x00739510.
// NOTE: same code as ATrackBoneAniEffect (different TClass and setters).
//-----------------------------------------------------------------------------
class AHudTrackBoneAniEffect : public AAniEffect
{
public:
	TDECLARE_CLASS( AHudTrackBoneAniEffect, AAniEffect );

public:
	AHudTrackBoneAniEffect();
	~AHudTrackBoneAniEffect();

	//-----------------------------------------------------------------------------
	// AAniEffect
	//-----------------------------------------------------------------------------
	virtual AUnit* GetUnit() OVERRIDE;                     // +0x10
	virtual TBOOL  Update( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x14

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void Unk_0x28() {} // +0x28, DOES_NOTHING
	virtual void Unk_0x2c() {} // +0x2c, DOES_NOTHING

	// Moves a_pUnit to the a_rcBone bone of a_pTarget
	void Setup( AUnit* a_pUnit, AUnit* a_pTarget, const Toshi::TPString8& a_rcBone, TFLOAT a_fMaxSpeed, TFLOAT a_fFadeDistance );

	void SetUnit( AUnit* a_pUnit );
	void SetTarget( AUnit* a_pTarget );

	// The bone of the target to move to, the target has to be set
	void SetTargetBone( const Toshi::TPString8& a_rcBone );

	// The bone of the unit that is moved to the target (instead of the position of the unit)
	void SetUnitBone( TINT a_iBone ) { m_iUnitBone = a_iBone; } // folded setter 0054f230

	// The scale the unit gets once it reaches the target
	void SetTargetScale( TFLOAT a_fScale ) { m_fTargetScale = a_fScale; }

private:
	TBOOL                   m_bFirstUpdate;   // 0x0c
	TINT                    m_iTargetBone;    // 0x10, -1: the position of the target
	TINT                    m_iUnitBone;      // 0x14, -1: the position of the unit
	TFLOAT                  m_fSpeed;         // 0x18
	TFLOAT                  m_fMaxSpeed;      // 0x1c
	TFLOAT                  m_fAcceleration;  // 0x20
	TFLOAT                  m_fUnk24;         // 0x24
	TFLOAT                  m_fMinDistance;   // 0x28, finishes when closer than this
	TFLOAT                  m_fFadeDistance;  // 0x2c, fades the unit out in this distance before m_fMinDistance (if > 0)
	TFLOAT                  m_fTargetScale;   // 0x30, -1: the scale doesn't change
	TFLOAT                  m_fStartScale;    // 0x34
	TFLOAT                  m_fStartDistance; // 0x38
	Toshi::TQuaternion      m_qUnk3C;         // 0x3c
	Toshi::TRef<AUnitProxy> m_oUnit;          // 0x4c
	Toshi::TRef<AUnitProxy> m_oTarget;        // 0x50
};

TSTATICASSERT( sizeof( AHudTrackBoneAniEffect ) == 0x54 );
