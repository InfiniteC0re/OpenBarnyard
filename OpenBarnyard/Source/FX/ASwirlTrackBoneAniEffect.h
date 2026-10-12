#pragma once
#include "AAniEffect.h"
#include "Units/AUnitProxy.h"

#include <Math/TVector2.h>

//-----------------------------------------------------------------------------
// Makes the unit swirl down towards a bone of the target unit (following the
// target's velocity). Finishes once the unit is low enough or, if a screen
// position is set, once it's in front of that point of the screen.
//
// TClass 0x007b4e70 (size 0x38), vtable 0x0073962c.
// NOTE: nothing in retail sets its fields, it's probably unused.
//-----------------------------------------------------------------------------
class ASwirlTrackBoneAniEffect : public AAniEffect
{
public:
	TDECLARE_CLASS( ASwirlTrackBoneAniEffect, AAniEffect );

public:
	ASwirlTrackBoneAniEffect();
	~ASwirlTrackBoneAniEffect();

	//-----------------------------------------------------------------------------
	// AAniEffect
	//-----------------------------------------------------------------------------
	virtual AUnit* GetUnit() OVERRIDE;                     // +0x10
	virtual TBOOL  Update( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x14

private:
	TBOOL                   m_bFirstUpdate;  // 0x0c
	TINT                    m_iBone;         // 0x10, bone of the target
	TFLOAT                  m_fSpeed;        // 0x14
	TFLOAT                  m_fUnk18;        // 0x18
	TFLOAT                  m_fMinDistance;  // 0x1c
	TFLOAT                  m_fFadeDistance; // 0x20, fades the unit in this distance after m_fMinDistance (if > 0)
	TFLOAT                  m_fDepth;        // 0x24
	Toshi::TVector2         m_vScreenPos;    // 0x28, x is TMath::MAXFLOAT when not used
	Toshi::TRef<AUnitProxy> m_oUnit;         // 0x30
	Toshi::TRef<AUnitProxy> m_oTarget;       // 0x34
};

TSTATICASSERT( sizeof( ASwirlTrackBoneAniEffect ) == 0x38 );
