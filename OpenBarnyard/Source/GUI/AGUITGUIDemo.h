#pragma once

#if defined( BARNYARD_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )
#  include <Input/TInputInterface.h>
#  include <TGUI/TGUIEvents.h>
#  include <Toshi/TEvent.h>

class AGUISystem;
class PGUIButton;
class PGUIMenuTree;

namespace Toshi
{
class TGUIScreen;
class TGUIWidget;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: development (not in Final builds), "-tguidemo": shows the dialogs
// of Barnyard's Data/GUI/main.gui one at a time on the TGUI screen of
// AGUISystem. F8 shows the next one, the arrows move the focus, Return
// presses the focused button. The menu dialogs get a few items.
//-----------------------------------------------------------------------------
class AGUITGUIDemo
{
public:
	static constexpr TINT NUM_DIALOGS = 8;
	static constexpr TINT MAX_BUTTONS = 64;

	// The first dialogs are the menu templates
	static constexpr TINT NUM_MENU_DIALOGS = 3;

	using ButtonListener = Toshi::TListener<PGUIButton, Toshi::TGUIActionEvent, AGUITGUIDemo>;
	using FocusListener  = Toshi::TListener<Toshi::TGUIScreen, Toshi::TGUIFocusChangeEvent, AGUITGUIDemo>;
	using SelectListener = Toshi::TListener<PGUIMenuTree, Toshi::TGUIActionEvent, AGUITGUIDemo>;

public:
	AGUITGUIDemo( AGUISystem* a_pGUISystem );
	~AGUITGUIDemo();

	// Is "-tguidemo" on the command line
	static TBOOL IsRequested();

	// F8 shows the next dialog, TTRUE if the event was used
	TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pEvent );

	// Every frame, before the screen is painted
	void Update();

	void ShowDialog( TINT a_iIndex );

private:
	void HideDialog();

	static TBOOL __stdcall FindButtonsCallback( Toshi::TGUIWidget* a_pWidget, void* a_pUserData );
	static TBOOL __stdcall OnButtonAction( AGUITGUIDemo* a_pDemo, PGUIButton* a_pButton, Toshi::TGUIActionEvent* a_pEvent );
	static TBOOL __stdcall OnMenuSelect( AGUITGUIDemo* a_pDemo, PGUIMenuTree* a_pMenuTree, Toshi::TGUIActionEvent* a_pEvent );
	static TBOOL __stdcall OnFocusChange( AGUITGUIDemo* a_pDemo, Toshi::TGUIScreen* a_pScreen, Toshi::TGUIFocusChangeEvent* a_pEvent );

private:
	AGUISystem*        m_pGUISystem;
	TBOOL              m_bLoaded;
	TINT               m_iIndex;
	Toshi::TGUIWidget* m_pDialog;
	PGUIMenuTree*      m_pMenuTree;
	TINT               m_iNumButtons;
	ButtonListener     m_aButtonListeners[ MAX_BUTTONS ];
	FocusListener      m_oFocusListener;
	SelectListener     m_oSelectListener;
};

#endif // BARNYARD_COMMUNITY_PATCH && !TOSHI_FINAL
