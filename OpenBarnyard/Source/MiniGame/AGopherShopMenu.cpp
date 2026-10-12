#include "pch.h"
#include "AGopherShopMenu.h"
#include "AGopherShopGame.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AModel.h"
#include "HUD/AHudManager.h"
#include "HUD/AMenuNavHudElement.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Render/TCameraObject.h>
#include <Math/TMatrix44.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>
#include "SoundBank/ui.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The text boxes
// NOTE: names are invented
enum
{
	TEXTBOX_WOOD_TITLE, // 898
	TEXTBOX_DESCRIPTION,
	TEXTBOX_MONEY_TITLE, // 900
	TEXTBOX_MONEY,
	TEXTBOX_NAME,
	TEXTBOX_COUNT,
	TEXTBOX_SELL_PRICE, // 905
	TEXTBOX_PRICE,
	TEXTBOX_QUANTITY_TITLE, // 897
	TEXTBOX_SOLD_OUT,       // 911
	TEXTBOX_UNAVAILABLE,    // 909
	TEXTBOX_FULL,           // 911
	TEXTBOX_QUANTITY,
	TEXTBOX_CONFIRM, // 899
};

// The rectangles
enum
{
	RECTANGLE_PANEL,       // "Swirl_Base"
	RECTANGLE_WOOD,        // "Wood_Tall"
	RECTANGLE_WOOD_MONEY,  // "Wood_Tall"
	RECTANGLE_LEFT_ARROW,  // "left_arrow"
	RECTANGLE_RIGHT_ARROW, // "left_arrow2"
	RECTANGLE_NEGATIVE,    // "Negative"
	RECTANGLE_FLARE_1,     // "Flare"
	RECTANGLE_FLARE_2,
	RECTANGLE_FLARE_3,
	RECTANGLE_PHONE, // "Reward_Phone"
	RECTANGLE_PLATE, // "Gopher_Plate"
};

// The elements
enum
{
	ELEMENT_MAIN,
	ELEMENT_CATEGORIES,
	ELEMENT_ITEM,
	ELEMENT_UNUSED,
	ELEMENT_SELL_PRICE,
};

static constexpr TUINT32 COLOUR_TITLE  = 0xfffff0af;
static constexpr TUINT32 COLOUR_TEXT   = 0xffffc800;
static constexpr TUINT32 COLOUR_GREY   = 0xff4b4b4b;
static constexpr TUINT32 COLOUR_RED    = 0xffff0500;
static constexpr TUINT32 COLOUR_BUTTON = 0xffb56f22;
static constexpr TUINT32 COLOUR_WOOD   = 0xe0ffffff;
static constexpr TUINT32 COLOUR_SHADOW = 0xa0000000;

// The rectangle the pages are attached to (DAT_00782c08), set by CreateBackground
static TINT s_iPlateRectangle;

// The gap of the buttons created by CreateButton (DAT_00782c0c)
static TFLOAT s_fButtonGap;

// The "Negative" rectangle while it's shown (DAT_00782c10)
static T2GUIElement* s_pNegative;

// NOTE: retail shares these with ABYardMenuState (DAT_0078198c, DAT_00781990)
static TFLOAT s_fOscillatorPeriod    = TMath::PI / 1.5f;
static TFLOAT s_fOscillatorAmplitude = 2.0f - TMath::Sin( s_fOscillatorPeriod );

// Width of a space (T2GUIFont::GetCharWidth)
static TFLOAT GetSpaceWidth( T2GUIFont* a_pFont )
{
	return a_pFont->GetTextWidth( L" ", 1, 1.0f );
}

// $Barnyard: FUNCTION 00496690
AGopherShopMenu::AGopherShopMenu()
    : m_eMode( 0 )
    , m_pStatus( TNULL )
    , m_fWidth( 0.0f )
    , m_fHeight( 0.0f )
    , m_pFont( TNULL )
    , m_pSmallFont( TNULL )
    , m_eCurrentPage( PAGE_NONE )
    , m_eNextPage( PAGE_NONE )
    , m_pPageElement( TNULL )
    , m_pShownPage( TNULL )
    , m_eTransition( TRANSITION_CLOSED )
    , m_fTime( 0.0f )
    , m_bLeftFlash( TFALSE )
    , m_bRightFlash( TFALSE )
    , m_fTransitionAngle( 0.0f )
    , m_fFade( 0.0f )
    , m_vecPanelSize( TVector2::VEC_ZERO )
    , m_bFlareGrow( TFALSE )
    , m_bFlareShrink( TFALSE )
    , m_fFlareScale( 0.1f )
    , m_bStatusPop( TFALSE )
    , m_pParent( TNULL )
    , m_bConfirmShown( TFALSE )
    , m_bQuantityShown( TFALSE )
{
}

// $Barnyard: FUNCTION 004975e0
AGopherShopMenu::~AGopherShopMenu()
{
	Destroy();
}

// $Barnyard: FUNCTION 00496910
void AGopherShopMenu::Destroy()
{
	m_oModelRenderer.RemoveAllObjects();
	m_oModelRenderer.RemoveSelf();

	for ( TINT i = 0; i < NUM_ELEMENTS; i++ )
		m_aElements[ i ].RemoveSelf();

	for ( TINT i = 0; i < NUM_TEXTBOXES; i++ )
		m_aTextBoxes[ i ].RemoveSelf();

	for ( TINT i = 0; i < MENU_NUMOF; i++ )
	{
		m_aMenus[ i ].RemoveAllItems();
		m_aMenus[ i ].RemoveSelf();
	}

	for ( TINT i = 0; i < NUM_RECTANGLES; i++ )
		m_aRectangles[ i ].RemoveSelf();

	T2GUIFontManager::ReleaseFont( m_pFont );
}

// $Barnyard: FUNCTION 00498dd0
void AGopherShopMenu::Create( T2GUIElement* a_pParent )
{
	m_pFont      = T2GUIFontManager::FindFont( "rekord26" );
	m_pSmallFont = T2GUIFontManager::FindFont( "rekord18" );
	m_pParent    = a_pParent;
	a_pParent->GetDimensions( m_fWidth, m_fHeight );

	CreateBackground();
	CreateMainPage();
	CreateCategoryPage();
	CreateItemPage();

	for ( TINT i = 0; i < NUM_TEXTBOXES; i++ )
	{
		m_aTextBoxes[ i ].SetShadow( TTRUE, COLOUR_SHADOW );
		m_aTextBoxes[ i ].SetShadowAlpha( 0.6f );
		m_aTextBoxes[ i ].SetShadowOffset( 5.0f, 5.0f );
	}

	for ( TINT i = 0; i < BUTTON_NUMOF; i++ )
	{
		AGUI2TextBox& rText = m_aButtons[ i ].GetTextBox();
		rText.SetShadow( TTRUE, COLOUR_SHADOW );
		rText.SetShadowAlpha( 0.6f );

		// NOTE: retail uses the scale of the text box of the same index, not the one of the button
		TFLOAT fOffset = m_aTextBoxes[ i ].GetScale() * 5.0f;
		rText.SetShadowOffsetUnscaled( fOffset, fOffset );
	}
}

// $Barnyard: FUNCTION 00496830
void AGopherShopMenu::CreateButton( T2GUIFont* a_pFont, TINT a_iTextId, TINT a_iMenu, TINT a_iButton, TINT a_iRectangle, TFLOAT a_fFocusedScale, TFLOAT a_fScale, TBOOL a_bFitText )
{
	TFLOAT fWidth = m_aRectangles[ a_iRectangle ].GetWidth();

	if ( a_bFitText )
		fWidth = a_pFont->GetTextWidth( ALocaleManager::Interface()->GetString( a_iTextId ), 1.0f );

	AGUI2Button& rButton = m_aButtons[ a_iButton ];
	rButton.Create( a_pFont, fWidth, ALocaleManager::Interface()->GetString( a_iTextId ), s_fButtonGap );
	rButton.SetFocusedColour( 0xffffffff );
	rButton.SetEnabledColour( COLOUR_BUTTON );
	rButton.SetFocusedScale( a_fFocusedScale );
	rButton.SetScale( a_fScale );
	rButton.OnFocusLost();

	m_aMenus[ a_iMenu ].AddMenuItem( rButton );
}

