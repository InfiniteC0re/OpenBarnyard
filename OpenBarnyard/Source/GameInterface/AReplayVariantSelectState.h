#pragma once
#include "ABYardMenuState.h"
#include "GUI/AGUI2Button.h"
#include "Helpers/AReplayManager.h"

//-----------------------------------------------------------------------------
// Purpose: lists the unlocked variants of a replayable minigame (the stars of
// the farm) and replays the chosen one. Pushed by AReplayManager::ReplayMiniGame
// for replays with more than one unlocked variant.
// TClass 00781ad4 (its parent class is AGameState there), vtable 0071ae88, size 0x80c.
// NOTE: member names are invented, see AFrontEndVariantSelectState for the front end version.
//-----------------------------------------------------------------------------
class AReplayVariantSelectState
    : public ABYardMenuState
{
public:
	TDECLARE_CLASS( AReplayVariantSelectState, ABYardMenuState );

public:
	AReplayVariantSelectState( AReplayableGame* a_pReplay );
	~AReplayVariantSelectState();

	// AGameState
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;

	// ABYardMenuState
	virtual void OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem ) OVERRIDE;
	virtual void OnMenuClose() OVERRIDE;

private:
	TINT             m_aiVariants[ 10 ]; // 0x7d8, variant of every button (NOTE: AReplayableGame has up to 12)
	AGUI2Button*     m_aButtons;         // 0x800, one per unlocked variant
	TINT             m_iSelectedButton;  // 0x804, -1 if cancelled
	AReplayableGame* m_pReplay;          // 0x808
};
