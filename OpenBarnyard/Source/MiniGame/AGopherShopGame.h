#pragma once
#include "GameInterface/AGameState.h"
#include "AGopherShopCategory.h"
#include "GameInterface/AHackedQuickSave.h"
#include "GameInterface/AMessagePopupState.h"

#include <Toshi/TEvent.h>
#include <Math/TVector2.h>
#include <Math/TVector4.h>

class AGopherShopMenu;
class AAnimatableObject;
class AGUI2FadeRectangle;
class AStartMenuState;

//-----------------------------------------------------------------------------
// Purpose: the gopher shop (the black market terrain), opened at night by the
// "startgamestate" function call of an activity. The gopher sells the items of
// six categories (which are saved to AHackedQuickSave::BUFFER_OFFERS, see
// AGopherShopInterface) and buys the items of the inventory back, the one of
// Joey's offer for twice the price. Leaving it adds the bought items to the
// inventory, unlocks the bikes and returns to the sim.
// TClass 00782bd4, vtable 0071eaa0, size 0x2084.
//-----------------------------------------------------------------------------
class AGopherShopGame
    : public AGameState
{
public:
	TDECLARE_CLASS( AGopherShopGame, AGameState );

	static constexpr TINT MAX_PURCHASES  = 100;
	static constexpr TINT MAX_OWNED      = 100;
	static constexpr TINT NUM_CATEGORIES = 7;

	// The category of the items of the inventory that can be sold
	static constexpr TINT CATEGORY_SELL = 6;

	// NOTE: names are invented
	using MENU = TINT;
	enum MENU_ : MENU
	{
		MENU_MAIN,
		MENU_CATEGORIES,
		MENU_ITEM,
	};

	// NOTE: names are invented
	using MODE = TINT;
	enum MODE_ : MODE
	{
		MODE_BUY,
		MODE_SELL,
	};

public:
	AGopherShopGame();
	~AGopherShopGame();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL    ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE;                                                        // +0x0c
	virtual TBOOL    ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual void     OnLoaded() OVERRIDE;                                                                                                                     // +0x24
	virtual TBOOL    ShouldPlacePlayerAtStartGoal() OVERRIDE { return TFALSE; }                                                                               // +0x28, RETURNS_FALSE
	virtual ASoundId GetSound() OVERRIDE;                                                                                                                     // +0x2c
	virtual TBOOL    OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x4c
	virtual void     OnInsertion() OVERRIDE;                                                                                                                  // +0x50
	virtual void     OnRemoval() OVERRIDE;                                                                                                                    // +0x54
	virtual void     OnActivate() OVERRIDE;                                                                                                                   // +0x60
	virtual void     OnDeactivate() OVERRIDE;                                                                                                                 // +0x64

	// Buys a_pItem
	void Buy( AGopherShopItem* a_pItem );

	// Leaves the shop: adds the bought items to the inventory and saves
	void Exit();

	AGopherShopItem* GetCurrentItem() const { return m_pCurrentItem; }
	MENU             GetMenu() const { return m_eMenu; }
	MODE             GetMode() const { return m_eMode; }
	TBOOL            IsExiting() const { return m_bExiting; }
	AGopherShopMenu* GetShopMenu() const { return m_pMenu; }

#ifdef BARNYARD_COMMUNITY_PATCH
	// Development: the item "-devgophershop" bought, -1 if none
	inline static TINT ms_iDevBoughtId = -1;
#endif

private:
	// The locale id of the name of category a_iCategory
	static TINT GetCategoryTextId( TINT a_iCategory );

	void InitCategories();
	void CreateGopher();
	void CreateMenu();

	// Loads the price list and creates the items
	TBOOL LoadPriceList();

	// Counts the items that can be bought
	void CountAvailableItems();

	void Save( AHackedQuickSave::Buffer* a_pBuffer );
	void Load( AHackedQuickSave::Buffer* a_pBuffer );

	// Shows the items of a_iCategory
	void SetCategory( TINT a_iCategory );

	// Switches to the page of the menu
	void SetPage( TINT a_ePage );

	// Shows the selected item (and its status if a_bUpdateStatus)
	void UpdateItemInfo( TBOOL a_bUpdateStatus );

	// Shows the arrows if there are items to scroll to
	void UpdateArrows( AGopherShopCategory* a_pCategory );

	// Finds the arrows and the model under the mouse
	void UpdateMouse( TFLOAT a_fDeltaTime );

	// 2 if a_pItem is the item of Joey's offer, 1 otherwise
	TINT GetPriceMultiplier( AGopherShopItem* a_pItem, TINT a_iUnused );

	// At least one of the items a_pItem requires has been bought (or it doesn't require any)
	TBOOL HasRequiredItems( AGopherShopItem* a_pItem );

	// Bought 20 of a_pItem already
	TBOOL IsFull( AGopherShopItem* a_pItem );

	// Finds the purchase of the item a_iId, copies it to a_pOutPurchase
	TBOOL FindPurchase( TINT a_iId, AGopherShopPurchase* a_pOutPurchase );

	// Index of the purchase that has the id of a_rcPurchase, m_iNumPurchases if there's none
	TINT FindPurchaseIndex( const AGopherShopPurchase& a_rcPurchase );

	// Removes the purchase at a_iIndex
	void RemovePurchase( TINT a_iIndex );

	// Sells one a_pItem for a_iMultiplier times its price
	void Sell( AGopherShopItem* a_pItem, TINT a_iMultiplier, TBOOL a_bPlaySound );

	// Sells a_iCount of a_pItem
	void SellMultiple( AGopherShopItem* a_pItem, TINT a_iCount );

	void OnMainMenuItem( AGUI2MenuItem* a_pItem );
	void OnCategoryMenuItem( AGUI2MenuItem* a_pItem );
	void OnMenuItemActivated( AGUI2MenuItem* a_pItem );

	static void OnMenuItemActivatedCallback( void* a_pUserData, AGUI2MenuItem* a_pItem );

#ifdef BARNYARD_COMMUNITY_PATCH
	// Development: "-devgophershop" buys an item and leaves
	void Dev_Update( TFLOAT a_fDeltaTime );
#endif
	static TBOOL __stdcall OnQuitCallback( AStartMenuState* a_pStartMenu, void* a_pUserData, AMessagePopupState* a_pPopup, AMessagePopupState::ResponseEvent* a_pEvent );

private:
	TFLOAT                  m_fInputTime;                    // 0x00a4
	TUINT8                  m_aUnkA8[ 0x100 ];               // 0x00a8
	MENU                    m_eMenu;                         // 0x01a8
	MODE                    m_eMode;                         // 0x01ac
	TBOOL                   m_bLeft;                         // 0x01b0
	TBOOL                   m_bRight;                        // 0x01b1
	TBOOL                   m_bUnk1B2;                       // 0x01b2
	TBOOL                   m_bUnk1B3;                       // 0x01b3
	TBOOL                   m_bConfirm;                      // 0x01b4
	TBOOL                   m_bCancel;                       // 0x01b5
	TINT                    m_iNumPurchases;                 // 0x01b8
	AGopherShopPurchase     m_aPurchases[ MAX_PURCHASES ];   // 0x01bc
	TINT                    m_iNumOwned;                     // 0x0fcc
	AGopherShopItem*        m_apOwned[ MAX_OWNED ];          // 0x0fd0, the items of the inventory (category 3)
	AGopherShopItem*        m_pCurrentItem;                  // 0x1160
	TINT                    m_iUnk1164;                      // 0x1164
	TINT                    m_iCategory;                     // 0x1168
	AGopherShopCategory     m_aCategories[ NUM_CATEGORIES ]; // 0x116c
	AGopherShopMenu*        m_pMenu;                         // 0x2014
	TINT                    m_iUnk2018;                      // 0x2018
	TINT                    m_iUnk201C;                      // 0x201c
	TINT                    m_iUnk2020;                      // 0x2020
	AAnimatableObject*      m_pGopher;                       // 0x2024
	Toshi::TVector2         m_vecStick;                      // 0x2028
	TBOOL                   m_bStick;                        // 0x2030
	TBOOL                   m_bFirstVisit;                   // 0x2031
	TINT                    m_iNumAvailable;                 // 0x2034
	TINT                    m_iSavedNumAvailable;            // 0x2038
	TBOOL                   m_bBought;                       // 0x203c
	TINT                    m_iCue;                          // 0x2040
	TINT                    m_iIdleCue;                      // 0x2044
	Toshi::TVector4         m_vecGopherPos;                  // 0x2048
	TFLOAT                  m_fTalkTime;                     // 0x2058
	TFLOAT                  m_fTime;                         // 0x205c
	TBOOL                   m_bExiting;                      // 0x2060
	TBOOL                   m_bShowingItem;                  // 0x2061
	AGUI2FadeRectangle*     m_pFade;                         // 0x2064
	TBOOL                   m_bMouseOnLeftArrow;             // 0x2068
	TBOOL                   m_bMouseOnRightArrow;            // 0x2069
	TBOOL                   m_bMouseOnModel;                 // 0x206a
	TUINT8                  m_uiJoeyOffer;                   // 0x206b, 0xff if none
	Toshi::TGenericListener m_oListener;                     // 0x206c, never connected
	TBOOL                   m_bDiscount;                     // 0x2080, item 896 was bought: 10% off
};

TSTATICASSERT( sizeof( AGopherShopGame ) == 0x2084 );
