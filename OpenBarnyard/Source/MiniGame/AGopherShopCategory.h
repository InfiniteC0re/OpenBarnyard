#pragma once
#include "AGopherShopItem.h"

class AGUI2ModelRenderer;

//-----------------------------------------------------------------------------
// Purpose: a category of the gopher shop: its items are shown in the model
// renderer of the shop menu, the selected one in the middle and its neighbours
// on the sides, scrolling left and right.
//-----------------------------------------------------------------------------
class AGopherShopCategory
{
public:
	static constexpr TINT MAX_ITEMS = 60;

	// NOTE: names are invented
	using SCROLL = TINT;
	enum SCROLL_ : SCROLL
	{
		SCROLL_NONE,
		SCROLL_RIGHT, // the item on the right becomes the selected one
		SCROLL_LEFT,  // the item on the left becomes the selected one
	};

public:
	AGopherShopCategory();

	// Shows the items in the model renderer, the first ones selected
	void Show();

	// Selects the next (a_bRight) or the previous item
	void Scroll( TBOOL a_bRight );

	// $Barnyard: FUNCTION 0049a640
	void ScrollRight() { Scroll( TTRUE ); }

	// $Barnyard: FUNCTION 0049a650
	void ScrollLeft() { Scroll( TFALSE ); }

	// Moves the items while scrolling
	void Update( TFLOAT a_fDeltaTime, TINT a_iUnused );

	void AddItem( AGopherShopItem* a_pItem );

	// Removes a_pItem from the items, TFALSE if it isn't one of them
	TBOOL RemoveItem( AGopherShopItem* a_pItem );

	// Removes a_pItem from the items and keeps it in the removed ones (deleted by DestroyItems)
	void RemoveItemAndKeep( AGopherShopItem* a_pItem );

	// Forgets every item (doesn't delete them)
	void Clear();

	// Deletes every item (and the removed ones)
	void DestroyItems();

	AGopherShopItem* GetCurrentItem();

	// The item whose model is a_strModel
	AGopherShopItem* FindItemByModel( Toshi::TPString8 a_strModel );

	// The item that has the id of a_rcPurchase
	AGopherShopItem* FindItem( const AGopherShopPurchase& a_rcPurchase );

	void SetName( TINT a_iTextId );

	// $Barnyard: FUNCTION 0049a220
	TINT GetTextId() const { return m_iTextId; }

	// $Barnyard: FUNCTION 0049a230
	void SetActive( TBOOL a_bActive ) { m_bActive = a_bActive; }

	// NOTE: folded getter (00555e00)
	TINT GetNumItems() const { return m_iNumItems; }

	// NOTE: folded getter (005fcce0)
	AGopherShopItem* GetItem( TINT a_iIndex ) const { return m_apItems[ a_iIndex ]; }

	TINT GetCurrentIndex() const { return m_iSelected; }

	void SetModelRenderer( AGUI2ModelRenderer* a_pModelRenderer ) { m_pModelRenderer = a_pModelRenderer; }

	// 1 - ( 1 - a_fTime )^4
	static TFLOAT EaseOut( TFLOAT a_fTime );

private:
	SCROLL              m_eScroll;                // 0x000
	TINT                m_iCurrent;               // 0x004
	TINT                m_iLeft;                  // 0x008
	TINT                m_iRight;                 // 0x00c
	AGopherShopItem*    m_pCurrent;               // 0x010
	AGopherShopItem*    m_pLeft;                  // 0x014
	AGopherShopItem*    m_pRight;                 // 0x018
	TBOOL               m_bActive;                // 0x01c
	TINT                m_iNumItems;              // 0x020
	AGopherShopItem*    m_apItems[ MAX_ITEMS ];   // 0x024
	TINT                m_iNumRemoved;            // 0x114
	AGopherShopItem*    m_apRemoved[ MAX_ITEMS ]; // 0x118
	const TWCHAR*       m_wszName;                // 0x208
	TINT                m_iSelected;              // 0x20c
	TINT                m_iTextId;                // 0x210
	AGUI2ModelRenderer* m_pModelRenderer;         // 0x214
};

TSTATICASSERT( sizeof( AGopherShopCategory ) == 0x218 );
