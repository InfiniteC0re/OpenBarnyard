#include "pch.h"
#include "AGUITGUIDemo.h"

#if defined( BARNYARD_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )
#  include "AGUISystem.h"

#  include <TGUI/TGUIManager.h>
#  include <TGUI/TGUIScreen.h>
#  include <Plugins/PGUI/PGUIButton.h>
#  include <Plugins/PGUI/PGUIMenuDialog.h>
#  include <Plugins/PGUI/PGUIMenuTreeItem.h>
#  include <Plugins/PGUI/PGUIMenuTreeItemStyle.h>
#  include <Plugins/PGUI/PGUIMenuTree.h>
#  include <Input/TInputDeviceKeyboard.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The dialogs of Data/GUI/main.gui
static const TCHAR* s_aDialogNames[ AGUITGUIDemo::NUM_DIALOGS ] = {
	"CurrentMenu",
	"CurrentPopupMenu",
	"NextMenuTemplate",
	"AInteriorNetworkPlacementDialog",
	"AInteriorNetworkPlacementDialog2",
	"ACameraVolumePlacementDialog",
	"ATargetPlacementState",
	"AAmbientSoundPlacementDialog",
};

AGUITGUIDemo::AGUITGUIDemo( AGUISystem* a_pGUISystem )
    : m_pGUISystem( a_pGUISystem )
    , m_bLoaded( TFALSE )
    , m_iIndex( -1 )
    , m_pDialog( TNULL )
    , m_pMenuTree( TNULL )
    , m_iNumButtons( 0 )
{
	AGUISystem::TGUIMemoryScope oMemoryScope;

	if ( !m_pGUISystem->CreateTGUI() )
		return;

	m_bLoaded = m_pGUISystem->LoadTGUIDefinitionFile( "Data/GUI/main.gui" );

	m_oFocusListener.Connect( m_pGUISystem->GetTGUIScreen()->GetFocusChangeEmitter(), this, OnFocusChange, 0 );

	if ( m_bLoaded )
	{
		TINFO( "[tguidemo] %d fonts, %d texture sections, F8 shows the next dialog\n", TGUIManager::GetSingleton()->GetFontList().Count(), TGUIManager::GetSingleton()->GetTextureSectionList().Count() );
		ShowDialog( 0 );
	}
}

AGUITGUIDemo::~AGUITGUIDemo()
{
	HideDialog();
	m_oFocusListener.Disconnect();
}

TBOOL AGUITGUIDemo::IsRequested()
{
	static TINT s_iRequested = -1;

	if ( s_iRequested < 0 )
		s_iRequested = TStringManager::String8FindString( GetCommandLineA(), "-tguidemo" ) ? 1 : 0;

	return s_iRequested == 1;
}

TBOOL AGUITGUIDemo::ProcessInput( const TInputInterface::InputEvent* a_pEvent )
{
	if ( !m_bLoaded || a_pEvent->GetDoodad() != TInputDeviceKeyboard::KEY_F8 )
		return TFALSE;

	if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
		ShowDialog( ( m_iIndex + 1 ) % NUM_DIALOGS );

	return TTRUE;
}

void AGUITGUIDemo::Update()
{
}

