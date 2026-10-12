#include "pch.h"
#include "ACookingHUD.h"
#include "ACookingMiniGame.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AModel.h"
#include "GUI/AGUI2.h"
#include "HUD/AHudManager.h"
#include "HUD/AMenuNavHudElement.h"
#include "Locale/ALocaleManager.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"
#include "Units/AObjectTraits.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>
#include <Render/TCameraObject.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Colours the models of the ingredients are rendered with (0x00773ce8 / 0x00773cd8)
static const TVector4 s_vLightColour( 1.0f, 1.0f, 1.0f, 1.0f );
static const TVector4 s_vAmbientColour( 0.2f, 0.2f, 0.2f, 0.0f );

static const TVector4 s_vLightDirection( -0.5831f, -0.2409f, -0.8418f, 1.0f );

// Positions of the ingredients on the left and the right of the carousel (0x00773cf8 / 0x00773d08)
static const TVector4 s_vCarouselLeft( -1.0f, 0.0f, 0.0f, 1.0f );
static const TVector4 s_vCarouselRight( 1.0f, 0.0f, 0.0f, 1.0f );

// The angle the top of the blender shakes by (DAT_007833a4)
static TFLOAT s_fShakeAngle = TMath::PI * ( 1.0f / 36.0f );

// $Barnyard: FUNCTION 004f9690
ACookingHUD::ACookingHUD()
    : m_fWidth( 0.0f )
    , m_fHeight( 0.0f )
    , m_pFont( TNULL )
    , m_bMouseMoved( TFALSE )
    , m_bMouseOnOK( TFALSE )
    , m_bMouseOnLeftArrow( TFALSE )
    , m_bMouseOnRightArrow( TFALSE )
    , m_iNumIngredients( 0 )
    , m_iNumInPan( 0 )
    , m_pRoot( TNULL )
    , m_eCarouselState( CAROUSELSTATE_IDLE )
    , m_fTime( 0.0f )
    , m_fSmokeTime( 0.0f )
    , m_bSmoke( TFALSE )
    , m_fPanTime( 0.0f )
    , m_iUnkF84( 0 )
    , m_iUnkF88( 0 )
    , m_vUnkF8C( TVector2::VEC_ZERO )
    , m_pCookerTexture( TNULL )
    , m_pCookerOpenTexture( TNULL )
    , m_bShaking( TFALSE )
    , m_bFadingDishInfo( TFALSE )
{
	for ( TINT i = 0; i < CAROUSEL_NUMOF; i++ )
	{
		m_apCarousel[ i ] = TNULL;
		m_aiCarousel[ i ] = -1;
	}

	TStringManager::String16Format( m_wszCount, TARRAYSIZE( m_wszCount ), L"" );
	TStringManager::String16Format( m_wszDishName, TARRAYSIZE( m_wszDishName ), L"" );
	TStringManager::String16Format( m_wszPrice, TARRAYSIZE( m_wszPrice ), L"" );
}

// $Barnyard: FUNCTION 004fab80
ACookingHUD::~ACookingHUD()
{
	Destroy();
}

// $Barnyard: FUNCTION 004fac30
void ACookingHUD::Create( T2GUIElement* a_pRoot )
{
	m_pFont = T2GUIFontManager::FindFont( "rekord26" );
	m_pRoot = a_pRoot;
	a_pRoot->GetDimensions( m_fWidth, m_fHeight );

	// The GUI is made for 800x600
	m_fScaleX = m_fWidth * ( 1.0f / 800.0f );
	m_fScaleY = m_fHeight * ( 1.0f / 600.0f );

	CreateGUI();
	AGUI2::GetSingleton()->SetCursorVisible( TTRUE );

	for ( TINT i = 0; i < TEXT_NUMOF; i++ )
	{
		m_aTexts[ i ].SetShadow( TTRUE, 0xa0000000 );
		m_aTexts[ i ].SetShadowAlpha( 0.6f );
		m_aTexts[ i ].SetShadowOffset( 5.0f, 5.0f );
	}
}

// $Barnyard: FUNCTION 004fa360
void ACookingHUD::Destroy()
{
	AGUI2::GetSingleton()->SetCursorVisible( TFALSE );

	for ( TINT i = 0; i < MODEL_NUMOF; i++ )
	{
		m_aModels[ i ].RemoveAllObjects();
		m_aModels[ i ].RemoveSelf();
	}

	for ( TINT i = 0; i < TEXT_NUMOF; i++ )
		m_aTexts[ i ].RemoveSelf();

	for ( TINT i = 0; i < RECT_NUMOF; i++ )
		m_aRects[ i ].RemoveSelf();

	for ( TINT i = 0; i < ELEMENT_NUMOF; i++ )
		m_aElements[ i ].RemoveSelf();

	T2GUIFontManager::ReleaseFont( m_pFont );
}

// $Barnyard: FUNCTION 004f91c0
void ACookingHUD::CreateSlot( TINT a_iSlot, TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	AGUI2ModelRenderer& rModel = m_aModels[ a_iSlot ];

	rModel.Create( a_fWidth, a_fHeight, TFALSE, TTRUE );
	m_aRects[ RECT_COOKER ].AddChildTail( rModel );
	rModel.GetCamera()->SetFOV( TMath::PI / 6.0f );

	TMatrix44 matCamera = TMatrix44::IDENTITY;
	matCamera.GetTranslation().Set( 0.0f, 0.0f, -1.5f, 1.0f );
	rModel.SetCameraMatrix( matCamera );
	rModel.SetLightDirection( s_vLightDirection );
}

