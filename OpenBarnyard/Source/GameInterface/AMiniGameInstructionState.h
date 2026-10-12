#pragma once
#include "AGameState.h"
#include "GUI/AGUI2TextBox.h"

#include <GUI/T2GUIRectangle.h>

//-----------------------------------------------------------------------------
// Instruction screen shown before a minigame starts (TClass 00782700, vtable 0071d828).
// Pushed by AGameLoader::LoadMiniGameImpl (and when a minigame is restarted)
// with m_pMiniGame set to the minigame state. Shows the loading picture of the
// minigame and a blinking "press ... to continue" text. Pressing START, ESCAPE,
// RETURN, SPACE or clicking removes it and starts the minigame (OnStart).
//-----------------------------------------------------------------------------
class AMiniGameInstructionState
    : public AGameState
{
public:
	TDECLARE_CLASS( AMiniGameInstructionState, AGameState );

	static constexpr TFLOAT BLINK_INTERVAL = 0.75f;

public:
	AMiniGameInstructionState();
	~AMiniGameInstructionState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE;

	// NOTE: a jump to a shared function returning TFALSE in retail
	// $Barnyard: FUNCTION 0046c550
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE { return TFALSE; }

	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Removes the instruction screen and starts the minigame.
	virtual void Dismiss(); // +0x68

	void        SetMiniGame( AGameState* a_pMiniGame ) { m_pMiniGame = a_pMiniGame; }
	AGameState* GetMiniGame() const { return m_pMiniGame; }

private:
	// Dismisses the screen if the minigame allows it.
	void TryDismiss();

private:
	AGameState*           m_pMiniGame;   // 0xa4
	TFLOAT                m_fBlinkTimer; // 0xa8
	TFLOAT                m_fTotalTime;  // 0xac
	Toshi::T2GUIRectangle m_oTextFrame;  // 0xb0, "Wood_Thin" frame of the text
	AGUI2TextBox          m_oTextBox;    // 0x128, locale 0x20c
};

TSTATICASSERT( sizeof( AMiniGameInstructionState ) == 0x1a0 );
