#include "ToshiPCH.h"
#include "PGUIMenuBar.h"
#include "PGUIMenuTree.h"
#include "PGUIMenuTreeItem.h"

#include <TGUI/TGUIManager.h>
#include <TGUI/TGUIWidgetInfo.h>
#include <TGUI/TGUIStyle.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_INIT( PGUIMenuBar );

TGUIDeserialisable::PropertySet* PGUIMenuBar::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* PGUIMenuBar::ms_oPropertySetOrdered = TNULL;
TGUIWidgetInfo*                  PGUIMenuBar::ms_pWidgetInfo         = TNULL;

// $JPOG: FUNCTION PGUICore.dll+0000d290
void PGUIMenuBar::InitialiseClass()
{
	ms_pWidgetInfo = new TGUIWidgetInfo();
	CreatePropertySets( ms_oPropertySet, ms_oPropertySetOrdered, *PGUIPanel::ms_oPropertySet );

	ms_pWidgetInfo->AddComponentName( TPString8( "background" ) );
	ms_pWidgetInfo->SetWidgetName( TPString8( "PGUIMenuBar" ) );
	ms_pWidgetInfo->SetWidgetLabel( TPString8( "Menu Bar" ) );
	ms_pWidgetInfo->SetProperties( ms_oPropertySet, ms_oPropertySetOrdered );
	ms_pWidgetInfo->AddStateVars( PGUIPanel::ms_pWidgetInfo->GetStateVarSet() );
	ms_pWidgetInfo->SetConcrete( TTRUE );

	TGUIManager::AddWidgetClass( ms_pWidgetInfo );
}

// $JPOG: FUNCTION PGUICore.dll+0000d610
void PGUIMenuBar::DeinitialiseClass()
{
	ms_pWidgetInfo->Clear();
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );

	delete ms_pWidgetInfo;
	ms_pWidgetInfo = TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+0000d6d0
PGUIMenuBar::PGUIMenuBar()
    : m_pMenuTree( TNULL )
    , m_pRootItem( TNULL )
    , m_pSelectedItem( TNULL )
    , m_pSelectedButton( TNULL )
    , m_oButtonListeners( this, OnButtonAction )
{
}

// $JPOG: FUNCTION PGUICore.dll+0000d9a0
PGUIMenuBar::PGUIMenuBar( const PGUIMenuBar& a_rcOther )
    : PGUIPanel( a_rcOther )
    , m_pMenuTree( a_rcOther.m_pMenuTree )
    , m_pRootItem( a_rcOther.m_pRootItem )
    , m_pSelectedItem( TNULL )
    , m_pSelectedButton( TNULL )
    , m_oButtonListeners( this, OnButtonAction )
{
}

// $JPOG: FUNCTION PGUICore.dll+0000de30
PGUIMenuBar::~PGUIMenuBar()
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