// $Barnyard: FUNCTION 004f9840
void ACookingHUD::CreateGUI()
{
	TBOOL  bCocktails = ACookingMiniGame::IsCocktails();
	TFLOAT fWidth     = m_fWidth * 0.5f;
	TFLOAT fHeight    = m_fHeight;

	// Everything is on the right half of the screen
	T2GUIElement& rMain = m_aElements[ ELEMENT_MAIN ];
	rMain.SetDimensions( fWidth, fHeight );
	rMain.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );
	m_pRoot->AddChildTail( rMain );

	// The name of the selected ingredient
	T2GUIRectangle&      rWoodThin   = m_aRects[ RECT_WOOD_THIN ];
	T2GUITextureSection* pWoodThin   = T2GUITextureSectionManager::GetTextureSection( "Score_Wood_Thin" );
	TFLOAT               fThinHeight = fWidth * 0.12f;
	TFLOAT               fThinWidth  = fWidth * 0.8f;
	rWoodThin.SetDimensions( fThinWidth, fThinHeight );
	rWoodThin.SetTextureSection( pWoodThin );
	rWoodThin.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	rWoodThin.SetColour( 0xffffffff );
	rWoodThin.SetTransform( 0.0f, fThinHeight * 0.5f );
	rMain.AddChildTail( rWoodThin );

	T2GUIElement& rIngredient = m_aElements[ ELEMENT_INGREDIENT ];
	rIngredient.SetDimensions( 0.0f, fThinHeight );
	rIngredient.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	rWoodThin.AddChildTail( rIngredient );

	AGUI2TextBox& rIngredientText = m_aTexts[ TEXT_INGREDIENT ];
	rIngredientText.Create( m_pFont, fThinWidth );
	rIngredientText.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	rIngredientText.SetTextAlign( T2GUIFont::TextAlign_Center );
	rIngredientText.SetColour( 0xffffffff );
	rIngredientText.SetScale( 1.0f );
	rIngredient.AddChildTail( rIngredientText );

	AGUI2TextBox& rCountText = m_aTexts[ TEXT_COUNT ];
	rCountText.Create( m_pFont, fThinWidth * 0.25f );
	rCountText.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLELEFT );
	rCountText.SetTransform( m_pFont->GetTextWidth( L"-", 1.0f ), 0.0f );
	rCountText.SetTextAlign( T2GUIFont::TextAlign_Left );
	rCountText.SetColour( 0xffffc800 );
	rCountText.SetScale( 1.0f );
	rIngredientText.AddChildTail( rCountText );

	// The carousel
	T2GUIRectangle&      rWood       = m_aRects[ RECT_WOOD ];
	T2GUITextureSection* pWood       = T2GUITextureSectionManager::GetTextureSection( "Score_Wood" );
	TFLOAT               fWoodHeight = rWoodThin.GetHeight();
	rWood.SetDimensions( rWoodThin.GetWidth() * 0.8f, fWoodHeight );
	rWood.SetTextureSection( pWood );
	rWood.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );
	rWood.SetColour( 0xffffffff );
	rWood.SetTransform( 0.0f, fWoodHeight * 0.25f );
	rWoodThin.AddChildTail( rWood );

	AGUI2ModelRenderer& rCarousel = m_aModels[ MODEL_CAROUSEL ];
	rCarousel.Create( rWood.GetWidth() * 1.3f, rWood.GetHeight() * 1.8f, TFALSE, TTRUE );
	rCarousel.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	rCarousel.SetLightDirection( s_vLightDirection );
	rWood.AddChildTail( rCarousel );
	rCarousel.GetCamera()->SetFOV( TMath::PI / 3.0f );

	TMatrix44 matCamera;
	matCamera.Identity();
	matCamera.GetTranslation().Set( 0.0f, 0.0f, -1.5f, 1.0f );
	rCarousel.SetCameraMatrix( matCamera );

	// The arrows
	T2GUITextureSection* pLeftArrow   = T2GUITextureSectionManager::GetTextureSection( "left_arrow" );
	TFLOAT               fArrowHeight = fWoodHeight * 0.8f;
	m_aRects[ RECT_LEFT_ARROW ].SetDimensions( TMath::Abs( TFLOAT( pLeftArrow->GetPoint2X() - pLeftArrow->GetPoint1X() ) ), fArrowHeight );
	m_aRects[ RECT_LEFT_ARROW ].SetTextureSection( pLeftArrow );
	m_aRects[ RECT_LEFT_ARROW ].SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLERIGHT );
	rWood.AddChildTail( m_aRects[ RECT_LEFT_ARROW ] );

	T2GUITextureSection* pRightArrow = T2GUITextureSectionManager::GetTextureSection( "left_arrow2" );
	m_aRects[ RECT_RIGHT_ARROW ].SetDimensions( TMath::Abs( TFLOAT( pRightArrow->GetPoint2X() - pRightArrow->GetPoint1X() ) ), fArrowHeight );
	m_aRects[ RECT_RIGHT_ARROW ].SetTextureSection( pRightArrow );
	m_aRects[ RECT_RIGHT_ARROW ].SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLELEFT );
	rWood.AddChildTail( m_aRects[ RECT_RIGHT_ARROW ] );

	// The cooker (or the blender)
	if ( bCocktails )
	{
		m_pCookerTexture     = T2GUITextureSectionManager::GetTextureSection( "Blender" );
		m_pCookerOpenTexture = TNULL;
	}
	else
	{
		m_pCookerTexture     = T2GUITextureSectionManager::GetTextureSection( "Cooker" );
		m_pCookerOpenTexture = T2GUITextureSectionManager::GetTextureSection( "Cooker_Open" );
	}

	T2GUIRectangle& rCooker       = m_aRects[ RECT_COOKER ];
	TFLOAT          fCookerWidth  = rWoodThin.GetWidth();
	TFLOAT          fCookerHeight = fHeight * 0.5f;
	rCooker.SetDimensions( fCookerWidth, fCookerHeight );
	rCooker.SetTextureSection( m_pCookerTexture );
	rCooker.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );
	rWood.AddChildTail( rCooker );

	if ( bCocktails )
	{
		T2GUITextureSection* pBlenderTop = T2GUITextureSectionManager::GetTextureSection( "Blender_Top" );
		m_aRects[ RECT_BLENDER_TOP ].SetDimensions( fCookerWidth, fCookerHeight );
		m_aRects[ RECT_BLENDER_TOP ].SetTextureSection( pBlenderTop );
		m_aRects[ RECT_BLENDER_TOP ].SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
		rCooker.AddChildTail( m_aRects[ RECT_BLENDER_TOP ] );

		T2GUITextureSection* pBlenderLid = T2GUITextureSectionManager::GetTextureSection( "Bledner_Lid" );
		m_aRects[ RECT_BLENDER_LID ].SetDimensions( fCookerWidth, fCookerHeight );
		m_aRects[ RECT_BLENDER_LID ].SetTextureSection( pBlenderLid );
		m_aRects[ RECT_BLENDER_LID ].SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
		m_aRects[ RECT_BLENDER_LID ].Hide();
		rCooker.AddChildTail( m_aRects[ RECT_BLENDER_LID ] );
	}

	rCooker.SetTransform( 0.0f, rWood.GetHeight() );

	// The pans (the cooker only)
	T2GUITextureSection* pPan       = T2GUITextureSectionManager::GetTextureSection( "Cooking_pan" );
	TFLOAT               fPanWidth  = TMath::Abs( TFLOAT( pPan->GetPoint2X() - pPan->GetPoint1X() ) ) * m_fScaleX;
	TFLOAT               fPanHeight = TMath::Abs( TFLOAT( pPan->GetPoint2Y() - pPan->GetPoint1Y() ) ) * m_fScaleY;

	if ( !bCocktails )
	{
		TFLOAT fPanY = fPanHeight * 0.5f;

		for ( TINT i = RECT_PAN1; i <= RECT_PAN3; i++ )
		{
			T2GUIRectangle& rPan = m_aRects[ i ];
			rPan.SetDimensions( fPanWidth, fPanHeight );
			rPan.SetTextureSection( pPan );
			rPan.SetTransform( 0.0f, fPanY );
			rCooker.AddChildTail( rPan );

			switch ( i )
			{
				case RECT_PAN1:
					rPan.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
					break;
				case RECT_PAN2:
					rPan.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
					rPan.SetTransform( fPanWidth * -0.1f, fPanY );
					break;
				case RECT_PAN3:
					rPan.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );
					rPan.SetTransform( fPanWidth * -0.2f, fPanY );
					break;
			}
		}
	}

	// The ingredients in the pan (or in the blender)
	TFLOAT fSlotHeight = fPanHeight + fPanHeight;

	for ( TINT i = MODEL_SLOT1; i <= MODEL_SLOT3; i++ )
	{
		CreateSlot( i, fPanWidth, fSlotHeight );

		TFLOAT fSlotY = fSlotHeight * ( bCocktails ? -0.5f : -0.25f );

		switch ( i )
		{
			case MODEL_SLOT1:
				m_aModels[ i ].SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
				m_aModels[ i ].SetTransform( fPanWidth * 0.06f, fSlotY );
				break;
			case MODEL_SLOT2:
				m_aModels[ i ].SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
				m_aModels[ i ].SetTransform( fPanWidth * -0.02f, fSlotY );
				break;
			case MODEL_SLOT3:
				m_aModels[ i ].SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );
				m_aModels[ i ].SetTransform( fPanWidth * -0.11f, fSlotY );
				break;
		}
	}

	// The dish that was made
	AGUI2ModelRenderer& rDish = m_aModels[ MODEL_DISH ];
	rDish.Create( fCookerWidth, fCookerHeight, TFALSE, TTRUE );
	rDish.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	rDish.Hide();
	rCooker.AddChildTail( rDish );

	TCameraObject* pDishCamera = rDish.GetCamera();
	pDishCamera->SetFOV( TMath::PI / 6.0f );
	pDishCamera->SetNear( 0.01f );
	pDishCamera->SetFar( 10.0f );

	matCamera = TMatrix44::IDENTITY;

	if ( bCocktails )
		matCamera.GetTranslation().Set( 0.0f, -0.25f, -2.0f, 1.0f );
	else
		matCamera.GetTranslation().Set( 0.0f, -0.2f, -3.0f, 1.0f );

	rDish.SetCameraMatrix( matCamera );
	rDish.SetLightDirection( s_vLightDirection );

	// The smoke out of the pan
	if ( !bCocktails )
	{
		T2GUITextureSection* pSmoke = T2GUITextureSectionManager::GetTextureSection( "Smoke" );

		for ( TINT i = RECT_SMOKE1; i <= RECT_SMOKE3; i++ )
		{
			T2GUIRectangle& rSmoke = m_aRects[ i ];
			rSmoke.SetTextureSection( *pSmoke );
			rCooker.AddChildTail( rSmoke );
			rSmoke.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
			rSmoke.SetTransform( 0.0f, m_aRects[ RECT_PAN1 ].GetHeight() * -3.0f );
			rSmoke.Hide();
		}
	}

	// "Success" / "Failed", the name of the dish and its price
	AGUI2TextBox& rResult = m_aTexts[ TEXT_RESULT ];
	rResult.Create( m_pFont, fWidth );
	rResult.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	rResult.SetTextAlign( T2GUIFont::TextAlign_Center );
	rResult.SetColour( 0xffffc800 );
	rResult.SetScale( 1.0f );
	rResult.Hide();
	rResult.SetText( ALocaleManager::Interface()->GetString( 326 ) );
	m_pRoot->AddChildTail( rResult );

	AGUI2TextBox& rDishName = m_aTexts[ TEXT_DISH ];
	rDishName.Create( m_pFont, fWidth );
	rDishName.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );
	rDishName.SetTextAlign( T2GUIFont::TextAlign_Center );
	rDishName.SetColour( 0xffffffff );
	rDishName.SetScale( 1.0f );
	rDishName.SetText( m_wszDishName );
	rDishName.SetTransform( 0.0f, rDishName.GetHeight() * 0.5f );
	rResult.AddChildTail( rDishName );

	T2GUIRectangle&      rBuck = m_aRects[ RECT_BUCK ];
	T2GUITextureSection* pBuck = T2GUITextureSectionManager::GetTextureSection( "BarnYard_Buck" );
	rBuck.SetDimensions( TMath::Abs( TFLOAT( pBuck->GetPoint2X() - pBuck->GetPoint1X() ) ) * m_fScaleX, TMath::Abs( TFLOAT( pBuck->GetPoint2Y() - pBuck->GetPoint1Y() ) ) * m_fScaleY );
	rBuck.SetTextureSection( pBuck );
	rBuck.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_TOPLEFT );
	rDishName.AddChildTail( rBuck );

	AGUI2TextBox& rPrice = m_aTexts[ TEXT_PRICE ];
	rPrice.Create( m_pFont, fWidth );
	rPrice.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLELEFT );
	rPrice.SetTextAlign( T2GUIFont::TextAlign_Left );
	rPrice.SetColour( 0xffffc800 );
	rPrice.SetScale( 1.0f );
	rPrice.SetText( m_wszPrice );
	rBuck.AddChildTail( rPrice );

	rResult.SetTransform( 0.0f, fHeight * 0.1f );
	rMain.SetTransform( m_pFont->GetTextWidth( L"M", 1, 1.0f ) * -2.0f, 0.0f );

	for ( TINT i = 0; i < MODEL_NUMOF; i++ )
		m_aModels[ i ].UpdateViewport();
}

