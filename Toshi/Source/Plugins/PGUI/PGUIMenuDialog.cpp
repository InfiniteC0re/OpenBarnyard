#include "ToshiPCH.h"
#include "PGUIMenuDialog.h"
#include "PGUIMenuTree.h"
#include "PGUIMenuTreeItem.h"

#include <TGUI/TGUIManager.h>
#include <TGUI/TGUIWidgetInfo.h>
#include <TGUI/TGUIStyle.h>
#include <TGUI/TGUIScreen.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_INIT( PGUIMenuDialog );

TGUIDeserialisable::PropertySet* PGUIMenuDialog::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* PGUIMenuDialog::ms_oPropertySetOrdered = TNULL;
TGUIWidgetInfo*                  PGUIMenuDialog::ms_pWidgetInfo         = TNULL;

TPString8 PGUIMenuDialog::str_menubucket;
TPString8 PGUIMenuDialog::str_menuheading;
TPString8 PGUIMenuDialog::str_itemselected;
TPString8 PGUIMenuDialog::str_itemback;

// NOTE: the names of the children are function statics of UpdateMenu in the original, they're created here
// $JPOG: FUNCTION PGUICore.dll+0000e0d0
void PGUIMenuDialog::InitialiseClass()
{
	str_menubucket   = TPString8( "_MenuBucket" );
	str_menuheading  = TPString8( "_MenuHeading" );
	str_itemselected = TPString8( "_ItemSelected" );
	str_itemback     = TPString8( "_ItemBack" );

	ms_pWidgetInfo = new TGUIWidgetInfo();
	CreatePropertySets( ms_oPropertySet, ms_oPropertySetOrdered, *PGUIDialog::ms_oPropertySet );

	ms_pWidgetInfo->AddComponentName( TPString8( "background" ) );
	ms_pWidgetInfo->AddComponentName( TPString8( "menubackground" ) );
	ms_pWidgetInfo->SetWidgetName( TPString8( "PGUIMenuDialog" ) );
	ms_pWidgetInfo->SetWidgetLabel( TPString8( "Menu Dialog" ) );
	ms_pWidgetInfo->SetProperties( ms_oPropertySet, ms_oPropertySetOrdered );
	ms_pWidgetInfo->AddStateVars( PGUIDialog::ms_pWidgetInfo->GetStateVarSet() );
	ms_pWidgetInfo->SetConcrete( TTRUE );

	TGUIManager::AddWidgetClass( ms_pWidgetInfo );
}

// $JPOG: FUNCTION PGUICore.dll+0000e4e0
void PGUIMenuDialog::DeinitialiseClass()
{
	ms_pWidgetInfo->Clear();
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );

	delete ms_pWidgetInfo;
	ms_pWidgetInfo = TNULL;

	str_itemback     = TPString8();
	str_itemselected = TPString8();
	str_menuheading  = TPString8();
	str_menubucket   = TPString8();
}

// $JPOG: FUNCTION PGUICore.dll+0000e5a0
PGUIMenuDialog::PGUIMenuDialog()
    : m_pMenuTree( TNULL )
    , m_pRootItem( TNULL )
    , m_pSelectedItem( TNULL )
    , m_pSelectedButton( TNULL )
    , m_pMenuBucket( TNULL )
    , m_pMenuHeading( TNULL )
    , m_pItemSelected( TNULL )
    , m_pItemBack( TNULL )
    , m_eDisplayMode( PGUIMenuTreeItemStyle::DisplayLevel_Current )
    , m_oButtonListeners( this, OnButtonAction )
{
}

// $JPOG: FUNCTION PGUICore.dll+0000ecc0
PGUIMenuDialog::PGUIMenuDialog( const PGUIMenuDialog& a_rcOther )
    : PGUIDialog( a_rcOther )
    , m_pMenuTree( a_rcOther.m_pMenuTree )
    , m_pRootItem( TNULL )
    , m_pSelectedItem( TNULL )
    , m_pSelectedButton( TNULL )
    , m_pMenuBucket( TNULL )
    , m_pMenuHeading( TNULL )
    , m_pItemSelected( TNULL )
    , m_pItemBack( TNULL )
    , m_eDisplayMode( a_rcOther.m_eDisplayMode )
    , m_oButtonListeners( this, OnButtonAction )
{
}