// $Barnyard: FUNCTION 004976d0
void AGopherShopMenu::CreateBackground()
{
	T2GUIElement* pParent = m_pParent;

	// The wood with the description
	TFLOAT fWoodWidth  = m_fWidth * 0.5f;
	TFLOAT fWoodHeight = m_fHeight * 0.15f;

	T2GUITextureSection* pWoodSection = T2GUITextureSectionManager::GetTextureSection( "Wood_Tall" );
	T2GUIRectangle&      rWood        = m_aRectangles[ RECTANGLE_WOOD ];
	rWood.SetDimensions( fWoodWidth, fWoodHeight );
	rWood.SetColour( COLOUR_WOOD );
	rWood.SetAttachment( T2GUIATTACHMENT( 0 ), T2GUIATTACHMENT( 0 ) );
	rWood.SetTextureSection( pWoodSection );
	rWood.SetTransform( 0.0f, fWoodHeight * 0.15f, 0.0f );
	pParent->AddChildTail( rWood );

	AGUI2TextBox& rWoodTitle = m_aTextBoxes[ TEXTBOX_WOOD_TITLE ];
	rWoodTitle.Create( m_pFont, fWoodWidth * 0.9f );
	rWoodTitle.SetScale( 1.0f );
	rWoodTitle.SetAttachment( T2GUIATTACHMENT( 1 ), T2GUIATTACHMENT( 1 ) );
	rWoodTitle.SetTextAlign( T2GUIFont::TextAlign( 1 ) );
	rWoodTitle.SetText( ALocaleManager::Interface()->GetString( 898 ) );
	rWoodTitle.SetColour( COLOUR_TITLE );
	rWoodTitle.SetTransform( 0.0f, m_pFont->GetLineHeight() * 0.25f, 0.0f );

	TFLOAT fRatio = rWood.GetWidth() / m_pFont->GetTextWidth( rWoodTitle.GetText(), 1.0f );
	rWoodTitle.SetScale( ( fRatio >= 1.0f ) ? 1.0f : fRatio * 0.9f );
	rWood.AddChildTail( rWoodTitle );

	AGUI2TextBox& rDescription = m_aTextBoxes[ TEXTBOX_DESCRIPTION ];
	rDescription.Create( m_pFont, fWoodWidth );
	rDescription.SetAttachment( T2GUIATTACHMENT( 7 ), T2GUIATTACHMENT( 7 ) );
	rDescription.SetTextAlign( T2GUIFont::TextAlign( 1 ) );
	rDescription.SetText( TNULL );
	rDescription.SetColour( COLOUR_TEXT );
	rDescription.SetScale( 1.0f );
	rDescription.SetAlpha( 0.0f );
	rDescription.SetShadowAlpha( 0.0f );
	rDescription.SetTransform( 0.0f, m_pFont->GetLineHeight() * -0.25f, 0.0f );
	rWood.AddChildTail( rDescription );

	// The panel
	TFLOAT fPanelWidth  = m_fWidth * 0.5f;
	TFLOAT fPanelHeight = m_fHeight * 0.5f;
	m_vecPanelSize.x    = fPanelWidth;
	m_vecPanelSize.y    = fPanelHeight;

	T2GUIRectangle& rPanel = m_aRectangles[ RECTANGLE_PANEL ];
	rPanel.SetDimensions( fPanelWidth, fPanelHeight );
	rPanel.SetAttachment( T2GUIATTACHMENT( 3 ), T2GUIATTACHMENT( 3 ) );
	rPanel.SetTextureSection( T2GUITextureSectionManager::GetTextureSection( "Swirl_Base" ) );
	pParent->AddChildTail( rPanel );

	T2GUITextureSection* pPlateSection = T2GUITextureSectionManager::GetTextureSection( "Gopher_Plate" );
	T2GUIRectangle&      rPlate        = m_aRectangles[ RECTANGLE_PLATE ];
	rPlate.SetDimensions( rPanel.GetWidth(), rPanel.GetHeight() );
	rPlate.SetTextureSection( pPlateSection );
	rPlate.SetAttachment( T2GUIATTACHMENT( 4 ), T2GUIATTACHMENT( 4 ) );
	rPanel.AddChildTail( rPlate );

	// The wood with the money
	TFLOAT fMoneyWidth  = m_fWidth * 0.4f;
	s_iPlateRectangle   = RECTANGLE_PLATE;
	TFLOAT fMoneyHeight = m_fHeight * 0.15f;

	T2GUIRectangle& rMoneyWood = m_aRectangles[ RECTANGLE_WOOD_MONEY ];
	rMoneyWood.SetDimensions( fMoneyWidth, fMoneyHeight );
	rMoneyWood.SetColour( COLOUR_WOOD );
	rMoneyWood.SetAttachment( T2GUIATTACHMENT( 2 ), T2GUIATTACHMENT( 2 ) );

	TFLOAT fMoneyY = fMoneyWidth * 0.15f;
	rMoneyWood.SetTransform( 0.0f, fMoneyY, 0.0f );
	rMoneyWood.SetTransform( fPanelWidth * -0.1f, fMoneyY, 0.0f );
	rMoneyWood.SetTextureSection( T2GUITextureSectionManager::GetTextureSection( "Wood_Tall" ) );
	pParent->AddChildTail( rMoneyWood );

	AGUI2TextBox& rMoneyTitle = m_aTextBoxes[ TEXTBOX_MONEY_TITLE ];
	rMoneyTitle.Create( m_pFont, fPanelWidth );
	rMoneyTitle.SetAttachment( T2GUIATTACHMENT( 1 ), T2GUIATTACHMENT( 1 ) );
	rMoneyTitle.SetTextAlign( T2GUIFont::TextAlign( 1 ) );
	rMoneyTitle.SetText( ALocaleManager::Interface()->GetString( 900 ) );
	rMoneyTitle.SetColour( COLOUR_TITLE );
	rMoneyTitle.SetScale( 1.0f );
	rMoneyTitle.SetTransform( 0.0f, m_pFont->GetLineHeight() * 0.25f, 0.0f );

	fRatio = rMoneyWood.GetWidth() / m_pFont->GetTextWidth( rMoneyTitle.GetText(), 1.0f );
	rMoneyTitle.SetScale( ( fRatio >= 1.0f ) ? 1.0f : fRatio * 0.9f );
	rMoneyWood.AddChildTail( rMoneyTitle );

	AGUI2TextBox& rMoney = m_aTextBoxes[ TEXTBOX_MONEY ];
	rMoney.Create( m_pFont, fPanelWidth );
	rMoney.SetAttachment( T2GUIATTACHMENT( 7 ), T2GUIATTACHMENT( 7 ) );
	rMoney.SetTextAlign( T2GUIFont::TextAlign( 1 ) );
	rMoney.SetText( TNULL );
	rMoney.SetColour( COLOUR_TEXT );
	rMoney.SetScale( 1.0f );
	rMoney.SetTransform( 0.0f, m_pFont->GetLineHeight() * -0.25f, 0.0f );
	rMoneyWood.AddChildTail( rMoney );

	// The statuses of the item
	TFLOAT fStatusWidth = rPanel.GetWidth();

	AGUI2TextBox& rSoldOut = m_aTextBoxes[ TEXTBOX_SOLD_OUT ];
	rSoldOut.Create( m_pFont, fStatusWidth );
	rSoldOut.SetText( ALocaleManager::Interface()->GetString( 911 ) );
	rSoldOut.SetScale( 1.0f );
	rSoldOut.SetAttachment( T2GUIATTACHMENT( 7 ), T2GUIATTACHMENT( 7 ) );
	rSoldOut.SetTransform( 0.0f, m_pFont->GetLineHeight() * -0.5f, 0.0f );
	rSoldOut.SetColour( COLOUR_RED );
	rSoldOut.SetAlpha( 0.0f );
	rSoldOut.SetShadowAlpha( 0.0f );
	rSoldOut.Hide();

	AGUI2TextBox& rUnavailable = m_aTextBoxes[ TEXTBOX_UNAVAILABLE ];
	rUnavailable.Create( m_pFont, fStatusWidth );
	rUnavailable.SetScale( 1.0f );
	rUnavailable.SetText( ALocaleManager::Interface()->GetString( 909 ) );
	rUnavailable.SetAttachment( T2GUIATTACHMENT( 7 ), T2GUIATTACHMENT( 7 ) );
	rUnavailable.SetTransform( 0.0f, m_pFont->GetLineHeight() * -0.5f, 0.0f );
	rUnavailable.SetColour( COLOUR_RED );
	rUnavailable.SetAlpha( 0.0f );
	rUnavailable.SetShadowAlpha( 0.0f );
	rUnavailable.Hide();

	AGUI2TextBox& rFull = m_aTextBoxes[ TEXTBOX_FULL ];
	rFull.Create( m_pFont, fStatusWidth * 0.5f );
	rFull.SetText( ALocaleManager::Interface()->GetString( 911 ) );
	rFull.SetScale( 1.0f );
	rFull.SetTransform( 0.0f, 0.0f, -0.78539819f );
	rFull.SetColour( COLOUR_RED );

	m_fScaleX = m_fWidth * 0.00125f;
	m_fScaleY = m_fHeight * 0.0016666667f;

	T2GUITextureSection* pNegativeSection = T2GUITextureSectionManager::GetTextureSection( "Negative" );
	T2GUIRectangle&      rNegative        = m_aRectangles[ RECTANGLE_NEGATIVE ];
	rNegative.SetDimensions(
	    TMath::Abs( TFLOAT( pNegativeSection->GetPoint2X() - pNegativeSection->GetPoint1X() ) ) * m_fScaleX,
	    TMath::Abs( TFLOAT( pNegativeSection->GetPoint2Y() - pNegativeSection->GetPoint1Y() ) ) * m_fScaleY
	);
	rNegative.Hide();
	rNegative.SetTextureSection( pNegativeSection );
}

