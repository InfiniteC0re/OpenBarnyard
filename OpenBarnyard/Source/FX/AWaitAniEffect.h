#pragma once
#include "AAniEffect.h"

//-----------------------------------------------------------------------------
// Finishes after a time, used to delay the effects chained after it.
//
// TClass 0x007b4f00 (size 0x10), vtable 0x007396fc.
//-----------------------------------------------------------------------------
class AWaitAniEffect : public AAniEffect
{
public:
	TDECLARE_CLASS( AWaitAniEffect, AAniEffect );

public:
	AWaitAniEffect();
	~AWaitAniEffect();

	//-----------------------------------------------------------------------------
	// AAniEffect
	//-----------------------------------------------------------------------------
	virtual TBOOL Update( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x14

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void Unk_0x28() {} // +0x28, DOES_NOTHING
	virtual void Unk_0x2c() {} // +0x2c, DOES_NOTHING

	void SetTime( TFLOAT a_fTime ) { m_fTime = a_fTime; } // folded setter 0064e660

private:
	TFLOAT m_fTime; // 0x0c, seconds left
};

TSTATICASSERT( sizeof( AWaitAniEffect ) == 0x10 );
