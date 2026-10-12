#pragma once
#include "ABYardMenuState.h"
#include "GUI/AGUI2Button.h"
#include "GUI/AGUI2TextBox.h"
#include "Helpers/APlayerProgress.h"

#include <GUI/T2GUIRectangle.h>

// The used save slots to continue a game from (Load Game of the main menu, once the save file is loaded).
// The chosen slot is loaded by AGameLoader::LoadSavedGame.
//
// TClass 0x0076ed80 (size 0x19b4), vtable 0x00716bc8.
class AFrontEndLoadState
    : public ABYardMenuState
{
public:
	TDECLARE_CLASS( AFrontEndLoadState, ABYardMenuState );

public:
	AFrontEndLoadState();
	~AFrontEndLoadState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual void  OnInsertion() OVERRIDE;                                                                                                                  // +0x50
	virtual void  OnRemoval() OVERRIDE;                                                                                                                    // +0x54
	virtual void  OnActivate() OVERRIDE;                                                                                                                   // +0x60

	//-----------------------------------------------------------------------------
	// ABYardMenuState
	//-----------------------------------------------------------------------------
	virtual void OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem ) OVERRIDE; // +0x6c
	virtual void OnMenuClose() OVERRIDE;                                     // +0x74

private:
	AGUI2Button           m_aSlotButtons[ APlayerProgress::NUM_SAVE_SLOTS ];        // 0x7d8
	Toshi::T2GUIRectangle m_aSlotPictures[ APlayerProgress::NUM_SAVE_SLOTS ];       // 0xfe8, the player character of the slot
	AGUI2Button           m_oNoSavesButton;                                         // 0x11c8, shown when no slot is used
	TWCHAR                m_wszNoSaves[ 256 ];                                      // 0x13cc
	TWCHAR                m_awszSlotNames[ APlayerProgress::NUM_SAVE_SLOTS ][ 32 ]; // 0x15cc
	TWCHAR                m_awszSlotDates[ APlayerProgress::NUM_SAVE_SLOTS ][ 32 ]; // 0x16cc
	AGUI2TextBox          m_aSlotDateTexts[ APlayerProgress::NUM_SAVE_SLOTS ];      // 0x17cc
	TINT                  m_iActivatedSlot;                                         // 0x19ac
	TINT                  m_iNumSlots;                                              // 0x19b0, number of used slots
};
