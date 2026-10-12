#pragma once
#include "ABYardMenuState.h"
#include "GUI/AGUI2Button.h"

#ifdef BARNYARD_LATEST

//-----------------------------------------------------------------------------
// Purpose: the difficulty menu of the latest version of the game (Wii), size 0xc08 there.
// The Wii shows it after a new game slot is picked (AFrontEndNewGameSelectState) and stores the choice in
// the save slot (APlayerProgress::SLOTFLAGS_EASY) before the game starts.
// NOTE: here it's reached from Options -> Difficulty and stores the choice in OpenBarnyard.cfg
// (AOptions), so the save stays compatible with PC retail. See APlayerProgress::IsEasyDifficulty.
//-----------------------------------------------------------------------------
class ADifficultyOptionState : public ABYardMenuState
{
public:
	TDECLARE_CLASS( ADifficultyOptionState, ABYardMenuState );

	enum DIFFICULTYBUTTON
	{
		DIFFICULTYBUTTON_EASY,
		DIFFICULTYBUTTON_NORMAL,
		DIFFICULTYBUTTON_NUMOF
	};

	// Wii locale strings of the menu, the PC locale doesn't have them (see GetText)
	static constexpr TINT TITLE_TEXT                             = 3476;
	static constexpr TINT BUTTON_TEXTS[ DIFFICULTYBUTTON_NUMOF ] = { 3477, 3478 };

public:
	ADifficultyOptionState();
	~ADifficultyOptionState();

	// English text of a Wii locale string of the menu (TITLE_TEXT or BUTTON_TEXTS)
	static const TWCHAR* GetText( TINT a_iLocaleString );

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;

	//-----------------------------------------------------------------------------
	// ABYardMenuState
	//-----------------------------------------------------------------------------
	virtual void OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem ) OVERRIDE;
	virtual void OnMenuClose() OVERRIDE;

private:
	AGUI2Button m_aButtons[ DIFFICULTYBUTTON_NUMOF ]; // 0x7e4
	TINT        m_iActivatedButtonID;                 // 0xc04, -1 if the menu is cancelled
};

#endif // BARNYARD_LATEST
