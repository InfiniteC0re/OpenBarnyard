#pragma once
#include "AInventoryComponentInterface.h"
#include "AInventoryComponent.h"

class AInventoryGroupComponentData;
class AGameState;

//-----------------------------------------------------------------------------
// The items of a unit split into categories, one AInventoryComponent per
// category of its AInventoryGroupComponentData (the player has "Hand" for the
// carried item and the categories of the inventory screen).
// The interface methods go to the category that has the item (or the type).
//-----------------------------------------------------------------------------
class AInventoryGroupComponent
    : public AInventoryComponentInterface
{
public:
	TDECLARE_CLASS( AInventoryGroupComponent, AUnitComponent );

public:
	AInventoryGroupComponent();
	~AInventoryGroupComponent();

	//-----------------------------------------------------------------------------
	// AUnitComponent
	//-----------------------------------------------------------------------------
	virtual void Save( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE; // +0x14
	virtual void Load( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE; // +0x18

	//-----------------------------------------------------------------------------
	// AInventoryComponentInterface
	//-----------------------------------------------------------------------------
	virtual void            Clear() OVERRIDE;                                                                                                   // +0x38
	virtual TBOOL           AddItem( AInventoryItem* a_pItem, Toshi::TPString8 a_strInventory ) OVERRIDE;                                       // +0x3c
	virtual TBOOL           AddItems( const Toshi::TPString8& a_rcType, TINT a_iCount, Toshi::TPString8 a_strInventory ) OVERRIDE;              // +0x40
	virtual TBOOL           AddUnit( AUnit* a_pUnit, TINT a_iCount, Toshi::TPString8 a_strInventory ) OVERRIDE;                                 // +0x44
	virtual AUnit*          DropItem( AInventoryItem* a_pItem ) OVERRIDE;                                                                       // +0x48
	virtual AUnit*          DropItemOfType( const Toshi::TPString8& a_rcType ) OVERRIDE;                                                        // +0x4c
	virtual TINT            RemoveItems( const Toshi::TPString8& a_rcType, TINT a_iCount, TBOOL a_bDestroy ) OVERRIDE;                          // +0x50
	virtual AInventoryItem* RemoveItem( AInventoryItem* a_pItem ) OVERRIDE;                                                                     // +0x54
	virtual AInventoryItem* FindItemByUnit( AUnit* a_pUnit ) OVERRIDE;                                                                          // +0x58
	virtual AInventoryItem* FindItemByType( const Toshi::TPString8& a_rcType ) OVERRIDE;                                                        // +0x5c
	virtual TINT            GetNumItems() OVERRIDE;                                                                                             // +0x60
	virtual TINT            GetNumItemsOfType( const Toshi::TPString8& a_rcType ) OVERRIDE;                                                     // +0x64
	virtual TINT            TransferItems( const Toshi::TPString8& a_rcType, AInventoryComponentInterface* a_pTarget, TINT a_iCount ) OVERRIDE; // +0x68
	virtual TINT            TransferItem( AInventoryItem* a_pItem, AInventoryComponentInterface* a_pTarget, TINT a_iCount ) OVERRIDE;           // +0x6c

	// Creates a category per category of a_pData
	void Create( AInventoryGroupComponentData* a_pData );

	// The category named a_rcName, the first one if a_rcName is empty, TNULL if there's no such category
	AInventoryComponent* GetCategory( const Toshi::TPString8& a_rcName );

	// DropItemOfType for the category named a_rcCategory
	AUnit* DropItemOfType( const Toshi::TPString8& a_rcType, const Toshi::TPString8& a_rcCategory );

	// Removes the slot of the unit from its category
	AInventoryItem* RemoveUnit( AUnit* a_pUnit );

	// The category that has the slot/type, TNULL if none has it
	AInventoryComponent* FindCategoryWithItem( AInventoryItem* a_pItem );
	AInventoryComponent* FindCategoryWithType( const Toshi::TPString8& a_rcType );

	TBOOL ContainsItem( AInventoryItem* a_pItem );
	TBOOL ContainsUnit( AUnit* a_pUnit );

	// Moves the item between the "InventoryForUse" and the "InventoryForStore" of its AObjectTraits.
	// Only one item goes back to the store inventory. Returns TRUE if every item was moved.
	TBOOL SwapItemCategory( AInventoryItem* a_pItem, TINT a_iCount );

	// Moves a_iCount items to the "InventoryForUse" of their AObjectTraits, TRUE if they were moved
	TBOOL MoveItemToUseCategory( AInventoryItem* a_pItem, TINT a_iCount );

	// Drops the selected item if the inventory still has it, the selection is cleared
	AUnit* DropSelectedItem();

	void SetSelectedItem( AInventoryItem* a_pItem ) { m_pSelectedItem = a_pItem; } // folded setter 00566c60

	TINT                 GetNumCategories() const;
	AInventoryComponent* GetCategory( TINT a_iIndex ) const { return &m_pCategories[ a_iIndex ]; }

	// Static helpers working on the inventory of the player

	// Plays the "putaway" animation of the player if the item in the "Hand" can be stored
	static TBOOL PutAwayHandItem();

	// Plays the "getiteminv" effect of the item taken out of the inventory of the player
	static TBOOL TakeOutItem( AInventoryItem* a_pItem );

	// TakeOutItem that shows the item in the inventory screen, which is created as a child of a_pState
	// (or of the sim) if there's none
	static TBOOL TakeOutItem( AInventoryItem* a_pItem, AGameState* a_pState );

	// Flies a_pUnit into the pick up counter (pick ups) or the inventory screen and collects it into the
	// inventory of the player. The inventory screen is added to a_pParentState (if TNULL, to ASimState or
	// ABarnExploreMinigameState on the stack). Returns TFALSE if there's no room for the effects.
	static TBOOL CollectUnit( AUnit* a_pUnit, AGameState* a_pParentState );

	// Shows the unit taken out of the inventory over the player
	static void ShowTakenOutUnit( AUnit* a_pUnit );

private:
	AInventoryGroupComponentData* m_pData;         // 0x10
	AInventoryComponent*          m_pCategories;   // 0x14
	AInventoryItem*               m_pSelectedItem; // 0x18, see DropSelectedItem
};

TSTATICASSERT( sizeof( AInventoryGroupComponent ) == 0x1c );