// $Barnyard: FUNCTION 00497d70
void AGopherShopMenu::CreateMainPage()
{
	T2GUIRectangle& rPanel  = m_aRectangles[ RECTANGLE_PANEL ];
	TFLOAT          fWidth  = rPanel.GetWidth();
	TFLOAT          fHeight = rPanel.GetHeight();

	T2GUIElement& rPage = m_aElements[ ELEMENT_MAIN ];
	rPage.SetAttachment( T2GUIATTACHMENT( 0 ), T2GUIATTACHMENT( 0 ) );
	rPage.SetDimensions( fWidth, fHeight );

	CreateButton( m_pFont, 908, MENU_MAIN, BUTTON_BUY, s_iPlateRectangle, 1.0f, 1.0f, TFALSE );
	CreateButton( m_pFont, 907, MENU_MAIN, BUTTON_SELL, s_iPlateRectangle, 1.0f, 1.0f, TFALSE );
	CreateButton( m_pFont, 906, MENU_MAIN, BUTTON_LEAVE, s_iPlateRectangle, 1.0f, 1.0f, TFALSE );

	AGUI2Menu& rMenu = m_aMenus[ MENU_MAIN ];
	rMenu.SetCancelItem( &m_aButtons[ BUTTON_LEAVE ] );
	rMenu.SetDefaultFocusElement( &m_aButtons[ BUTTON_BUY ] );
	rMenu.SetItemSpacing( 0.0f );
	rPage.AddChildTail( rMenu );
	rMenu.ReflowChildrenVertically();
	rMenu.SetFocusSound( soundbank::UI_MENUCLICK );
	rMenu.SetSelectSound( soundbank::UI_MENUOK );
	rMenu.SetBackSound( soundbank::UI_MENUBACK );
	rMenu.SetNegativeSound( soundbank::UI_NEGATIVE );
}

// $Barnyard: FUNCTION 00497eb0
void AGopherShopMenu::CreateCategoryPage()
{
	T2GUIRectangle& rPanel  = m_aRectangles[ RECTANGLE_PANEL ];
	TFLOAT          fWidth  = rPanel.GetWidth();
	TFLOAT          fHeight = rPanel.GetHeight();

	T2GUIElement& rPage = m_aElements[ ELEMENT_CATEGORIES ];
	rPage.SetAttachment( T2GUIATTACHMENT( 0 ), T2GUIATTACHMENT( 0 ) );
	rPage.SetDimensions( fWidth, fHeight );

	CreateButton( m_pFont, 977, MENU_CATEGORIES, BUTTON_CATEGORY_0, s_iPlateRectangle, 0.8f, 0.8f, TFALSE );
	CreateButton( m_pFont, 978, MENU_CATEGORIES, BUTTON_CATEGORY_1, s_iPlateRectangle, 0.8f, 0.8f, TFALSE );
	CreateButton( m_pFont, 979, MENU_CATEGORIES, BUTTON_CATEGORY_1 + 1, s_iPlateRectangle, 0.8f, 0.8f, TFALSE );
	CreateButton( m_pFont, 980, MENU_CATEGORIES, BUTTON_CATEGORY_1 + 2, s_iPlateRectangle, 0.8f, 0.8f, TFALSE );
	CreateButton( m_pFont, 981, MENU_CATEGORIES, BUTTON_CATEGORY_1 + 3, s_iPlateRectangle, 0.8f, 0.8f, TFALSE );
	CreateButton( m_pFont, 982, MENU_CATEGORIES, BUTTON_CATEGORY_1 + 4, s_iPlateRectangle, 0.8f, 0.8f, TFALSE );

	// Make the texts of the categories fit
	TFLOAT fMaxWidth    = 0.0f;
	TFLOAT fTotalHeight = 0.0f;

	for ( TINT i = BUTTON_CATEGORY_1; i <= BUTTON_CATEGORY_0; i++ )
	{
		TFLOAT fTextWidth = m_pFont->GetTextWidth( m_aButtons[ i ].GetText(), 1.0f );

		if ( fTextWidth > fMaxWidth )
			fMaxWidth = fTextWidth;
	}

	TFLOAT fScale = fWidth / fMaxWidth;

	if ( fScale >= 1.0f )
		fScale = 1.0f;

	for ( TINT i = BUTTON_CATEGORY_1; i <= BUTTON_CATEGORY_0; i++ )
	{
		AGUI2Button& rButton = m_aButtons[ i ];
		rButton.GetTextBox().SetScale( fScale );

		TFLOAT fOffsets = rButton.GetFlowVisualOffset() + rButton.GetFlowOffset();
		fTotalHeight += rButton.GetTextBox().GetHeight() + fOffsets;
	}

	T2GUIRectangle& rPlate   = m_aRectangles[ s_iPlateRectangle ];
	TFLOAT          fSpacing = ( rPlate.GetHeight() * 0.7f - fTotalHeight ) * 0.25f;

	// The hidden back button
	AGUI2Button& rBack = m_aButtons[ BUTTON_BACK ];
	rBack.Create( m_pFont, rPlate.GetWidth(), ALocaleManager::Interface()->GetString( 977 ), s_fButtonGap );
	rBack.SetFocusedColour( 0xffffffff );
	rBack.SetEnabledColour( COLOUR_BUTTON );
	rBack.SetFocusedScale( 0.8f );
	rBack.SetScale( 0.8f );
	rBack.OnFocusLost();

	AGUI2Menu& rMenu = m_aMenus[ MENU_CATEGORIES ];
	rMenu.AddMenuItem( rBack );
	rBack.Hide();

	rMenu.SetCancelItem( &rBack );
	rMenu.SetDefaultFocusElement( &m_aButtons[ BUTTON_CATEGORY_0 ] );
	rMenu.SetItemSpacing( fSpacing );
	rPage.AddChildTail( rMenu );
	rMenu.ReflowChildrenVertically();
	rMenu.SetFocusSound( soundbank::UI_MENUCLICK );
	rMenu.SetSelectSound( soundbank::UI_MENUOK );
	rMenu.SetBackSound( soundbank::UI_MENUBACK );
	rMenu.SetNegativeSound( soundbank::UI_NEGATIVE );
}

