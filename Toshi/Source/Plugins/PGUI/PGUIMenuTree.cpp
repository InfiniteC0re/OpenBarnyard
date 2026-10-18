#include "ToshiPCH.h"
#include "PGUIMenuTree.h"
#include "PGUIMenuDialog.h"
#include "PGUICore.h"

#include <TGUI/TGUIManager.h>
#include <TGUI/TGUIScreen.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_INIT( PGUIMenuTree );

TGUIDeserialisable::PropertySet* PGUIMenuTree::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* PGUIMenuTree::ms_oPropertySetOrdered = TNULL;

// The depth of the SetSelectedItem calls (the selection events can select another item)
static TINT s_iSetSelectedItemDepth = 0;

// $JPOG: FUNCTION PGUICore.dll+0000fe40
void PGUIMenuTree::InitialiseClass()
{
	ms_oPropertySet        = new PropertySet( 10, 0 );
	ms_oPropertySetOrdered = new PropertySet( 10, 0 );
}

// $JPOG: FUNCTION PGUICore.dll+0000fea0
void PGUIMenuTree::DeinitialiseClass()
{
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );
}

// $JPOG: FUNCTION PGUICore.dll+0000ff50
PGUIMenuTree::PGUIMenuTree()
    : m_pScreen( TNULL )
    , m_pRootItem( TNULL )
    , m_pInvokedItem( TNULL )
    , m_pCurrentItem( TNULL )
    , m_pSelectedItem( TNULL )
    , m_aMenus( 10, 0 )
    , m_iScreenMarginLeft( 16 )
    , m_iScreenMarginRight( 16 )
    , m_iScreenMarginTop( 16 )
    , m_iScreenMarginBottom( 16 )
    , m_bUpdating( TFALSE )
    , m_bInvoked( TFALSE )
    , m_oSelectEmitter( this )
    , m_oCloseEmitter( this )
    , m_oSelectionEmitter( this )
{
	m_pRootItem = new PGUIMenuTreeItem( TNULL, L"" );
}

// The menus and the item styles aren't copied, the items are cloned
// $JPOG: FUNCTION PGUICore.dll+00011770
PGUIMenuTree::PGUIMenuTree( const PGUIMenuTree& a_rcOther )
    : TGUIDeserialisable( a_rcOther )
    , m_pScreen( a_rcOther.m_pScreen )
    , m_pRootItem( a_rcOther.m_pRootItem->Clone() )
    , m_pInvokedItem( TNULL )
    , m_pCurrentItem( TNULL )
    , m_pSelectedItem( TNULL )
    , m_aMenus( 10, 0 )
    , m_iScreenMarginLeft( a_rcOther.m_iScreenMarginLeft )
    , m_iScreenMarginRight( a_rcOther.m_iScreenMarginRight )
    , m_iScreenMarginTop( a_rcOther.m_iScreenMarginTop )
    , m_iScreenMarginBottom( a_rcOther.m_iScreenMarginBottom )
    , m_bUpdating( TFALSE )
    , m_bInvoked( TFALSE )
    , m_oSelectEmitter( this )
    , m_oCloseEmitter( this )
    , m_oSelectionEmitter( this )
{
}

// $JPOG: FUNCTION PGUICore.dll+00010030
PGUIMenuTree::~PGUIMenuTree()
{
	if ( m_pRootItem )
		delete m_pRootItem;

	for ( TINT i = 0; i < m_aMenus.Size(); i++ )
	{
		if ( m_aMenus[ i ] )
			delete m_aMenus[ i ];
	}

	while ( !m_oRecycledDialogs.IsEmpty() )
	{
		TGUIWidget* pDialog = m_oRecycledDialogs.Head().Get();
		m_oRecycledDialogs.Remove( pDialog );

		if ( pDialog )
			delete pDialog;
	}

	while ( !m_oItemStyles.IsEmpty() )
	{
		PGUIMenuTreeItemStyle* pItemStyle = m_oItemStyles.Head().Get();
		m_oItemStyles.Remove( pItemStyle );

		if ( pItemStyle )
			delete pItemStyle;
	}
}

