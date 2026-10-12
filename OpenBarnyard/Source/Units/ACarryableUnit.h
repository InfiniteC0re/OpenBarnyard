#pragma once
#include "AAnimatedUnit.h"
#include "ACarryableUnitHelper.h"

// An animated unit that can be carried, put in an inventory, thrown, used...
// (the items). What happens to it is described by the AObjectTraits of its type
// and handled by its ACarryableUnitHelper.
//
// TClass 0x0077e6c8 (version 1.1, size 0xdc, runtime creatable), vtable 0x007180b8,
// secondary (AGoalCollisionUpdateInterface) vtable 0x007180ac.
class ACarryableUnit : public AAnimatedUnit
{
public:
	TDECLARE_CLASS( ACarryableUnit, AAnimatedUnit );

public:
	ACarryableUnit();
	~ACarryableUnit();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual ACarryableUnitHelper* GetCarryableHelper() override;                   // +0x54
	virtual TBOOL                 Create( AUnitTransform* a_pTransform ) override; // +0x58
	virtual void                  OnDestroy() override;                            // +0x60
	virtual void                  UpdateAI( TFLOAT a_fDeltaTime ) override;        // +0x68
	virtual TBOOL                 Think( TFLOAT a_fDeltaTime ) override;           // +0x6c, also updates the helper

private:
	ACarryableUnitHelper m_oHelper; // 0x98
};

TSTATICASSERT( sizeof( ACarryableUnit ) == 0xdc );
