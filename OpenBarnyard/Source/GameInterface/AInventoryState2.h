#pragma once
#include "AGameState.h"
#include "GUI/AGUI2TextBox.h"
#include "GUI/AGUI2ModelRenderer.h"

#include <GUI/T2GUIRectangle.h>
#include <Math/TVector2.h>
#include <Math/TVector4.h>
#include <Toshi/TPString8.h>

class AUnit;
class AUnitType;
class AObjectTraits;
class AInventoryItem;
class AInventoryComponent;
class AAnimatableObject;

//-----------------------------------------------------------------------------
// Purpose: The inventory of the sim. A wooden shelf at the top of the screen
// shows the items of a category ("BackPack", "Knapsack" or "FlowerPack") as
// spinning 3D models: the selected one in the middle and its neighbours on
// the sides. It's a child state of the sim that stays inserted: it pops up
// when the player opens it (or when an item is picked up) and hides itself
// after a while. Using an item takes it out into the hand of the player or
// opens its screen (phone, map, cookbook, notebook...).
// TClass 007819dc, vtable 0071a7a8.
//-----------------------------------------------------------------------------
class AInventoryState2
    : public AGameState
{
public:
	TDECLARE_CLASS( AInventoryState2, AGameState );

	static constexpr TINT MAX_ITEMS = 30;

	// NOTE: names are invented
	using STATE = TINT;
	enum STATE_ : STATE
	{
		STATE_OPENING,    // the shelf pops up
		STATE_CLOSING,    // the shelf pops down, the state removes itself after that
		STATE_FADING_IN,  // the items and texts fade in
		STATE_SHOWING,    // the arrows (and the side items) fade in
		STATE_HIDING,     // the arrows (and the side items) fade out
		STATE_FADING_OUT, // the items and texts fade out
		STATE_HIDDEN,
		STATE_IDLE, // the items can be selected
	};

	// NOTE: names are invented
	using SCROLL = TINT;
	enum SCROLL_ : SCROLL
	{
		SCROLL_NONE,
		SCROLL_START_RIGHT, // the next item becomes the selected one
		SCROLL_LEFT,        // the items slide to the left
		SCROLL_START_LEFT,  // the previous item becomes the selected one
		SCROLL_RIGHT,       // the items slide to the right
	};

public:
	AInventoryState2();
	~AInventoryState2();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE;                                                        // +0x0c
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x4c
	virtual void  OnInsertion() OVERRIDE;                                                                                                                  // +0x50
	virtual void  OnRemoval() OVERRIDE;                                                                                                                    // +0x54
	virtual void  OnActivate() OVERRIDE;                                                                                                                   // +0x60
	virtual void  OnDeactivate() OVERRIDE;                                                                                                                 // +0x64

	// Opens the inventory so that the player can select an item
	void Open();

	// Selects the item of the type a_pType, switching to its category. a_bAdd adds the
	// type if the category doesn't have it yet (an item that was just picked up)
	void SelectType( AUnitType* a_pType, TBOOL a_bAdd );

	// Shows the name and the count of the selected item
	void UpdateItemText();

	// Position of the selected item on the screen (the middle of the shelf, a quarter from its top)
	void GetItemScreenPosition( Toshi::TVector2& a_rOutPosition );

	TBOOL IsOpen() const { return m_bOpen; }

private:
	// Shows the shelf
	void Show();

	// Gives the controls back to the player (a_bOpen = FALSE) or takes them
	void SetOpen( TBOOL a_bOpen );

	// Shows the items of the category a_rcName
	void SetCategory( const Toshi::TPString8& a_rcName );

	// Creates the model of the item at a_iIndex
	void CreateItemObject( AUnitType* a_pType, TINT a_iIndex );

	// Scales the shelf, a_bFull uses its full size (otherwise it depends on m_fPopTime)
	void UpdatePopScale( TBOOL a_bFull );

	// Fades the texts to a_fAlpha and the selected item by a_fDelta
	void UpdateFade( TFLOAT a_fAlpha, TFLOAT a_fDelta );

	// Fades the arrows and the side items by a_fDelta
	void UpdateArrowFade( TFLOAT a_fDelta );

	// Moves the items towards their places
	void UpdateSlide( TFLOAT a_fDeltaTime, SCROLL a_eScroll );

	// Starts and updates the scrolling
	void UpdateScroll( TFLOAT a_fDeltaTime );

	// Places the model of an item in the model renderer
	static void SetObjectTransform( AAnimatableObject* a_pObject, AObjectTraits* a_pTraits, TBOOL a_bSetPosition, Toshi::TVector4 a_vecPosition, Toshi::TVector4 a_vecRotation, TFLOAT a_fScale );

private:
	TWCHAR                m_wszCount[ 4 ];          // 0x0a4
	Toshi::T2GUIRectangle m_oFrame;                 // 0x0ac, "Inv_Frame"
	AGUI2TextBox          m_oTitle;                 // 0x124, the name of the category
	Toshi::T2GUIRectangle m_oWoodLeft;              // 0x19c, "Inv_Wood_Left"
	Toshi::T2GUIRectangle m_oWoodMid;               // 0x214, "Inv_Wood_Mid"
	Toshi::T2GUIRectangle m_oWoodRight;             // 0x28c, "Inv_Wood_Right"
	Toshi::T2GUIElement   m_oShelf;                 // 0x304
	Toshi::T2GUIRectangle m_oLeftArrow;             // 0x358, "left_arrow"
	Toshi::T2GUIRectangle m_oRightArrow;            // 0x3d0, "left_arrow2"
	AGUI2TextBox          m_oItemName;              // 0x448
	AGUI2TextBox          m_oItemCount;             // 0x4c0
	AGUI2ModelRenderer    m_oModelRenderer;         // 0x538
	Toshi::TVector2       m_vecShelfSize;           // 0x5b8
	Toshi::TVector2       m_vecFrameSize;           // 0x5c0
	TFLOAT                m_fWoodMidWidth;          // 0x5c8
	TFLOAT                m_fWoodLeftWidth;         // 0x5cc
	TFLOAT                m_fWoodRightWidth;        // 0x5d0
	TFLOAT                m_fPopTime;               // 0x5d4, angle of the pop up animation
	TFLOAT                m_fFade;                  // 0x5d8
	TFLOAT                m_fArrowFade;             // 0x5dc
	STATE                 m_eState;                 // 0x5e0
	TINT                  m_iCurrent;               // 0x5e4, the selected item
	TINT                  m_iPrevious;              // 0x5e8, the item on the left
	TINT                  m_iNext;                  // 0x5ec, the item on the right
	TINT                  m_iLeaving;               // 0x5f0, the item that slides out
	SCROLL                m_eScroll;                // 0x5f4
	AInventoryComponent*  m_pCategory;              // 0x5f8
	AAnimatableObject*    m_apObjects[ MAX_ITEMS ]; // 0x5fc
	AAnimatableObject*    m_pCurrentObject;         // 0x674
	AAnimatableObject*    m_pPreviousObject;        // 0x678
	AAnimatableObject*    m_pNextObject;            // 0x67c
	AAnimatableObject*    m_pLeavingObject;         // 0x680
	Toshi::TVector4       m_vecCurrentPos;          // 0x684
	Toshi::TVector4       m_vecPreviousPos;         // 0x694
	Toshi::TVector4       m_vecNextPos;             // 0x6a4
	Toshi::TVector4       m_vecLeavingPos;          // 0x6b4
	TFLOAT                m_fSpinTime;              // 0x6c4, the selected item spins
	TINT                  m_iNumObjects;            // 0x6c8
	TINT                  m_iNumItems;              // 0x6cc
	AUnitType*            m_apTypes[ MAX_ITEMS ];   // 0x6d0
	AObjectTraits*        m_apTraits[ MAX_ITEMS ];  // 0x748
	TINT                  m_iUnk7C0;                // 0x7c0, unused
	AInventoryItem*       m_apItems[ MAX_ITEMS ];   // 0x7c4
	TFLOAT                m_fStickTime;             // 0x83c, seconds since the analog stick scrolled
	TBOOL                 m_bOpen;                  // 0x840, the player is selecting an item
	TFLOAT                m_fCloseTime;             // 0x844, seconds since the inventory popped up on its own
	Toshi::TPString8      m_strCategory;            // 0x848
	TINT                  m_iUnk84C;                // 0x84c
	TINT                  m_iUnk850;                // 0x850
	TINT                  m_iUnk854;                // 0x854
	TINT                  m_iUnk858;                // 0x858
	AUnit*                m_pPlayer;                // 0x85c
	Toshi::T2GUIElement   m_oPromptBar;             // 0x860
	AGUI2TextBox          m_aPrompts[ 2 ];          // 0x8b4
	TWCHAR                m_wszUsePrompt[ 32 ];     // 0x9a4
	TWCHAR                m_wszClosePrompt[ 32 ];   // 0x9e4
	TBOOL                 m_bUnkA24;                // 0xa24, unused
	TBOOL                 m_bMouseMoved;            // 0xa25
	TBOOL                 m_bMouseOnOK;             // 0xa26, never set
	TBOOL                 m_bMouseOnLeftArrow;      // 0xa27
	TBOOL                 m_bMouseOnRightArrow;     // 0xa28
};

TSTATICASSERT( sizeof( AInventoryState2 ) == 0xa2c );