// $Barnyard: FUNCTION 00498200
void AGopherShopMenu::CreateItemPage()
{
	T2GUIRectangle& rPanel  = m_aRectangles[ RECTANGLE_PANEL ];
	TFLOAT          fWidth  = rPanel.GetWidth();
	TFLOAT          fHeight = rPanel.GetHeight();

	T2GUIElement& rPage = m_aElements[ ELEMENT_ITEM ];
	rPage.SetAttachment( T2GUIATTACHMENT( 0 ), T2GUIATTACHMENT( 0 ) );
	rPage.SetDimensions( fWidth, fHeight );

	// The models of the items
	m_oModelRenderer.Create( fWidth * 0.9f, fHeight * 0.9f, TFALSE, TTRUE );
	m_oModelRenderer.SetAttachment( T2GUIATTACHMENT( 4 ), T2GUIATTACHMENT( 4 ) );
	rPage.AddChildTail( m_oModelRenderer );
	m_oModelRenderer.UpdateViewport();
	m_oModelRenderer.Hide();
	m_oModelRenderer.SetLightDirection( TVector4( -0.5831f, -0.2409f, -0.8418f, 1.0f ) );
	m_oModelRenderer.GetCamera()->SetFOV( 1.0471976f );

	TMatrix44 matCamera                                 = TMatrix44::IDENTITY;
	matCamera.AsBasisVector4( BASISVECTOR_TRANSLATION ) = TVector4( 0.0f, 0.0f, -6.0f, 1.0f );
	m_oModelRenderer.SetCameraMatrix( matCamera );

	// The name of the item
	AGUI2TextBox& rName = m_aTextBoxes[ TEXTBOX_NAME ];
	rName.Create( m_pFont, fWidth );
	rName.SetAttachment( T2GUIATTACHMENT( 0 ), T2GUIATTACHMENT( 0 ) );
	rName.SetTextAlign( T2GUIFont::TextAlign( 1 ) );
	rName.SetColour( COLOUR_TITLE );
	rName.SetScale( 0.9f );
	rName.SetTransform( 0.0f, m_pFont->GetLineHeight() * 0.45f, 0.0f );
	rPage.AddChildTail( rName );

	// The count of the item
	AGUI2TextBox& rCount = m_aTextBoxes[ TEXTBOX_COUNT ];
	rCount.Create( m_pFont, fWidth );
	rCount.SetAttachment( T2GUIATTACHMENT( 6 ), T2GUIATTACHMENT( 0 ) );
	rCount.SetTextAlign( T2GUIFont::TextAlign( 1 ) );
	rCount.SetColour( COLOUR_TEXT );
	rCount.SetScale( 1.0f );
	rCount.SetTransform( 0.0f, m_pFont->GetLineHeight() * 0.25f, 0.0f );
	rName.AddChildTail( rCount );

	// The price
	AGUI2TextBox& rPrice = m_aTextBoxes[ TEXTBOX_PRICE ];
	rPrice.Create( m_pFont, fWidth );
	rPrice.SetAttachment( T2GUIATTACHMENT( 7 ), T2GUIATTACHMENT( 1 ) );
	rPrice.SetTextAlign( T2GUIFont::TextAlign( 1 ) );
	rPrice.SetColour( COLOUR_TEXT );
	rPrice.SetScale( 1.0f );
	rPrice.SetText( m_wszPrice );
	rPrice.SetRenderBeforeChildren();

	// The arrows
	T2GUITextureSection* pLeftSection = T2GUITextureSectionManager::GetTextureSection( "left_arrow" );
	TFLOAT               fArrowWidth  = TMath::Abs( TFLOAT( pLeftSection->GetPoint2X() - pLeftSection->GetPoint1X() ) );
	T2GUIRectangle&      rLeftArrow   = m_aRectangles[ RECTANGLE_LEFT_ARROW ];
	rLeftArrow.SetDimensions( fArrowWidth, TMath::Abs( TFLOAT( pLeftSection->GetPoint2Y() - pLeftSection->GetPoint1Y() ) ) );
	rLeftArrow.SetTextureSection( pLeftSection );
	rLeftArrow.SetAttachment( T2GUIATTACHMENT( 3 ), T2GUIATTACHMENT( 3 ) );
	rLeftArrow.SetTransform( TMath::Abs( TFLOAT( pLeftSection->GetPoint2X() - pLeftSection->GetPoint1X() ) ), 0.0f, 0.0f );
	rPage.AddChildTail( rLeftArrow );

	T2GUITextureSection* pRightSection = T2GUITextureSectionManager::GetTextureSection( "left_arrow2" );
	T2GUIRectangle&      rRightArrow   = m_aRectangles[ RECTANGLE_RIGHT_ARROW ];
	rRightArrow.SetDimensions( TMath::Abs( TFLOAT( pRightSection->GetPoint2X() - pRightSection->GetPoint1X() ) ), TMath::Abs( TFLOAT( pRightSection->GetPoint2Y() - pRightSection->GetPoint1Y() ) ) );
	rRightArrow.SetTextureSection( pRightSection );
	rRightArrow.SetAttachment( T2GUIATTACHMENT( 5 ), T2GUIATTACHMENT( 5 ) );
	rRightArrow.SetTransform( -TMath::Abs( TFLOAT( pRightSection->GetPoint2X() - pRightSection->GetPoint1X() ) ), 0.0f, 0.0f );
	rPage.AddChildTail( rRightArrow );

	TFLOAT fHalfWidth = fWidth * 0.5f;

	// The question of the yes/no menu
	AGUI2TextBox& rConfirm = m_aTextBoxes[ TEXTBOX_CONFIRM ];
	rConfirm.Create( m_pFont, fHalfWidth );
	rConfirm.SetText( ALocaleManager::Interface()->GetString( 899 ) );
	rConfirm.SetScale( 1.0f );
	rConfirm.SetColour( COLOUR_TITLE );
	rConfirm.SetAttachment( T2GUIATTACHMENT( 4 ), T2GUIATTACHMENT( 4 ) );
	rConfirm.Hide();
	rPage.AddChildTail( rConfirm );

	// The yes/no menu
	TFLOAT fOldGap = s_fButtonGap;
	s_fButtonGap   = 0.0f;
	CreateButton( m_pFont, 1218, MENU_CONFIRM, BUTTON_YES, s_iPlateRectangle, 1.0f, 1.0f, TTRUE );
	CreateButton( m_pFont, 1219, MENU_CONFIRM, BUTTON_NO, s_iPlateRectangle, 1.0f, 1.0f, TTRUE );

	AGUI2Menu& rConfirmMenu = m_aMenus[ MENU_CONFIRM ];
	rConfirmMenu.SetCancelItem( &m_aButtons[ BUTTON_NO ] );
	rConfirmMenu.SetAttachment( T2GUIATTACHMENT( 7 ), T2GUIATTACHMENT( 7 ) );
	rConfirmMenu.SetTransform( 0.0f, m_pFont->GetLineHeight() * -0.5f, 0.0f );
	rConfirmMenu.SetDefaultFocusElement( &m_aButtons[ BUTTON_YES ] );
	rConfirmMenu.SetItemSpacing( 0.0f );
	rPage.AddChildTail( rConfirmMenu );
	rConfirmMenu.ReflowChildrenHorizontally();
	rConfirmMenu.SetFocusSound( soundbank::UI_MENUCLICK );
	rConfirmMenu.SetSelectSound( soundbank::UI_MENUOK );
	rConfirmMenu.SetBackSound( soundbank::UI_MENUBACK );
	rConfirmMenu.SetNegativeSound( soundbank::UI_NEGATIVE );
	s_fButtonGap = fOldGap;
	rConfirmMenu.Hide();

	// The flares behind the price of Joey's offer
	T2GUITextureSection* pFlareSection = T2GUITextureSectionManager::GetTextureSection( "Flare" );

	for ( TINT i = RECTANGLE_FLARE_1; i <= RECTANGLE_FLARE_3; i++ )
	{
		T2GUIRectangle& rFlare = m_aRectangles[ i ];
		rFlare.SetTextureSection( *pFlareSection );
		rFlare.SetDimensions(
		    TMath::Abs( TFLOAT( pFlareSection->GetPoint2X() - pFlareSection->GetPoint1X() ) ) * m_fScaleX * 0.75f,
		    TMath::Abs( TFLOAT( pFlareSection->GetPoint2Y() - pFlareSection->GetPoint1Y() ) ) * m_fScaleY * 0.75f
		);
		rFlare.Hide();
		rPrice.AddChildTail( rFlare );
	}

	// The quantity
	AGUI2TextBox& rQuantityTitle = m_aTextBoxes[ TEXTBOX_QUANTITY_TITLE ];
	rQuantityTitle.Create( m_pFont, fHalfWidth );
	rQuantityTitle.SetText( ALocaleManager::Interface()->GetString( 897 ) );
	rQuantityTitle.SetScale( 0.8f );
	rQuantityTitle.SetColour( COLOUR_TITLE );
	rQuantityTitle.SetAttachment( T2GUIATTACHMENT( 7 ), T2GUIATTACHMENT( 7 ) );
	rQuantityTitle.SetTransform( 0.0f, m_pFont->GetLineHeight() * -2.5f, 0.0f );
	rQuantityTitle.Hide();
	rPage.AddChildTail( rQuantityTitle );

	// NOTE: retail clears only the first half of the string
	TUtil::MemClear( m_wszQuantity, 64 );

	AGUI2TextBox& rQuantity = m_aTextBoxes[ TEXTBOX_QUANTITY ];
	rQuantity.Create( m_pFont, fHalfWidth );
	rQuantity.SetText( m_wszQuantity );
	rQuantity.SetScale( 0.8f );
	rQuantity.SetColour( COLOUR_TITLE );
	rQuantity.SetAttachment( T2GUIATTACHMENT( 7 ), T2GUIATTACHMENT( 7 ) );
	rQuantity.SetTransform( 0.0f, -m_pFont->GetLineHeight(), 0.0f );
	rQuantity.Hide();
	m_aRectangles[ s_iPlateRectangle ].AddChildTail( rQuantity );

	// The price when selling
	T2GUIElement& rSellPrice = m_aElements[ ELEMENT_SELL_PRICE ];
	rSellPrice.SetDimensions( fWidth, m_pFont->GetLineHeight() );
	rSellPrice.SetAttachment( T2GUIATTACHMENT( 6 ), T2GUIATTACHMENT( 0 ) );
	rSellPrice.Hide();
	rPage.AddChildTail( rSellPrice );

	AGUI2TextBox& rSellTitle = m_aTextBoxes[ TEXTBOX_SELL_PRICE ];
	rSellTitle.Create( m_pFont, fWidth );
	rSellTitle.SetAttachment( T2GUIATTACHMENT( 0 ), T2GUIATTACHMENT( 0 ) );
	rSellTitle.SetTextAlign( T2GUIFont::TextAlign( 1 ) );
	rSellTitle.SetColour( COLOUR_TITLE );
	rSellTitle.SetScale( 1.0f );
	rSellTitle.SetText( ALocaleManager::Interface()->GetString( 905 ) );
	rSellTitle.SetSize( m_pFont->GetTextWidth( ALocaleManager::Interface()->GetString( 905 ), 1.0f ), 0.0f, TFALSE );
	rSellPrice.AddChildTail( rSellTitle );

	// Joey's offer
	T2GUITextureSection* pPhoneSection = T2GUITextureSectionManager::GetTextureSection( "Reward_Phone" );
	T2GUIRectangle&      rPhone        = m_aRectangles[ RECTANGLE_PHONE ];
	rPhone.SetTextureSection( *pPhoneSection );
	rPhone.SetDimensions(
	    TMath::Abs( TFLOAT( pPhoneSection->GetPoint2X() - pPhoneSection->GetPoint1X() ) ) * m_fScaleX,
	    TMath::Abs( TFLOAT( pPhoneSection->GetPoint2Y() - pPhoneSection->GetPoint1Y() ) ) * m_fScaleY
	);
	rPhone.SetAttachment( T2GUIATTACHMENT( 5 ), T2GUIATTACHMENT( 3 ) );
	rPhone.Hide();
	rSellPrice.AddChildTail( rPhone );
}

