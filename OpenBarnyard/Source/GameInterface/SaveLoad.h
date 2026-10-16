#pragma once
#include "GameInterface/AGameState.h"
#include "GUI/AGUI2TextBox.h"
#include "GUI/AGUI2Dialog.h"

#include <GUI/T2GUIRectangle.h>

class APlayerProgress;

// Game state that saves or loads the save data of APlayerProgress, showing a message while it works.
// The platform specific part (where the data goes) is in SaveLoadSKU.
class SaveLoad
    : public AGameState
{
public:
	TDECLARE_CLASS( SaveLoad, AGameState );

public:
	SaveLoad( APlayerProgress* a_pProgress, TBOOL a_bBackground );
	~SaveLoad();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual void OnInsertion() OVERRIDE; // +0x50
	virtual void OnRemoval() OVERRIDE;   // +0x54

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void  Cancel()    = 0;                                                            // +0x68
	virtual void  StartLoad() = 0;                                                            // +0x6c
	virtual TBOOL SaveToSlot( TBOOL a_bDontSave, TBOOL a_bTakeSnapshot );                     // +0x70
	virtual void  StartSave( TBOOL a_bUnused, TBOOL a_bDontSave, TBOOL a_bTakeSnapshot ) = 0; // +0x74
	virtual void  StartMode2()                                                           = 0; // +0x78
	virtual void  StartMode3()                                                           = 0; // +0x7c
	virtual void  ApplyLoadedSave();                                                          // +0x80, NOTE: not AGameState::OnLoaded (+0x24)
	virtual void  Unknown84() {}                                                              // +0x84, DOES_NOTHING
	virtual TINT  GetResult() = 0;                                                            // +0x88

	APlayerProgress* GetProgress() const { return m_pProgress; }

	// TTRUE uses the language of the loaded save (the PS2 sets it at boot, AFrontEndLanguageState)
	void SetUseSavedLanguage( TBOOL a_bUseSavedLanguage ) { m_bUseSavedLanguage = a_bUseSavedLanguage; }

protected:
	AGUI2TextBox*          m_pTextBox;          // 0xa4
	Toshi::T2GUIFont*      m_pFont;             // 0xa8
	TFLOAT                 m_fTextWidth;        // 0xac
	TFLOAT                 m_fMessageTime;      // 0xb0, how long a message is shown
	Toshi::T2GUIRectangle* m_pBackground;       // 0xb4
	AGUI2Dialog*           m_pDialog;           // 0xb8
	APlayerProgress*       m_pProgress;         // 0xbc
	TBOOL                  m_bBackground;       // 0xc0
	TBOOL                  m_bWasPaused;        // 0xc1
	TBOOL                  m_bWasPausedByMenu;  // 0xc2
	TBOOL                  m_bUseSavedLanguage; // 0xc3, TFALSE keeps the current language when the options are loaded from the save
};
