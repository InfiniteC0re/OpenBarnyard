#pragma once
#include "ACookingIngredient.h"

#include <Math/TVector4.h>
#include <Toshi/TPString8.h>

//-----------------------------------------------------------------------------
// Purpose: A recipe of the cooking minigame (cooking.AllRecipes): the dish
// (or cocktail) and the ingredients it's made of.
// NOTE: the name is invented. Used by ACookingMiniGame and ARecipeDisplayState.
//-----------------------------------------------------------------------------
class ACookingRecipe
{
public:
	static constexpr TINT MAX_INGREDIENTS = 3;

	using TYPE = TINT;
	enum TYPE_ : TYPE
	{
		TYPE_DISH     = BITFLAG( 0 ), // can be cooked
		TYPE_COCKTAIL = BITFLAG( 1 ), // can be mixed in the blender
	};

public:
	ACookingRecipe();
	~ACookingRecipe();

	ACookingRecipe& operator=( const ACookingRecipe& a_rcOther );

	// Creates the animatable object of the dish, a_szName is its unit type
	void Create( TINT a_iNameString, const TCHAR* a_szName );

	// Destroys the animatable object
	void Destroy();

	void SetQuest( const TCHAR* a_szQuest );

	// Adds an ingredient and returns its index
	TINT AddIngredient( ACookingIngredient* a_pIngredient );

	// Removes the last ingredient and returns it
	ACookingIngredient* PopIngredient();

	// Returns TTRUE if a_rcOther has the same ingredients (in any order)
	TBOOL HasSameIngredients( const ACookingRecipe& a_rcOther ) const;

	// $Barnyard: FUNCTION 004ffeb0
	ACookingIngredient* GetIngredient( TINT a_iIndex ) { return m_oIngredients[ a_iIndex ]; }

	// $Barnyard: FUNCTION 004fd810
	Toshi::TPString8 GetName() const { return m_strName; }

	// NOTE: shared getter 0049b5b0 (AGopherShopItem)
	Toshi::TPString8 GetQuest() const { return m_strQuest; }

	TINT  GetNumIngredients() const { return m_oIngredients.Size(); }
	TINT  GetNameString() const { return m_iNameString; }
	TINT  GetPrice() const { return m_iPrice; }
	TYPE  GetType() const { return m_eType; }
	TBOOL IsCocktail() const { return m_eType == TYPE_COCKTAIL; }

	void SetPrice( TINT a_iPrice ) { m_iPrice = a_iPrice; }
	void SetType( TYPE a_eType ) { m_eType = a_eType; }

	TUINT8 GetNumMade() const { return m_uiNumMade; }
	TUINT8 GetNumTotal() const { return m_uiNumTotal; }
	void   SetNumMade( TUINT8 a_uiNumMade ) { m_uiNumMade = a_uiNumMade; }
	void   SetNumTotal( TUINT8 a_uiNumTotal ) { m_uiNumTotal = a_uiNumTotal; }

	// NOTE: shared getter 004ffe40 (TD3DAdapter::Mode::GetModeIndex)
	AAnimatableObject* GetObject() const { return m_pObject; }

private:
	TFLOAT                                             m_fScale;       // 0x00, AObjectTraits::m_fItemScale
	Toshi::TVector4                                    m_vOffset;      // 0x04, AObjectTraits::m_vecItemOffset
	Toshi::TVector4                                    m_vFacing;      // 0x14, AObjectTraits::m_vecItemFacing
	AAnimatableObject*                                 m_pObject;      // 0x24
	TINT                                               m_iNameString;  // 0x28
	ACookingList<ACookingIngredient*, MAX_INGREDIENTS> m_oIngredients; // 0x2c
	TINT                                               m_iPrice;       // 0x3c, the bucks ACookingMiniGame gives for it
	TINT                                               m_iUnk40;       // 0x40
	TYPE                                               m_eType;        // 0x44
	Toshi::TPString8                                   m_strQuest;     // 0x48, the dish is given to the player while this quest is the current one
	Toshi::TPString8                                   m_strName;      // 0x4c, the unit type
	TUINT8                                             m_uiNumMade;    // 0x50, ACookingMiniGame: how many were made this time
	TUINT8                                             m_uiNumTotal;   // 0x51, ACookingMiniGame: how many were ever made (saved)
};

TSTATICASSERT( sizeof( ACookingRecipe ) == 0x54 );

// An entry of cooking.AllRecipes
struct RecipeEntryData
{
	TINT         iNameString;                                       // 0x00
	const TCHAR* szName;                                            // 0x04, the unit type of the dish
	TINT         iPrice;                                            // 0x08
	TINT         iUnk0C;                                            // 0x0c
	TINT         iType;                                             // 0x10, ACookingRecipe::TYPE
	const TCHAR* szQuest;                                           // 0x14
	const TCHAR* aszIngredients[ ACookingRecipe::MAX_INGREDIENTS ]; // 0x18, TNULL terminated if less
};

TSTATICASSERT( sizeof( RecipeEntryData ) == 0x24 );
