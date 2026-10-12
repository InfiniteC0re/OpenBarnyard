#pragma once
#include "AAniEffect.h"
#include "Units/AUnitProxy.h"

class AAnimatableObject;

//-----------------------------------------------------------------------------
// Fades the model of the unit (or of the object) out, then finishes.
//
// TClass 0x007b4d80 (size 0x18), vtable 0x007394cc.
//-----------------------------------------------------------------------------
class AFadeAniEffect : public AAniEffect
{
public:
	TDECLARE_CLASS( AFadeAniEffect, AAniEffect );

public:
	AFadeAniEffect();
	~AFadeAniEffect();

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

	void SetUnit( AUnit* a_pUnit );

	// Fades a_pObject instead of a unit
	void SetObject( AAnimatableObject* a_pObject );

	// Opacity lost per second
	void SetSpeed( TFLOAT a_fSpeed ) { m_fSpeed = a_fSpeed; } // folded setter 0064e660

private:
	TFLOAT                  m_fSpeed;  // 0x0c
	Toshi::TRef<AUnitProxy> m_oUnit;   // 0x10
	AAnimatableObject*      m_pObject; // 0x14, used if there's no unit
};

TSTATICASSERT( sizeof( AFadeAniEffect ) == 0x18 );