// $Barnyard: FUNCTION 004f92a0
void ACookingHUD::SetIngredientText( const TWCHAR* a_wszName, TINT a_iCount )
{
	AGUI2TextBox& rName  = m_aTexts[ TEXT_INGREDIENT ];
	AGUI2TextBox& rCount = m_aTexts[ TEXT_COUNT ];

	rName.Create( m_pFont, m_pFont->GetTextWidth( a_wszName, 1.0f ) );
	rName.SetText( a_wszName );

	TStringManager::String16Format( m_wszCount, TARRAYSIZE( m_wszCount ), L"x%i", a_iCount );
	rCount.Create( m_pFont, m_pFont->GetTextWidth( m_wszCount, 1.0f ) );
	rCount.SetText( m_wszCount );

	// The name and the count are centred together
	TFLOAT fWidth = rName.GetWidth() + ( rCount.GetWidth() + m_pFont->GetTextWidth( L"-", 1.0f ) );
	m_aElements[ ELEMENT_INGREDIENT ].SetDimensions( fWidth, m_aElements[ ELEMENT_INGREDIENT ].GetHeight() );
}

// $Barnyard: FUNCTION 004f9380
void ACookingHUD::SetCookerOpen( TBOOL a_bOpen )
{
	if ( m_pCookerOpenTexture )
		m_aRects[ RECT_COOKER ].SetTextureSection( a_bOpen ? m_pCookerOpenTexture : m_pCookerTexture );
}

// $Barnyard: FUNCTION 004f93b0
void ACookingHUD::SetDishOpacity( TFLOAT a_fOpacity )
{
	m_aModels[ MODEL_DISH ].SetObjectOpacity( 0, a_fOpacity );
}

// $Barnyard: FUNCTION 004f93d0
void ACookingHUD::SetPanOpacity( TFLOAT a_fOpacity )
{
	for ( TINT i = MODEL_SLOT1; i <= MODEL_SLOT3; i++ )
		m_aModels[ i ].SetObjectOpacity( 0, a_fOpacity );
}

// $Barnyard: FUNCTION 004f9400
void ACookingHUD::SetBlenderShaking( TBOOL a_bShaking )
{
	if ( a_bShaking )
	{
		m_bShaking    = TTRUE;
		m_fShakeTime  = 0.04f;
		m_fShakeAngle = s_fShakeAngle;
	}
	else
	{
		m_bShaking = TFALSE;
		m_aRects[ RECT_BLENDER_TOP ].SetTransform( 0.0f, 0.0f, 0.0f );
		m_aRects[ RECT_BLENDER_LID ].SetTransform( 0.0f, 0.0f, 0.0f );
	}
}

// $Barnyard: FUNCTION 004f9460
void ACookingHUD::ShowDishInfo( const TWCHAR* a_wszName, TINT a_iPrice )
{
	TStringManager::String16Format( m_wszDishName, TARRAYSIZE( m_wszDishName ), L"%s", a_wszName );
	TStringManager::String16Format( m_wszPrice, TARRAYSIZE( m_wszPrice ), L"x %i", a_iPrice );

	AGUI2TextBox&   rResult   = m_aTexts[ TEXT_RESULT ];
	AGUI2TextBox&   rDishName = m_aTexts[ TEXT_DISH ];
	AGUI2TextBox&   rPrice    = m_aTexts[ TEXT_PRICE ];
	T2GUIRectangle& rBuck     = m_aRects[ RECT_BUCK ];

	rDishName.SetText( m_wszDishName );
	rPrice.SetTextAndFitWidth( m_wszPrice );

	// The buck and the price are centred under the name
	TFLOAT fResultWidth = rResult.GetWidth();
	TFLOAT fBuckWidth   = rBuck.GetWidth();
	rBuck.SetTransform( ( fResultWidth - ( rPrice.GetWidth() + fBuckWidth ) ) * 0.5f, 0.0f, 0.0f );

	rResult.Show();
	rResult.SetAlpha( 1.0f );
	rResult.SetShadowAlpha( 1.0f );
	rDishName.Show();
	rDishName.SetAlpha( 1.0f );
	rDishName.SetShadowAlpha( 1.0f );
	rPrice.SetAlpha( 1.0f );
	rPrice.SetShadowAlpha( 1.0f );
	rBuck.SetAlpha( 1.0f );

	m_bFadingDishInfo = TFALSE;
}

// $Barnyard: FUNCTION 004f95b0
void ACookingHUD::ShowDish( AAnimatableObject* a_pDish )
{
	AGUI2ModelRenderer& rDish = m_aModels[ MODEL_DISH ];
	rDish.AddObject( a_pDish, &s_vLightColour, &s_vAmbientColour );
	rDish.Show();
	rDish.SetObjectOpacity( 0, 1.0f );

	for ( TINT i = MODEL_SLOT1; i <= MODEL_SLOT3; i++ )
		m_aModels[ i ].Hide();
}

// $Barnyard: FUNCTION 004f9610
void ACookingHUD::HideDish( AAnimatableObject* a_pDish )
{
	AGUI2ModelRenderer& rDish = m_aModels[ MODEL_DISH ];
	rDish.RemoveObject( a_pDish );
	rDish.Hide();

	for ( TINT i = MODEL_SLOT1; i <= MODEL_SLOT3; i++ )
		m_aModels[ i ].Show();
}

// $Barnyard: FUNCTION 004f9650
void ACookingHUD::SetResultText( TBOOL a_bSuccess )
{
	m_aTexts[ TEXT_RESULT ].SetText( ALocaleManager::Interface()->GetString( a_bSuccess ? 326 : 276 ) );
}

// $Barnyard: FUNCTION 004fa400
void ACookingHUD::StartSmoke()
{
	ASoundManager::GetSingleton()->PlayCue( soundbank::UI_BRAND );

	for ( TINT i = RECT_SMOKE1; i <= RECT_SMOKE3; i++ )
	{
		T2GUITransform& rTransform = m_aRects[ i ].GetTransform();
		rTransform.SetIdentity();
		rTransform.Scale( 2.0f, 0.8f );
		rTransform.SetTranslation( 0.0f, m_aRects[ RECT_PAN1 ].GetHeight() * -2.0f );

		m_aRects[ i ].Hide();
	}

	m_aRects[ RECT_SMOKE1 ].Show();
	m_aRects[ RECT_SMOKE1 ].SetAlpha( 0.0f );

	m_fSmokeTime = 0.0f;
	m_bSmoke     = TTRUE;
}