// $Barnyard: FUNCTION 00498ae0
void AGopherShopMenu::ShowPage( PAGE a_ePage )
{
	if ( m_pShownPage )
		m_pShownPage->RemoveSelf();

	AMenuNavHudElement* pNavElement;

	if ( a_ePage == PAGE_CATEGORIES )
	{
		pNavElement = AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>();
		pNavElement->SetTexts( 916, 917, -1 );
		m_pShownPage = &m_aElements[ ELEMENT_CATEGORIES ];
	}
	else if ( a_ePage == PAGE_MAIN )
	{
		pNavElement = AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>();
		pNavElement->SetTexts( 916, 917, -1 );
		m_pShownPage = &m_aElements[ ELEMENT_MAIN ];
	}
	else if ( a_ePage == PAGE_ITEM )
	{
		pNavElement = AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>();

		if ( m_eMode == AGopherShopGame::MODE_SELL )
			pNavElement->SetTexts( 919, 917, -1 );
		else
			pNavElement->SetTexts( 918, 917, -1 );

		m_pShownPage = &m_aElements[ ELEMENT_ITEM ];
	}

	m_eCurrentPage = a_ePage;
	m_aRectangles[ s_iPlateRectangle ].AddChildTail( *m_pShownPage );
}

// $Barnyard: FUNCTION 004970a0
void AGopherShopMenu::SetPage( PAGE a_ePage, TBOOL a_bUnused )
{
	m_eNextPage   = a_ePage;
	m_eTransition = TRANSITION_FADING_OUT;

	if ( a_ePage == PAGE_CATEGORIES )
	{
		m_pPageElement = &m_aElements[ ELEMENT_CATEGORIES ];
	}
	else if ( a_ePage == PAGE_MAIN )
	{
		m_pPageElement = &m_aElements[ ELEMENT_MAIN ];
	}
	else if ( a_ePage == PAGE_ITEM )
	{
		m_pPageElement = &m_aElements[ ELEMENT_ITEM ];

		AGUI2TextBox& rPrice = m_aTextBoxes[ TEXTBOX_PRICE ];
		rPrice.RemoveSelf();

		if ( m_eMode == AGopherShopGame::MODE_SELL )
		{
			// The price follows the "sells for" text
			rPrice.SetAttachment( T2GUIATTACHMENT( 5 ), T2GUIATTACHMENT( 3 ) );
			rPrice.SetTransform( GetSpaceWidth( m_pFont ), 0.0f, 0.0f );
			m_aTextBoxes[ TEXTBOX_SELL_PRICE ].AddChildTail( rPrice );
			m_aElements[ ELEMENT_SELL_PRICE ].Show();
		}
		else
		{
			rPrice.SetAttachment( T2GUIATTACHMENT( 7 ), T2GUIATTACHMENT( 1 ) );
			rPrice.SetTransform( 0.0f, 0.0f, 0.0f );
			m_aTextBoxes[ TEXTBOX_NAME ].AddChildTail( rPrice );
			m_aElements[ ELEMENT_SELL_PRICE ].Hide();
		}
	}
}

// $Barnyard: FUNCTION 004971b0
void AGopherShopMenu::ProcessCommand( AInputCommand a_eCommand, const TInputInterface::InputEvent* a_pEvent )
{
	if ( m_eTransition != TRANSITION_NONE )
		return;

	if ( m_eCurrentPage == PAGE_CATEGORIES )
		m_aMenus[ MENU_CATEGORIES ].ProcessInputCommand( a_eCommand, a_pEvent );
	else if ( m_eCurrentPage == PAGE_MAIN )
		m_aMenus[ MENU_MAIN ].ProcessInputCommand( a_eCommand, a_pEvent );
	else if ( m_eCurrentPage == PAGE_ITEM && m_bConfirmShown )
		m_aMenus[ MENU_CONFIRM ].ProcessInputCommand( a_eCommand, a_pEvent );
}

// $Barnyard: FUNCTION 00497520
TBOOL AGopherShopMenu::ProcessInputEvent( const TInputInterface::InputEvent* a_pEvent )
{
	AGUI2Menu* pMenu;

	if ( m_eCurrentPage == PAGE_CATEGORIES )
		pMenu = &m_aMenus[ MENU_CATEGORIES ];
	else if ( m_eCurrentPage == PAGE_MAIN )
		pMenu = &m_aMenus[ MENU_MAIN ];
	else if ( m_eCurrentPage == PAGE_ITEM && m_bConfirmShown )
		pMenu = &m_aMenus[ MENU_CONFIRM ];
	else
		return TFALSE;

	// NOTE: AGUI2Menu::ProcessInputEvent is inlined here
	return pMenu->ProcessInputEvent( a_pEvent );
}

// $Barnyard: FUNCTION 00497220
void AGopherShopMenu::FlashArrow( TBOOL a_bLeft )
{
	if ( a_bLeft == TTRUE )
	{
		m_aRectangles[ RECTANGLE_LEFT_ARROW ].SetColour( COLOUR_BUTTON );
		m_fLeftFlashTime = 0.2f;
		m_bLeftFlash     = TTRUE;
	}
	else
	{
		m_aRectangles[ RECTANGLE_RIGHT_ARROW ].SetColour( COLOUR_BUTTON );
		m_fRightFlashTime = 0.2f;
		m_bRightFlash     = TTRUE;
	}
}

