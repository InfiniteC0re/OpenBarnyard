#pragma once
#include "GameInterface/AGameState.h"
#include "GameInterface/AMessagePopupState.h"
#include "ACookingIngredient.h"
#include "ACookingRecipe.h"

#include <File/TTRB.h>
#include <Toshi/T2Vector.h>
#include <Toshi/TEvent.h>

class ACookingHUD;
class AAnimatableObject;
class AStartMenuState;

//-----------------------------------------------------------------------------
// Purpose: the cooking minigame, started by the "startgamestate" function call
// of an interaction (Pip's kitchen), or the cocktail one when it's started from
// the barn (ABarnExploreMinigameState). The player puts up to three ingredients
// of the inventory in the pan (or the blender) and the pig cooks them: a recipe
// of cooking.AllRecipes gives its dish and bucks, anything else gives slop (or
// swill). Leaving it takes the used ingredients from the inventory, gives the
// dishes of the current quest and the bucks, and returns to the sim.
// TClass 007833b8, vtable 00721788, size 0x2580.
//-----------------------------------------------------------------------------
class ACookingMiniGame
    : public AGameState
{
public:
	TDECLARE_CLASS( ACookingMiniGame, AGameState );

	static constexpr TINT MAX_INGREDIENTS = 32;
	static constexpr TINT MAX_RECIPES     = 40;
	static constexpr TINT MAX_OWNED       = 18;
	static constexpr TINT MAX_FREE        = MAX_INGREDIENTS * ACookingRecipe::MAX_INGREDIENTS;
	static constexpr TINT NUM_ANIMS       = 10;

	using STATE = TINT;
	enum STATE_ : STATE
	{
		STATE_CHOOSING,    // the player chooses the ingredients
		STATE_REMOVING,    // the last ingredient fades out of the pan
		STATE_START_CLEAR, // never set
		STATE_CLEARING,    // the ingredients fade out of the pan
		STATE_COOKING,     // the pig cooks while it talks
		STATE_WAITING,     // the cooking sound plays
		STATE_RESULT,      // the dish is shown until the player presses OK
		STATE_HIDING,      // the dish fades out
		STATE_CLEANING,    // waits for the pig to put the dish away
	};

	// Animations of the pig (names in cookingstrings.StringTable)
	enum ANIM
	{
		ANIM_IDLE,
		ANIM_IDLE2,
		ANIM_IDLE3,
		ANIM_WAIT,       // after ANIM_PRESENT
		ANIM_PRESENT,    // presents the dish
		ANIM_START_COOK, // then ANIM_COOK
		ANIM_COOK,
		ANIM_PUT_AWAY, // puts the dish away
		ANIM_UNUSED8,
		ANIM_UNUSED9,
	};

public:
	ACookingMiniGame();
	~ACookingMiniGame();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE;                                                        // +0x0c
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual void  OnLoaded() OVERRIDE;                                                                                                                     // +0x24
	virtual TBOOL ShouldPlacePlayerAtStartGoal() OVERRIDE { return TFALSE; }                                                                               // +0x28, RETURNS_FALSE
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x4c
	virtual void  OnInsertion() OVERRIDE;                                                                                                                  // +0x50
	virtual void  OnRemoval() OVERRIDE;                                                                                                                    // +0x54
	virtual void  OnActivate() OVERRIDE;                                                                                                                   // +0x60
	virtual void  OnDeactivate() OVERRIDE;                                                                                                                 // +0x64

	// TTRUE when the minigame was started from the barn: cocktails in the blender (DAT_007833b0)
	static TBOOL IsCocktails() { return ms_bCocktails; }

private:
	// Reads the ingredients and the recipes from the cooking asset pack
	void LoadRecipes();

	// Creates the GUI
	void CreateHUD();

	// Finds the ingredients the player has in the inventory and shows them
	void UpdateOwnedIngredients();

	// Animates the pig
	void UpdateCook( TFLOAT a_fDeltaTime );

	// Fades out the last ingredient of the pan, then removes it
	void UpdateRemoving( TFLOAT a_fDeltaTime );

	// Puts a_pIngredient in the pan, cooks once it's full
	void AddIngredient( ACookingIngredient* a_pIngredient );

	// Takes the last ingredient out of the pan
	void RemoveLastIngredient();

	// Finds the recipe of the ingredients of the pan and starts cooking
	void Cook();

	// Updates the inventory, gives the dishes and the bucks and saves
	void Finish();

	void Save();
	void Load();

	// Index of the owned ingredient a_rcName in m_apOwned (-1 if there's none)
	TINT FindOwned( const Toshi::TPString8& a_rcName );

	ACookingIngredient* FindIngredient( const TCHAR* a_szName );

	static TBOOL __stdcall OnQuitCallback( AStartMenuState* a_pStartMenu, void* a_pUserData, AMessagePopupState* a_pPopup, AMessagePopupState::ResponseEvent* a_pEvent );
	static TBOOL __stdcall OnLeavePopupResponse( ACookingMiniGame* a_pGame, AMessagePopupState* a_pPopup, AMessagePopupState::ResponseEvent* a_pEvent );

#ifdef BARNYARD_COMMUNITY_PATCH
	// Development: "-devautoplay" cooks the first recipe the player has the ingredients of, then leaves
	void Dev_Update( TFLOAT a_fDeltaTime );
#endif

private:
	inline static TBOOL ms_bCocktails   = TFALSE; // DAT_007833b0
	inline static TBOOL ms_bIgnoreShoot = TFALSE; // DAT_007833b4, the next Shoot closed the recipe book or the popup

	// cookingstrings.StringTable (DAT_007833ac / DAT_007833a8)
	inline static const TCHAR** ms_pszStrings  = TNULL;
	inline static TINT          ms_iNumStrings = 0;

	// The text scale of AMenuNavHudElement before the minigame (DAT_00773d18)
	inline static TFLOAT ms_fOldNavTextScale = 0.75f;

	TINT                                                                                      m_iMoney;                  // 0x00a4, the bucks earned
	TINT                                                                                      m_iUnkA8;                  // 0x00a8
	ACookingHUD*                                                                              m_pHUD;                    // 0x00ac
	AAnimatableObject*                                                                        m_pCook;                   // 0x00b0, the pig
	AAnimatableObject*                                                                        m_pTray;                   // 0x00b4, the cooker only
	AAnimatableObject*                                                                        m_apDishes[ MAX_RECIPES ]; // 0x00b8, the dishes of m_oRecipes
	TINT                                                                                      m_iCookAnim;               // 0x0158, ANIM
	TFLOAT                                                                                    m_fIdleTime;               // 0x015c
	ACookingList<ACookingIngredient, MAX_INGREDIENTS>                                         m_oIngredients;            // 0x0160, every ingredient
	ACookingList<ACookingRecipe, MAX_RECIPES>                                                 m_oRecipes;                // 0x0664, every recipe, then slop (or swill)
	ACookingIngredient*                                                                       m_apOwned[ MAX_OWNED ];    // 0x1388, the ingredients of the inventory (in m_oIngredients)
	ACookingRecipe                                                                            m_oPan;                    // 0x13d0, the ingredients in the pan (from m_vecFree)
	ACookingList<ACookingRecipe, MAX_RECIPES>                                                 m_oMade;                   // 0x1424, the recipes that were made (saved)
	STATE                                                                                     m_eState;                  // 0x2148
	TFLOAT                                                                                    m_fTimer;                  // 0x214c
	TUINT8                                                                                    m_aUnk2150[ 0x100 ];       // 0x2150
	ACookingRecipe*                                                                           m_pResult;                 // 0x2250, the recipe that was cooked
	TINT                                                                                      m_iResult;                 // 0x2254, its index
	TFLOAT                                                                                    m_fRemoveTime;             // 0x2258
	Toshi::T2Vector<ACookingIngredient*, MAX_FREE>                                            m_vecFree;                 // 0x225c, three copies of each ingredient that can go into the pan
	TINT                                                                                      m_iUnk23E0;                // 0x23e0
	TINT                                                                                      m_iCookingCue;             // 0x23e4, the pig talks while cooking
	TINT                                                                                      m_iSoundCue;               // 0x23e8, cooking (or blending)
	TINT                                                                                      m_iVoiceCue;               // 0x23ec, the pig about the dish
	TFLOAT                                                                                    m_fClearTime;              // 0x23f0
	Toshi::TListener<AMessagePopupState, AMessagePopupState::ResponseEvent, ACookingMiniGame> m_oPopupListener;          // 0x23f4
	TBOOL                                                                                     m_bQuestRecipeNotMade;     // 0x2408, saved
	TBOOL                                                                                     m_bLeaveAfterResult;       // 0x2409, the recipe of the quest was made
	Toshi::TPString8                                                                          m_astrAnims[ NUM_ANIMS ];  // 0x240c, ANIM
	Toshi::TTRB                                                                               m_oTRB;                    // 0x2434, never loaded
	TBOOL                                                                                     m_bFailed;                 // 0x2574, the ingredients aren't a recipe
	TBOOL                                                                                     m_bExiting;                // 0x2575
	TINT                                                                                      m_iByeCue;                 // 0x2578
	TBOOL                                                                                     m_bQuestRecipe;            // 0x257c, the recipe of the quest was just made
};
