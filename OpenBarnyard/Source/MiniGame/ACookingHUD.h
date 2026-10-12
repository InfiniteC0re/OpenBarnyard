#pragma once
#include "ACookingIngredient.h"
#include "GUI/AGUI2TextBox.h"
#include "GUI/AGUI2ModelRenderer.h"
#include "Input/AInputMap.h"

#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUITextureSection.h>
#include <Input/TInputInterface.h>
#include <Math/TVector2.h>
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Purpose: the GUI of ACookingMiniGame: the carousel of the ingredients the
// player has (with the arrows), the cooker (or the blender) with the pan and
// up to three ingredients in it, the smoke, and the dish that was made with
// its name and price.
// NOTE: the name is invented, the class has no RTTI. Allocated by
// ACookingMiniGame (0x11ac bytes).
//-----------------------------------------------------------------------------
class ACookingHUD
{
public:
	static constexpr TINT MAX_INGREDIENTS = 18;

	enum MODEL
	{
		MODEL_CAROUSEL, // the ingredients the player can choose from
		MODEL_SLOT1,    // the ingredients in the pan
		MODEL_SLOT2,
		MODEL_SLOT3,
		MODEL_DISH, // the dish that was made
		MODEL_NUMOF
	};

	enum TEXT
	{
		TEXT_INGREDIENT, // name of the ingredient in the middle of the carousel
		TEXT_COUNT,      // "x%i" of it
		TEXT_RESULT,     // the dish was made or it failed
		TEXT_DISH,       // name of the dish
		TEXT_PRICE,      // "x %i" bucks
		TEXT_UNUSED,
		TEXT_NUMOF
	};

	enum RECT
	{
		RECT_WOOD, // background of the carousel
		RECT_UNUSED,
		RECT_WOOD_THIN, // background of the name of the ingredient
		RECT_COOKER,    // "Cooker" or "Blender"
		RECT_BLENDER_TOP,
		RECT_BLENDER_LID,
		RECT_PAN1,
		RECT_PAN2,
		RECT_PAN3,
		RECT_LEFT_ARROW,
		RECT_RIGHT_ARROW,
		RECT_SMOKE1,
		RECT_SMOKE2,
		RECT_SMOKE3,
		RECT_BUCK, // next to the price of the dish
		RECT_NUMOF
	};

	enum ELEMENT
	{
		ELEMENT_UNUSED,
		ELEMENT_MAIN,       // everything but the result texts
		ELEMENT_INGREDIENT, // the name of the ingredient
		ELEMENT_NUMOF
	};

	// The carousel shows the current ingredient, the previous and the next one (and the one going away)
	enum CAROUSEL
	{
		CAROUSEL_CENTRE,
		CAROUSEL_LEFT,
		CAROUSEL_RIGHT,
		CAROUSEL_OUT,
		CAROUSEL_NUMOF
	};

	using CAROUSELSTATE = TINT;
	enum CAROUSELSTATE_ : CAROUSELSTATE
	{
		CAROUSELSTATE_IDLE,
		CAROUSELSTATE_START_NEXT,     // Right was pressed
		CAROUSELSTATE_MOVING_NEXT,    // the ingredients move to the left
		CAROUSELSTATE_START_PREVIOUS, // Left was pressed
		CAROUSELSTATE_MOVING_PREVIOUS,
	};

public:
	ACookingHUD();
	~ACookingHUD();

	// Creates the GUI in a_pRoot
	void Create( Toshi::T2GUIElement* a_pRoot );

	// Removes the GUI
	void Destroy();

	// Scrolls the carousel with Left and Right
	TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents );

	void Update( TFLOAT a_fDeltaTime );

	// Puts the ingredients (m_apIngredients) into the carousel
	void ResetCarousel();

	// The ingredient in the middle of the carousel (TNULL if there are none)
	ACookingIngredient* GetSelectedIngredient();

	void SetIngredientText( const TWCHAR* a_wszName, TINT a_iCount );

	// Adds a_pIngredient to the pan
	void AddToPan( ACookingIngredient* a_pIngredient );

	// Removes the last ingredient of the pan
	void RemoveLastFromPan();

	// Removes all ingredients of the pan
	void ClearPan();

	void SetPanOpacity( TFLOAT a_fOpacity );

	// Shows/hides the cooked dish
	void ShowDish( AAnimatableObject* a_pDish );
	void HideDish( AAnimatableObject* a_pDish );
	void SetDishOpacity( TFLOAT a_fOpacity );

	// Shows the name and the price of the dish
	void ShowDishInfo( const TWCHAR* a_wszName, TINT a_iPrice );

	// Starts fading out the texts of the dish
	// $Barnyard: FUNCTION 004f95a0
	void FadeDishInfo() { m_bFadingDishInfo = TTRUE; }

	// "Success" or "Failed"
	void SetResultText( TBOOL a_bSuccess );

	// a_bOpen: shows Cooker_Open (the cooker only)
	void SetCookerOpen( TBOOL a_bOpen );

	// Shakes the top of the blender
	void SetBlenderShaking( TBOOL a_bShaking );

	// Puffs of smoke out of the pan
	void StartSmoke();

	void  SetMouseMoved() { m_bMouseMoved = TTRUE; }
	TBOOL IsMouseOnOK() const { return m_bMouseOnOK; }
	TBOOL IsMouseOnLeftArrow() const { return m_bMouseOnLeftArrow; }
	TBOOL IsMouseOnRightArrow() const { return m_bMouseOnRightArrow; }

	// Sets the ingredients the carousel shows (ACookingMiniGame fills them)
	void ClearIngredients() { m_iNumIngredients = 0; }
	void AddIngredient( ACookingIngredient* a_pIngredient ) { m_apIngredients[ m_iNumIngredients++ ] = a_pIngredient; }

	Toshi::T2GUIRectangle& GetRectangle( RECT a_eRect ) { return m_aRects[ a_eRect ]; }

	// Hides/shows AMenuNavHudElement
	static void SetNavHidden( TBOOL a_bHidden );

	// Sets the texts of AMenuNavHudElement, a_bCentre adds "Recipe book" (or "Cocktail book")
	static void SetNavTexts( TINT a_iLeft, TINT a_iRight, TBOOL a_bCentre );

	// Places the model of a_pIngredient (same as ARecipeDisplayState::PlaceIngredient)
	static void PlaceIngredient( ACookingIngredient* a_pIngredient, TBOOL a_bSetPosition, Toshi::TVector4 a_vPosition, Toshi::TVector4 a_vRotation, TFLOAT a_fScale );

