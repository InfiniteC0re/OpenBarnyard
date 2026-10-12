#pragma once
#include "AAniEffect.h"
#include "Units/AUnitProxy.h"

#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Keeps rotating the direction of the unit's steer around the X, Y and Z axes.
// Finishes when the unit is gone.
//
// TClass 0x007b4e10 (size 0x20), vtable 0x007395a0.
//-----------------------------------------------------------------------------
class ARotateAniEffect : public AAniEffect
{
public:
	TDECLARE_CLASS( ARotateAniEffect, AAniEffect );

public:
	ARotateAniEffect();
	~ARotateAniEffect();

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

	// Radians per second around the X, Y and Z axes
	void SetRotationSpeed( const Toshi::TVector4& a_rcSpeed );

private:
	Toshi::TVector4         m_vRotationSpeed; // 0x0c
	Toshi::TRef<AUnitProxy> m_oUnit;          // 0x1c
};

TSTATICASSERT( sizeof( ARotateAniEffect ) == 0x20 );