// $Barnyard: FUNCTION 004fa500
TBOOL ACookingHUD::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	TInputInterface::EVENT_TYPE eEventType = a_pInputEvent->GetEventType();

	if ( ( eEventType != TInputInterface::EVENT_TYPE_GONE_DOWN && eEventType != TInputInterface::EVENT_TYPE_REPEAT ) || m_iNumIngredients < 1 )
		return TFALSE;

	if ( a_eInputCommand == AInputCommand_Left )
	{
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUCLICK );
		m_aRects[ RECT_LEFT_ARROW ].GetTransform().SetScale( 1.25f, 1.25f );
		a_pInputEvent->StartRepeat( 0.5f, 0.2f );

		if ( m_eCarouselState == CAROUSELSTATE_MOVING_NEXT )
		{
			// Go back the way the carousel was moving
			TINT iCentre                    = m_aiCarousel[ CAROUSEL_CENTRE ];
			TINT iOut                       = m_aiCarousel[ CAROUSEL_OUT ];
			m_aiCarousel[ CAROUSEL_OUT ]    = m_aiCarousel[ CAROUSEL_RIGHT ];
			m_aiCarousel[ CAROUSEL_RIGHT ]  = iCentre;
			m_aiCarousel[ CAROUSEL_CENTRE ] = m_aiCarousel[ CAROUSEL_LEFT ];
			m_aiCarousel[ CAROUSEL_LEFT ]   = iOut;

			TVector4 vOut                   = m_avCarousel[ CAROUSEL_OUT ];
			m_avCarousel[ CAROUSEL_OUT ]    = m_avCarousel[ CAROUSEL_RIGHT ];
			m_avCarousel[ CAROUSEL_RIGHT ]  = m_avCarousel[ CAROUSEL_CENTRE ];
			m_avCarousel[ CAROUSEL_CENTRE ] = m_avCarousel[ CAROUSEL_LEFT ];
			m_avCarousel[ CAROUSEL_LEFT ]   = vOut;

			SetCarouselIngredients();

			ACookingIngredient* pIngredient = m_apIngredients[ m_aiCarousel[ CAROUSEL_CENTRE ] % m_iNumIngredients ];
			SetIngredientText( pIngredient->GetScreenName(), pIngredient->GetCount() );

			m_fFadeInTime    = 0.0f;
			m_fFadeOutTime   = 0.0f;
			m_eCarouselState = CAROUSELSTATE_MOVING_PREVIOUS;
		}
		else if ( m_iNumIngredients >= 4 || m_aiCarousel[ CAROUSEL_CENTRE ] > 0 )
		{
			m_eCarouselState = CAROUSELSTATE_START_PREVIOUS;
		}

		return TTRUE;
	}

	if ( a_eInputCommand == AInputCommand_Right )
	{
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUCLICK );
		m_aRects[ RECT_RIGHT_ARROW ].GetTransform().SetScale( 1.25f, 1.25f );
		a_pInputEvent->StartRepeat( 0.5f, 0.2f );

		if ( m_eCarouselState == CAROUSELSTATE_MOVING_PREVIOUS )
		{
			// Go back the way the carousel was moving
			TINT iCentre                    = m_aiCarousel[ CAROUSEL_CENTRE ];
			TINT iOut                       = m_aiCarousel[ CAROUSEL_OUT ];
			m_aiCarousel[ CAROUSEL_OUT ]    = m_aiCarousel[ CAROUSEL_LEFT ];
			m_aiCarousel[ CAROUSEL_CENTRE ] = m_aiCarousel[ CAROUSEL_RIGHT ];
			m_aiCarousel[ CAROUSEL_RIGHT ]  = iOut;
			m_aiCarousel[ CAROUSEL_LEFT ]   = iCentre;

			TVector4 vOut                   = m_avCarousel[ CAROUSEL_OUT ];
			m_avCarousel[ CAROUSEL_OUT ]    = m_avCarousel[ CAROUSEL_LEFT ];
			m_avCarousel[ CAROUSEL_LEFT ]   = m_avCarousel[ CAROUSEL_CENTRE ];
			m_avCarousel[ CAROUSEL_CENTRE ] = m_avCarousel[ CAROUSEL_RIGHT ];
			m_avCarousel[ CAROUSEL_RIGHT ]  = vOut;

			SetCarouselIngredients();

			ACookingIngredient* pIngredient = m_apIngredients[ m_aiCarousel[ CAROUSEL_CENTRE ] % m_iNumIngredients ];
			SetIngredientText( pIngredient->GetScreenName(), pIngredient->GetCount() );

			m_fFadeInTime    = 0.0f;
			m_fFadeOutTime   = 0.0f;
			m_eCarouselState = CAROUSELSTATE_MOVING_NEXT;
		}
		else if ( m_iNumIngredients >= 4 || m_aiCarousel[ CAROUSEL_CENTRE ] < m_iNumIngredients - 1 )
		{
			m_eCarouselState = CAROUSELSTATE_START_NEXT;
		}

		return TTRUE;
	}

	return TFALSE;
}

// NOTE: inlined in ProcessCommand (004fa500)
void ACookingHUD::SetCarouselIngredients()
{
	for ( TINT i = 0; i < CAROUSEL_NUMOF; i++ )
		m_apCarousel[ i ] = ( m_aiCarousel[ i ] == -1 ) ? TNULL : m_apIngredients[ m_aiCarousel[ i ] ];
}

// $Barnyard: FUNCTION 004fa990
ACookingIngredient* ACookingHUD::GetSelectedIngredient()
{
	if ( m_aiCarousel[ CAROUSEL_CENTRE ] < 0 )
		return TNULL;

	return m_apIngredients[ m_aiCarousel[ CAROUSEL_CENTRE ] ];
}

// $Barnyard: FUNCTION 004fa9b0
void ACookingHUD::AddToPan( ACookingIngredient* a_pIngredient )
{
	AGUI2ModelRenderer& rSlot = m_aModels[ MODEL_SLOT1 + m_iNumInPan ];
	rSlot.AddObject( a_pIngredient->GetObject(), &s_vLightColour, &s_vAmbientColour );
	rSlot.SetObjectOpacity( 0, 1.0f );
	rSlot.Show();

	m_apInPan[ m_iNumInPan++ ] = a_pIngredient;
	a_pIngredient->SetTime( 0.0f );
}

// $Barnyard: FUNCTION 004faa10
void ACookingHUD::ClearPan()
{
	for ( TINT i = 0; i < m_iNumInPan; i++ )
	{
		ACookingIngredient* pIngredient = m_apInPan[ i ];
		m_aModels[ MODEL_SLOT1 + i ].RemoveObject( pIngredient->GetObject() );
		m_aModels[ MODEL_SLOT1 + i ].Hide();
		pIngredient->GetObject()->SetOpacity( 1.0f, TTRUE );
	}

	for ( TINT i = 0; i < m_iNumInPan; i++ )
		m_apInPan[ i ] = TNULL;

	m_iNumInPan = 0;
}

// $Barnyard: FUNCTION 004faaa0
void ACookingHUD::RemoveLastFromPan()
{
	if ( m_iNumInPan == 0 )
		return;

	ACookingIngredient* pIngredient = m_apInPan[ m_iNumInPan - 1 ];
	m_apInPan[ --m_iNumInPan ]      = TNULL;

	m_aModels[ MODEL_SLOT1 + m_iNumInPan ].RemoveObject( pIngredient->GetObject() );
	m_aModels[ MODEL_SLOT1 + m_iNumInPan ].Hide();
	pIngredient->GetObject()->SetOpacity( 1.0f, TTRUE );
}

// $Barnyard: FUNCTION 004fab10
void ACookingHUD::SetNavHidden( TBOOL a_bHidden )
{
	AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>()->SetForceHide( a_bHidden );
}

// $Barnyard: FUNCTION 004fab30
void ACookingHUD::SetNavTexts( TINT a_iLeft, TINT a_iRight, TBOOL a_bCentre )
{
	AMenuNavHudElement* pMenuNav = AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>();

	if ( a_bCentre )
		pMenuNav->SetTexts( a_iLeft, a_iRight, ACookingMiniGame::IsCocktails() ? 860 : 859 );
	else
		pMenuNav->SetTexts( a_iLeft, a_iRight, -1 );
}

