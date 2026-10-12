#pragma once
#include "AUnitComponent.h"

#include <Toshi/TPString8.h>

class AInventoryItem;

//-----------------------------------------------------------------------------
// Interface of the components that keep items: AInventoryComponent (a single
// category of items) and AInventoryGroupComponent (several categories, e.g.
// the player's "Hand" and the inventory screen). Items are units of a type,
// the methods taking a_rcType use the name of the type.
// The a_strInventory arguments name the category the items go to, with
// ms_strDefaultInventory the group component picks it from the
// "InventoryForStore" of the AObjectTraits of the item.
// NOTE: vtable 0x00737ac8, it has no TClass of its own.
//-----------------------------------------------------------------------------
class AInventoryComponentInterface
    : public AUnitComponent
{
public:
	AInventoryComponentInterface() = default;
	~AInventoryComponentInterface();

	//-----------------------------------------------------------------------------
	// AUnitComponent
	//-----------------------------------------------------------------------------
	virtual AUnitComponentManager* GetManager() OVERRIDE;                                    // +0x10
	virtual void                   Save( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE = 0; // +0x14
	virtual void                   Load( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE = 0; // +0x18

	//-----------------------------------------------------------------------------
	// AInventoryComponentInterface
	//-----------------------------------------------------------------------------
	// Empties the inventory, the units of the items are released
	virtual void Clear() = 0; // +0x38

	// Moves the item (a slot of another inventory) to this inventory
	virtual TBOOL AddItem( AInventoryItem* a_pItem, Toshi::TPString8 a_strInventory ) = 0; // +0x3c

	// Spawns a_iCount items of the type
	virtual TBOOL AddItems( const Toshi::TPString8& a_rcType, TINT a_iCount, Toshi::TPString8 a_strInventory ) = 0; // +0x40

	// Takes the unit out of the world and keeps it as a_iCount items of its type
	virtual TBOOL AddUnit( AUnit* a_pUnit, TINT a_iCount, Toshi::TPString8 a_strInventory ) = 0; // +0x44

	// Takes one item of the slot out of the inventory and spawns it in the world.
	// Returns the unit that represents the item in the world.
	virtual AUnit* DropItem( AInventoryItem* a_pItem ) = 0; // +0x48

	// DropItem for the slot of the type
	virtual AUnit* DropItemOfType( const Toshi::TPString8& a_rcType ) = 0; // +0x4c

	// Removes up to a_iCount items of the type, their units are destroyed if a_bDestroy is set.
	// Returns the number of removed items.
	virtual TINT RemoveItems( const Toshi::TPString8& a_rcType, TINT a_iCount, TBOOL a_bDestroy ) = 0; // +0x50

	// Removes the slot (without releasing its unit). Returns a_pItem or TNULL if it wasn't removed.
	virtual AInventoryItem* RemoveItem( AInventoryItem* a_pItem ) = 0; // +0x54

	virtual AInventoryItem* FindItemByUnit( AUnit* a_pUnit )                   = 0; // +0x58
	virtual AInventoryItem* FindItemByType( const Toshi::TPString8& a_rcType ) = 0; // +0x5c

	// Number of items (not slots) in the inventory
	virtual TINT GetNumItems()                                         = 0; // +0x60
	virtual TINT GetNumItemsOfType( const Toshi::TPString8& a_rcType ) = 0; // +0x64

	// Gives up to a_iCount items of the type to a_pTarget, returns the number of given items
	virtual TINT TransferItems( const Toshi::TPString8& a_rcType, AInventoryComponentInterface* a_pTarget, TINT a_iCount ) = 0; // +0x68

	// Gives up to a_iCount items of the slot to a_pTarget, returns the number of given items
	virtual TINT TransferItem( AInventoryItem* a_pItem, AInventoryComponentInterface* a_pTarget, TINT a_iCount ) = 0; // +0x6c

	// Name of the category of items
	virtual const Toshi::TPString8& GetName(); // +0x70

	// The AInventoryGroupComponent of the unit or, if it has none, its AInventoryComponent
	static AInventoryComponentInterface* GetInventory( AUnit* a_pUnit );

public:
	// Lets the group component choose the inventory, DAT_007b4a50
	static Toshi::TPString8 ms_strDefaultInventory;
};
