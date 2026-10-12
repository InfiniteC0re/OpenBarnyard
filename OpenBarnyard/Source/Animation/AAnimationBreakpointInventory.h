#pragma once
#include "AAnimationBreakpoint.h"

//-----------------------------------------------------------------------------
// Moves items of the owner's inventory between the hand and the inventory.
//-----------------------------------------------------------------------------
class AAnimationBreakpointInventory
    : public AAnimationBreakpoint
{
public:
	TDECLARE_CLASS( AAnimationBreakpointInventory, AAnimationBreakpoint );

	// "Mode", order of the name table at 0x007750b4
	using MODE = TINT;
	enum MODE_ : MODE
	{
		MODE_HAND_TO_INVENTORY, // "HandToInventory"
		MODE_INVENTORY_TO_HAND, // "InventoryToHand", does nothing
		MODE_NUMOF,
	};

public:
	AAnimationBreakpointInventory();
	~AAnimationBreakpointInventory();

	//-----------------------------------------------------------------------------
	// AAnimationBreakpoint
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;             // +0x10
	virtual void  Execute( AAnimatableObject* a_pObject, TFLOAT a_fTime ) OVERRIDE; // +0x14

private:
	MODE m_eMode; // 0x08, not initialised by the constructor
};

TSTATICASSERT( sizeof( AAnimationBreakpointInventory ) == 0xc );
