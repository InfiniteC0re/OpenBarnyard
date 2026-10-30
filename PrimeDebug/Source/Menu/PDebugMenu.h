#pragma once
#include "PDebugMenuNode.h"

#ifdef PRIMEDEBUG_DEV
#  include "Console/PCVar.h"
#  include "Console/PCFunc.h"

//-----------------------------------------------------------------------------
// Purpose: the tree of the "Prime Debug Menu" (Prime's geDebugMenu): submenus,
// labels, console variables and command lines. Paths name menus from the root
// "." with '/' ("./debug"); the open menus are a stack of selections.
//-----------------------------------------------------------------------------
class PDebugMenuTree
{
public:
	static constexpr TINT MAX_DEPTH = 0x20;

public:
	PDebugMenuTree( TFLOAT a_fScale, TFLOAT a_fAlpha );
	~PDebugMenuTree();

	TINT            AddCommandLine( const TCHAR* a_szParent, const TCHAR* a_szName, const TCHAR* a_szCommand );
	TINT            AddLabel( const TCHAR* a_szParent, const TCHAR* a_szName, PDebugLabelCallback a_fnCallback, void* a_pUserData );
	TINT            AddSubmenu( const TCHAR* a_szParent, const TCHAR* a_szName );
	PDebugMenuNode* GetSubmenu( PDebugMenuNode* a_pParent, const TCHAR* a_szName );
	TINT            AddVar( const TCHAR* a_szParent, const TCHAR* a_szName );
	TBOOL           Remove( const TCHAR* a_szPath );
	void            Reset();

	PDebugNode*     FindPath( const TCHAR* a_szPath, PDebugMenuNode** a_ppParent );
	PDebugMenuNode* GetLevel( TINT a_iLevel );
	TINT            GetCount();
	PDebugNode*     GetSelected();
	TINT            GetSelectedId();
	TBOOL           GetSelectedLabelData( void** a_ppUserData );
	TBOOL           IsSelectedMenu();
	TINT            GetDepth() const { return m_iDepth; }

	TBOOL Next( TBOOL a_bWrap );
	TBOOL Prev( TBOOL a_bBackAtTop, TBOOL a_bWrap );
	TBOOL Enter();
	TBOOL Back();
	TBOOL Increase( TFLOAT a_fHeldTime, TFLOAT a_fDeltaTime, TBOOL a_bEnter );
	TBOOL Decrease( TFLOAT a_fHeldTime, TFLOAT a_fDeltaTime );
	TBOOL SetTyped( const TCHAR* a_szText );

	void SetScale( TFLOAT a_fScale );
	void SetAlpha( TFLOAT a_fAlpha );
	void Draw( TFLOAT a_fX, TFLOAT a_fY, TFLOAT* a_pHeight );
	void DrawPath( TFLOAT a_fX, TFLOAT a_fY, TFLOAT* a_pHeight );

private:
	struct AddVars
	{
		PDebugMenuTree* pMenu;
		PDebugMenuNode* pParent;
		const TCHAR*    szMatch; // the name without its '*'
	};

	static void AddMatch( const TCHAR* a_szName, void* a_pUserData );

private:
	PDebugMenuNode* m_pRoot;
	TINT            m_aiSelection[ MAX_DEPTH ]; // the selected entry of each open menu (-1: none)
	TINT            m_iDepth;                   // 1: the root is open
	TFLOAT          m_fScale;
	TFLOAT          m_fAlpha;
	TBOOL           m_bShowPath;
};

//-----------------------------------------------------------------------------
// Purpose: the "Prime Debug Menu" (Prime's geDebugMenu_* and its system
// object): the menu tree, its variables (debug.text_size,
// debug.transparency) and commands (debug.menu.*), the keyboard: Home shows
// or hides it, Up/Down select, Return/Escape (released) go in/back, Right
// or Return held (Left held: down) change the selected entry's value,
// faster the longer they're held; digits and '.' type a value in, applied
// by Return or Right.
// NOTE: Prime also takes the pad (left thumb, the d-pad, A, B); the game
// feeds the keyboard only (PKeyboard).
//-----------------------------------------------------------------------------
class PDebugMenu
{
public:
	static constexpr TINT MAX_TYPED = 0x10;

	static constexpr const TCHAR* ROOT = ".";

public:
	static TBOOL Startup();
	static void  Shutdown();
	static void  Reset( TBOOL a_bDefaults );
	static TBOOL SetVisible( TBOOL a_bVisible );
	static TBOOL IsVisible() { return ms_bVisible; }
	static TBOOL IsStarted() { return ms_bStarted; }
	static void  SetEnabled( TBOOL a_bEnabled );
	static void  SetPad( TINT a_iPad );
	static TINT  AddCommandLine( const TCHAR* a_szParent, const TCHAR* a_szName, const TCHAR* a_szCommand );
	static TINT  AddLabel( const TCHAR* a_szParent, const TCHAR* a_szName, PDebugLabelCallback a_fnCallback, void* a_pUserData );
	static TINT  AddSubmenu( const TCHAR* a_szParent, const TCHAR* a_szName );
	static TINT  AddVar( const TCHAR* a_szParent, const TCHAR* a_szName );
	static TBOOL Remove( const TCHAR* a_szPath );
	static TBOOL GetSelectedLabelData( void** a_ppUserData );
	static void  Update( TFLOAT a_fDeltaTime );
	static void  Draw();

	static PDebugMenuTree* GetTree() { return ms_pMenu; }

private:
	static TBOOL PushTyped( TCHAR a_cChar );
	static void  ClearTyped();
	static void  RemoveVars();
	static void  OnVisibilityChanged();
	static TBOOL HandleInput( TFLOAT a_fDeltaTime );
	static void  ReadTyped();
	static void  AddDefaults();
	static void  DrawBox( TFLOAT a_fWidth, TFLOAT a_fHeight, TFLOAT a_fX, TFLOAT a_fY, TUINT32 a_uiColour );
	static void  OnVarChanged( PCVar* a_pVar );

	static TBOOL AddCommandLineCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );
	static TBOOL AddLabelCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );
	static TBOOL AddSubmenuCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );
	static TBOOL AddVarCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );
	static TBOOL RemoveCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );
	static TBOOL ResetCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );

private:
	static TBOOL           ms_bStarted;
	static TBOOL           ms_bEnabled;
	static TBOOL           ms_bVisible;
	static PDebugMenuTree* ms_pMenu;
	static TINT            ms_iPad; // and keyboard
	static PCVar           ms_oTextSize;
	static PCVar           ms_oTransparency;
	static PCFunc          ms_oAddSubmenuFunc;
	static PCFunc          ms_oAddLabelFunc;
	static PCFunc          ms_oAddVarFunc;
	static PCFunc          ms_oAddCommandLineFunc;
	static PCFunc          ms_oRemoveFunc;
	static PCFunc          ms_oResetFunc;
	static TCHAR           ms_szTyped[ MAX_TYPED + 1 ]; // digits and '.'
	static TINT            ms_iNumTyped;
	static TFLOAT          ms_fHeldTime; // how long left or right has been held
};

#endif // PRIMEDEBUG_DEV
