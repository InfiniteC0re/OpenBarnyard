#pragma once
#include <Toshi/T2DList.h>
#include <Toshi/TObject.h>

class AHudElement
    : public Toshi::TObject
    , public Toshi::T2DList<AHudElement>::Node
{
public:
	TDECLARE_CLASS( AHudElement, Toshi::TObject );

	// An element is shown while its mask shares a bit with the HUD mask of the current
	// game state (AGameState::HUDParams, AGameState::GetHUDMask)
	using Mask = TUINT32;
	enum Mask_ : Mask
	{
		MASK_NONE            = 0,
		MASK_CONTEXT_BUTTON  = BITFLAG( 0 ), // AContextButtonHudElement
		MASK_MINIGAME        = BITFLAG( 1 ), // minigame HUD: rounds, race placing, bashing, the start screen, time left
		MASK_UNK2            = BITFLAG( 2 ), // micro games: button slam, game time, mini map, the player's main HUD
		MASK_LETTERBOX       = BITFLAG( 3 ), // ALetterBoxHudElement (scripted sequences)
		MASK_UNK4            = BITFLAG( 4 ), // darts, the power bars of the minigames
		MASK_UNK5            = BITFLAG( 5 ),
		MASK_MENU_NAV        = BITFLAG( 6 ), // AMenuNavHudElement
		MASK_INTERACTION     = BITFLAG( 7 ), // AInteractionHudElement, the exclamation marks above the animals
		MASK_UNK8            = BITFLAG( 8 ),
		MASK_UNK9            = BITFLAG( 9 ),
		MASK_MINIMAP         = BITFLAG( 10 ), // ASimMiniMap
		MASK_UNK11           = BITFLAG( 11 ),
		MASK_UNK12           = BITFLAG( 12 ), // the power bars of the minigames
		MASK_UNK13           = BITFLAG( 13 ),
		MASK_UNK14           = BITFLAG( 14 ),
		MASK_UNK15           = BITFLAG( 15 ), // micro games: button slam, pick ups, time left
		MASK_UNK17           = BITFLAG( 17 ),
		MASK_HONEY_COUNT     = BITFLAG( 18 ), // AHoneyCountHudElement
		MASK_GAME_TIME       = BITFLAG( 19 ), // AGameTimeHudElement
		MASK_UNK23           = BITFLAG( 23 ),
		MASK_UNK24           = BITFLAG( 24 ),
		MASK_UNK25           = BITFLAG( 25 ),
		MASK_UNK26           = BITFLAG( 26 ),
		MASK_ITEM_COUNT      = BITFLAG( 27 ), // AItemCountHudElement
		MASK_PICK_UP         = BITFLAG( 28 ), // ASimPlayerPickUpHudElement
		MASK_TERRAIN_SECTION = BITFLAG( 29 ), // ATerrainSectionHudElement
		MASK_PLAYER_MAIN     = BITFLAG( 30 ), // ASimPlayerMainHudElement (and the pick ups)
		MASK_TIME_LEFT       = BITFLAG( 31 ), // ATimeLeftHudElement

		// HUD of AMiniGameState (also the default of most minigames)
		MASK_MINIGAME_DEFAULT = MASK_MINIGAME | MASK_UNK4 | MASK_UNK12,
		// HUD of ASimState
		MASK_SIM = MASK_UNK4 | MASK_INTERACTION | MASK_UNK8 | MASK_MINIMAP | MASK_UNK11 | MASK_GAME_TIME | MASK_ITEM_COUNT | MASK_TERRAIN_SECTION | MASK_PLAYER_MAIN | MASK_TIME_LEFT,

		MASK_ALL = 0xFFFFFFFF
	};

	friend class AHudManager;

public:
	AHudElement( Mask a_eMask = MASK_ALL );
	virtual ~AHudElement();

	virtual void  Unknown1( void* a_pUnk1, void* a_pUnk2 );
	virtual void  GetDimensions( TINT& a_rWidth, TINT& a_rHeight );
	virtual TBOOL OnAttach();
	virtual void  OnDetach();
	virtual void  Show();
	virtual void  Hide();
	virtual void  Update( TFLOAT a_fDeltaTime );
	virtual void  Reset();

	void Detach();

	void SetMask( Mask a_eMask ) { m_eMask = a_eMask; }
	Mask GetMask() const { return m_eMask; }

	void  SetForceHide( TBOOL a_bForceHide ) { m_bForceHide = a_bForceHide; }
	TBOOL IsForceHidden() const { return m_bForceHide; }
	TBOOL IsVisible() const { return m_bCurrentlyVisible; }
	TBOOL IsOwnedByManager() const { return m_bOwnedByManager; }

protected:
	Mask  m_eMask;
	TBOOL m_bForceHide;
	TBOOL m_bCurrentlyVisible;
	TBOOL m_bOwnedByManager;
};