// $Barnyard: FUNCTION 004facd0
void ACookingHUD::PlaceIngredient( ACookingIngredient* a_pIngredient, TBOOL a_bSetPosition, TVector4 a_vPosition, TVector4 a_vRotation, TFLOAT a_fScale )
{
	AAnimatableObject* pObject = a_pIngredient->GetObject();
	AObjectTraits*     pTraits = a_pIngredient->GetTraits();

	TMatrix44 matTransform = TMatrix44::IDENTITY;
	matTransform.RotateX( TMath::HALF_PI );
	matTransform.Rotate( a_vRotation.x, a_vRotation.y, a_vRotation.z );
	matTransform.Rotate( pTraits->m_vecHoldRotate.x, pTraits->m_vecHoldRotate.y, pTraits->m_vecHoldRotate.z );

	TFLOAT fScale = pTraits->m_fItemScale * a_pIngredient->GetScale() * a_fScale;
	matTransform.Scale( fScale, fScale, fScale );

	if ( a_bSetPosition )
		matTransform.GetTranslation() = a_vPosition;

	if ( a_pIngredient->IsCentred() )
	{
		// Move the centre of the model to the position
		const TVector3& rcOrigin = pObject->GetModelInstance()->GetSceneObject()->GetInstance()->GetModel()->m_LODs[ 0 ].BoundingSphere.GetOrigin();
		TVector3        vecCentre( -rcOrigin.x, -rcOrigin.y, -rcOrigin.z );

		TMatrix44::TransformVector( matTransform.GetTranslation3(), matTransform, vecCentre );
	}

	pObject->GetModelInstance()->SetTransform( matTransform );
}

// $Barnyard: FUNCTION 004faeb0
void ACookingHUD::ResetCarousel()
{
	// The second ingredient is in the middle (the first one if there's only one)
	m_aiCarousel[ CAROUSEL_CENTRE ] = 1;

	if ( m_iNumIngredients == 0 )
	{
		m_aiCarousel[ CAROUSEL_CENTRE ] = -1;
		m_aiCarousel[ CAROUSEL_LEFT ]   = -1;
		m_aiCarousel[ CAROUSEL_RIGHT ]  = -1;
		m_aiCarousel[ CAROUSEL_OUT ]    = -1;
	}
	else if ( m_iNumIngredients == 1 )
	{
		m_aiCarousel[ CAROUSEL_CENTRE ] = 0;
		m_aiCarousel[ CAROUSEL_LEFT ]   = -1;
		m_aiCarousel[ CAROUSEL_RIGHT ]  = -1;
		m_aiCarousel[ CAROUSEL_OUT ]    = -1;
	}
	else
	{
		m_aiCarousel[ CAROUSEL_LEFT ] = 0;

		if ( m_iNumIngredients == 2 )
		{
			m_aiCarousel[ CAROUSEL_CENTRE ] = 1;
			m_aiCarousel[ CAROUSEL_RIGHT ]  = -1;
			m_aiCarousel[ CAROUSEL_OUT ]    = -1;
		}
		else
		{
			// NOTE: CAROUSEL_OUT is left as it is
			m_aiCarousel[ CAROUSEL_RIGHT ] = 2;
		}
	}

	AGUI2ModelRenderer& rCarousel = m_aModels[ MODEL_CAROUSEL ];

	for ( TINT i = 0; i < m_iNumIngredients; i++ )
	{
		ACookingIngredient* pIngredient = m_apIngredients[ i ];
		AAnimatableObject*  pObject     = pIngredient->GetObject();

		if ( i == m_aiCarousel[ CAROUSEL_LEFT ] )
		{
			rCarousel.AddObject( pObject, &s_vLightColour, &s_vAmbientColour );
			rCarousel.SetObjectOpacity( i, 0.0f );
			m_apCarousel[ CAROUSEL_LEFT ] = pIngredient;
			m_avCarousel[ CAROUSEL_LEFT ] = s_vCarouselLeft;
			PlaceIngredient( pIngredient, TTRUE, m_avCarousel[ CAROUSEL_LEFT ], TVector4::VEC_ZERO, 1.0f );
		}
		else if ( i == m_aiCarousel[ CAROUSEL_RIGHT ] )
		{
			rCarousel.AddObject( pObject, &s_vLightColour, &s_vAmbientColour );
			rCarousel.SetObjectOpacity( i, 0.0f );
			m_apCarousel[ CAROUSEL_RIGHT ] = pIngredient;
			m_avCarousel[ CAROUSEL_RIGHT ] = s_vCarouselRight;
			PlaceIngredient( pIngredient, TTRUE, m_avCarousel[ CAROUSEL_RIGHT ], TVector4::VEC_ZERO, 1.0f );
		}
		else if ( i == m_aiCarousel[ CAROUSEL_CENTRE ] )
		{
			rCarousel.AddObject( pObject, &s_vLightColour, &s_vAmbientColour );
			rCarousel.SetObjectOpacity( i, 1.0f );
			m_apCarousel[ CAROUSEL_CENTRE ] = pIngredient;
			m_avCarousel[ CAROUSEL_CENTRE ] = TVector4::VEC_ZERO;
			PlaceIngredient( pIngredient, TTRUE, m_avCarousel[ CAROUSEL_CENTRE ], TVector4::VEC_ZERO, 1.0f );
			SetIngredientText( pIngredient->GetScreenName(), pIngredient->GetCount() );
		}
		else
		{
			rCarousel.AddObject( pObject, &s_vLightColour, &s_vAmbientColour );
			rCarousel.SetObjectVisible( i, TFALSE );
		}
	}
}