// $Barnyard: FUNCTION 00497280
void AGopherShopMenu::SetStatus( STATUS a_eStatus, TBOOL a_bAnimate )
{
	T2GUIElement* pOldStatus = m_pStatus;

	if ( pOldStatus )
		pOldStatus->RemoveSelf();

	if ( s_pNegative )
		s_pNegative->RemoveSelf();

	AGUI2TextBox& rName  = m_aTextBoxes[ TEXTBOX_NAME ];
	AGUI2TextBox& rPrice = m_aTextBoxes[ TEXTBOX_PRICE ];
	T2GUIElement& rPage  = m_aElements[ ELEMENT_ITEM ];

	switch ( a_eStatus )
	{
		case STATUS_SOLD_OUT:
			m_pStatus = &m_aTextBoxes[ TEXTBOX_SOLD_OUT ];
			m_pStatus->Show();

			if ( a_bAnimate )
			{
				// Pops up
				m_pStatus->SetAttachment( T2GUIATTACHMENT( 4 ), T2GUIATTACHMENT( 4 ) );
				m_fStatusPopAngle = 0.0f;
				m_pStatus->GetTransform().Scale( 0.1f, 0.1f );
				m_bStatusPop = TTRUE;
			}
			else
			{
				m_pStatus->SetAttachment( T2GUIATTACHMENT( 7 ), T2GUIATTACHMENT( 7 ) );

				T2GUITransform oTransform;
				oTransform.SetTranslation( 0.0f, m_pFont->GetLineHeight() * -0.5f );
				m_pStatus->GetTransform() = oTransform;

				rName.SetColour( COLOUR_GREY );
				rPrice.SetColour( COLOUR_GREY );

				s_pNegative = &m_aRectangles[ RECTANGLE_NEGATIVE ];
				rPage.AddChildTail( m_aRectangles[ RECTANGLE_NEGATIVE ] );
				s_pNegative->Show();
			}
			break;
		case STATUS_AVAILABLE:
			rName.SetColour( COLOUR_TITLE );
			rPrice.SetColour( COLOUR_TEXT );
			m_pStatus   = TNULL;
			s_pNegative = TNULL;
			break;
		case STATUS_UNAVAILABLE:
			rName.SetColour( COLOUR_GREY );
			rPrice.SetColour( COLOUR_GREY );
			m_pStatus   = &m_aTextBoxes[ TEXTBOX_UNAVAILABLE ];
			s_pNegative = &m_aRectangles[ RECTANGLE_NEGATIVE ];
			rPage.AddChildTail( m_aRectangles[ RECTANGLE_NEGATIVE ] );
			m_pStatus->Show();
			s_pNegative->Show();
			break;
		case STATUS_FULL:
			rName.SetColour( COLOUR_GREY );
			rPrice.SetColour( COLOUR_GREY );
			m_pStatus   = &m_aTextBoxes[ TEXTBOX_FULL ];
			s_pNegative = TNULL;
			break;
		case STATUS_EXPENSIVE:
			rName.SetColour( COLOUR_GREY );
			rPrice.SetColour( COLOUR_GREY );
			m_pStatus   = TNULL;
			s_pNegative = TNULL;
			break;
	}

	if ( m_pStatus )
		rPage.AddChildTail( *m_pStatus );

	if ( pOldStatus )
	{
		pOldStatus->GetTransform().SetFromScale( 1.0f, 1.0f );
		m_bStatusPop = TFALSE;
	}
}

// $Barnyard: FUNCTION 00497570
void AGopherShopMenu::SetQuantity( TINT a_iQuantity )
{
	m_iQuantity = a_iQuantity;
	ALocale::FormatTags( m_wszQuantity, 64, L"%1", PPropertyValue( a_iQuantity ) );
}

// $Barnyard: FUNCTION 00496b20
void AGopherShopMenu::SetMoney( TINT a_iMoney )
{
	ALocale::FormatTags( m_wszMoney, 64, 983, PPropertyValue( a_iMoney ) );

	AGUI2TextBox& rMoney = m_aTextBoxes[ TEXTBOX_MONEY ];
	rMoney.SetText( m_wszMoney );
	rMoney.SetSize( m_aRectangles[ RECTANGLE_WOOD_MONEY ].GetWidth(), 0.0f, TFALSE );
}

// $Barnyard: FUNCTION 00496a50
void AGopherShopMenu::SetDescription( TINT a_iTextId )
{
	AGUI2TextBox& rDescription = m_aTextBoxes[ TEXTBOX_DESCRIPTION ];

	if ( a_iTextId == -1 )
	{
		rDescription.Hide();
		return;
	}

	ALocale::FormatTags( m_wszDescription, 64, a_iTextId );

	T2GUIRectangle& rWood = m_aRectangles[ RECTANGLE_WOOD ];
	rDescription.SetText( m_wszDescription );
	rDescription.SetSize( rWood.GetWidth(), 0.0f, TFALSE );
	rDescription.Show();
	rDescription.SetAlpha( 0.0f );
	rDescription.SetShadowAlpha( 0.0f );

	TFLOAT fRatio = rWood.GetWidth() / m_pFont->GetTextWidth( rDescription.GetText(), 1.0f );
	rDescription.SetScale( ( fRatio >= 1.0f ) ? 1.0f : fRatio * 0.9f );
}

// $Barnyard: FUNCTION 00496bb0
void AGopherShopMenu::SetItemName( TINT a_iTextId )
{
	ALocale::FormatTags( m_wszItemName, 64, a_iTextId );

	T2GUIRectangle& rPanel = m_aRectangles[ RECTANGLE_PANEL ];
	AGUI2TextBox&   rName  = m_aTextBoxes[ TEXTBOX_NAME ];
	rName.SetText( m_wszItemName );
	rName.SetSize( rPanel.GetWidth(), 0.0f, TFALSE );

	TFLOAT fRatio = ( rPanel.GetWidth() * 0.9f ) / m_pFont->GetTextWidth( rName.GetText(), 0.9f );
	TBOOL  bSmall = TFALSE;
	TFLOAT fScale;

	if ( fRatio >= 0.735f )
	{
		fScale = ( fRatio >= 1.1f ) ? 1.0f : fRatio * 0.9f;
	}
	else
	{
		fScale = 0.8f;
		bSmall = TTRUE;
	}

	rName.SetScale( fScale );

	AGUI2TextBox& rPrice = m_aTextBoxes[ TEXTBOX_PRICE ];

	if ( fScale == 1.0f )
	{
		rName.SetTransform( 0.0f, m_pFont->GetLineHeight() * fScale * 0.5f, 0.0f );
		rPrice.SetTransform( 0.0f, 0.0f, 0.0f );
	}
	else
	{
		if ( bSmall )
			rName.SetTransform( 0.0f, 0.0f, 0.0f );
		else
			rName.SetTransform( 0.0f, m_pFont->GetLineHeight() * fScale * 0.8f, 0.0f );

		rPrice.SetTransform( 0.0f, m_pFont->GetLineHeight() * fScale * 0.1f, 0.0f );
	}
}

// $Barnyard: FUNCTION 00496d20
void AGopherShopMenu::SetItemCount( TINT a_iCount )
{
	AGUI2TextBox& rCount = m_aTextBoxes[ TEXTBOX_COUNT ];

	if ( a_iCount == -1 )
	{
		rCount.Hide();
		return;
	}

	ALocale::FormatTags( m_wszCount, 64, 913, PPropertyValue( a_iCount ) );
	rCount.SetText( m_wszCount );
	rCount.SetSize( m_aRectangles[ RECTANGLE_PANEL ].GetWidth(), 0.0f, TFALSE );
	rCount.Show();
}

