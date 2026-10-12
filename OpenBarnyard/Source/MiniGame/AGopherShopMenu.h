#pragma once
#include "GUI/AGUI2TextBox.h"
#include "GUI/AGUI2Menu.h"
#include "GUI/AGUI2Button.h"
#include "GUI/AGUI2ModelRenderer.h"

#include <GUI/T2GUIRectangle.h>
#include <Math/TVector2.h>

//-----------------------------------------------------------------------------
// Purpose: the GUI of the gopher shop (AGopherShopGame): the main menu (buy,
// sell, leave), the categories and the item page that shows the items of a
// category as 3D models with their name, count and price.
// The pages pop in and out on the swirl panel.
//-----------------------------------------------------------------------------
class AGopherShopMenu
{
public:
	// NOTE: names are invented
	using PAGE = TINT;
	enum PAGE_ : PAGE
	{
		PAGE_CATEGORIES,
		PAGE_MAIN,
		PAGE_ITEM,
		PAGE_NONE,
	};

	// NOTE: names are invented
	using TRANSITION = TINT;
	enum TRANSITION_ : TRANSITION
	{
		TRANSITION_OPENING,    // the panel pops up
		TRANSITION_CLOSING,    // the panel pops down
		TRANSITION_FADING_IN,  // the page fades in
		TRANSITION_FADING_OUT, // the page fades out
		TRANSITION_CLOSED,
		TRANSITION_NONE, // the page can be used
	};

	// NOTE: names are invented
	using STATUS = TINT;
	enum STATUS_ : STATUS
	{
		STATUS_SOLD_OUT, // text 911
		STATUS_AVAILABLE,
		STATUS_UNAVAILABLE, // text 909
		STATUS_FULL,        // text 911, rotated
		STATUS_EXPENSIVE,
	};

	enum MENU
	{
		MENU_MAIN,
		MENU_CATEGORIES,
		MENU_CONFIRM,
		MENU_NUMOF
	};

	enum BUTTON
	{
		BUTTON_BUY,
		BUTTON_SELL,
		BUTTON_LEAVE,
		BUTTON_CATEGORY_1, // 3-7: categories 1-5
		BUTTON_CATEGORY_0 = 8,
		BUTTON_BACK       = 9, // hidden, the cancel item of the categories
		BUTTON_YES        = 10,
		BUTTON_NO         = 11,
		BUTTON_NUMOF
	};

	static constexpr TINT NUM_TEXTBOXES  = 14;
	static constexpr TINT NUM_RECTANGLES = 11;
	static constexpr TINT NUM_ELEMENTS   = 5;

public:
	AGopherShopMenu();
	~AGopherShopMenu();

	// Creates the GUI on a_pParent
	void Create( Toshi::T2GUIElement* a_pParent );

	// Unlinks every element and releases the font
	void Destroy();

	void  Update( TFLOAT a_fDeltaTime );
	TBOOL ProcessInputEvent( const Toshi::TInputInterface::InputEvent* a_pEvent );
	void  ProcessCommand( AInputCommand a_eCommand, const Toshi::TInputInterface::InputEvent* a_pEvent );

	// Switches to a_ePage (with a transition)
	void SetPage( PAGE a_ePage, TBOOL a_bUnused );

	// $Barnyard: FUNCTION 00497510
	TBOOL IsTransitioning() const { return m_eTransition != TRANSITION_NONE; }

	void SetMoney( TINT a_iMoney );
	void SetDescription( TINT a_iTextId );
	void SetItemName( TINT a_iTextId );
	void SetItemCount( TINT a_iCount );
	void SetPrice( TINT a_iPrice, TINT a_iMultiplier );
	void SetQuantity( TINT a_iQuantity );
	void SetStatus( STATUS a_eStatus, TBOOL a_bAnimate );

	// Flashes the left (a_bLeft) or the right arrow
	void FlashArrow( TBOOL a_bLeft );

	// Shows the yes/no menu of the item page
	void ShowConfirm( TBOOL a_bShow );

	// Shows the quantity of the item page (selling)
	void ShowQuantity( TBOOL a_bShow );

	// $Barnyard: FUNCTION 004969d0
	AGUI2ModelRenderer* GetModelRenderer( TINT a_iIndex ) { return &m_oModelRenderer + a_iIndex; }

	// $Barnyard: FUNCTION 004969f0
	Toshi::T2GUIRectangle* GetRectangle( TINT a_iIndex ) { return &m_aRectangles[ a_iIndex ]; }

	// $Barnyard: FUNCTION 00496a10
	AGUI2Menu* GetMenu( TINT a_iIndex ) { return &m_aMenus[ a_iIndex ]; }

