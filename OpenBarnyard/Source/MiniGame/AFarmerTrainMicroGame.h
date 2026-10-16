#pragma once
#include "GameInterface/AGameState.h"
#include "GUI/AGUI2TextBox.h"

class AInteractionHudElement;

//-----------------------------------------------------------------------------
// Purpose: micro game that can only be pushed by name (TClass); nothing in the
// retail data pushes it (the scripts don't name it). A blinking text
// (locale string 920) and the interaction HUD (locale string 921) ask the
// player to press the Quad command, which swaps the player model to its "land"
// version; the state ends 5 seconds later.
// TClass "AFarmerTrainMicroGame", vtable 007237e0, size 0x1b4.
//-----------------------------------------------------------------------------
class AFarmerTrainMicroGame
    : public AGameState
{
public:
	TDECLARE_CLASS( AFarmerTrainMicroGame, AGameState );

	using FLAGS = TUINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE    = 0,
		FLAGS_SWAPPED = BITFLAG( 1 ), // the model of the player is swapped
	};

public:
	AFarmerTrainMicroGame();
	~AFarmerTrainMicroGame();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	// NOTE: folded with ASleepCycleState::ProcessInput (0043fc30), it only calls AGameState::ProcessInput
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE { return AGameState::ProcessInput( a_pInputEvent ); }   // +0x0c
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x4c
	virtual void  OnInsertion() OVERRIDE;                                                                                                                  // +0x50
	virtual void  OnRemoval() OVERRIDE;                                                                                                                    // +0x54
	virtual void  OnActivate() OVERRIDE;                                                                                                                   // +0x60
	virtual void  OnDeactivate() OVERRIDE;                                                                                                                 // +0x64

private:
	// Swaps the model of the player (or swaps it back) and starts the timer
	void ToggleSwappedModel();

private:
	FLAGS                   m_eFlags;           // 0x0a4
	AGUI2TextBox            m_oText;            // 0x0a8, locale string 920, blinks
	TFLOAT                  m_fBlinkTime;       // 0x120
	TFLOAT                  m_fToggleDelay;     // 0x124, NOTE: never decreases, the model can only be swapped once
	TFLOAT                  m_fSwappedTime;     // 0x128
	TFLOAT                  m_fMaxSwappedTime;  // 0x12c, the state ends once m_fSwappedTime exceeds it
	TWCHAR                  m_wszHudText[ 64 ]; // 0x130
	AInteractionHudElement* m_pHudElement;      // 0x1b0
};

TSTATICASSERT( sizeof( AFarmerTrainMicroGame ) == 0x1b4 );
