#pragma once
#include "AInventoryComponentInterface.h"
#include "AInventoryItem.h"

#include <Toshi/TQList.h>

class AInventoryComponentData;

//-----------------------------------------------------------------------------
// A category of items of a unit (see AInventoryComponentData). The items are
// kept in slots (AInventoryItem), one per type, taken from a pool of
// "MaxItems" + 1 free slots.
//-----------------------------------------------------------------------------
class AInventoryComponent
    : public AInventoryComponentInterface
{
public:
	TDECLARE_CLASS( AInventoryComponent, AUnitComponent );

	// Lets the owner of the inventory react to (and veto) the changes of its items.
	// The default ones (0x00778d10) hide the units of the added items.
	// NOTE: Vtable 0x00737bcc.
	class Callbacks
	{
	public:
		// Called before a_iCount items are taken out of the slot, FALSE cancels it
		virtual TBOOL OnRemove( AInventoryItem* a_pItem, AInventoryComponent* a_pInventory, TINT a_iCount ) { return TTRUE; } // +0x00, RETURNS_TRUE 006232a0

		// Called before the slot is added to the inventory, FALSE cancels it
		virtual TBOOL OnAdd( AInventoryItem* a_pItem, AInventoryComponent* a_pInventory, TINT a_iCount ); // +0x04

		// Called once the slot holds a_iCount items after an addition
		virtual TBOOL OnCountChanged( AInventoryItem* a_pItem, AInventoryComponent* a_pInventory, TINT a_iCount ) { return TTRUE; } // +0x08, RETURNS_TRUE 006232a0

		// Called before a_iCount items of the slot are given to a_pTarget, FALSE cancels it
		virtual TBOOL OnTransfer( AInventoryItem* a_pItem, AInventoryComponent* a_pInventory, AInventoryComponentInterface* a_pTarget, TINT a_iCount ) { return TTRUE; } // +0x0c, RETURNS_TRUE 00622750

		// Called once a_iCount items were given to a_pTarget, a_pItem is the slot of the target
		virtual TBOOL OnTransferred( AInventoryItem* a_pItem, AInventoryComponent* a_pInventory, AInventoryComponentInterface* a_pTarget, TINT a_iCount ) { return TTRUE; } // +0x10, RETURNS_TRUE 00622750
	};

	using ItemList = Toshi::TQList<AInventoryItem>;

public:
	AInventoryComponent();
	~AInventoryComponent();

	//-----------------------------------------------------------------------------
	// AUnitComponent
	//-----------------------------------------------------------------------------
	virtual void Save( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE; // +0x14
	virtual void Load( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE; // +0x18

	//-----------------------------------------------------------------------------
	// AInventoryComponentInterface
	//-----------------------------------------------------------------------------
	virtual void                    Clear() OVERRIDE;                                                                                                   // +0x38
	virtual TBOOL                   AddItem( AInventoryItem* a_pItem, Toshi::TPString8 a_strInventory ) OVERRIDE;                                       // +0x3c
	virtual TBOOL                   AddItems( const Toshi::TPString8& a_rcType, TINT a_iCount, Toshi::TPString8 a_strInventory ) OVERRIDE;              // +0x40
	virtual TBOOL                   AddUnit( AUnit* a_pUnit, TINT a_iCount, Toshi::TPString8 a_strInventory ) OVERRIDE;                                 // +0x44
	virtual AUnit*                  DropItem( AInventoryItem* a_pItem ) OVERRIDE;                                                                       // +0x48
	virtual AUnit*                  DropItemOfType( const Toshi::TPString8& a_rcType ) OVERRIDE;                                                        // +0x4c
	virtual TINT                    RemoveItems( const Toshi::TPString8& a_rcType, TINT a_iCount, TBOOL a_bDestroy ) OVERRIDE;                          // +0x50
	virtual AInventoryItem*         RemoveItem( AInventoryItem* a_pItem ) OVERRIDE;                                                                     // +0x54
	virtual AInventoryItem*         FindItemByUnit( AUnit* a_pUnit ) OVERRIDE;                                                                          // +0x58
	virtual AInventoryItem*         FindItemByType( const Toshi::TPString8& a_rcType ) OVERRIDE;                                                        // +0x5c
	virtual TINT                    GetNumItems() OVERRIDE;                                                                                             // +0x60
	virtual TINT                    GetNumItemsOfType( const Toshi::TPString8& a_rcType ) OVERRIDE;                                                     // +0x64
	virtual TINT                    TransferItems( const Toshi::TPString8& a_rcType, AInventoryComponentInterface* a_pTarget, TINT a_iCount ) OVERRIDE; // +0x68
	virtual TINT                    TransferItem( AInventoryItem* a_pItem, AInventoryComponentInterface* a_pTarget, TINT a_iCount ) OVERRIDE;           // +0x6c
	virtual const Toshi::TPString8& GetName() OVERRIDE;                                                                                                 // +0x70

	// Creates the slots of a_pData (and its starting items), owned by a_pOwner
	void Create( AInventoryComponentData* a_pData, AUnit* a_pOwner );

	void SetCallbacks( Callbacks* a_pCallbacks ) { m_pCallbacks = a_pCallbacks; }

	// The a_iIndex-th slot, TNULL if there are fewer
	AInventoryItem* GetItem( TINT a_iIndex );

	// Number of used slots
	TINT GetNumSlots();

	TBOOL ContainsItem( AInventoryItem* a_pItem );
	TBOOL ContainsUnit( AUnit* a_pUnit );
	TBOOL ContainsType( const Toshi::TPString8& a_rcType );

	ItemList& GetItems() { return m_llItems; }

	// $Barnyard: FUNCTION 0052cdb0
	ItemList::TNode::Iterator GetFirstItem() const { return m_llItems.Begin(); }

private:
	// Frees the slots whose unit is gone
	void RemoveEmptyItems();

	// Takes the last free slot, TNULL if there's none
	AInventoryItem* PopFreeItem();

	TBOOL NotifyCountChanged( AInventoryItem* a_pItem, TINT a_iCount );
	TBOOL NotifyTransferred( AInventoryItem* a_pItem, AInventoryComponentInterface* a_pTarget, TINT a_iCount );

public:
	static Callbacks ms_oDefaultCallbacks;

private:
	AInventoryComponentData* m_pData;      // 0x10
	ItemList                 m_llItems;    // 0x14, the used slots
	ItemList                 m_llFree;     // 0x1c, the free slots
	Callbacks*               m_pCallbacks; // 0x24
	TINT                     m_iNumItems;  // 0x28, number of used slots
};

TSTATICASSERT( sizeof( AInventoryComponent ) == 0x2c );
