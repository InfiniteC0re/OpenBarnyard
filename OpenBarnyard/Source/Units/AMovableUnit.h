#pragma once
#include "AAnimatedUnit.h"

// An animated unit that thinks (can be moved by its steer) and casts a shadow
// (the bike racks, the pick ups, levers...).
//
// TClass 0x0077e7e8 (size 0x9c, runtime creatable), vtable 0x00718608,
// secondary (AGoalCollisionUpdateInterface) vtable 0x007185fc.
class AMovableUnit : public AAnimatedUnit
{
public:
	TDECLARE_CLASS( AMovableUnit, AAnimatedUnit );

public:
	AMovableUnit();
	~AMovableUnit();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AUnitTransform* a_pTransform ) override; // +0x58
	virtual void  UpdateAI( TFLOAT a_fDeltaTime ) override;        // +0x68
	virtual TBOOL Think( TFLOAT a_fDeltaTime ) override;           // +0x6c

private:
	TINT m_iUnk98; // 0x98
};

TSTATICASSERT( sizeof( AMovableUnit ) == 0x9c );
