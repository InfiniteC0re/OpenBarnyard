#pragma once
#include "AInventoryComponent.h"

//-----------------------------------------------------------------------------
// Callbacks of the categories of the inventory of the player ("Hand",
// "Equip" and the inventory screen ones, see APlayerControlComponent): the
// item in the "Hand" is carried, the "Equip" one is used, the others show up
// in the inventory screen.
// NOTE: Vtable 0x00737aa8. APlayerControlComponent.
// keeps one (Create) at +0x7c.
//-----------------------------------------------------------------------------
class APlayerInventoryCallbacks
    : public AInventoryComponent::Callbacks
{
public:
	virtual TBOOL OnRemove( AInventoryItem* a_pItem, AInventoryComponent* a_pInventory, TINT a_iCount ) OVERRIDE;                                            // +0x00
	virtual TBOOL OnAdd( AInventoryItem* a_pItem, AInventoryComponent* a_pInventory, TINT a_iCount ) OVERRIDE;                                               // +0x04
	virtual TBOOL OnCountChanged( AInventoryItem* a_pItem, AInventoryComponent* a_pInventory, TINT a_iCount ) OVERRIDE;                                      // +0x08
	virtual TBOOL OnTransfer( AInventoryItem* a_pItem, AInventoryComponent* a_pInventory, AInventoryComponentInterface* a_pTarget, TINT a_iCount ) OVERRIDE; // +0x0c

	static APlayerInventoryCallbacks* Create();
};