	// $Barnyard: FUNCTION 00496a30
	AGUI2Button* GetButton( TINT a_iIndex ) { return &m_aButtons[ a_iIndex ]; }

	void  SetMode( TINT a_eMode ) { m_eMode = a_eMode; }
	TBOOL IsConfirmShown() const { return m_bConfirmShown; }
	TBOOL IsQuantityShown() const { return m_bQuantityShown; }
	TINT  GetQuantity() const { return m_iQuantity; }

private:
	void CreateBackground();
	void CreateMainPage();
	void CreateCategoryPage();
	void CreateItemPage();
	void CreateButton( Toshi::T2GUIFont* a_pFont, TINT a_iTextId, TINT a_iMenu, TINT a_iButton, TINT a_iRectangle, TFLOAT a_fFocusedScale, TFLOAT a_fScale, TBOOL a_bFitText );

	// Attaches the element of a_ePage to the plate
	void ShowPage( PAGE a_ePage );

	void SetPageAlpha( PAGE a_ePage, TFLOAT a_fAlpha, TFLOAT a_fShadowAlpha, TFLOAT a_fModelSpeed );
	void UpdateTransition( TFLOAT a_fDeltaTime );

private:
	TINT                  m_eMode;                         // 0x0000, AGopherShopGame::MODE
	Toshi::T2GUIElement*  m_pStatus;                       // 0x0004
	TWCHAR                m_wszMoney[ 64 ];                // 0x0008
	TWCHAR                m_wszDescription[ 64 ];          // 0x0088
	TWCHAR                m_wszItemName[ 64 ];             // 0x0108
	TWCHAR                m_wszCount[ 64 ];                // 0x0188
	TWCHAR                m_wszPrice[ 64 ];                // 0x0208
	TWCHAR                m_wszQuantity[ 64 ];             // 0x0288
	TFLOAT                m_fWidth;                        // 0x0308
	TFLOAT                m_fHeight;                       // 0x030c
	Toshi::T2GUIFont*     m_pFont;                         // 0x0310, "rekord26"
	Toshi::T2GUIFont*     m_pSmallFont;                    // 0x0314, "rekord18"
	PAGE                  m_eCurrentPage;                  // 0x0318
	PAGE                  m_eNextPage;                     // 0x031c
	Toshi::T2GUIElement*  m_pPageElement;                  // 0x0320
	Toshi::T2GUIElement*  m_pShownPage;                    // 0x0324
	TRANSITION            m_eTransition;                   // 0x0328
	AGUI2ModelRenderer    m_oModelRenderer;                // 0x032c
	AGUI2TextBox          m_aTextBoxes[ NUM_TEXTBOXES ];   // 0x03ac
	AGUI2Menu             m_aMenus[ MENU_NUMOF ];          // 0x0a3c
	AGUI2Button           m_aButtons[ BUTTON_NUMOF ];      // 0x0c28
	Toshi::T2GUIRectangle m_aRectangles[ NUM_RECTANGLES ]; // 0x2458
	Toshi::T2GUIElement   m_aElements[ NUM_ELEMENTS ];     // 0x2980
	TFLOAT                m_fTime;                         // 0x2b24
	TFLOAT                m_fLeftFlashTime;                // 0x2b28
	TFLOAT                m_fRightFlashTime;               // 0x2b2c
	TBOOL                 m_bLeftFlash;                    // 0x2b30
	TBOOL                 m_bRightFlash;                   // 0x2b31
	TFLOAT                m_fTransitionAngle;              // 0x2b34
	TFLOAT                m_fFade;                         // 0x2b38
	Toshi::TVector2       m_vecPanelSize;                  // 0x2b3c
	TFLOAT                m_fScaleX;                       // 0x2b44
	TFLOAT                m_fScaleY;                       // 0x2b48
	TBOOL                 m_bFlareGrow;                    // 0x2b4c
	TBOOL                 m_bFlareShrink;                  // 0x2b4d
	TFLOAT                m_fFlareScale;                   // 0x2b50
	TBOOL                 m_bStatusPop;                    // 0x2b54
	TFLOAT                m_fStatusPopAngle;               // 0x2b58
	Toshi::T2GUIElement*  m_pParent;                       // 0x2b5c
	TBOOL                 m_bConfirmShown;                 // 0x2b60
	TBOOL                 m_bQuantityShown;                // 0x2b61
	TINT                  m_iQuantity;                     // 0x2b64
};

TSTATICASSERT( sizeof( AGopherShopMenu ) == 0x2b68 );