// $JPOG: FUNCTION PGUICore.dll+0000f600
PGUIMenuDialog::~PGUIMenuDialog()
{
	m_oButtonListeners.DeleteAll();

	while ( !m_oRecycledButtons.IsEmpty() )
	{
		TGUIWidget* pButton = m_oRecycledButtons.Head().Get();
		m_oRecycledButtons.Remove( pButton );

		if ( pButton )
			delete pButton;
	}
}

// $JPOG: FUNCTION PGUICore.dll+0000f5a0
TBOOL PGUIMenuDialog::Deserialise( const PProperties* a_pProperties )
{
	return PGUIDialog::Deserialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+0000f570
TBOOL PGUIMenuDialog::Serialise( PProperties* a_pProperties )
{
	return PGUIDialog::Serialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+0000f6f0
TGUIDeserialisable::PropertySet* PGUIMenuDialog::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $JPOG: FUNCTION PGUICore.dll+0000f6e0
TGUIDeserialisable::PropertySet* PGUIMenuDialog::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

const TGUIWidgetInfo* PGUIMenuDialog::GetWidgetInfo() const
{
	return ms_pWidgetInfo;
}

// $JPOG: FUNCTION PGUICore.dll+0000f5d0
PGUIMenuDialog* PGUIMenuDialog::Clone() const
{
	return new PGUIMenuDialog( *this );
}

// $JPOG: FUNCTION PGUICore.dll+00005da0
TBOOL PGUIMenuDialog::AcceptsFocus() const
{
	return TTRUE;
}

// $JPOG: FUNCTION PGUICore.dll+0000f530
void PGUIMenuDialog::NotifyFocusLost( TGUIWidget* a_pNewFocus, TGUIWidget* a_pOldFocus )
{
	TGUIWindow::NotifyFocusLost( a_pNewFocus, a_pOldFocus );
}

// $JPOG: FUNCTION PGUICore.dll+0000f510
void PGUIMenuDialog::NotifyFocusGained( TGUIWidget* a_pNewFocus, TGUIWidget* a_pOldFocus )
{
	TGUIWindow::NotifyFocusGained( a_pNewFocus, a_pOldFocus );
}

// North/south scroll the menu (repeated while held), east/west descend/ascend, select selects (back with nothing
// selected), cancel goes back
// $JPOG: FUNCTION PGUICore.dll+0000e7d0
TBOOL PGUIMenuDialog::OnButtonPress( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	if ( TGUIWidget::OnButtonPress( a_pEvent, a_iActions ) )
		return TTRUE;

	if ( a_iActions & 0x200 )
	{
		a_pEvent->StartRepeat( 0.5f, 0.25f );
		m_pMenuTree->ScrollMenu( -1 );
		return TTRUE;
	}

	if ( a_iActions & 0x800 )
	{
		a_pEvent->StartRepeat( 0.5f, 0.25f );
		m_pMenuTree->ScrollMenu( 1 );
		return TTRUE;
	}

	if ( a_iActions & 0x400 )
	{
		m_pMenuTree->DescendMenu();
		return TTRUE;
	}

	if ( a_iActions & 0x1000 )
	{
		m_pMenuTree->AscendMenu();
		return TTRUE;
	}

	if ( a_iActions & 1 )
	{
		if ( m_pSelectedItem || !m_pItemBack )
			m_pMenuTree->SelectMenu();
		else
			m_pMenuTree->BackMenu();

		return TTRUE;
	}

	if ( a_iActions & 2 )
	{
		m_pMenuTree->BackMenu();
		return TTRUE;
	}

	return TFALSE;
}

// $JPOG: FUNCTION PGUICore.dll+0000e900
TBOOL PGUIMenuDialog::OnButtonRepeat( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	if ( TGUIWidget::OnButtonRepeat( a_pEvent, a_iActions ) )
		return TTRUE;

	if ( a_iActions & 0x200 )
	{
		m_pMenuTree->ScrollMenu( -1 );
		return TTRUE;
	}

	if ( a_iActions & 0x800 )
	{
		m_pMenuTree->ScrollMenu( 1 );
		return TTRUE;
	}

	return TFALSE;
}

// $JPOG: FUNCTION PGUICore.dll+0000f550
TBOOL PGUIMenuDialog::OnButtonRelease( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	return TGUIWidget::OnButtonRelease( a_pEvent, a_iActions );
}

// $JPOG: FUNCTION PGUICore.dll+0000e640
void PGUIMenuDialog::Pack()
{
	PGUIDialog::Pack();

	if ( m_pStyle )
	{
		SetComponentBBox( 0, 0, 0, m_iWidth, m_iHeight );
		SetComponentBBox( 1, 0, 0, m_iWidth, m_iHeight );
	}

	m_bValidated = TTRUE;
}

// The children are laid out first, the size that isn't fixed fits them, the background and the menu background
// $JPOG: FUNCTION PGUICore.dll+0000e690
void PGUIMenuDialog::Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight )
{
	m_bLayoutValid = TTRUE;
	LayoutChildren( a_iMaxWidth, a_iMaxHeight );

	if ( m_eSizeModeHoriz == SizeMode_Fixed && m_eSizeModeVert == SizeMode_Fixed )
		return;

	TINT iWidth  = m_iWidth;
	TINT iHeight = m_iHeight;

	if ( m_pStyle )
	{
		for ( TINT i = 0; i < 2; i++ )
		{
			TINT iPreferredWidth  = 0;
			TINT iPreferredHeight = 0;

			if ( m_pStyle->GetComponentPreferredSize( i, m_pParameters, iPreferredWidth, iPreferredHeight ) )
			{
				iWidth  = TMath::Max( iWidth, iPreferredWidth );
				iHeight = TMath::Max( iHeight, iPreferredHeight );
			}
		}
	}

	if ( m_eSizeModeHoriz != SizeMode_Fixed )
		SetWidth( iWidth );

	if ( m_eSizeModeVert != SizeMode_Fixed )
		SetHeight( iHeight );
}

// $JPOG: FUNCTION PGUICore.dll+0000ea50
void PGUIMenuDialog::UpdatePosition( TINT& a_rX, TINT& a_rY )
{
	TINT iScreenWidth  = GetScreen()->GetWidth();
	TINT iScreenHeight = GetScreen()->GetHeight();
	TINT iHalfWidth    = iScreenWidth / 2;
	TINT iX            = 0;
	TINT iY            = 0;

	if ( m_eDisplayMode == PGUIMenuTreeItemStyle::DisplayLevel_Next )
	{
		iX = iHalfWidth;
		iY = a_rY - m_iHeight / 2;
	}
	else if ( m_eDisplayMode == PGUIMenuTreeItemStyle::DisplayLevel_Current )
	{
		iX = a_rX;
		iY = a_rY;
	}

	// Within the margins of the screen
	TINT iRight = iScreenWidth - m_pMenuTree->GetScreenMarginRight();

	if ( m_iWidth + iX > iRight )
		iX = iRight - m_iWidth;

	if ( iX < m_pMenuTree->GetScreenMarginLeft() )
		iX = m_pMenuTree->GetScreenMarginLeft();

	TINT iBottom = iScreenHeight - m_pMenuTree->GetScreenMarginBottom();

	if ( iY + m_iHeight > iBottom )
		iY = iBottom - m_iHeight;

	if ( iY < m_pMenuTree->GetScreenMarginTop() )
		iY = m_pMenuTree->GetScreenMarginTop();

	if ( m_eDisplayMode == PGUIMenuTreeItemStyle::DisplayLevel_Next )
	{
		SetPosition( iX, iY );
	}
	else if ( m_eDisplayMode == PGUIMenuTreeItemStyle::DisplayLevel_Current )
	{
		SetPosition( iX, iY );

		if ( m_pSelectedButton )
		{
			// The selected button stays in the screen, the next menu goes in the middle of the screen at its height
			m_pSelectedButton->SetPosition( TMath::Max( m_pMenuTree->GetScreenMarginLeft() - m_iXPos, m_pSelectedButton->GetXPos() ), m_pSelectedButton->GetYPos() );

			TINT iButtonX, iButtonY;
			m_pSelectedButton->TransformWidgetToDevice( 0, 0, iButtonX, iButtonY );

			a_rX = iHalfWidth + 8;
			a_rY = iButtonY + m_pSelectedButton->GetHeight() / 2;
		}
	}
}

// $JPOG: FUNCTION PGUICore.dll+0000ec60
PGUIMenuTreeItem* PGUIMenuDialog::FindItemData( TGUIWidget* a_pWidget )
{
	auto itButton = m_pMenuBucket->GetChildren().Begin();

	for ( auto it = m_pRootItem->GetChildren().Begin(); it != m_pRootItem->GetChildren().End(); it++ )
	{
		if ( !( it->GetState() & PGUIItemData::State_Visible ) )
			continue;

		TGUIWidget* pButton = itButton.Get();

		if ( pButton )
			itButton++;

		if ( pButton == a_pWidget )
			return TSTATICCAST( PGUIMenuTreeItem, it.Get() );
	}

	return TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+0000ed70
void PGUIMenuDialog::SetSelectedButton( TGUIWidget* a_pButton )
{
	if ( a_pButton )
	{
		UpdateSelection();

		if ( m_pItemSelected )
		{
			m_pSelectedItem->UpdateWidget( m_pItemSelected );
			m_pItemSelected->SetState( State_Visible, State_Visible );
		}

		if ( m_pItemBack )
			m_pItemBack->SetState( State_Visible, 0u );
	}
	else
	{
		if ( m_pItemSelected )
			m_pItemSelected->SetState( State_Visible, 0u );

		if ( m_pItemBack )
		{
			m_pItemBack->SetState( State_Visible, State_Visible );
			m_pItemBack->SetState( State_Focused, ( m_uiState & State_Focused ) ? TUINT( State_Focused ) : 0u );
		}
	}

	Layout( -1, -1 );
	Pack();
}

// $JPOG: FUNCTION PGUICore.dll+0000ee40
void PGUIMenuDialog::UpdateSelection()
{
	if ( !m_pSelectedButton )
		return;

	m_pSelectedButton->SetState( State_Focused, ( m_uiState & State_Focused ) ? TUINT( State_Focused ) : 0u );

	TINT iOldHeight = m_pSelectedButton->GetHeight();
	m_pSelectedButton->SetStyle( m_pSelectedItem->GetStyle()->GetSelectedItemStyle( m_eDisplayMode ) );
	m_pSelectedButton->Layout( -1, -1 );
	m_pSelectedButton->SetFixedSize( TMath::Max( m_pSelectedButton->GetWidth(), TINT( m_iWidth ) ), m_pSelectedButton->GetHeight() );
	m_pSelectedButton->SetPosition( m_pSelectedButton->GetXPos(), ( iOldHeight - m_pSelectedButton->GetHeight() ) / 2 + m_pSelectedButton->GetYPos() );
	m_pSelectedButton->Pack();
}

// $JPOG: FUNCTION PGUICore.dll+0000ef40
void PGUIMenuDialog::UpdateMenu()
{
	if ( !m_pMenuBucket )
		m_pMenuBucket = FindNamedChild( str_menubucket );

	if ( !m_pMenuHeading )
		m_pMenuHeading = FindNamedChild( str_menuheading );

	if ( !m_pItemSelected )
		m_pItemSelected = FindNamedChild( str_itemselected );

	if ( !m_pItemBack )
		m_pItemBack = FindNamedChild( str_itemback );

	if ( m_pMenuHeading )
		m_pRootItem->UpdateWidget( m_pMenuHeading );

	while ( !m_pMenuBucket->GetChildren().IsEmpty() )
		DiscardButton( m_pMenuBucket->GetChildren().Head().Get() );

	for ( auto it = m_pRootItem->GetChildren().Begin(); it != m_pRootItem->GetChildren().End(); it++ )
	{
		PGUIMenuTreeItem* pItem = TSTATICCAST( PGUIMenuTreeItem, it.Get() );

		if ( !( pItem->GetState() & PGUIItemData::State_Visible ) )
			continue;

		TGUIWidget* pButton = RecycleButton( pItem->GetStyle()->GetItemStyle( m_eDisplayMode ) );
		m_pMenuBucket->AddWidget( pButton, TFALSE );
		pButton->SetSizeMode( SizeMode_Free, SizeMode_Free );
		pItem->UpdateWidget( pButton );
		pButton->SetState( State_Enabled, ( pItem->GetState() & PGUIItemData::State_Enabled ) && ( m_pRootItem->GetState() & PGUIItemData::State_Enabled ) );
		pButton->SetState( State_Visible, State_Visible );
		pButton->SetState( State_Focused, 0u );
	}

	Layout( -1, -1 );
	Pack();
}

// $JPOG: FUNCTION PGUICore.dll+0000e970
void PGUIMenuDialog::SetRootItem( PGUIMenuTreeItem* a_pItem )
{
	m_pRootItem     = a_pItem;
	m_pSelectedItem = TNULL;
	UpdateMenu();
	SetSelectedButton( TNULL );
}

// $JPOG: FUNCTION PGUICore.dll+0000e9b0
void PGUIMenuDialog::SetSelectedItem( PGUIMenuTreeItem* a_pItem )
{
	m_pRootItem     = a_pItem->GetParent();
	m_pSelectedItem = a_pItem;
	UpdateMenu();

	// The button of the item
	TGUIWidget* pSelectedButton = TNULL;
	auto        itButton        = m_pMenuBucket->GetChildren().Begin();

	for ( auto it = m_pRootItem->GetChildren().Begin(); it != m_pRootItem->GetChildren().End(); it++ )
	{
		if ( !( it->GetState() & PGUIItemData::State_Visible ) )
			continue;

		TGUIWidget* pButton = itButton.Get();

		if ( pButton )
			itButton++;

		if ( it.Get() == a_pItem )
			pSelectedButton = pButton;
	}

	m_pSelectedButton = pSelectedButton;
	SetSelectedButton( pSelectedButton );
}

// $JPOG: FUNCTION PGUICore.dll+0000ec10
PGUIMenuTreeItem* PGUIMenuDialog::GetItemAtIndex( TINT a_iIndex )
{
	for ( auto it = m_pRootItem->GetChildren().Begin(); it != m_pRootItem->GetChildren().End(); it++ )
	{
		if ( !( it->GetState() & PGUIItemData::State_Visible ) )
			continue;

		if ( a_iIndex-- == 0 )
			return TSTATICCAST( PGUIMenuTreeItem, it.Get() );
	}

	return TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+0000f260
void PGUIMenuDialog::DiscardButton( TGUIWidget* a_pButton )
{
	if ( a_pButton->GetParentWidget() )
		a_pButton->GetParentWidget()->RemoveWidget( a_pButton );

	m_oRecycledButtons.InsertTail( a_pButton );
}

// NOTE: the recycled buttons are matched by their widget class (the widget info of the style)
// $JPOG: FUNCTION PGUICore.dll+0000f2b0
TGUIWidget* PGUIMenuDialog::RecycleButton( TGUIStyle* a_pStyle )
{
	for ( auto it = m_oRecycledButtons.Begin(); it != m_oRecycledButtons.End(); it++ )
	{
		TGUIWidget* pButton = it.Get();

		if ( a_pStyle->GetWidgetInfo() != pButton->GetWidgetInfo() )
			continue;

		m_oRecycledButtons.Remove( pButton );

		if ( a_pStyle != pButton->GetStyle() )
		{
			pButton->SetStyle( TNULL );
			pButton->SetStyle( a_pStyle );
		}

		pButton->SetState( 0xFFFFFFFF, State_Enabled | State_Visible );
		return pButton;
	}

	TGUIWidget* pButton = a_pStyle->CreateWidget();
	pButton->SetPlaceMode( PlaceMode_Managed );
	pButton->SetState( 0xFFFFFFFF, State_Enabled | State_Visible );

	if ( pButton->IsA( &TGetClass( PGUIButton ) ) )
		m_oButtonListeners.Add( TSTATICCAST( PGUIButton, pButton ) );

	return pButton;
}

// $JPOG: FUNCTION PGUICore.dll+0000f3f0
TBOOL __stdcall PGUIMenuDialog::OnButtonAction( PGUIMenuDialog* a_pMenu, PGUIButton* a_pButton, TGUIActionEvent* a_pEvent )
{
	if ( a_pMenu->FindItemData( a_pButton ) )
		a_pMenu->m_pMenuTree->SelectMenu();

	return TTRUE;
}
