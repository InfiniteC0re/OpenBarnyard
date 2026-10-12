#pragma once
#include "ABYardMenuState.h"

//-----------------------------------------------------------------------------
// Purpose: the "Barnyard Antics" carousel that lets the player pick a minigame.
// With m_bAntics set (what the front-end uses) it lists the visible minigames and
// respects their unlock flags. Without it (debug mode, only created by name) it
// lists every registered minigame and pushes AFrontEndVariantSelectState for the
// ones that have selectable variants.
//-----------------------------------------------------------------------------
class AFrontEndMiniGameState2
    : public ABYardMenuState
{
public:
	TDECLARE_CLASS( AFrontEndMiniGameState2, ABYardMenuState );

	enum LocalState
	{
		LocalState_Ready,
		LocalState_InitRotateLeft, // Icons move left, the right one becomes selected
		LocalState_RotateLeft,
		LocalState_InitRotateRight, // Icons move right, the left one becomes selected
		LocalState_RotateRight,
	};

	enum Arrow
	{
		Arrow_Left,
		Arrow_Right,
		Arrow_NUMOF
	};

public:
	AFrontEndMiniGameState2( TBOOL a_bAntics = TFALSE );
	~AFrontEndMiniGameState2();

	// AGameState
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE;
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;

	// ABYardMenuState
	virtual void OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem ) OVERRIDE;
	virtual void OnMenuClose() OVERRIDE;

private:
	// Sets the label and the description of the current selection
	void SetTexts();

	TBOOL CurIsAvailableToPlay() const;

public:
	inline static const TFLOAT ms_fAnalogTimeDelay = 0.2f;

private:
	AGUI2MenuItem          m_oGamesMenuItem;         // 0x7d8
	TINT                   m_iUnk838;                // 0x838
	AGUI2MenuItem          m_oUnusedMenuItem;        // 0x83c, never used, probably the AI count/variant item of the debug menu that was cut
	TINT                   m_iUnk89c;                // 0x89c
	Toshi::T2GUIRectangle* m_aIconRects;             // 0x8a0
	TINT                   m_iNumRects;              // 0x8a4
	TINT                   m_iCurrSelection;         // 0x8a8
	TINT                   m_iLeftSelection;         // 0x8ac
	TINT                   m_iRightSelection;        // 0x8b0
	TINT                   m_iNewSelection;          // 0x8b4, the selection that fades out
	LocalState             m_eLocalState;            // 0x8b8
	Toshi::T2GUIRectangle  m_aArrows[ Arrow_NUMOF ]; // 0x8bc
	AGUI2TextBox           m_oGameLabel;             // 0x9ac
	AGUI2TextBox           m_oGameDesc;              // 0xa24
	TINT                   m_iNumAI;                 // 0xa9c, never initialised or used (the AI count of the cut debug menu)
	TINT                   m_iVariant;               // 0xaa0, never initialised or used (the variant of the cut debug menu)
	TBOOL                  m_bAntics;                // 0xaa4
	TFLOAT                 m_fAnalogTime;            // 0xaa8
	TBOOL                  m_bMouseMoved;            // 0xaac
	TBOOL                  m_bMouseOverSelected;     // 0xaad
	TBOOL                  m_bMouseOverLeftArrow;    // 0xaae
	TBOOL                  m_bMouseOverRightArrow;   // 0xaaf
};
