#pragma once
#include "ABYardMenuState.h"
#include "GUI/AGUI2Button.h"

#include <T2Locale/T2Locale.h>

//-----------------------------------------------------------------------------
// Purpose: the language select screen. A list of language names (each in its
// own language); the dialog title and the menu navigation bar show the focused
// language's "Language" / "Confirm" / "Cancel". Picking one sets AOptions and
// the locale and refreshes the texts of the screens under it.
// The EU PS2 builds (SLES_543.76/.78) push it from Options ("Language", item 2)
// and, at boot, before the save is loaded when the console language isn't one
// the disc has (then it can't be cancelled and replaces itself with the save
// load state). PC retail only has the TClass (size 0x11fc, parent AGameState)
// and an unreferenced 5 language table (0x007167b8, with German); the class is
// recovered from SLES_543.78 (vtable 0x0050a800, size 0x1020, 4 languages).
// In community builds it's reached from Options -> Language.
//-----------------------------------------------------------------------------
class AFrontEndLanguageState
    : public ABYardMenuState
{
public:
	TDECLARE_CLASS( AFrontEndLanguageState, ABYardMenuState );

	struct LanguageInfo
	{
		Toshi::T2Locale::Lang eLanguage;
		const TWCHAR*         wszName;   // the button
		const TWCHAR*         wszTitle;  // the dialog title while the language is focused
		const TWCHAR*         wszOK;     // left text of AMenuNavHudElement
		const TWCHAR*         wszCancel; // right text of AMenuNavHudElement
	};

	enum LANGUAGE
	{
#ifdef BARNYARD_COMMUNITY_PATCH
		// NOTE: not in any build, the PC data has both English locales (eng-uk.trb is the PC default)
		LANGUAGE_ENGLISH_UK,
		LANGUAGE_ENGLISH_US,
#endif
		LANGUAGE_DUTCH,
		LANGUAGE_GERMAN, // PC table only, the EU PS2 discs have no German
		LANGUAGE_ITALIAN,
		LANGUAGE_SPANISH,
		LANGUAGE_FRENCH,
		LANGUAGE_NUMOF
	};

	static const LanguageInfo ms_aLanguages[ LANGUAGE_NUMOF ];

public:
	AFrontEndLanguageState( AGameState* a_pNextState = TNULL, TBOOL a_bFirstBoot = TFALSE );
	~AFrontEndLanguageState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;
	virtual void  OnDeactivate() OVERRIDE;

	//-----------------------------------------------------------------------------
	// ABYardMenuState
	//-----------------------------------------------------------------------------
	virtual void OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem ) OVERRIDE;
	virtual void OnMenuItemFocused( AGUI2MenuItem& a_rOldFocus, AGUI2MenuItem& a_rNewFocus ) OVERRIDE;
	virtual void OnMenuClose() OVERRIDE;

private:
	// Shows the title and the navigation texts of the focused language
	void UpdateTexts();

private:
	AGUI2Button           m_aButtons[ LANGUAGE_NUMOF ]; // PS2: 0x7e0
	TINT                  m_iSelectedLanguage;          // PS2: 0x1010, the focused or picked one, -1 when cancelled
	AGameState*           m_pNextState;                 // PS2: 0x1014, replaces this state when it closes (first boot)
	Toshi::T2Locale::Lang m_eLanguage;                  // PS2: 0x1018, the language when the screen opened
	TBOOL                 m_bFirstBoot;                 // PS2: 0x101c
};

// NOTE: the PC TClass size (0x11fc) is ABYardMenuState + 5 buttons + one dword, so the PC version of the class
// kept a single member besides the buttons. This one has the four PS2 members.