void AGUITGUIDemo::ShowDialog( TINT a_iIndex )
{
	AGUISystem::TGUIMemoryScope oMemoryScope;

	HideDialog();
	m_iIndex = a_iIndex;

	// The menu dialogs are the templates of the menus of a menu tree (the current menu, the menu of the children of the
	// selected item), shown with a few items
	if ( a_iIndex < NUM_MENU_DIALOGS )
	{
		static const TCHAR* s_aCurrentMenus[ NUM_MENU_DIALOGS ] = { "CurrentMenu", "CurrentPopupMenu", "NextMenuTemplate" };

		m_pMenuTree = new PGUIMenuTree();
		m_pMenuTree->SetScreen( m_pGUISystem->GetTGUIScreen() );
		m_oSelectListener.Connect( m_pMenuTree->GetSelectEmitter(), this, OnMenuSelect, 0 );

		PGUIMenuTreeItemStyle* pStyle = m_pMenuTree->CreateMenuStyleGroup( "Button_MenuCurr", "Button_MenuSel", "Button_MenuNext", "CurrentPopupMenu", s_aCurrentMenus[ a_iIndex ], "NextMenuTemplate" );

		const TUINT uiState = PGUIItemData::State_Enabled | PGUIItemData::State_Visible;

		PGUIMenuTreeItem* pMenu = new PGUIMenuTreeItem( pStyle, L"TGUI Demo" );
		pMenu->SetState( uiState );
		m_pMenuTree->GetRootItem()->AddItem( pMenu );

		PGUIMenuTreeItem* pItem = pMenu->AddItem( L"Placement Dialogs", uiState );
		pMenu->AddItem( L"Camera Volumes", uiState );
		pMenu->AddItem( L"Ambient Sounds", uiState );
		pMenu->AddItem( L"Exit", uiState );

		// The next menu (NextMenuTemplate) shows the children of the selected item
		if ( a_iIndex != 2 )
		{
			pItem->AddItem( L"Interior Network", uiState );
			pItem->AddItem( L"Targets", uiState );
		}

		// The current menu on the left of the middle of the screen, the next menu goes on the right (PGUIMenuDialog::UpdatePosition)
		m_pMenuTree->InvokeMenu( pMenu, AGUISystem::TGUI_SCREEN_WIDTH / 2 - 240, AGUISystem::TGUI_SCREEN_HEIGHT / 2 - 100 );

		// The menus are made for the pointer (nothing is selected), the keyboard and the pad scroll from the first item
		m_pMenuTree->SetSelectedItem( pItem );

		PGUIMenuDialog* pCurrent = m_pMenuTree->GetCurrentMenu();
		TINFO( "[tguidemo] showing %s (%d/%d) %dx%d, selected '%s'\n", s_aDialogNames[ a_iIndex ], a_iIndex + 1, NUM_DIALOGS, pCurrent ? pCurrent->GetWidth() : 0, pCurrent ? pCurrent->GetHeight() : 0, pCurrent ? pCurrent->GetName().GetString() : "" );
		return;
	}

	m_pDialog = m_pGUISystem->ShowTGUIDialog( TPString8( s_aDialogNames[ a_iIndex ] ) );

	if ( !m_pDialog )
	{
		TWARN( "[tguidemo] no dialog '%s'\n", s_aDialogNames[ a_iIndex ] );
		return;
	}

	// Centred on the screen, which grows (the GUI is drawn smaller) when the dialog doesn't fit in it
	TFLOAT fScaleX = TFLOAT( m_pDialog->GetWidth() + 20 ) / AGUISystem::TGUI_SCREEN_WIDTH;
	TFLOAT fScaleY = TFLOAT( m_pDialog->GetHeight() + 20 ) / AGUISystem::TGUI_SCREEN_HEIGHT;
	TFLOAT fScale  = TMath::Max( 1.0f, TMath::Max( fScaleX, fScaleY ) );
	TINT   iWidth  = TINT( AGUISystem::TGUI_SCREEN_WIDTH * fScale );
	TINT   iHeight = TINT( AGUISystem::TGUI_SCREEN_HEIGHT * fScale );

	m_pGUISystem->GetTGUIScreen()->SetFixedSize( iWidth, iHeight );
	m_pDialog->SetPosition( ( iWidth - m_pDialog->GetWidth() ) / 2, ( iHeight - m_pDialog->GetHeight() ) / 2 );

	m_iNumButtons = 0;
	m_pDialog->RecurseTree( FindButtonsCallback, TNULL, TNULL, this );

	TGUIWidget* pFocused = m_pGUISystem->GetTGUIScreen()->GetFocusedWidget();
	TINFO( "[tguidemo] showing %s (%d/%d) %dx%d, %d buttons, focus '%s'\n", s_aDialogNames[ a_iIndex ], a_iIndex + 1, NUM_DIALOGS, m_pDialog->GetWidth(), m_pDialog->GetHeight(), m_iNumButtons, pFocused ? pFocused->GetName().GetString() : "" );
}

void AGUITGUIDemo::HideDialog()
{
	AGUISystem::TGUIMemoryScope oMemoryScope;

	for ( TINT i = 0; i < m_iNumButtons; i++ )
		m_aButtonListeners[ i ].Disconnect();

	m_iNumButtons = 0;

	if ( m_pDialog )
	{
		m_pGUISystem->HideTGUIDialog( m_pDialog );
		m_pGUISystem->GetTGUIScreen()->SetFixedSize( AGUISystem::TGUI_SCREEN_WIDTH, AGUISystem::TGUI_SCREEN_HEIGHT );
		m_pDialog = TNULL;
	}

	if ( m_pMenuTree )
	{
		m_oSelectListener.Disconnect();

		if ( m_pMenuTree->IsInvoked() )
			m_pMenuTree->CancelMenu();

		delete m_pMenuTree;
		m_pMenuTree = TNULL;
	}
}

TBOOL __stdcall AGUITGUIDemo::FindButtonsCallback( TGUIWidget* a_pWidget, void* a_pUserData )
{
	AGUITGUIDemo* pDemo = TSTATICCAST( AGUITGUIDemo, a_pUserData );

	if ( a_pWidget->IsA( &TGetClass( PGUIButton ) ) && pDemo->m_iNumButtons < MAX_BUTTONS )
	{
		PGUIButton* pButton = TSTATICCAST( PGUIButton, a_pWidget );
		pDemo->m_aButtonListeners[ pDemo->m_iNumButtons++ ].Connect( pButton->GetActionEmitter(), pDemo, OnButtonAction, 0 );
	}

	return TTRUE;
}

TBOOL __stdcall AGUITGUIDemo::OnButtonAction( AGUITGUIDemo* a_pDemo, PGUIButton* a_pButton, TGUIActionEvent* a_pEvent )
{
	TINFO( "[tguidemo] button '%s' pressed\n", a_pButton->GetName().GetString() );
	return TTRUE;
}

TBOOL __stdcall AGUITGUIDemo::OnMenuSelect( AGUITGUIDemo* a_pDemo, PGUIMenuTree* a_pMenuTree, TGUIActionEvent* a_pEvent )
{
	PGUIMenuTreeItem* pItem = TSTATICCAST( PGUIMenuTreeItem, a_pEvent->pItem );
	TINFO( "[tguidemo] menu item %p selected\n", pItem );
	return TTRUE;
}

TBOOL __stdcall AGUITGUIDemo::OnFocusChange( AGUITGUIDemo* a_pDemo, TGUIScreen* a_pScreen, TGUIFocusChangeEvent* a_pEvent )
{
	TINFO( "[tguidemo] focus '%s' -> '%s'\n", a_pEvent->pOldFocus ? a_pEvent->pOldFocus->GetName().GetString() : "", a_pEvent->pNewFocus ? a_pEvent->pNewFocus->GetName().GetString() : "" );
	return TTRUE;
}

#endif // BARNYARD_COMMUNITY_PATCH && !TOSHI_FINAL