// $JPOG: FUNCTION PGUICore.dll+0000ddd0
TBOOL PGUIMenuBar::Deserialise( const PProperties* a_pProperties )
{
	return PGUIPanel::Deserialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+0000dda0
TBOOL PGUIMenuBar::Serialise( PProperties* a_pProperties )
{
	return PGUIPanel::Serialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+0000df20
TGUIDeserialisable::PropertySet* PGUIMenuBar::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $JPOG: FUNCTION PGUICore.dll+0000df10
TGUIDeserialisable::PropertySet* PGUIMenuBar::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

const TGUIWidgetInfo* PGUIMenuBar::GetWidgetInfo() const
{
	return ms_pWidgetInfo;
}

// $JPOG: FUNCTION PGUICore.dll+0000de00
PGUIMenuBar* PGUIMenuBar::Clone() const
{
	return new PGUIMenuBar( *this );
}

// $JPOG: FUNCTION PGUICore.dll+0000d940
PGUIMenuTreeItem* PGUIMenuBar::FindItemData( TGUIWidget* a_pWidget )
{
	auto itButton = m_oChildren.Begin();

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

// $JPOG: FUNCTION PGUICore.dll+0000dd70
void PGUIMenuBar::SetRootItem( PGUIMenuTreeItem* a_pItem )
{
	m_pRootItem     = a_pItem;
	m_pSelectedItem = TNULL;
	UpdateMenu();
	SetSelectedButton( TNULL );
}

// $JPOG: FUNCTION PGUICore.dll+0000d750
void PGUIMenuBar::SetSelectedItem( PGUIMenuTreeItem* a_pItem )
{
	m_pRootItem     = a_pItem->GetParent();
	m_pSelectedItem = a_pItem;
	UpdateMenu();

	// The button of the item
	TGUIWidget* pSelectedButton = TNULL;
	auto        itButton        = m_oChildren.Begin();

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

// $JPOG: FUNCTION PGUICore.dll+0000d7d0
void PGUIMenuBar::UpdateMenu()
{
	while ( !m_oChildren.IsEmpty() )
		DiscardButton( m_oChildren.Head().Get() );

	for ( auto it = m_pRootItem->GetChildren().Begin(); it != m_pRootItem->GetChildren().End(); it++ )
	{
		PGUIMenuTreeItem* pItem = TSTATICCAST( PGUIMenuTreeItem, it.Get() );

		if ( !( pItem->GetState() & PGUIItemData::State_Visible ) )
			continue;

		TGUIWidget* pButton = RecycleButton( pItem->GetStyle()->GetItemStyle( PGUIMenuTreeItemStyle::DisplayLevel_Current ) );
		AddWidget( pButton, TFALSE );
		pButton->SetSizeMode( SizeMode_Free, SizeMode_Free );
		pItem->UpdateWidget( pButton );
		pButton->SetState( State_Enabled, ( pItem->GetState() & PGUIItemData::State_Enabled ) && ( m_pRootItem->GetState() & PGUIItemData::State_Enabled ) );
		pButton->SetState( State_Visible, State_Visible );
		pButton->SetState( State_Focused, 0u );
	}

	Layout( -1, -1 );
	Pack();
}

// $JPOG: FUNCTION PGUICore.dll+0000dd50
void PGUIMenuBar::SetSelectedButton( TGUIWidget* a_pButton )
{
	if ( a_pButton )
		UpdateSelection();
}

// $JPOG: FUNCTION PGUICore.dll+0000da30
void PGUIMenuBar::UpdateSelection()
{
	if ( !m_pSelectedButton )
		return;

	m_pSelectedButton->SetState( State_Focused, ( m_uiState & State_Focused ) ? TUINT( State_Focused ) : 0u );
	m_pSelectedButton->Layout( -1, -1 );
	m_pSelectedButton->Pack();
}

// $JPOG: FUNCTION PGUICore.dll+0000da90
void PGUIMenuBar::DiscardButton( TGUIWidget* a_pButton )
{
	if ( a_pButton->GetParentWidget() )
		a_pButton->GetParentWidget()->RemoveWidget( a_pButton );

	m_oRecycledButtons.InsertTail( a_pButton );
}

// NOTE: the recycled buttons are matched by their widget class (the widget info of the style)
// $JPOG: FUNCTION PGUICore.dll+0000dae0
TGUIWidget* PGUIMenuBar::RecycleButton( TGUIStyle* a_pStyle )
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

// Selects the item of the button in the menu tree
// $JPOG: FUNCTION PGUICore.dll+0000dc20
TBOOL __stdcall PGUIMenuBar::OnButtonAction( PGUIMenuBar* a_pMenuBar, PGUIButton* a_pButton, TGUIActionEvent* a_pEvent )
{
	if ( PGUIMenuTreeItem* pItem = a_pMenuBar->FindItemData( a_pButton ) )
	{
		a_pMenuBar->m_pMenuTree->SetSelectedItem( pItem );
		a_pMenuBar->m_pMenuTree->SelectMenu();
	}

	return TTRUE;
}