// $Barnyard: FUNCTION 00496de0
void AGopherShopMenu::SetPrice( TINT a_iPrice, TINT a_iMultiplier )
{
	ALocale::FormatTags( m_wszPrice, 64, 983, PPropertyValue( a_iPrice ) );

	TFLOAT        fPriceWidth = m_pFont->GetTextWidth( m_wszPrice, 1.0f );
	AGUI2TextBox& rPrice      = m_aTextBoxes[ TEXTBOX_PRICE ];
	rPrice.SetSize( fPriceWidth, 0.0f, TFALSE );

	if ( m_eMode != AGopherShopGame::MODE_SELL )
		return;

	// Centre the "sells for" text and the price
	TFLOAT fWidth = GetSpaceWidth( m_pFont );
	fWidth        = m_aTextBoxes[ TEXTBOX_SELL_PRICE ].GetWidth() + fWidth;
	fWidth        = rPrice.GetWidth() + fWidth;

	T2GUIElement& rSellPrice = m_aElements[ ELEMENT_SELL_PRICE ];
	rSellPrice.SetDimensions( fWidth, m_pFont->GetLineHeight() );
	rSellPrice.SetTransform( ( m_aElements[ ELEMENT_ITEM ].GetWidth() - fWidth ) * 0.5f, 0.0f, 0.0f );

	// Joey pays more for the item of his offer
	m_aRectangles[ RECTANGLE_PHONE ].SetVisible( a_iMultiplier > 1 );

	if ( a_iMultiplier > 1 )
	{
		if ( !m_bFlareGrow && m_fFlareScale < 1.0f )
		{
			m_bFlareGrow   = TTRUE;
			m_bFlareShrink = TFALSE;

			for ( TINT i = RECTANGLE_FLARE_1; i <= RECTANGLE_FLARE_3; i++ )
			{
				m_aRectangles[ i ].Show();
				m_aRectangles[ i ].GetTransform().SetScale( m_fFlareScale, m_fFlareScale );
			}

			return;
		}
	}
	else if ( a_iMultiplier == 1 && m_aRectangles[ RECTANGLE_FLARE_1 ].IsVisible() && !m_bFlareShrink )
	{
		m_bFlareShrink = TTRUE;
		m_bFlareGrow   = TFALSE;
	}
}

// $Barnyard: FUNCTION 00498be0
void AGopherShopMenu::ShowConfirm( TBOOL a_bShow )
{
	m_bConfirmShown = a_bShow;
	m_aMenus[ MENU_CONFIRM ].SetVisible( a_bShow == TTRUE );
	m_aTextBoxes[ TEXTBOX_CONFIRM ].SetVisible( a_bShow == TTRUE );

	AMenuNavHudElement* pNavElement = AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>();

	if ( m_bConfirmShown )
	{
		pNavElement->SetTexts( 916, 917, -1 );
		m_aMenus[ MENU_CONFIRM ].SetFocusAt( m_aButtons[ BUTTON_YES ] );
		m_aRectangles[ RECTANGLE_LEFT_ARROW ].Hide();
		m_aRectangles[ RECTANGLE_RIGHT_ARROW ].Hide();
	}
	else
	{
		pNavElement->SetTexts( 918, 917, -1 );
		m_aRectangles[ RECTANGLE_LEFT_ARROW ].Show();
		m_aRectangles[ RECTANGLE_RIGHT_ARROW ].Show();
	}
}

// $Barnyard: FUNCTION 00498cb0
void AGopherShopMenu::ShowQuantity( TBOOL a_bShow )
{
	m_bQuantityShown = a_bShow;
	m_aTextBoxes[ TEXTBOX_QUANTITY_TITLE ].SetVisible( a_bShow == TTRUE );

	T2GUIRectangle& rLeftArrow  = m_aRectangles[ RECTANGLE_LEFT_ARROW ];
	T2GUIRectangle& rRightArrow = m_aRectangles[ RECTANGLE_RIGHT_ARROW ];
	AGUI2TextBox&   rQuantity   = m_aTextBoxes[ TEXTBOX_QUANTITY ];

	if ( m_bQuantityShown )
	{
		// The arrows change the quantity
		rLeftArrow.RemoveSelf();
		rLeftArrow.SetAttachment( T2GUIATTACHMENT( 3 ), T2GUIATTACHMENT( 5 ) );
		rLeftArrow.Show();
		rQuantity.AddChildTail( rLeftArrow );

		rRightArrow.RemoveSelf();
		rRightArrow.SetAttachment( T2GUIATTACHMENT( 5 ), T2GUIATTACHMENT( 3 ) );
		rRightArrow.Show();
		rQuantity.AddChildTail( rRightArrow );

		rQuantity.Show();
		m_aTextBoxes[ TEXTBOX_QUANTITY_TITLE ].Show();
		SetQuantity( 1 );
	}
	else
	{
		rLeftArrow.RemoveSelf();
		rLeftArrow.SetAttachment( T2GUIATTACHMENT( 3 ), T2GUIATTACHMENT( 3 ) );
		m_aElements[ ELEMENT_ITEM ].AddChildTail( rLeftArrow );

		rRightArrow.RemoveSelf();
		rRightArrow.SetAttachment( T2GUIATTACHMENT( 5 ), T2GUIATTACHMENT( 5 ) );
		m_aElements[ ELEMENT_ITEM ].AddChildTail( rRightArrow );

		rQuantity.Hide();
		m_aTextBoxes[ TEXTBOX_QUANTITY_TITLE ].Hide();
	}
}

// $Barnyard: FUNCTION 00498ef0
void AGopherShopMenu::SetPageAlpha( PAGE a_ePage, TFLOAT a_fAlpha, TFLOAT a_fShadowAlpha, TFLOAT a_fModelSpeed )
{
	if ( a_ePage == PAGE_CATEGORIES )
	{
		m_aMenus[ MENU_CATEGORIES ].SetMenuAlpha( a_fAlpha, a_fShadowAlpha );
	}
	else if ( a_ePage == PAGE_MAIN )
	{
		m_aMenus[ MENU_MAIN ].SetMenuAlpha( a_fAlpha, a_fShadowAlpha );
	}
	else if ( a_ePage == PAGE_ITEM )
	{
		m_aTextBoxes[ TEXTBOX_DESCRIPTION ].SetAlpha( a_fAlpha );
		m_aTextBoxes[ TEXTBOX_DESCRIPTION ].SetShadowAlpha( a_fShadowAlpha );
		m_aTextBoxes[ TEXTBOX_UNAVAILABLE ].SetAlpha( a_fAlpha );
		m_aTextBoxes[ TEXTBOX_UNAVAILABLE ].SetShadowAlpha( a_fShadowAlpha );
		m_aTextBoxes[ TEXTBOX_SOLD_OUT ].SetAlpha( a_fAlpha );
		m_aTextBoxes[ TEXTBOX_SOLD_OUT ].SetShadowAlpha( a_fShadowAlpha );
		m_aRectangles[ RECTANGLE_NEGATIVE ].SetAlpha( a_fAlpha );

		for ( TINT i = TEXTBOX_NAME; i <= TEXTBOX_PRICE; i++ )
		{
			m_aTextBoxes[ i ].SetAlpha( a_fAlpha );
			m_aTextBoxes[ i ].SetShadowAlpha( a_fShadowAlpha );
		}

		for ( TINT i = 0; i < m_oModelRenderer.GetNumObjects(); i++ )
		{
			TFLOAT fOpacity = m_oModelRenderer.GetObject( i )->GetModelInstance()->GetOpacity();

			if ( a_fModelSpeed <= 0.0f )
			{
				m_oModelRenderer.SetObjectOpacity( i, a_fAlpha );
			}
			else
			{
				if ( m_eTransition == TRANSITION_FADING_IN )
					fOpacity = TMath::LERP( fOpacity, 1.0f, a_fModelSpeed );
				else
					fOpacity = TMath::LERP( fOpacity, 0.0f, a_fModelSpeed );

				TMath::Clip( fOpacity, 0.0f, 1.0f );
				m_oModelRenderer.SetObjectOpacity( i, fOpacity );
			}
		}

		m_aRectangles[ RECTANGLE_LEFT_ARROW ].SetAlpha( a_fAlpha );
		m_aRectangles[ RECTANGLE_RIGHT_ARROW ].SetAlpha( a_fAlpha );
	}
}