// $Barnyard: FUNCTION 004fb150
void ACookingHUD::UpdateCarousel( TFLOAT a_fDeltaTime )
{
	AGUI2ModelRenderer& rCarousel = m_aModels[ MODEL_CAROUSEL ];
	ACookingIngredient* pCentre   = m_apIngredients[ m_aiCarousel[ CAROUSEL_CENTRE ] ];

	switch ( m_eCarouselState )
	{
		case CAROUSELSTATE_IDLE:
		{
			// The ingredient in the middle spins
			TVector4 vRotation = TVector4::VEC_ZERO;
			vRotation.z        = TMath::HALF_PI * m_fTime;
			PlaceIngredient( pCentre, TFALSE, TVector4::VEC_ZERO, vRotation, 1.0f );
			return;
		}
		case CAROUSELSTATE_START_NEXT:
		{
			if ( m_aiCarousel[ CAROUSEL_OUT ] != -1 )
				rCarousel.SetObjectVisible( m_aiCarousel[ CAROUSEL_OUT ], TFALSE );

			PlaceIngredient( m_apCarousel[ CAROUSEL_CENTRE ], TFALSE, TVector4::VEC_ZERO, TVector4::VEC_ZERO, 1.0f );

			// Everything moves to the left
			m_aiCarousel[ CAROUSEL_OUT ]    = m_aiCarousel[ CAROUSEL_LEFT ];
			m_aiCarousel[ CAROUSEL_LEFT ]   = m_aiCarousel[ CAROUSEL_CENTRE ];
			m_aiCarousel[ CAROUSEL_CENTRE ] = m_aiCarousel[ CAROUSEL_RIGHT ];
			m_aiCarousel[ CAROUSEL_RIGHT ]  = m_aiCarousel[ CAROUSEL_RIGHT ] + 1;

			TBOOL bHasRight = TTRUE;

			if ( m_iNumIngredients >= 4 )
			{
				if ( m_aiCarousel[ CAROUSEL_RIGHT ] >= m_iNumIngredients )
					m_aiCarousel[ CAROUSEL_RIGHT ] = 0;
			}
			else if ( m_aiCarousel[ CAROUSEL_RIGHT ] >= m_iNumIngredients )
			{
				m_aiCarousel[ CAROUSEL_RIGHT ] = -1;
				m_apCarousel[ CAROUSEL_RIGHT ] = TNULL;
				bHasRight                      = TFALSE;
			}

			if ( bHasRight )
			{
				// The new one comes from further right
				m_apCarousel[ CAROUSEL_RIGHT ] = m_apIngredients[ m_aiCarousel[ CAROUSEL_RIGHT ] ];
				rCarousel.SetObjectVisible( m_aiCarousel[ CAROUSEL_RIGHT ], TTRUE );
				m_avCarousel[ CAROUSEL_RIGHT ] = s_vCarouselRight;
				m_avCarousel[ CAROUSEL_RIGHT ].x += m_avCarousel[ CAROUSEL_RIGHT ].x;
				PlaceIngredient( m_apCarousel[ CAROUSEL_RIGHT ], TTRUE, m_avCarousel[ CAROUSEL_RIGHT ], TVector4::VEC_ZERO, 1.0f );
				rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_RIGHT ], 0.0f );
			}

			if ( m_aiCarousel[ CAROUSEL_OUT ] != -1 )
			{
				m_apCarousel[ CAROUSEL_OUT ] = m_apIngredients[ m_aiCarousel[ CAROUSEL_OUT ] ];
				m_avCarousel[ CAROUSEL_OUT ] = s_vCarouselLeft;
				rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_OUT ], 0.0f );
			}

			m_apCarousel[ CAROUSEL_CENTRE ] = m_apIngredients[ m_aiCarousel[ CAROUSEL_CENTRE ] ];
			m_avCarousel[ CAROUSEL_CENTRE ] = s_vCarouselRight;
			m_apCarousel[ CAROUSEL_LEFT ]   = m_apIngredients[ m_aiCarousel[ CAROUSEL_LEFT ] ];
			m_avCarousel[ CAROUSEL_LEFT ]   = TVector4::VEC_ZERO;
			rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_CENTRE ], 0.0f );
			rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_LEFT ], 1.0f );
			m_fFadeInTime  = 0.0f;
			m_fFadeOutTime = 0.0f;

			ACookingIngredient* pIngredient = m_apIngredients[ m_aiCarousel[ CAROUSEL_CENTRE ] % m_iNumIngredients ];
			SetIngredientText( pIngredient->GetScreenName(), pIngredient->GetCount() );

			m_eCarouselState = CAROUSELSTATE_MOVING_NEXT;
			return;
		}
		case CAROUSELSTATE_START_PREVIOUS:
		{
			if ( m_aiCarousel[ CAROUSEL_OUT ] != -1 )
				rCarousel.SetObjectVisible( m_aiCarousel[ CAROUSEL_OUT ], TFALSE );

			PlaceIngredient( m_apCarousel[ CAROUSEL_CENTRE ], TFALSE, TVector4::VEC_ZERO, TVector4::VEC_ZERO, 1.0f );

			// Everything moves to the right
			TINT iLeft                      = m_aiCarousel[ CAROUSEL_LEFT ];
			TINT iCentre                    = m_aiCarousel[ CAROUSEL_CENTRE ];
			m_aiCarousel[ CAROUSEL_OUT ]    = m_aiCarousel[ CAROUSEL_RIGHT ];
			m_aiCarousel[ CAROUSEL_CENTRE ] = iLeft;
			m_aiCarousel[ CAROUSEL_RIGHT ]  = iCentre;
			m_aiCarousel[ CAROUSEL_LEFT ]   = iLeft - 1;

			TBOOL bHasLeft = TTRUE;

			if ( m_iNumIngredients >= 4 )
			{
				if ( m_aiCarousel[ CAROUSEL_LEFT ] < 0 )
					m_aiCarousel[ CAROUSEL_LEFT ] = m_iNumIngredients - 1;
			}
			else if ( m_aiCarousel[ CAROUSEL_LEFT ] < 0 )
			{
				m_aiCarousel[ CAROUSEL_LEFT ] = -1;
				m_apCarousel[ CAROUSEL_LEFT ] = TNULL;
				bHasLeft                      = TFALSE;
			}

			if ( bHasLeft )
			{
				// The new one comes from further left
				rCarousel.SetObjectVisible( m_aiCarousel[ CAROUSEL_LEFT ], TTRUE );
				m_apCarousel[ CAROUSEL_LEFT ] = m_apIngredients[ m_aiCarousel[ CAROUSEL_LEFT ] ];
				m_avCarousel[ CAROUSEL_LEFT ] = s_vCarouselLeft;
				m_avCarousel[ CAROUSEL_LEFT ].x += m_avCarousel[ CAROUSEL_LEFT ].x;
				PlaceIngredient( m_apCarousel[ CAROUSEL_LEFT ], TTRUE, m_avCarousel[ CAROUSEL_LEFT ], TVector4::VEC_ZERO, 1.0f );
				rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_LEFT ], 0.0f );
			}

			m_apCarousel[ CAROUSEL_CENTRE ] = m_apIngredients[ m_aiCarousel[ CAROUSEL_CENTRE ] ];
			m_avCarousel[ CAROUSEL_CENTRE ] = s_vCarouselLeft;
			m_apCarousel[ CAROUSEL_RIGHT ]  = m_apIngredients[ m_aiCarousel[ CAROUSEL_RIGHT ] ];
			m_avCarousel[ CAROUSEL_RIGHT ]  = TVector4::VEC_ZERO;

			if ( m_aiCarousel[ CAROUSEL_OUT ] != -1 )
			{
				m_apCarousel[ CAROUSEL_OUT ] = m_apIngredients[ m_aiCarousel[ CAROUSEL_OUT ] ];
				m_avCarousel[ CAROUSEL_OUT ] = s_vCarouselRight;
				rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_OUT ], 0.0f );
			}

			rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_CENTRE ], 0.0f );
			rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_RIGHT ], 1.0f );
			m_fFadeInTime  = 0.0f;
			m_fFadeOutTime = 0.0f;

			ACookingIngredient* pIngredient = m_apIngredients[ m_aiCarousel[ CAROUSEL_CENTRE ] % m_iNumIngredients ];
			SetIngredientText( pIngredient->GetScreenName(), pIngredient->GetCount() );

			m_eCarouselState = CAROUSELSTATE_MOVING_PREVIOUS;
			return;
		}
		case CAROUSELSTATE_MOVING_NEXT:
		{
			if ( TMath::Abs( m_avCarousel[ CAROUSEL_CENTRE ].x ) <= 0.01f )
			{
				// Arrived
				m_avCarousel[ CAROUSEL_LEFT ] = s_vCarouselLeft;
				PlaceIngredient( m_apCarousel[ CAROUSEL_LEFT ], TTRUE, m_avCarousel[ CAROUSEL_LEFT ], TVector4::VEC_ZERO, 1.0f );

				if ( m_aiCarousel[ CAROUSEL_RIGHT ] != -1 )
				{
					m_avCarousel[ CAROUSEL_RIGHT ] = s_vCarouselRight;
					PlaceIngredient( m_apCarousel[ CAROUSEL_RIGHT ], TTRUE, m_avCarousel[ CAROUSEL_RIGHT ], TVector4::VEC_ZERO, 1.0f );
					rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_RIGHT ], 0.0f );
				}

				if ( m_aiCarousel[ CAROUSEL_OUT ] != -1 )
				{
					m_avCarousel[ CAROUSEL_OUT ] = s_vCarouselLeft;
					m_avCarousel[ CAROUSEL_OUT ].x += m_avCarousel[ CAROUSEL_OUT ].x;
					PlaceIngredient( m_apCarousel[ CAROUSEL_OUT ], TTRUE, m_avCarousel[ CAROUSEL_OUT ], TVector4::VEC_ZERO, 1.0f );
					rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_OUT ], 0.0f );
					rCarousel.SetObjectVisible( m_aiCarousel[ CAROUSEL_OUT ], TFALSE );
				}

				m_avCarousel[ CAROUSEL_CENTRE ] = TVector4::VEC_ZERO;
				PlaceIngredient( m_apCarousel[ CAROUSEL_CENTRE ], TTRUE, m_avCarousel[ CAROUSEL_CENTRE ], TVector4::VEC_ZERO, 1.0f );
				rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_CENTRE ], 1.0f );
				rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_LEFT ], 0.0f );

				m_fTime                      = 0.0f;
				m_apCarousel[ CAROUSEL_OUT ] = TNULL;
				m_aiCarousel[ CAROUSEL_OUT ] = -1;
				m_eCarouselState             = CAROUSELSTATE_IDLE;
				return;
			}

			if ( m_aiCarousel[ CAROUSEL_RIGHT ] != -1 )
			{
				m_avCarousel[ CAROUSEL_RIGHT ].x = TMath::LERP( m_avCarousel[ CAROUSEL_RIGHT ].x, s_vCarouselRight.x, a_fDeltaTime * 5.0f );

				TFLOAT fOpacity = m_apCarousel[ CAROUSEL_RIGHT ]->GetObject()->GetModelInstance()->GetOpacity();
				rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_RIGHT ], TMath::LERP( fOpacity, 0.0f, a_fDeltaTime * 10.0f ) );
			}

			if ( m_aiCarousel[ CAROUSEL_OUT ] != -1 )
			{
				m_avCarousel[ CAROUSEL_OUT ].x = TMath::LERP( m_avCarousel[ CAROUSEL_OUT ].x, s_vCarouselLeft.x + s_vCarouselLeft.x, a_fDeltaTime * 5.0f );

				TFLOAT fOpacity = m_apCarousel[ CAROUSEL_OUT ]->GetObject()->GetModelInstance()->GetOpacity();
				rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_OUT ], TMath::LERP( fOpacity, 0.0f, a_fDeltaTime * 10.0f ) );
			}

			TFLOAT fMove                      = a_fDeltaTime * 5.0f;
			m_avCarousel[ CAROUSEL_CENTRE ].x = TMath::LERP( m_avCarousel[ CAROUSEL_CENTRE ].x, 0.0f, fMove );
			m_avCarousel[ CAROUSEL_LEFT ].x   = TMath::LERP( m_avCarousel[ CAROUSEL_LEFT ].x, s_vCarouselLeft.x, fMove );

			// NOTE: m_fFadeInTime isn't clipped
			m_fFadeInTime += a_fDeltaTime;
			rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_CENTRE ], TMath::LERP( 0.0f, 1.0f, TMath::EaseInOut( m_fFadeInTime ) ) );

			m_fFadeOutTime += a_fDeltaTime * 4.0f;
			TMath::Clip( m_fFadeOutTime, 0.0f, 1.0f );
			rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_LEFT ], TMath::LERP( 0.0f, 1.0f, TMath::EaseInOut( 1.0f - m_fFadeOutTime ) ) );

			PlaceIngredient( m_apCarousel[ CAROUSEL_CENTRE ], TTRUE, m_avCarousel[ CAROUSEL_CENTRE ], TVector4::VEC_ZERO, 1.0f );
			PlaceIngredient( m_apCarousel[ CAROUSEL_LEFT ], TTRUE, m_avCarousel[ CAROUSEL_LEFT ], TVector4::VEC_ZERO, 1.0f );

			if ( m_aiCarousel[ CAROUSEL_RIGHT ] != -1 )
				PlaceIngredient( m_apCarousel[ CAROUSEL_RIGHT ], TTRUE, m_avCarousel[ CAROUSEL_RIGHT ], TVector4::VEC_ZERO, 1.0f );

			if ( m_aiCarousel[ CAROUSEL_OUT ] != -1 )
				PlaceIngredient( m_apCarousel[ CAROUSEL_OUT ], TTRUE, m_avCarousel[ CAROUSEL_OUT ], TVector4::VEC_ZERO, 1.0f );

			return;
		}
		case CAROUSELSTATE_MOVING_PREVIOUS:
		{
			if ( TMath::Abs( m_avCarousel[ CAROUSEL_CENTRE ].x ) <= 0.01f )
			{
				// Arrived
				if ( m_aiCarousel[ CAROUSEL_LEFT ] != -1 )
				{
					m_avCarousel[ CAROUSEL_LEFT ] = s_vCarouselLeft;
					PlaceIngredient( m_apCarousel[ CAROUSEL_LEFT ], TTRUE, m_avCarousel[ CAROUSEL_LEFT ], TVector4::VEC_ZERO, 1.0f );
					rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_LEFT ], 0.0f );
				}

				m_avCarousel[ CAROUSEL_RIGHT ] = s_vCarouselRight;
				PlaceIngredient( m_apCarousel[ CAROUSEL_RIGHT ], TTRUE, m_avCarousel[ CAROUSEL_RIGHT ], TVector4::VEC_ZERO, 1.0f );

				m_avCarousel[ CAROUSEL_CENTRE ] = TVector4::VEC_ZERO;
				PlaceIngredient( m_apCarousel[ CAROUSEL_CENTRE ], TTRUE, m_avCarousel[ CAROUSEL_CENTRE ], TVector4::VEC_ZERO, 1.0f );

				if ( m_aiCarousel[ CAROUSEL_OUT ] != -1 )
				{
					m_avCarousel[ CAROUSEL_OUT ] = s_vCarouselRight;
					m_avCarousel[ CAROUSEL_OUT ].x += m_avCarousel[ CAROUSEL_OUT ].x;
					PlaceIngredient( m_apCarousel[ CAROUSEL_OUT ], TTRUE, m_avCarousel[ CAROUSEL_OUT ], TVector4::VEC_ZERO, 1.0f );
					rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_OUT ], 0.0f );
					rCarousel.SetObjectVisible( m_aiCarousel[ CAROUSEL_OUT ], TFALSE );
				}

				rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_CENTRE ], 1.0f );
				rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_RIGHT ], 0.0f );

				m_fTime                      = 0.0f;
				m_apCarousel[ CAROUSEL_OUT ] = TNULL;
				m_aiCarousel[ CAROUSEL_OUT ] = -1;
				m_eCarouselState             = CAROUSELSTATE_IDLE;
				return;
			}

			if ( m_aiCarousel[ CAROUSEL_LEFT ] != -1 )
			{
				m_avCarousel[ CAROUSEL_LEFT ].x = TMath::LERP( m_avCarousel[ CAROUSEL_LEFT ].x, s_vCarouselLeft.x, a_fDeltaTime * 5.0f );

				TFLOAT fOpacity = m_apCarousel[ CAROUSEL_LEFT ]->GetObject()->GetModelInstance()->GetOpacity();
				rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_LEFT ], TMath::LERP( fOpacity, 0.0f, a_fDeltaTime * 10.0f ) );
			}

			if ( m_aiCarousel[ CAROUSEL_OUT ] != -1 )
			{
				m_avCarousel[ CAROUSEL_OUT ].x = TMath::LERP( m_avCarousel[ CAROUSEL_OUT ].x, s_vCarouselRight.x + s_vCarouselRight.x, a_fDeltaTime * 5.0f );

				TFLOAT fOpacity = m_apCarousel[ CAROUSEL_OUT ]->GetObject()->GetModelInstance()->GetOpacity();
				rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_OUT ], TMath::LERP( fOpacity, 0.0f, a_fDeltaTime * 10.0f ) );
			}

			TFLOAT fMove                      = a_fDeltaTime * 5.0f;
			m_avCarousel[ CAROUSEL_CENTRE ].x = TMath::LERP( m_avCarousel[ CAROUSEL_CENTRE ].x, 0.0f, fMove );
			m_avCarousel[ CAROUSEL_RIGHT ].x  = TMath::LERP( m_avCarousel[ CAROUSEL_RIGHT ].x, s_vCarouselRight.x, fMove );

			// NOTE: m_fFadeInTime isn't clipped
			m_fFadeInTime += a_fDeltaTime;
			rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_CENTRE ], TMath::LERP( 0.0f, 1.0f, TMath::EaseInOut( m_fFadeInTime ) ) );

			m_fFadeOutTime += a_fDeltaTime * 4.0f;
			TMath::Clip( m_fFadeOutTime, 0.0f, 1.0f );
			rCarousel.SetObjectOpacity( m_aiCarousel[ CAROUSEL_RIGHT ], TMath::LERP( 0.0f, 1.0f, TMath::EaseInOut( 1.0f - m_fFadeOutTime ) ) );

			PlaceIngredient( m_apCarousel[ CAROUSEL_CENTRE ], TTRUE, m_avCarousel[ CAROUSEL_CENTRE ], TVector4::VEC_ZERO, 1.0f );

			if ( m_aiCarousel[ CAROUSEL_LEFT ] != -1 )
				PlaceIngredient( m_apCarousel[ CAROUSEL_LEFT ], TTRUE, m_avCarousel[ CAROUSEL_LEFT ], TVector4::VEC_ZERO, 1.0f );

			PlaceIngredient( m_apCarousel[ CAROUSEL_RIGHT ], TTRUE, m_avCarousel[ CAROUSEL_RIGHT ], TVector4::VEC_ZERO, 1.0f );

			if ( m_aiCarousel[ CAROUSEL_OUT ] != -1 )
				PlaceIngredient( m_apCarousel[ CAROUSEL_OUT ], TTRUE, m_avCarousel[ CAROUSEL_OUT ], TVector4::VEC_ZERO, 1.0f );

			return;
		}
	}
}

