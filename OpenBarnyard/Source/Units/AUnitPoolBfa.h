#pragma once
#include "AAnimatedUnit.h"

//-----------------------------------------------------------------------------
// Purpose: A unit of the Barn Pool minigame whose model is placed at the
// position of its steer, facing m_vecDirection (set directly by the state).
// TClass 0x00783208 (version 1.0, size 0xa8, runtime creatable), vtable 0x007206a0,
// secondary (AGoalCollisionUpdateInterface) vtable 0x00720690.
// NOTE: original names of the members are unknown.
//-----------------------------------------------------------------------------
class AUnitPoolBfa : public AAnimatedUnit
{
public:
	TDECLARE_CLASS( AUnitPoolBfa, AAnimatedUnit );

public:
	AUnitPoolBfa();
	~AUnitPoolBfa();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AUnitTransform* a_pTransform ) override; // +0x58
	virtual void  UpdateAI( TFLOAT a_fDeltaTime ) override;        // +0x68
	virtual TBOOL Think( TFLOAT a_fDeltaTime ) override;           // +0x6c

	void                   SetDirection( const Toshi::TVector4& a_rcDirection ) { m_vecDirection = a_rcDirection; }
	const Toshi::TVector4& GetDirection() const { return m_vecDirection; }

public:
	Toshi::TVector4 m_vecDirection; // 0x98, facing of the model
};

TSTATICASSERT( sizeof( AUnitPoolBfa ) == 0xa8 );