// $Barnyard: FUNCTION 00499110
void AGopherShopMenu::UpdateTransition( TFLOAT a_fDeltaTime )
{
	T2GUIRectangle& rPanel = m_aRectangles[ RECTANGLE_PANEL ];

	switch ( m_eTransition )
	{
		case TRANSITION_OPENING:
		{
			m_fTransitionAngle += a_fDeltaTime * 6.0f;

			if ( m_fTransitionAngle <= s_fOscillatorPeriod )
			{
				TFLOAT fScale = TMath::Sin( m_fTransitionAngle ) * s_fOscillatorAmplitude;

				T2GUITransform oTransform;
				oTransform.Scale( fScale, fScale );
				oTransform.SetTranslation( ( m_vecPanelSize.x - fScale * m_vecPanelSize.x ) * 0.5f, 0.0f );
				rPanel.GetTransform() = oTransform;
			}
			else
			{
				m_fTransitionAngle    = s_fOscillatorPeriod;
				m_eTransition         = TRANSITION_FADING_IN;
				m_fFade               = 0.0f;
				rPanel.GetTransform() = T2GUITransform();

				if ( m_eCurrentPage == PAGE_ITEM )
					m_oModelRenderer.Show();
			}

			TFLOAT fAlpha = rPanel.GetAlpha() + a_fDeltaTime * 5.0f;
			TMath::Clip( fAlpha, 0.0f, 1.0f );
			rPanel.SetAlpha( fAlpha );
			m_aRectangles[ RECTANGLE_PLATE ].SetAlpha( fAlpha );
			break;
		}
		case TRANSITION_CLOSING:
		{
			m_fTransitionAngle -= a_fDeltaTime * 6.0f;

			if ( m_fTransitionAngle < 0.0f || m_eCurrentPage == PAGE_NONE )
			{
				if ( m_eNextPage == PAGE_NONE )
				{
					m_eTransition = TRANSITION_CLOSED;
				}
				else
				{
					ShowPage( m_eNextPage );
					SetPageAlpha( m_eCurrentPage, 0.0f, 0.0f, -1.0f );
					m_eTransition = TRANSITION_OPENING;
				}

				m_fTransitionAngle = 0.0f;
				m_fFade            = 1.0f;
				break;
			}

			TFLOAT fScale = TMath::Sin( m_fTransitionAngle ) * s_fOscillatorAmplitude;

			T2GUITransform oTransform;
			oTransform.Scale( fScale, fScale );
			oTransform.SetTranslation( ( m_vecPanelSize.x - fScale * m_vecPanelSize.x ) * 0.5f, 0.0f );
			rPanel.GetTransform() = oTransform;

			if ( fScale < 0.8f )
			{
				TFLOAT fAlpha = rPanel.GetAlpha() - a_fDeltaTime * 5.0f;
				TMath::Clip( fAlpha, 0.0f, 1.0f );
				rPanel.SetAlpha( fAlpha );
				m_aRectangles[ RECTANGLE_PLATE ].SetAlpha( fAlpha );
			}

			break;
		}
		case TRANSITION_FADING_IN:
		{
			TFLOAT fSpeed = a_fDeltaTime * 5.0f;
			m_fFade += fSpeed;

			if ( m_fFade < 1.0f )
			{
				TMath::Clip( m_fFade, 0.0f, 1.0f );
				SetPageAlpha( m_eCurrentPage, m_fFade, m_fFade * 0.6f, fSpeed );
				break;
			}

			m_fFade            = 1.0f;
			m_eTransition      = TRANSITION_NONE;
			m_fTransitionAngle = 0.0f;

			if ( m_eCurrentPage == PAGE_ITEM )
				m_oModelRenderer.UpdateViewport();

			SetPageAlpha( m_eCurrentPage, 1.0f, 0.6f, -1.0f );
			break;
		}
		case TRANSITION_FADING_OUT:
		{
			TFLOAT fSpeed = a_fDeltaTime * 5.0f;
			m_fFade -= fSpeed;

			if ( m_fFade > 0.0f && m_eCurrentPage != PAGE_NONE )
			{
				TMath::Clip( m_fFade, 0.0f, 1.0f );
				SetPageAlpha( m_eCurrentPage, m_fFade, m_fFade * 0.6f, fSpeed );
				break;
			}

			m_fFade            = 0.0f;
			m_eTransition      = TRANSITION_CLOSING;
			m_fTransitionAngle = s_fOscillatorPeriod;

			if ( m_eCurrentPage != PAGE_NONE )
				SetPageAlpha( m_eCurrentPage, 0.0f, 0.0f, -1.0f );

			if ( m_eCurrentPage == PAGE_ITEM )
				m_oModelRenderer.Hide();

			break;
		}
	}
}

// $Barnyard: FUNCTION 00499530
void AGopherShopMenu::Update( TFLOAT a_fDeltaTime )
{
	m_fTime += a_fDeltaTime;

	if ( m_bStatusPop && m_pStatus )
	{
		// The status pops up
		m_fStatusPopAngle += a_fDeltaTime * 6.0f;

		if ( m_fStatusPopAngle > s_fOscillatorPeriod )
		{
			m_fStatusPopAngle = s_fOscillatorPeriod;
			m_bStatusPop      = TFALSE;
		}

		TFLOAT fScale = TMath::Sin( m_fStatusPopAngle ) * s_fOscillatorAmplitude;
		m_pStatus->GetTransform().SetFromScale( fScale, fScale );
	}

	T2GUIRectangle* aFlares[ 3 ] = { &m_aRectangles[ RECTANGLE_FLARE_1 ], &m_aRectangles[ RECTANGLE_FLARE_2 ], &m_aRectangles[ RECTANGLE_FLARE_3 ] };

	if ( aFlares[ 0 ]->IsVisible() )
	{
		// The flares grow or shrink and spin
		if ( m_bFlareGrow )
		{
			m_fFlareScale += a_fDeltaTime * 4.0f;

			if ( m_fFlareScale > 1.0f )
			{
				m_fFlareScale = 1.0f;
				m_bFlareGrow  = TFALSE;
			}
		}
		else if ( m_bFlareShrink )
		{
			m_fFlareScale -= a_fDeltaTime * 4.0f;

			if ( m_fFlareScale < 0.01f )
			{
				m_bFlareShrink = TFALSE;
				aFlares[ 0 ]->Hide();
				aFlares[ 1 ]->Hide();
				aFlares[ 2 ]->Hide();
			}
		}

		static constexpr TFLOAT s_aSpinSpeeds[ 3 ] = { 0.5f, -0.75f, -0.25f };

		for ( TINT i = 0; i < 3; i++ )
		{
			T2GUITransform& rTransform     = aFlares[ i ]->GetTransform();
			TVector2        vecTranslation = rTransform.GetTranslation();
			rTransform.SetIdentity();
			rTransform.Scale( m_fFlareScale, m_fFlareScale );
			rTransform.Rotate( m_fTime * s_aSpinSpeeds[ i ] );
			rTransform.SetTranslation( vecTranslation );
		}
	}

	if ( m_eTransition != TRANSITION_NONE )
	{
		UpdateTransition( a_fDeltaTime );
		return;
	}

	// The focused button pulses
	TINT iMenu = -1;

	if ( m_eCurrentPage == PAGE_CATEGORIES )
	{
		TFLOAT fScale = TMath::Sin( TMath::TWO_PI * m_fTime ) * 0.05f + 0.8f;
		m_aMenus[ MENU_CATEGORIES ].GetFocusedMenuItem()->GetTransform().SetScale( fScale, fScale );
		iMenu = MENU_CATEGORIES;
	}
	else if ( m_eCurrentPage == PAGE_MAIN )
	{
		TFLOAT fScale = TMath::Sin( TMath::TWO_PI * m_fTime ) * 0.05f + 1.0f;
		m_aMenus[ MENU_MAIN ].GetFocusedMenuItem()->GetTransform().SetScale( fScale, fScale );
		iMenu = MENU_MAIN;
	}
	else if ( m_eCurrentPage == PAGE_ITEM )
	{
		if ( m_bConfirmShown )
		{
			TFLOAT fScale = TMath::Sin( TMath::TWO_PI * m_fTime ) * 0.05f + 1.0f;
			m_aMenus[ MENU_CONFIRM ].GetFocusedMenuItem()->GetTransform().SetScale( fScale, fScale );
			iMenu = MENU_CONFIRM;
		}
		else if ( m_bQuantityShown )
		{
			m_aTextBoxes[ TEXTBOX_QUANTITY ].GetTransform().SetScale( 1.0f, 1.0f );
		}
	}

	if ( iMenu != -1 )
		m_aMenus[ iMenu ].Update( a_fDeltaTime );

	// The arrows flash for a moment when they're used
	if ( m_bLeftFlash == TTRUE )
	{
		m_fLeftFlashTime -= a_fDeltaTime;

		if ( m_fLeftFlashTime < 0.0f )
		{
			m_aRectangles[ RECTANGLE_LEFT_ARROW ].SetColour( COLOUR_TEXT );
			m_bLeftFlash = TFALSE;
		}
	}

	if ( m_bRightFlash == TTRUE )
	{
		m_fRightFlashTime -= a_fDeltaTime;

		if ( m_fRightFlashTime < 0.0f )
		{
			m_aRectangles[ RECTANGLE_RIGHT_ARROW ].SetColour( COLOUR_TEXT );
			m_bRightFlash = TFALSE;
		}
	}
}
