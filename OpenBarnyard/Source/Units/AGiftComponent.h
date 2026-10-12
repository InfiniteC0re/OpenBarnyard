#pragma once
#include "AUnitComponent.h"
#include "AUnitStaticData.h"

class AGift;
class AGiftComponentData;

//-----------------------------------------------------------------------------
// Holds the AGift a unit was given (see AGift_DeprecatedActivity). The unit
// gets the interaction of the gift as soon as it has none.
// TClass 0x0078c1d4 (version 1.0, size 0x18, runtime creatable), vtable 0x0072d1b0.
//-----------------------------------------------------------------------------
class AGiftComponent
    : public AUnitComponent
{
public:
	TDECLARE_CLASS( AGiftComponent, AUnitComponent );

	friend class AGiftComponentData;

public:
	AGiftComponent();
	~AGiftComponent();

	virtual AUnitComponentManager* GetManager() OVERRIDE;                    // +0x10
	virtual void                   UpdateAI( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x2c, assigns the interaction of the gift

	// Takes a_pGift if the unit has no gift yet
	TBOOL GiveGift( AGift* a_pGift );

	// Gives the gift away (the caller owns it then)
	AGift* TakeGift();

	AGift* GetGift() const { return m_pGift; }

private:
	AGiftComponentData* m_pData; // 0x10, not initialised by the constructor
	AGift*              m_pGift; // 0x14
};

TSTATICASSERT( sizeof( AGiftComponent ) == 0x18 );

//-----------------------------------------------------------------------------
// Static data that gives every unit of the type an AGiftComponent.
// TClass version 1.0, size 0xc, vtable 0x0072d198.
//-----------------------------------------------------------------------------
class AGiftComponentData
    : public AUnitStaticData
{
public:
	TDECLARE_CLASS( AGiftComponentData, AUnitStaticData );

public:
	AGiftComponentData();
	~AGiftComponentData();

	virtual void OnUnitCreate( AUnit* a_pUnit ) OVERRIDE; // +0x10
};

TSTATICASSERT( sizeof( AGiftComponentData ) == 0xc );
