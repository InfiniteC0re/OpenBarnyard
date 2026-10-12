#pragma once
#include "ABYardMenuState.h"
#include "GUI/AGUI2Button.h"

//-----------------------------------------------------------------------------
// Purpose: lists the selectable variants of a minigame and starts the chosen one.
// Pushed by AFrontEndMiniGameState2 in debug mode (not m_bAntics) for minigames
// with more than one selectable variant.
// NOTE: member names are invented, the original source of this class is unknown.
//-----------------------------------------------------------------------------
class AFrontEndVariantSelectState
    : public ABYardMenuState
{
public:
	TDECLARE_CLASS( AFrontEndVariantSelectState, ABYardMenuState );

public:
	AFrontEndVariantSelectState( TINT a_iMiniGame );
	~AFrontEndVariantSelectState();

	// AGameState
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;

	// ABYardMenuState
	virtual void OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem ) OVERRIDE;
	virtual void OnMenuClose() OVERRIDE;

private:
	AGUI2Button* m_aButtons;         // 0x7d8, one per selectable variant
	TINT         m_iSelectedVariant; // 0x7dc, -1 if cancelled
	TINT         m_iMiniGame;        // 0x7e0
};
