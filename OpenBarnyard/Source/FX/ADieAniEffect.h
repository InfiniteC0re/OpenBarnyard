#pragma once
#include "AAniEffect.h"
#include "Units/AUnitProxy.h"

//-----------------------------------------------------------------------------
// Destroys the unit and finishes. The unit is kept out of its pool while the
// effect exists.
//
// TClass 0x007b4d50 (size 0x10), vtable 0x00739488.
//-----------------------------------------------------------------------------
class ADieAniEffect : public AAniEffect
{
public:
	TDECLARE_CLASS( ADieAniEffect, AAniEffect );

public:
	ADieAniEffect();
	~ADieAniEffect();

	//-----------------------------------------------------------------------------
	// AAniEffect
	//-----------------------------------------------------------------------------
	virtual AUnit* GetUnit() OVERRIDE;                           // +0x10, folded with AInventoryAniEffect::GetUnit (00631070)
	virtual TBOOL  Update( TFLOAT a_fDeltaTime ) OVERRIDE;       // +0x14
	virtual TBOOL  IsUnitDisposable() OVERRIDE { return TTRUE; } // +0x18, RETURNS_TRUE 0046cde0

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void Unk_0x28() {} // +0x28, DOES_NOTHING
	virtual void Unk_0x2c() {} // +0x2c, DOES_NOTHING

	void SetUnit( AUnit* a_pUnit );

private:
	Toshi::TRef<AUnitProxy> m_oUnit; // 0x0c
};

TSTATICASSERT( sizeof( ADieAniEffect ) == 0x10 );