private:
	void CreateGUI();

	// Creates the model renderer of a slot of the pan
	void CreateSlot( TINT a_iSlot, TFLOAT a_fWidth, TFLOAT a_fHeight );

	// Moves the carousel
	void UpdateCarousel( TFLOAT a_fDeltaTime );

	void SetCarouselIngredients();

private:
	TFLOAT                      m_fWidth;                           // 0x00
	TFLOAT                      m_fHeight;                          // 0x04
	TFLOAT                      m_fScaleX;                          // 0x08, m_fWidth / 800
	TFLOAT                      m_fScaleY;                          // 0x0c, m_fHeight / 600
	Toshi::T2GUIFont*           m_pFont;                            // 0x10
	TBOOL                       m_bMouseMoved;                      // 0x14
	TBOOL                       m_bMouseOnOK;                       // 0x15, never set
	TBOOL                       m_bMouseOnLeftArrow;                // 0x16
	TBOOL                       m_bMouseOnRightArrow;               // 0x17
	TINT                        m_iNumIngredients;                  // 0x18
	ACookingIngredient*         m_apIngredients[ MAX_INGREDIENTS ]; // 0x1c
	TINT                        m_iNumInPan;                        // 0x64
	ACookingIngredient*         m_apInPan[ MAX_INGREDIENTS ];       // 0x68, at most 3 are used
	TFLOAT                      m_fFadeInTime;                      // 0xb0, of the ingredient coming to the middle
	TFLOAT                      m_fFadeOutTime;                     // 0xb4, of the ingredient leaving the middle
	ACookingIngredient*         m_apCarousel[ CAROUSEL_NUMOF ];     // 0xb8
	Toshi::TVector4             m_avCarousel[ CAROUSEL_NUMOF ];     // 0xc8, positions
	Toshi::T2GUIElement*        m_pRoot;                            // 0x108
	AGUI2ModelRenderer          m_aModels[ MODEL_NUMOF ];           // 0x10c
	AGUI2TextBox                m_aTexts[ TEXT_NUMOF ];             // 0x38c
	Toshi::T2GUIRectangle       m_aRects[ RECT_NUMOF ];             // 0x65c
	Toshi::T2GUIElement         m_aElements[ ELEMENT_NUMOF ];       // 0xd64
	TWCHAR                      m_wszCount[ 128 ];                  // 0xe60
	TINT                        m_aiCarousel[ CAROUSEL_NUMOF ];     // 0xf60, indices of m_apIngredients (-1 if none)
	CAROUSELSTATE               m_eCarouselState;                   // 0xf70
	TFLOAT                      m_fTime;                            // 0xf74, spins the ingredient in the middle and the dish
	TFLOAT                      m_fSmokeTime;                       // 0xf78
	TBOOL                       m_bSmoke;                           // 0xf7c
	TFLOAT                      m_fPanTime;                         // 0xf80
	TINT                        m_iUnkF84;                          // 0xf84
	TINT                        m_iUnkF88;                          // 0xf88
	Toshi::TVector2             m_vUnkF8C;                          // 0xf8c
	Toshi::T2GUITextureSection* m_pCookerTexture;                   // 0xf94, "Cooker" or "Blender"
	Toshi::T2GUITextureSection* m_pCookerOpenTexture;               // 0xf98, "Cooker_Open" (TNULL for the blender)
	TBOOL                       m_bShaking;                         // 0xf9c
	TFLOAT                      m_fShakeTime;                       // 0xfa0
	TFLOAT                      m_fShakeAngle;                      // 0xfa4
	TWCHAR                      m_wszDishName[ 128 ];               // 0xfa8
	TWCHAR                      m_wszPrice[ 128 ];                  // 0x10a8
	TBOOL                       m_bFadingDishInfo;                  // 0x11a8
};

TSTATICASSERT( sizeof( ACookingHUD ) == 0x11ac );
