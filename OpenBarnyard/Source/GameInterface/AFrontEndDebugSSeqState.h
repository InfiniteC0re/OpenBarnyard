#pragma once
#include "AGameState.h"
#include "GUI/AGUI2Menu.h"
#include "GUI/AGUI2Button.h"
#include "Units/AScriptedSequenceActivity.h"

//-----------------------------------------------------------------------------
// Purpose: Debug menu listing the scripted sequences of the game, the chosen
// one is played (AGameLoader::SetTransitionToScriptedSequence). Opened by
// AFrontEndCharacterSelectState once the character is chosen when it's
// created with a_bDebugSSeq (nothing does that in retail).
// TClass 0077e00c, vtable 007166c0, size 0x3bc4.
//-----------------------------------------------------------------------------
class AFrontEndDebugSSeqState
    : public AGameState
{
public:
	TDECLARE_CLASS( AFrontEndDebugSSeqState, AGameState );

	static constexpr TINT NUM_BUTTONS = AScriptedSequenceActivity::NUM_SEQUENCES;

public:
	AFrontEndDebugSSeqState();
	~AFrontEndDebugSSeqState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x4c
	virtual void  OnInsertion() OVERRIDE;                                                                                                                  // +0x50
	virtual void  OnRemoval() OVERRIDE;                                                                                                                    // +0x54
	virtual void  OnActivate() OVERRIDE;                                                                                                                   // +0x60

private:
	// Plays the scripted sequence of the button
	static void OnButtonActivated( void* a_pUserData, AGUI2MenuItem* a_pItem );

private:
	AGUI2Menu   m_oMenu;                   // 0x0a4
	AGUI2Button m_aButtons[ NUM_BUTTONS ]; // 0x148
	TINT        m_iUnk3BBC;                // 0x3bbc, unused
	TBOOL       m_bOwnsGateManager;        // 0x3bc0, never set
	TBOOL       m_bOwnsInstanceManager;    // 0x3bc1, the instance manager is created by the state
};

#ifndef BARNYARD_LATEST
// NOTE: BARNYARD_LATEST has a button for each of the 33 sequences
TSTATICASSERT( sizeof( AFrontEndDebugSSeqState ) == 0x3bc4 );
#endif
