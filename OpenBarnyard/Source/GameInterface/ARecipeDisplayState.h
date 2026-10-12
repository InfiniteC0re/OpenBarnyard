#pragma once
#include "AGameState.h"
#include "GUI/AGUI2TextBox.h"
#include "GUI/AGUI2ModelRenderer.h"
#include "MiniGame/ACookingIngredient.h"
#include "MiniGame/ACookingRecipe.h"

#include <GUI/T2GUIRectangle.h>

//-----------------------------------------------------------------------------
// Purpose: Pip's cookbook. Shows the recipe the player has just found or,
// opened from the inventory ("cookbook" / "cocktails"), lets the player page
// through the found recipes (or cocktails) with the models of their
// ingredients, with a page that tells how many of them were found.
// TClass 00781dd0, vtable 0071bb60.
// NOTE: OnActivate/OnDeactivate (+0x60/+0x64) are 004490f0/00449110, shared with
// AHintDisplayState (same layout of the background).
//-----------------------------------------------------------------------------
class ARecipeDisplayState
    : public AGameState
{
public:
	TDECLARE_CLASS( ARecipeDisplayState, AGameState );

	static constexpr TINT MAX_INGREDIENTS = 32;
	static constexpr TINT MAX_RECIPES     = 40;

	using INPUT = TUINT32;
	enum INPUT_ : INPUT
	{
		INPUT_NONE     = 0,
		INPUT_UP       = BITFLAG( 0 ),
		INPUT_DOWN     = BITFLAG( 1 ),
		INPUT_PREVIOUS = BITFLAG( 2 ),
		INPUT_NEXT     = BITFLAG( 3 ),
		INPUT_OK       = BITFLAG( 4 ),
		INPUT_CANCEL   = BITFLAG( 5 ),
	};

	using STATE = TUINT32;
	enum STATE_ : STATE
	{
		STATE_START,   // starts the loading screen
		STATE_LOADING, // waits for the terrain
		STATE_SHOWING,
		STATE_CREATE, // creates the GUI (set by OnInsertion)
		STATE_LOADED, // the terrain is loaded, creates the GUI
	};

public:
	// Shows the recipe the player is collecting
	ARecipeDisplayState();

	// a_bCocktails: show the found cocktails instead of the found recipes
	ARecipeDisplayState( TBOOL a_bCocktails );
	~ARecipeDisplayState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x4c
	virtual void  OnInsertion() OVERRIDE;                                                                                                                  // +0x50
	virtual void  OnRemoval() OVERRIDE;                                                                                                                    // +0x54
	virtual void  OnActivate() OVERRIDE;                                                                                                                   // +0x60
	virtual void  OnDeactivate() OVERRIDE;                                                                                                                 // +0x64

private:
	void Create();
	void CreateGUI();
	void UpdatePage();

	// Reads the ingredients and the recipes from the cooking asset pack
	void LoadRecipes();

	ACookingIngredient* FindIngredient( const TCHAR* a_szName );

	// Places the model of a_pIngredient to be shown by an AGUI2ModelRenderer
	void PlaceIngredient( ACookingIngredient* a_pIngredient, TBOOL a_bSetPosition, Toshi::TVector4 a_vPosition, Toshi::TVector4 a_vRotation, TFLOAT a_fScale );

	// Recipes of the shown type (recipes or cocktails)
	TBOOL IsShownType( TINT a_iRecipe ) const { return m_bCocktails == m_oRecipes[ a_iRecipe ].IsCocktail(); }
	TBOOL IsShownAndFound( TINT a_iRecipe ) const { return m_pRecipesFound[ a_iRecipe ] && IsShownType( a_iRecipe ); }

	TINT GetNumFoundRecipes();
	TINT GetNumRecipes();
	TINT GetNextRecipe();
	TINT GetPreviousRecipe();
	TINT GetLastRecipe();
	TINT GetFirstRecipe();

	// Returns the number of the page of the current recipe
	TINT GetPageNumber();

private:
	Toshi::T2GUIRectangle                             m_oBackground;                                          // 0x0a4
	Toshi::T2GUIRectangle                             m_oPage;                                                // 0x11c, "Recipie_Page"
	Toshi::T2GUIRectangle                             m_oRecipeNameArea;                                      // 0x194
	Toshi::T2GUIRectangle                             m_oNavigation;                                          // 0x20c, the arrows and the page number
	Toshi::T2GUIRectangle                             m_oPageNumberArea;                                      // 0x284
	Toshi::T2GUIRectangle                             m_oIngredientsArea;                                     // 0x2fc
	Toshi::T2GUIRectangle                             m_aIngredientAreas[ ACookingRecipe::MAX_INGREDIENTS ];  // 0x374
	Toshi::T2GUIRectangle                             m_oProgressArea;                                        // 0x4dc
	AGUI2TextBox                                      m_oProgressTitle;                                       // 0x554
	AGUI2TextBox                                      m_oProgress;                                            // 0x5cc, "n%" of the recipes found
	AGUI2TextBox                                      m_oTitle;                                               // 0x644
	AGUI2TextBox                                      m_oPrompt;                                              // 0x6bc
	AGUI2TextBox                                      m_oRecipeName;                                          // 0x734
	AGUI2TextBox                                      m_aIngredientNames[ ACookingRecipe::MAX_INGREDIENTS ];  // 0x7ac
	AGUI2TextBox                                      m_aPlusSigns[ ACookingRecipe::MAX_INGREDIENTS ];        // 0x914, the last one is hidden
	AGUI2TextBox                                      m_oPageLabel;                                           // 0xa7c
	AGUI2TextBox                                      m_oPageNumber;                                          // 0xaf4
	Toshi::T2GUIRectangle                             m_oLeftArrow;                                           // 0xb6c
	Toshi::T2GUIRectangle                             m_oRightArrow;                                          // 0xbe4
	AGUI2ModelRenderer                                m_aIngredientModels[ ACookingRecipe::MAX_INGREDIENTS ]; // 0xc5c
	ACookingList<ACookingIngredient, MAX_INGREDIENTS> m_oIngredients;                                         // 0xddc
	ACookingList<ACookingRecipe, MAX_RECIPES>         m_oRecipes;                                             // 0x12e0
	TBOOL*                                            m_pRecipesFound;                                        // 0x2004, AHintManager::GetRecipes
	TINT                                              m_iRecipe;                                              // 0x2008, -1 shows the progress page
	TBOOL                                             m_bUnk200C;                                             // 0x200c
	TBOOL                                             m_bCollectRecipe;                                       // 0x200d
	TWCHAR                                            m_wszPrompt[ 256 ];                                     // 0x200e
	TWCHAR                                            m_wszPageNumber[ 16 ];                                  // 0x220e
	TWCHAR                                            m_wszUnk222E[ 8 ];                                      // 0x222e
	TWCHAR                                            m_wszUnk223E[ 8 ];                                      // 0x223e
	TWCHAR                                            m_wszProgress[ 8 ];                                     // 0x224e
	TFLOAT                                            m_fTime;                                                // 0x2260, seconds since the last analog stick page change
	INPUT                                             m_eInput;                                               // 0x2264
	TINT                                              m_iUnk2268;                                             // 0x2268
	TINT                                              m_iUnk226C;                                             // 0x226c
	TBOOL                                             m_bCocktails;                                           // 0x2270
	Toshi::T2GUIRectangle                             m_oFade;                                                // 0x2274
	STATE                                             m_eState;                                               // 0x22ec
	TFLOAT                                            m_fLoadTime;                                            // 0x22f0
};

TSTATICASSERT( sizeof( ARecipeDisplayState ) == 0x22f4 );
