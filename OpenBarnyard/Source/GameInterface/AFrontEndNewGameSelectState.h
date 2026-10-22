#pragma once
#include "ABYardMenuState.h"
#include "AMessagePopupState.h"
#include "GUI/AGUI2Button.h"
#include "GUI/AGUI2TextBox.h"
#include "Helpers/APlayerProgress.h"

#include <GUI/T2GUIRectangle.h>

// The save slot a new game goes to (New Game of the main menu). A slot that is in use asks to be
// overwritten first. Once a slot is chosen, the SS01 intro sequence is started (AGameLoader::StartNewGame).
//
// TClass 0x0076ee10 (size 0x15c4), vtable 0x00716e58.
class MODAPI AFrontEndNewGameSelectState
    : public ABYardMenuState
{
public:
	TDECLARE_CLASS( AFrontEndNewGameSelectState, ABYardMenuState );

	using PopupListener = Toshi::TListener<AMessagePopupState, AMessagePopupState::ResponseEvent, AFrontEndNewGameSelectState>;

public:
	AFrontEndNewGameSelectState();
	~AFrontEndNewGameSelectState();

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

#ifdef BARNYARD_COMMUNITY_PATCH
	// Development: the save slot buttons
	AGUI2Button& Dev_GetSlotButton( TINT a_iSlot ) { return m_aSlotButtons[ a_iSlot ]; }

	// What choosing a slot does, for a mod replacing the screen: a_bOverwrite once the player agreed
	// to overwrite the slot in use. Returns TFALSE when the slot is in use and a_bOverwrite isn't set
	static TBOOL StartNewGameInSlot( TINT a_iSlot, TBOOL a_bOverwrite );
#endif

private:
	// Starts the new game in the current save slot
	static void StartNewGame();

	static TBOOL __stdcall OnOverwritePopupResponse( AFrontEndNewGameSelectState* a_pState, AMessagePopupState* a_pPopup, AMessagePopupState::ResponseEvent* a_pEvent );

private:
	TINT                  m_iActivatedSlot;                                         // 0x7d8
	AGUI2Button           m_aSlotButtons[ APlayerProgress::NUM_SAVE_SLOTS ];        // 0x7dc
	Toshi::T2GUIRectangle m_aSlotPictures[ APlayerProgress::NUM_SAVE_SLOTS ];       // 0xfec, the player character of the slot
	TWCHAR                m_awszSlotNames[ APlayerProgress::NUM_SAVE_SLOTS ][ 32 ]; // 0x11cc
	TWCHAR                m_awszSlotDates[ APlayerProgress::NUM_SAVE_SLOTS ][ 32 ]; // 0x12cc
	AGUI2TextBox          m_aSlotDateTexts[ APlayerProgress::NUM_SAVE_SLOTS ];      // 0x13cc
	PopupListener         m_oPopupListener;                                         // 0x15ac
	TBOOL                 m_bOverwrite;                                             // 0x15c0, the chosen slot is in use
};

// Reads the name, the gender and the costume of the player out of a used save slot and formats its
// save date (" (day/month/year hour:minute)" in the order of the language).
// NOTE: inlined in AFrontEndNewGameSelectState::OnInsertion and AFrontEndLoadState::OnInsertion
MODAPI void AFrontEnd_ReadSaveSlot( const APlayerProgress::SaveSlot* a_pSlot, TWCHAR* a_wszOutName, TWCHAR* a_wszOutDate, TINT& a_rOutGender, TINT& a_rOutCostume );
