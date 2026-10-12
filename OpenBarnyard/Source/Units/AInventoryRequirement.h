#pragma once
#include "AInteractionRequirement.h"

class AInventoryComponentInterface;

//-----------------------------------------------------------------------------
// Compares ("compare", "equal" if only "count" is set) the number of items of
// the type "itemtype" (or with the id "itemid") with "count". "search" picks the
// inventory: the unit's ("inventory", the default; "hand", "backpack", "equip"),
// the player's ("playerinventory", "playerhand", "playerbackpack", "playerequip")
// or the gopher shop's purchases ("shop"). "itemtype" "money" compares the
// player's money, "none" counts every item.
// TClass 0x0078acec, vtable 0x0072a67c, size 0x60.
//-----------------------------------------------------------------------------
class AInventoryRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AInventoryRequirement, AInteractionRequirement );

	using FLAGS = TUINT16;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE            = 0,
		FLAGS_SHOP            = BITFLAG( 0 ),  // "shop"
		FLAGS_UNK1            = BITFLAG( 1 ),  // never set, Evaluate does nothing
		FLAGS_INVENTORY       = BITFLAG( 2 ),  // the unit's inventory (default)
		FLAGS_HAND            = BITFLAG( 3 ),  // "hand"
		FLAGS_BACKPACK        = BITFLAG( 4 ),  // "backpack"
		FLAGS_EQUIP           = BITFLAG( 5 ),  // "equip"
		FLAGS_PLAYER          = BITFLAG( 6 ),  // the player's inventory
		FLAGS_PLAYER_HAND     = BITFLAG( 7 ),  // "playerhand"
		FLAGS_PLAYER_BACKPACK = BITFLAG( 8 ),  // "playerbackpack"
		FLAGS_PLAYER_EQUIP    = BITFLAG( 9 ),  // "playerequip"
		FLAGS_ANY_TYPE        = BITFLAG( 10 ), // "itemtype" is "none"
	};

public:
	AInventoryRequirement();
	~AInventoryRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	// Returns the "Hand", "BackPack" or "Equip" category of the inventory if it has categories
	// NOTE: inlined into Evaluate
	AInventoryComponentInterface* GetCategory( AInventoryComponentInterface* a_pInventory, FLAGS a_uiHand, FLAGS a_uiBackPack, FLAGS a_uiEquip ) const;

private:
	TINT                   m_iItemId;  // 0x4c, "itemid"
	Toshi::TPString8       m_strType;  // 0x50, "itemtype"
	Toshi::TPString8       m_strUnk;   // 0x54, unused
	TINT                   m_iCount;   // 0x58, "count"
	AInteractionComparison m_oCompare; // 0x5c, "compare"
	FLAGS                  m_uiFlags;  // 0x5e, "search"
};

TSTATICASSERT( sizeof( AInventoryRequirement ) == 0x60 );
