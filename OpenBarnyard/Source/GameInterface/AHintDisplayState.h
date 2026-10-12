#pragma once
#include "AGameState.h"
#include "GUI/AGUI2TextBox.h"

#include <GUI/T2GUIRectangle.h>

//-----------------------------------------------------------------------------
// Purpose: Pip's notebook. Shows the hint the player has just collected or,
// opened from the inventory ("notebook"), lets the player page through the
// collected hints, with a page that tells how many of them were found.
// TClass 00781da0, vtable 0071baa0.
//-----------------------------------------------------------------------------
class AHintDisplayState
    : public AGameState
{
public:
	TDECLARE_CLASS( AHintDisplayState, AGameState );

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

public:
	AHintDisplayState();

	// a_bCollectHint: show the hint the player is collecting instead of the collected ones
	AHintDisplayState( TBOOL a_bCollectHint );
	~AHintDisplayState();

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
	void CreateGUI();
	void UpdatePage();

private:
	Toshi::T2GUIRectangle m_oBackground;         // 0x0a4
	Toshi::T2GUIRectangle m_oPage;               // 0x11c, "Recipie_Page"
	Toshi::T2GUIRectangle m_oHintArea;           // 0x194
	Toshi::T2GUIRectangle m_oNavigation;         // 0x20c, the arrows and the page number
	Toshi::T2GUIRectangle m_oPageNumberArea;     // 0x284
	Toshi::T2GUIRectangle m_oProgressArea;       // 0x2fc
	AGUI2TextBox          m_oProgressTitle;      // 0x374
	AGUI2TextBox          m_oProgress;           // 0x3ec, "n%" of the hints found
	AGUI2TextBox          m_oTitle;              // 0x464
	AGUI2TextBox          m_oPrompt;             // 0x4dc
	AGUI2TextBox          m_oHint;               // 0x554
	AGUI2TextBox          m_oPageLabel;          // 0x5cc
	AGUI2TextBox          m_oPageNumber;         // 0x644
	Toshi::T2GUIRectangle m_oLeftArrow;          // 0x6bc
	Toshi::T2GUIRectangle m_oRightArrow;         // 0x734
	TWCHAR                m_wszPrompt[ 256 ];    // 0x7ac
	TWCHAR                m_wszPageNumber[ 16 ]; // 0x9ac
	TWCHAR                m_wszUnk9CC[ 8 ];      // 0x9cc
	TWCHAR                m_wszUnk9DC[ 8 ];      // 0x9dc
	TWCHAR                m_wszProgress[ 8 ];    // 0x9ec
	INPUT                 m_eInput;              // 0x9fc
	TINT                  m_iUnkA00;             // 0xa00
	TINT                  m_iUnkA04;             // 0xa04
	TFLOAT                m_fTime;               // 0xa08, seconds since the last analog stick page change
	TINT                  m_iUnkA0C;             // 0xa0c
	TINT                  m_iHint;               // 0xa10, -1 shows the progress page
	TBOOL                 m_bCollectHint;        // 0xa14
	Toshi::T2GUIRectangle m_oFade;               // 0xa18
};

TSTATICASSERT( sizeof( AHintDisplayState ) == 0xa90 );