// $Barnyard: FUNCTION 004fc320
void ACookingHUD::Update( TFLOAT a_fDeltaTime )
{
	// The texts of the dish fade out
	if ( m_bFadingDishInfo )
	{
		TFLOAT fAlpha = m_aTexts[ TEXT_RESULT ].GetAlpha() - a_fDeltaTime;

		if ( fAlpha < 0.0f )
		{
			fAlpha            = 0.0f;
			m_bFadingDishInfo = TFALSE;
		}

		m_aTexts[ TEXT_RESULT ].SetAlpha( fAlpha );
		m_aTexts[ TEXT_RESULT ].SetShadowAlpha( fAlpha );
		m_aTexts[ TEXT_DISH ].SetAlpha( fAlpha );
		m_aTexts[ TEXT_DISH ].SetShadowAlpha( fAlpha );
		m_aTexts[ TEXT_PRICE ].SetAlpha( fAlpha );
		m_aTexts[ TEXT_PRICE ].SetShadowAlpha( fAlpha );
		m_aRects[ RECT_BUCK ].SetAlpha( fAlpha );
	}

	// The top of the blender shakes
	if ( m_bShaking )
	{
		m_aRects[ RECT_BLENDER_TOP ].SetTransform( 0.0f, 0.0f, m_fShakeAngle );
		m_aRects[ RECT_BLENDER_LID ].SetTransform( 0.0f, 0.0f, m_fShakeAngle );

		m_fShakeTime -= a_fDeltaTime;

		if ( m_fShakeTime < 0.0f )
		{
			m_fShakeTime  = 0.04f;
			m_fShakeAngle = m_fShakeAngle * -1.0f;
		}
	}

	// The puffs of smoke go up, grow and fade
	for ( TINT i = RECT_SMOKE1; i <= RECT_SMOKE3; i++ )
	{
		T2GUIRectangle& rSmoke = m_aRects[ i ];

		if ( !rSmoke.IsVisible() )
			continue;

		T2GUITransform& rTransform = rSmoke.GetTransform();
		TFLOAT          fScaleX    = rTransform.GetScaleX();
		TFLOAT          fScaleY    = rTransform.GetScaleY();
		TFLOAT          fX         = rTransform.GetTranslation().x;
		TFLOAT          fY         = rTransform.GetTranslation().y;

		TFLOAT fAlpha = TMath::LERP( rSmoke.GetAlpha(), ( fScaleY <= 2.0f ) ? 0.75f : 0.0f, a_fDeltaTime );
		rSmoke.SetAlpha( fAlpha );

		fY      = fY - a_fDeltaTime * 30.0f;
		fScaleY = TMath::LERP( fScaleY, 5.0f, a_fDeltaTime * 0.5f );

		rTransform.SetIdentity();
		rTransform.Scale( fScaleX, fScaleY );
		rTransform.SetTranslation( fX, fY );

		if ( fAlpha < TMath::TFLOAT_EPSILON )
			rSmoke.Hide();
	}

	// Two more puffs of smoke after the first one
	if ( m_bSmoke )
	{
		m_fSmokeTime += a_fDeltaTime;
		T2GUIRectangle& rPan = m_aRects[ RECT_PAN1 ];

		if ( m_fSmokeTime >= 0.5f && !m_aRects[ RECT_SMOKE3 ].IsVisible() )
		{
			T2GUIRectangle& rSmoke = m_aRects[ RECT_SMOKE3 ];
			rSmoke.Show();
			rSmoke.SetAlpha( 0.0f );
			m_bSmoke = TFALSE;

			T2GUITransform& rTransform = rSmoke.GetTransform();
			rTransform.SetIdentity();
			rTransform.Scale( 1.5f, 0.8f );

			TFLOAT fY = rPan.GetHeight() * -2.0f;
			rTransform.SetTranslation( rPan.GetWidth() * 0.2f, fY );
		}
		else if ( m_fSmokeTime >= 0.25f && !m_aRects[ RECT_SMOKE2 ].IsVisible() )
		{
			T2GUIRectangle& rSmoke = m_aRects[ RECT_SMOKE2 ];
			rSmoke.Show();
			rSmoke.SetAlpha( 0.0f );

			T2GUITransform& rTransform = rSmoke.GetTransform();
			rTransform.SetIdentity();
			rTransform.Scale( 1.7f, 0.8f );

			TFLOAT fY = rPan.GetHeight() * -2.0f;
			rTransform.SetTranslation( rPan.GetWidth() * -0.2f, fY );
		}
	}

	// Is the mouse on the arrows
	if ( m_bMouseMoved == TTRUE )
	{
		T2GUITransform oInvTransform;
		TVector2       vecCursorPos = AGUI2::GetMouseCursor().GetCursorPos();
		TVector2       vecLocalPos;

		m_bMouseOnOK = TFALSE;

		m_aRects[ RECT_LEFT_ARROW ].GetInvScreenTransform( oInvTransform );
		oInvTransform.Transform( vecLocalPos, vecCursorPos );
		m_bMouseOnLeftArrow = m_aRects[ RECT_LEFT_ARROW ].IsPointInside( vecLocalPos.x, vecLocalPos.y );

		if ( m_bMouseOnOK == TFALSE && m_bMouseOnLeftArrow == TFALSE )
		{
			m_aRects[ RECT_RIGHT_ARROW ].GetInvScreenTransform( oInvTransform );
			oInvTransform.Transform( vecLocalPos, vecCursorPos );
			m_bMouseOnRightArrow = m_aRects[ RECT_RIGHT_ARROW ].IsPointInside( vecLocalPos.x, vecLocalPos.y );
		}

		m_bMouseMoved = TFALSE;
	}

	// The ingredients in the pan spin
	m_fPanTime += a_fDeltaTime;

	for ( TINT i = 0; i < m_iNumInPan; i++ )
	{
		ACookingIngredient* pIngredient = m_apInPan[ i ];

		TVector4 vRotation = TVector4::VEC_ZERO;
		vRotation.z        = TMath::HALF_PI * pIngredient->GetTime();
		PlaceIngredient( pIngredient, TFALSE, TVector4::VEC_ZERO, vRotation, 0.8f );

		pIngredient->AddTime( a_fDeltaTime );
	}

	// The arrows are shown when the carousel can be scrolled
	m_aRects[ RECT_LEFT_ARROW ].SetVisible( ( m_iNumIngredients >= 4 || m_aiCarousel[ CAROUSEL_CENTRE ] != 0 ) && m_iNumIngredients != 0 );
	m_aRects[ RECT_RIGHT_ARROW ].SetVisible( ( m_iNumIngredients >= 4 || m_aiCarousel[ CAROUSEL_CENTRE ] != m_iNumIngredients - 1 ) && m_iNumIngredients != 0 );

	// The arrow under the mouse is bigger
	m_fTime += a_fDeltaTime;

	TFLOAT fRightScale = ( m_bMouseOnRightArrow == TTRUE ) ? 1.25f : 1.0f;
	TFLOAT fLeftScale  = ( m_bMouseOnLeftArrow == TTRUE ) ? 1.25f : 1.0f;
	TFLOAT fScaleSpeed = a_fDeltaTime * 10.0f;

	T2GUITransform& rRightArrow = m_aRects[ RECT_RIGHT_ARROW ].GetTransform();
	rRightArrow.SetScale( TMath::LERP( rRightArrow.GetScaleX(), fRightScale, fScaleSpeed ), TMath::LERP( rRightArrow.GetScaleY(), fRightScale, fScaleSpeed ) );

	T2GUITransform& rLeftArrow = m_aRects[ RECT_LEFT_ARROW ].GetTransform();
	rLeftArrow.SetScale( TMath::LERP( rLeftArrow.GetScaleX(), fLeftScale, fScaleSpeed ), TMath::LERP( rLeftArrow.GetScaleY(), fLeftScale, fScaleSpeed ) );

	if ( m_iNumIngredients > 0 )
		UpdateCarousel( a_fDeltaTime );

	// The dish spins
	AGUI2ModelRenderer& rDish = m_aModels[ MODEL_DISH ];

	if ( rDish.IsVisible() )
	{
		TMatrix44 matTransform = TMatrix44::IDENTITY;
		matTransform.RotateX( TMath::HALF_PI + TMath::DegToRad( 20.0f ) );
		matTransform.RotateZ( TMath::HALF_PI * m_fTime );

		rDish.GetObject( 0 )->GetModelInstance()->SetTransform( matTransform );
	}
}
