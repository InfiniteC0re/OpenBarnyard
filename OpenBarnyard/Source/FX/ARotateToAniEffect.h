#pragma once
#include "AAniEffect.h"
#include "Units/AUnitProxy.h"

#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Keeps turning the direction of the unit's steer towards a direction.
// Finishes when the unit is gone.
//
// TClass 0x007b4e40 (size 0x24), vtable 0x007395e4.
// NOTE: nothing in retail sets its fields, it's probably unused.
//-----------------------------------------------------------------------------
class ARotateToAniEffect : public AAniEffect
{
public:
	TDECLARE_CLASS( ARotateToAniEffect, AAniEffect );

public:
	ARotateToAniEffect();
	~ARotateToAniEffect();

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

private:
	Toshi::TVector4         m_vDirection; // 0x0c
	TINT                    m_iUnk1;      // 0x1c
	Toshi::TRef<AUnitProxy> m_oUnit;      // 0x20
};

TSTATICASSERT( sizeof( ARotateToAniEffect ) == 0x24 );