// $JPOG: FUNCTION PGUICore.dll+00011b50
TBOOL PGUIMenuTree::Deserialise( const PProperties* a_pProperties )
{
	return TGUIDeserialisable::Deserialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+00011b30
TBOOL PGUIMenuTree::Serialise( PProperties* a_pProperties )
{
	return TGUIDeserialisable::Serialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+00011bb0
TGUIDeserialisable::PropertySet* PGUIMenuTree::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $JPOG: FUNCTION PGUICore.dll+00011ba0
TGUIDeserialisable::PropertySet* PGUIMenuTree::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

// $JPOG: FUNCTION PGUICore.dll+00011b70
PGUIMenuTree* PGUIMenuTree::Clone() const
{
	return new PGUIMenuTree( *this );
}

// A click outside the menus closes them (cancelled), the cancel action goes back, the other actions go to the screen
// when the pointer (click) or the focus is in a menu
// $JPOG: FUNCTION PGUICore.dll+000113b0
TBOOL PGUIMenuTree::DoButtonPress( TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions )
{
	if ( a_uiActions & 0x4000 )
	{
		TGUIWidget* pWidget = m_pScreen->GetWidgetUnderPointer();

		for ( TINT i = 0; i < m_aMenus.Size(); i++ )
		{
			if ( m_aMenus[ i ] && m_aMenus[ i ]->IsAncestorOf( pWidget ) )
				return m_pScreen->DoButtonPress( a_pEvent, a_uiActions );
		}

		CloseEvent oEvent;
		oEvent.bCancelled = TTRUE;
		m_oCloseEmitter.Throw( &oEvent );

		return TTRUE;
	}

	if ( a_uiActions & 0x8000 )
	{
		CloseEvent oEvent;
		oEvent.bCancelled = TTRUE;
		m_oCloseEmitter.Throw( &oEvent );

		return TTRUE;
	}

	if ( a_uiActions & 2 )
	{
		BackMenu();
		return TTRUE;
	}

	TGUIWidget* pFocusedWidget = m_pScreen->GetFocusedWidget();

	for ( TINT i = 0; i < m_aMenus.Size(); i++ )
	{
		if ( ( m_aMenus[ i ] && m_aMenus[ i ]->IsAncestorOf( pFocusedWidget ) ) || pFocusedWidget == m_aMenus[ i ] )
			return m_pScreen->DoButtonPress( a_pEvent, a_uiActions );
	}

	return TTRUE;
}

// $JPOG: FUNCTION PGUICore.dll+00011530
TBOOL PGUIMenuTree::DoButtonRepeat( TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions )
{
	return m_pScreen->DoButtonRepeat( a_pEvent, a_uiActions );
}

// $JPOG: FUNCTION PGUICore.dll+00011560
TBOOL PGUIMenuTree::DoButtonRelease( TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions )
{
	return m_pScreen->DoButtonRelease( a_pEvent, a_uiActions );
}

// $JPOG: FUNCTION PGUICore.dll+00011590
TBOOL PGUIMenuTree::DoAxisMove( TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions )
{
	return m_pScreen->DoAxisMove( a_pEvent, a_uiActions );
}

// $JPOG: FUNCTION PGUICore.dll+00011a70
TBOOL PGUIMenuTree::DoPointerMove( TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions )
{
	return m_pScreen->UpdatePointerPosition( a_pEvent, a_uiActions );
}

// The item under the pointer in a menu gets selected
// $JPOG: FUNCTION PGUICore.dll+000115c0
void PGUIMenuTree::DoRolloverUpdate()
{
	if ( m_bUpdating == TTRUE )
		return;

	TINT        iX, iY;
	TGUIWidget* pWidget = m_pScreen->HitTestDeepest( m_pScreen->GetPointerPositionX(), m_pScreen->GetPointerPositionY(), iX, iY, 0, 0 );

	if ( !pWidget )
		return;

	for ( TINT i = 0; i < m_aMenus.Size(); i++ )
	{
		if ( !m_aMenus[ i ] || !m_aMenus[ i ]->IsAncestorOf( pWidget ) )
			continue;

		m_pScreen->RefreshPointerPosition( pWidget, iX, iY );

		PGUIMenuTreeItem* pItem = m_aMenus[ i ]->FindItemData( pWidget );

		if ( pItem != m_pSelectedItem && pItem && pItem->CanSelect() )
			SetSelectedItem( pItem );

		return;
	}

	m_pScreen->RefreshPointerPosition( m_pScreen, m_pScreen->GetPointerPositionX(), m_pScreen->GetPointerPositionY() );
}

// $JPOG: FUNCTION PGUICore.dll+00010170
void PGUIMenuTree::SetScreen( TGUIScreen* a_pScreen )
{
	if ( m_pScreen == a_pScreen )
		return;

	if ( m_pScreen && m_bInvoked )
		RemoveMenus();

	m_pScreen = a_pScreen;

	if ( a_pScreen && m_bInvoked )
		AddMenus();
}

// $JPOG: FUNCTION PGUICore.dll+00010240
PGUIMenuTreeItemStyle* PGUIMenuTree::CreateItemStyleGroup( const TCHAR* a_szCurrentItemStyle, const TCHAR* a_szCurrentItemSelectedStyle, const TCHAR* a_szNextItemStyle )
{
	TGUIManager* pManager = TGUIManager::GetSingleton();

	PGUIMenuTreeItemStyle* pItemStyle = new PGUIMenuTreeItemStyle(
	    pManager->FindStyleInstance( TPString8( a_szCurrentItemStyle ) ),
	    pManager->FindStyleInstance( TPString8( a_szCurrentItemSelectedStyle ) ),
	    pManager->FindStyleInstance( TPString8( a_szNextItemStyle ) ),
	    TNULL,
	    TNULL,
	    TNULL
	);

	m_oItemStyles.InsertTail( pItemStyle );
	return pItemStyle;
}

// $JPOG: FUNCTION PGUICore.dll+00010380
PGUIMenuTreeItemStyle* PGUIMenuTree::CreateMenuStyleGroup( const TCHAR* a_szCurrentItemStyle, const TCHAR* a_szCurrentItemSelectedStyle, const TCHAR* a_szNextItemStyle, const TCHAR* a_szPreviousMenuTemplate, const TCHAR* a_szCurrentMenuTemplate, const TCHAR* a_szNextMenuTemplate )
{
	TGUIManager* pManager = TGUIManager::GetSingleton();

	TGUIStyle*  pCurrentItemStyle         = pManager->FindStyleInstance( TPString8( a_szCurrentItemStyle ) );
	TGUIStyle*  pCurrentItemSelectedStyle = pManager->FindStyleInstance( TPString8( a_szCurrentItemSelectedStyle ) );
	TGUIStyle*  pNextItemStyle            = pManager->FindStyleInstance( TPString8( a_szNextItemStyle ) );
	TGUIWidget* pPreviousMenuTemplate     = pManager->FindTopLevelInstance( TPString8( a_szPreviousMenuTemplate ), 0 );
	TGUIWidget* pCurrentMenuTemplate      = pManager->FindTopLevelInstance( TPString8( a_szCurrentMenuTemplate ), 0 );
	TGUIWidget* pNextMenuTemplate         = pManager->FindTopLevelInstance( TPString8( a_szNextMenuTemplate ), 0 );

	PGUIMenuTreeItemStyle* pItemStyle = new PGUIMenuTreeItemStyle( pCurrentItemStyle, pCurrentItemSelectedStyle, pNextItemStyle, pPreviousMenuTemplate, pCurrentMenuTemplate, pNextMenuTemplate );

	m_oItemStyles.InsertTail( pItemStyle );
	return pItemStyle;
}

// $JPOG: FUNCTION PGUICore.dll+00010590
void PGUIMenuTree::Reset()
{
	if ( m_pRootItem )
		delete m_pRootItem;

	m_pRootItem = new PGUIMenuTreeItem( TNULL, L"" );
}

// $JPOG: FUNCTION PGUICore.dll+000101d0
void PGUIMenuTree::RecurseTree( PGUIMenuTreeItem::t_RecurseCallback a_fnPre, PGUIMenuTreeItem::t_RecurseCallback a_fnPost, PGUIMenuTreeItem::t_RecurseCallback a_fnFilter, void* a_pUserData )
{
	if ( a_fnFilter && !a_fnFilter( m_pRootItem, a_pUserData ) )
		return;

	if ( a_fnPre )
		a_fnPre( m_pRootItem, a_pUserData );

	m_pRootItem->RecurseTree( a_fnPre, a_fnPost, a_fnFilter, a_pUserData );

	if ( a_fnPost )
		a_fnPost( m_pRootItem, a_pUserData );
}

// $JPOG: FUNCTION PGUICore.dll+00010b80
TBOOL PGUIMenuTree::InvokeMenu( TINT a_iIndex, TINT a_iX, TINT a_iY )
{
	auto it = m_pRootItem->GetChildren().Begin();

	for ( TINT i = 0; i < a_iIndex; i++ )
		it++;

	return InvokeMenu( TSTATICCAST( PGUIMenuTreeItem, it.Get() ), a_iX, a_iY );
}

// $JPOG: FUNCTION PGUICore.dll+00010bc0
TBOOL PGUIMenuTree::InvokeMenu( PGUIMenuTreeItem* a_pItem, TINT a_iX, TINT a_iY )
{
	PGUIMenuTreeItem* pChild = a_pItem->ChooseChildItem();

	if ( !pChild )
		return TFALSE;

	m_bInvoked     = TTRUE;
	m_iInvokeX     = a_iX;
	m_iInvokeY     = a_iY;
	m_pInvokedItem = pChild->GetParent();

	SetSelectedItem( pChild );
	DeselectMenu();
	m_pScreen->PushInputManager( this );

	return TTRUE;
}

// $JPOG: FUNCTION PGUICore.dll+00011aa0
void PGUIMenuTree::CancelMenu()
{
	m_pScreen->PopInputManager( this );
	RemoveMenus();
	m_bInvoked = TFALSE;
}

// The menus of the selected item (its next menu), of the current item and of the parents up to the root item, made
// from the menu templates of their item styles (recycled)
// $JPOG: FUNCTION PGUICore.dll+000105f0
void PGUIMenuTree::UpdateMenu( PGUIMenuTreeItem* a_pOldSelection )
{
	if ( !m_pScreen || !m_pCurrentItem )
		return;

	m_pScreen->AllowRolloverUpdate( TFALSE );

	// The items whose menus are shown, from the selected one up (TNULL for no selection)
	PGUIMenuTreeItem* apItems[ 64 ];
	TINT              iNumItems = 0;
	PGUIMenuTreeItem* pItem     = m_pSelectedItem;

	if ( !pItem )
	{
		apItems[ iNumItems++ ] = TNULL;
		pItem                  = m_pCurrentItem;
	}

	for ( ; pItem != m_pRootItem; pItem = pItem->GetParent() )
		apItems[ iNumItems++ ] = pItem;

	m_pScreen->RequestFocus();

	while ( m_aMenus.Size() > 0 )
	{
		if ( m_aMenus[ m_aMenus.Size() - 1 ] )
			DiscardDialog( m_aMenus[ m_aMenus.Size() - 1 ] );

		m_aMenus.Pop();
	}

	while ( iNumItems != 0 )
	{
		iNumItems--;
		pItem = apItems[ iNumItems ];

		if ( !pItem )
		{
			m_aMenus.Push( TNULL );
			continue;
		}

		PGUIMenuTreeItemStyle::DisplayLevel eLevel;
		TGUIWidget*                         pTemplate;

		if ( iNumItems == 0 )
		{
			pTemplate = pItem->GetStyle()->GetNextMenuTemplate();
			eLevel    = PGUIMenuTreeItemStyle::DisplayLevel_Next;
		}
		else if ( iNumItems == 1 )
		{
			pTemplate = pItem->GetStyle()->GetCurrentMenuTemplate();
			eLevel    = PGUIMenuTreeItemStyle::DisplayLevel_Current;
		}
		else
		{
			pTemplate = pItem->GetStyle()->GetPreviousMenuTemplate();
			eLevel    = PGUIMenuTreeItemStyle::DisplayLevel_Previous;
		}

		PGUIMenuDialog* pDialog = TNULL;

		if ( pTemplate && pItem->HasChildren() )
		{
			pDialog = RecycleDialog( TSTATICCAST( PGUIMenuDialog, pTemplate ) );
			pDialog->SetMenuTree( this );
			pDialog->SetDisplayMode( eLevel );

			PGUIMenuTreeItem* pNextItem = apItems[ ( iNumItems - 1 < 0 ) ? 0 : iNumItems - 1 ];

			if ( eLevel == PGUIMenuTreeItemStyle::DisplayLevel_Next || !pNextItem )
				pDialog->SetRootItem( pItem );
			else
				pDialog->SetSelectedItem( pNextItem );

			if ( !pDialog->GetParentWidget() )
				m_pScreen->AddWidget( pDialog, TFALSE );

			if ( eLevel == PGUIMenuTreeItemStyle::DisplayLevel_Current )
				pDialog->RequestFocus();
		}

		m_aMenus.Push( pDialog );
	}

	// Places the menus (each one gives the position of the next)
	TINT iX = m_iInvokeX;
	TINT iY = m_iInvokeY;

	for ( TINT i = 0; i < m_aMenus.Size(); i++ )
	{
		if ( m_aMenus[ i ] )
			m_aMenus[ i ]->UpdatePosition( iX, iY );
	}

	m_pScreen->AllowRolloverUpdate( TTRUE );
}

// $JPOG: FUNCTION PGUICore.dll+00010c30
void PGUIMenuTree::ScrollMenu( TINT a_iDirection )
{
	if ( !m_pSelectedItem )
		return;

	PGUIPlaySound( m_pScreen, "PGUIMenuTree", "navigate" );
	SetSelectedItem( m_pSelectedItem->ChooseNextItem( a_iDirection ) );
}

// Selects the first child of the selected item that can be selected
// $JPOG: FUNCTION PGUICore.dll+00010cf0
void PGUIMenuTree::DescendMenu()
{
	PGUIMenuTreeItem* pChild = TNULL;

	if ( m_pSelectedItem && m_pSelectedItem->CanSelect() && m_pSelectedItem->HasChildren() )
	{
		pChild = m_pSelectedItem->GetFirstChild();

		if ( !pChild->CanSelect() )
			pChild = pChild->ChooseNextItem( 1 );
	}

	if ( pChild )
	{
		PGUIPlaySound( m_pScreen, "PGUIMenuTree", "descend" );
		SetSelectedItem( pChild );
	}
	else
	{
		PGUIPlaySound( m_pScreen, "PGUIMenuTree", "descend_fail" );
	}
}

// Selects the current item (in the previous menu), not past the invoked item
// $JPOG: FUNCTION PGUICore.dll+00010e70
void PGUIMenuTree::AscendMenu()
{
	if ( !m_pCurrentItem )
		return;

	if ( m_pCurrentItem == m_pInvokedItem )
	{
		PGUIPlaySound( m_pScreen, "PGUIMenuTree", "ascend_fail" );
	}
	else
	{
		PGUIPlaySound( m_pScreen, "PGUIMenuTree", "ascend" );
		SetSelectedItem( m_pCurrentItem );
	}
}

// Ascends, or closes the menus (cancelled) from the invoked menu
// $JPOG: FUNCTION PGUICore.dll+00010fb0
void PGUIMenuTree::BackMenu()
{
	if ( !m_pCurrentItem )
		return;

	if ( m_pCurrentItem == m_pInvokedItem )
	{
		PGUIPlaySound( m_pScreen, "PGUIMenuTree", "exit" );

		CloseEvent oEvent;
		oEvent.bCancelled = TTRUE;
		m_oCloseEmitter.Throw( &oEvent );
	}
	else
	{
		AscendMenu();
	}
}

// A leaf item throws the select event (its listeners can refuse it), then the close event; an item with children
// opens its menu
// $JPOG: FUNCTION PGUICore.dll+00011080
void PGUIMenuTree::SelectMenu()
{
	if ( !m_pSelectedItem )
		return;

	if ( m_pSelectedItem->HasChildren() )
	{
		if ( !m_bInvoked )
			InvokeMenu( m_pSelectedItem, 0, 0 );

		DescendMenu();
		return;
	}

	if ( !m_pSelectedItem->CanSelect() )
		return;

	TGUIActionEvent oEvent;
	oEvent.pItem   = m_pSelectedItem;
	oEvent.bAccept = TTRUE;
	m_oSelectEmitter.Throw( &oEvent );

	if ( oEvent.bAccept == TTRUE )
	{
		PGUIPlaySound( m_pScreen, "PGUIMenuTree", "select" );

		CloseEvent oCloseEvent;
		oCloseEvent.bCancelled = TFALSE;
		m_oCloseEmitter.Throw( &oCloseEvent );
	}
	else
	{
		PGUIPlaySound( m_pScreen, "PGUIMenuTree", "select_fail" );
	}
}

// $JPOG: FUNCTION PGUICore.dll+00011220
void PGUIMenuTree::DeselectMenu()
{
	if ( !m_pCurrentItem )
		return;

	m_pSelectedItem = TNULL;

	SelectionEvent oEvent;
	oEvent.pItem = TNULL;
	m_oSelectionEmitter.Throw( &oEvent );

	UpdateMenu( TNULL );
}

// Throws the selection event and updates the menus when invoked (three nested calls at most)
// $JPOG: FUNCTION PGUICore.dll+00011330
void PGUIMenuTree::SetSelectedItem( PGUIMenuTreeItem* a_pItem )
{
	if ( ++s_iSetSelectedItemDepth >= 4 )
	{
		m_bUpdating = TFALSE;

#ifdef BARNYARD_COMMUNITY_PATCH
		// Fixes the depth not being decremented: the items couldn't be selected any more
		s_iSetSelectedItemDepth--;
#endif
		return;
	}

	m_bUpdating = TTRUE;

	PGUIMenuTreeItem* pOldSelection = m_pSelectedItem;
	m_pSelectedItem                 = a_pItem;
	m_pCurrentItem                  = a_pItem->GetParent();

	if ( m_bInvoked )
	{
		SelectionEvent oEvent;
		oEvent.pItem = a_pItem;
		m_oSelectionEmitter.Throw( &oEvent );

		m_pSelectedItem = a_pItem;
		m_pCurrentItem  = a_pItem->GetParent();
		UpdateMenu( pOldSelection );
	}

	m_bUpdating = TFALSE;
	DoRolloverUpdate();

	s_iSetSelectedItemDepth--;
}

// $JPOG: FUNCTION PGUICore.dll+000101b0
PGUIMenuDialog* PGUIMenuTree::GetCurrentMenu()
{
	if ( m_aMenus.Size() < 2 )
		return TNULL;

	return m_aMenus[ m_aMenus.Size() - 2 ];
}

// $JPOG: FUNCTION PGUICore.dll+00011260
TUINT PGUIMenuTree::PropagateState( TUINT a_uiMask, PGUIMenuTreeItem* a_pItem )
{
	if ( !a_pItem->HasChildren() )
		return a_pItem->GetState();

	TUINT uiState = a_pItem->GetState() & ~a_uiMask;

	for ( auto it = a_pItem->GetChildren().Begin(); it != a_pItem->GetChildren().End(); it++ )
		uiState |= a_uiMask & PropagateState( a_uiMask, TSTATICCAST( PGUIMenuTreeItem, it.Get() ) );

	a_pItem->SetState( uiState );
	return uiState;
}

// $JPOG: FUNCTION PGUICore.dll+000112f0
void PGUIMenuTree::PropagateState( TUINT a_uiMask, TUINT a_uiValue, PGUIMenuTreeItem* a_pFrom, PGUIMenuTreeItem* a_pTo )
{
	for ( PGUIMenuTreeItem* pItem = a_pFrom; pItem != a_pTo; pItem = pItem->GetParent() )
		pItem->SetState( a_uiMask, a_uiValue );

	UpdateMenu( TNULL );
}

// $JPOG: FUNCTION PGUICore.dll+000116b0
void PGUIMenuTree::FlushOldDialogs()
{
	while ( m_aMenus.Size() > 0 )
	{
		if ( m_aMenus[ m_aMenus.Size() - 1 ] )
			DiscardDialog( m_aMenus[ m_aMenus.Size() - 1 ] );

		m_aMenus.Pop();
	}

	m_aMenus.FreeData();

	while ( !m_oRecycledDialogs.IsEmpty() )
	{
		TGUIWidget* pDialog = m_oRecycledDialogs.Head().Get();
		m_oRecycledDialogs.Remove( pDialog );

		if ( pDialog )
			delete pDialog;
	}
}

// $JPOG: FUNCTION PGUICore.dll+00011b10
void PGUIMenuTree::SetScreenMargins( TINT a_iLeft, TINT a_iRight, TINT a_iTop, TINT a_iBottom )
{
	m_iScreenMarginLeft   = a_iLeft;
	m_iScreenMarginRight  = a_iRight;
	m_iScreenMarginTop    = a_iTop;
	m_iScreenMarginBottom = a_iBottom;
}

// $JPOG: FUNCTION PGUICore.dll+00011b00
void PGUIMenuTree::SetScreenMarginLeft( TINT a_iMargin )
{
	m_iScreenMarginLeft = a_iMargin;
}

// $JPOG: FUNCTION PGUICore.dll+00011af0
void PGUIMenuTree::SetScreenMarginRight( TINT a_iMargin )
{
	m_iScreenMarginRight = a_iMargin;
}

// $JPOG: FUNCTION PGUICore.dll+00011ae0
void PGUIMenuTree::SetScreenMarginTop( TINT a_iMargin )
{
	m_iScreenMarginTop = a_iMargin;
}

// $JPOG: FUNCTION PGUICore.dll+00011ad0
void PGUIMenuTree::SetScreenMarginBottom( TINT a_iMargin )
{
	m_iScreenMarginBottom = a_iMargin;
}

// $JPOG: FUNCTION PGUICore.dll+00011830
void PGUIMenuTree::RemoveMenus()
{
	for ( TINT i = 0; i < m_aMenus.Size(); i++ )
	{
		if ( m_aMenus[ i ] && m_aMenus[ i ]->GetParentWidget() )
			m_aMenus[ i ]->GetParentWidget()->RemoveWidget( m_aMenus[ i ] );
	}
}

// $JPOG: FUNCTION PGUICore.dll+00011880
void PGUIMenuTree::AddMenus()
{
	for ( TINT i = 0; i < m_aMenus.Size(); i++ )
	{
		if ( m_aMenus[ i ] && !m_aMenus[ i ]->GetParentWidget() )
			m_pScreen->AddWidget( m_aMenus[ i ], TFALSE );
	}
}

// $JPOG: FUNCTION PGUICore.dll+000118e0
void PGUIMenuTree::DiscardDialog( PGUIMenuDialog* a_pDialog )
{
	if ( a_pDialog->GetParentWidget() )
		a_pDialog->GetParentWidget()->RemoveWidget( a_pDialog );

	m_oRecycledDialogs.InsertTail( a_pDialog );
}

// $JPOG: FUNCTION PGUICore.dll+00011920
PGUIMenuDialog* PGUIMenuTree::RecycleDialog( PGUIMenuDialog* a_pTemplate )
{
	for ( auto it = m_oRecycledDialogs.Begin(); it != m_oRecycledDialogs.End(); it++ )
	{
		if ( it->GetName() == a_pTemplate->GetName() )
		{
			TGUIWidget* pDialog = it.Get();
			m_oRecycledDialogs.Remove( pDialog );

			return TSTATICCAST( PGUIMenuDialog, pDialog );
		}
	}

	return a_pTemplate->Clone();
}
