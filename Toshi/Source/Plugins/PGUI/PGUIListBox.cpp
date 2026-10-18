#include "ToshiPCH.h"
#include "PGUIListBox.h"
#include "PGUIListItem.h"
#include "PGUISliderBar.h"
#include "PGUICore.h"

#include <TGUI/TGUIManager.h>
#include <TGUI/TGUIWidgetInfo.h>
#include <TGUI/TGUIStyle.h>
#include <TGUI/TGUIScreen.h>
#include <TGUI/TGUIInputContext.h>
#include <TGUI/TGUIComponentCache.h>
#include <TGUI/TGUIPropertySpecSimple.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_INIT( PGUIListBox );

TGUIDeserialisable::PropertySet* PGUIListBox::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* PGUIListBox::ms_oPropertySetOrdered = TNULL;
TGUIWidgetInfo*                  PGUIListBox::ms_pWidgetInfo         = TNULL;

const TCHAR* const PGUIListBox::ms_csaArrangeModeNameStr[ ArrangeMode_NUMOF ]     = { "horizontal", "vertical", "grid" };
const TCHAR* const PGUIListBox::ms_csaSelectionModeNameStr[ SelectionMode_NUMOF ] = { "None", "Enabled", "Disabled", "Any" };

// The widget as a list item (TNULL if it isn't one)
static PGUIListItem* AsListItem( TGUIWidget* a_pWidget )
{
	return ( a_pWidget && !a_pWidget->IsA( &TGetClass( PGUIListItem ) ) ) ? TNULL : TSTATICCAST( PGUIListItem, a_pWidget );
}

static TBOOL IsInRect( const TGUIRectangle& a_rcRect, TINT a_iX, TINT a_iY )
{
	return a_iX >= a_rcRect.m_iX && a_iY >= a_rcRect.m_iY && a_iX < a_rcRect.m_iX + a_rcRect.m_iWidth && a_iY < a_rcRect.m_iY + a_rcRect.m_iHeight;
}

// NOTE: the category of the property specs (an argument the JPOG constructors don't have) is the name of the class
// $JPOG: FUNCTION PGUICore.dll+00008720
void PGUIListBox::InitialiseClass()
{
	using t_Setter = TGUIPropertySpecSimple::t_Setter;
	using t_Getter = TGUIPropertySpecSimple::t_Getter;

	ms_pWidgetInfo = new TGUIWidgetInfo();
	CreatePropertySets( ms_oPropertySet, ms_oPropertySetOrdered, *TGUIWidget::ms_oPropertySet );

	TPString8 strCategory( "PGUIListBox" );
	TINT      iSortHint = 0;
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "orientation" ), static_cast<t_Setter>( &PGUIListBox::Set_orientation_property ), static_cast<t_Getter>( &PGUIListBox::Get_orientation_property ), PPropertyValue::TYPE_INT, strCategory, TTRUE, &TGetClass( PGUIListBox ), iSortHint++, ArrangeMode_NUMOF, ms_csaArrangeModeNameStr ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "scrollbarpos" ), static_cast<t_Setter>( &PGUIListBox::Set_scrollbarpos_property ), static_cast<t_Getter>( &PGUIListBox::Get_scrollbarpos_property ), PPropertyValue::TYPE_INT, strCategory, TTRUE, &TGetClass( PGUIListBox ), iSortHint++, Placement_NUMOF, ms_csaPlacementNameStr ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "selectionmode" ), static_cast<t_Setter>( &PGUIListBox::Set_selectionmode_property ), static_cast<t_Getter>( &PGUIListBox::Get_selectionmode_property ), PPropertyValue::TYPE_INT, strCategory, TTRUE, &TGetClass( PGUIListBox ), iSortHint++, SelectionMode_NUMOF, ms_csaSelectionModeNameStr ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "scrollbarstyle" ), static_cast<t_Setter>( &PGUIListBox::Set_scrollbarstyle_property ), static_cast<t_Getter>( &PGUIListBox::Get_scrollbarstyle_property ), PPropertyValue::TYPE_TPSTRING8, strCategory, TTRUE, &TGetClass( PGUIListBox ), iSortHint++, &TGetClass( PGUISliderBar ) ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "listitemstyle" ), static_cast<t_Setter>( &PGUIListBox::Set_listitemstyle_property ), static_cast<t_Getter>( &PGUIListBox::Get_listitemstyle_property ), PPropertyValue::TYPE_TPSTRING8, strCategory, TTRUE, &TGetClass( PGUIListBox ), iSortHint++, &TGetClass( PGUIListItem ) ) );

	ms_pWidgetInfo->AddComponentName( TPString8( "background" ) );
	ms_pWidgetInfo->AddComponentName( TPString8( "bucket" ) );
	ms_pWidgetInfo->AddComponentName( TPString8( "scrollback" ) );
	ms_pWidgetInfo->AddComponentName( TPString8( "scrollforward" ) );
	ms_pWidgetInfo->AddComponentName( TPString8( "effect" ) );

	TPString8 strStateCategory( "State" );
	DeclareStateVar( &TGetClass( PGUIListBox ), "scrollableback", State_ScrollableBack, strStateCategory, iSortHint, ms_pWidgetInfo, ms_oPropertySet, ms_oPropertySetOrdered );
	DeclareStateVar( &TGetClass( PGUIListBox ), "scrollableforward", State_ScrollableForward, strStateCategory, iSortHint, ms_pWidgetInfo, ms_oPropertySet, ms_oPropertySetOrdered );

	ms_pWidgetInfo->SetWidgetName( TPString8( "PGUIListBox" ) );
	ms_pWidgetInfo->SetWidgetLabel( TPString8( "List Box" ) );
	ms_pWidgetInfo->SetProperties( ms_oPropertySet, ms_oPropertySetOrdered );
	ms_pWidgetInfo->AddStateVars( TGUIWidget::ms_pWidgetInfo->GetStateVarSet() );
	ms_pWidgetInfo->SetConcrete( TTRUE );

	TGUIManager::AddWidgetClass( ms_pWidgetInfo );
}

// $JPOG: FUNCTION PGUICore.dll+00009b20
void PGUIListBox::DeinitialiseClass()
{
	ms_pWidgetInfo->Clear();
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );

	delete ms_pWidgetInfo;
	ms_pWidgetInfo = TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+00009be0
PGUIListBox::PGUIListBox()
    : m_oItemChangeEmitter( this )
    , m_oActionEmitter( this )
    , m_eArrangeMode( ArrangeMode_Vertical )
    , m_eScrollBarPosition( Placement_Before )
    , m_pScrollBarStyle( TNULL )
    , m_pListItemStyle( TNULL )
    , m_pBlankItem( TNULL )
    , m_pFirstVisibleItem( TNULL )
    , m_pSelectedItem( TNULL )
    , m_pSelectedItemWidget( TNULL )
    , m_iNumItemWidgets( 0 )
    , m_iNumVisibleItems( 0 )
    , m_iNumRows( 0 )
    , m_iNumColumns( 0 )
    , m_iFirstItemWidget( 0 )
    , m_bHasScrollBar( TFALSE )
    , m_bHoverSelectMode( TFALSE )
    , m_eSelectionMode( SelectionMode_Enabled )
    , m_iItemWidth( 0 )
    , m_iItemHeight( 0 )
{
}

// The items are cloned, the first one is selected
// $JPOG: FUNCTION PGUICore.dll+0000acf0
PGUIListBox::PGUIListBox( const PGUIListBox& a_rcOther )
    : TGUIWidget( a_rcOther )
    , m_oItemChangeEmitter( this )
    , m_oActionEmitter( this )
    , m_eArrangeMode( a_rcOther.m_eArrangeMode )
    , m_eScrollBarPosition( a_rcOther.m_eScrollBarPosition )
    , m_pScrollBarStyle( TNULL )
    , m_pListItemStyle( a_rcOther.m_pListItemStyle )
    , m_pBlankItem( a_rcOther.m_pBlankItem ? a_rcOther.m_pBlankItem->Clone() : TNULL )
    , m_pFirstVisibleItem( TNULL )
    , m_pSelectedItem( TNULL )
    , m_pSelectedItemWidget( TNULL )
    , m_iNumItemWidgets( 0 )
    , m_iNumVisibleItems( 0 )
    , m_iNumRows( 0 )
    , m_iNumColumns( 0 )
    , m_iFirstItemWidget( 0 )
    , m_bHasScrollBar( TFALSE )
    , m_bHoverSelectMode( a_rcOther.m_bHoverSelectMode )
    , m_eSelectionMode( a_rcOther.m_eSelectionMode )
    , m_iItemWidth( 0 )
    , m_iItemHeight( 0 )
{
	SetScrollBarStyle( a_rcOther.m_pScrollBarStyle );

	for ( auto it = a_rcOther.m_oItems.Begin(); it != a_rcOther.m_oItems.End(); it++ )
		m_oItems.InsertTail( it->Clone() );

	if ( m_oItems.IsEmpty() )
	{
		m_pFirstVisibleItem = TNULL;
		m_pSelectedItem     = TNULL;
	}
	else
	{
		m_pFirstVisibleItem = m_oItems.Head().Get();
		m_pSelectedItem     = m_oItems.Head().Get();
	}
}

// NOTE: the items are removed without being deleted (as in the original)
// $JPOG: FUNCTION PGUICore.dll+00009cb0
PGUIListBox::~PGUIListBox()
{
	m_oItems.RemoveAll();
}

// The first item gets selected
// $JPOG: FUNCTION PGUICore.dll+00009de0
TBOOL PGUIListBox::Deserialise( const PProperties* a_pProperties )
{
	if ( !TGUIWidget::Deserialise( a_pProperties ) )
		return TFALSE;

	if ( !m_oItems.IsEmpty() )
	{
		SelectItemImpl( m_oItems.Head().Get() );
		RefreshItemList();
	}

	return TTRUE;
}

// $JPOG: FUNCTION PGUICore.dll+0000c560
TBOOL PGUIListBox::Serialise( PProperties* a_pProperties )
{
	return TGUIWidget::Serialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+0000c5c0
TGUIDeserialisable::PropertySet* PGUIListBox::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $JPOG: FUNCTION PGUICore.dll+0000c5b0
TGUIDeserialisable::PropertySet* PGUIListBox::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

const TGUIWidgetInfo* PGUIListBox::GetWidgetInfo() const
{
	return ms_pWidgetInfo;
}

// $JPOG: FUNCTION PGUICore.dll+0000c580
PGUIListBox* PGUIListBox::Clone() const
{
	return new PGUIListBox( *this );
}

// $JPOG: FUNCTION PGUICore.dll+0000c810
TBOOL PGUIListBox::ManagesChildren() const
{
	return TTRUE;
}

// $JPOG: FUNCTION PGUICore.dll+0000c800
TBOOL PGUIListBox::AcceptsFocus() const
{
	return TTRUE;
}

// The list box keeps the focus of its items
// $JPOG: FUNCTION PGUICore.dll+0000c550
TGUIWidget* PGUIListBox::AllowsFocus( TGUIWidget* a_pWidget )
{
	return this;
}

// $JPOG: FUNCTION PGUICore.dll+0000c370
void PGUIListBox::NotifyFocusLost( TGUIWidget* a_pNewFocus, TGUIWidget* a_pOldFocus )
{
	if ( m_pSelectedItemWidget )
		m_pSelectedItemWidget->SetState( State_Focused, 0u );

	TGUIWidget::NotifyFocusLost( a_pNewFocus, a_pOldFocus );
}

// Getting the focus selects the first visible item (if there's no selection), a list item getting it selects its item
// $JPOG: FUNCTION PGUICore.dll+0000b1a0
void PGUIListBox::NotifyFocusGained( TGUIWidget* a_pNewFocus, TGUIWidget* a_pOldFocus )
{
	if ( a_pNewFocus == this )
	{
		if ( !m_pSelectedItem && m_pFirstVisibleItem )
		{
			SelectItemImpl( m_pFirstVisibleItem );
			RefreshItemList();
		}

		if ( m_pSelectedItemWidget )
			m_pSelectedItemWidget->SetState( State_Focused, State_Focused );
	}
	else if ( this == a_pNewFocus->GetParentWidget() && a_pNewFocus->IsA( &TGetClass( PGUIListItem ) ) )
	{
		PGUIItemData* pItem = FindWidgetItem( a_pNewFocus );

		if ( pItem != m_pSelectedItem )
		{
			SelectItemImpl( pItem );
			RefreshItemList();
		}
	}

	TGUIWidget::NotifyFocusGained( a_pNewFocus, a_pOldFocus );
}

// Select hits the selected item; a click scrolls (the scroll components) or selects and hits the item under it
// $JPOG: FUNCTION PGUICore.dll+0000b020
TBOOL PGUIListBox::OnButtonPress( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	if ( TGUIWidget::OnButtonPress( a_pEvent, a_iActions ) || DoButtonEvent( a_pEvent, a_iActions ) )
		return TTRUE;

	if ( a_iActions & 1 )
	{
		HitSelectedItem();
		return TTRUE;
	}

	if ( !( a_iActions & 0x4000 ) )
		return TFALSE;

	TGUIScreen* pScreen = GetScreen();
	TINT        iX, iY;
	TransformDeviceToWidget( pScreen->GetPointerPositionX(), pScreen->GetPointerPositionY(), iX, iY );

	if ( IsInRect( m_pComponentCaches[ Component_ScrollBack ].GetBBox(), iX, iY ) )
	{
		Navigate( a_pEvent, -1 );
	}
	else if ( IsInRect( m_pComponentCaches[ Component_ScrollForward ].GetBBox(), iX, iY ) )
	{
		Navigate( a_pEvent, 1 );
	}
	else
	{
		SelectItemAtPoint( iX, iY );
		HitSelectedItem();
	}

	return TTRUE;
}

// $JPOG: FUNCTION PGUICore.dll+0000b150
TBOOL PGUIListBox::OnButtonRepeat( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	return TGUIWidget::OnButtonRepeat( a_pEvent, a_iActions ) || DoButtonEvent( a_pEvent, a_iActions );
}

// $JPOG: FUNCTION PGUICore.dll+0000c3b0
TBOOL PGUIListBox::OnPointerMove( TINT a_iX, TINT a_iY )
{
	if ( m_bHoverSelectMode )
		SelectItemAtPoint( a_iX, a_iY );

	return TFALSE;
}

// The scroll components at the ends of the bucket (across the orientation for the horizontal lists), then the items
// $JPOG: FUNCTION PGUICore.dll+00009e30
void PGUIListBox::Pack()
{
	TINT iBackWidth     = 0;
	TINT iBackHeight    = 0;
	TINT iForwardWidth  = 0;
	TINT iForwardHeight = 0;

	if ( m_pStyle )
	{
		m_pStyle->GetComponentPreferredSize( Component_ScrollBack, m_pParameters, iBackWidth, iBackHeight );
		m_pStyle->GetComponentPreferredSize( Component_ScrollForward, m_pParameters, iForwardWidth, iForwardHeight );
	}

	TINT iBucketX, iBucketY, iBucketWidth, iBucketHeight;

	if ( m_eArrangeMode != ArrangeMode_Horizontal )
	{
		SetComponentBBox( Component_ScrollBack, 0, 0, m_iWidth, iBackHeight );
		SetComponentBBox( Component_ScrollForward, 0, m_iHeight - iForwardHeight, m_iWidth, iForwardHeight );

		iBucketX      = 0;
		iBucketY      = iBackHeight;
		iBucketWidth  = m_iWidth;
		iBucketHeight = m_iHeight - ( iForwardHeight + iBackHeight );
	}
	else
	{
		SetComponentBBox( Component_ScrollBack, 0, 0, iBackWidth, m_iHeight );
		SetComponentBBox( Component_ScrollForward, m_iWidth - iForwardWidth, 0, iForwardWidth, m_iHeight );

		iBucketX      = iBackWidth;
		iBucketY      = 0;
		iBucketHeight = m_iHeight;
		iBucketWidth  = m_iWidth - ( iForwardWidth + iBackWidth );
	}

	SetComponentBBox( Component_Bucket, iBucketX, iBucketY, iBucketWidth, iBucketHeight );
	SetComponentBBox( Component_Background, 0, 0, m_iWidth, m_iHeight );
	SetComponentBBox( Component_Effect, 0, 0, m_iWidth, m_iHeight );

	GenerateItemList();
	RefreshItemList();

	m_bValidated = TTRUE;
}

// The scroll components and the bucket along the orientation, the background
// $JPOG: FUNCTION PGUICore.dll+00009f80
void PGUIListBox::Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight )
{
	m_bLayoutValid = TTRUE;

	if ( m_eSizeModeHoriz == SizeMode_Fixed && m_eSizeModeVert == SizeMode_Fixed )
		return;

	TINT iWidth  = 0;
	TINT iHeight = 0;

	if ( m_pStyle )
	{
		static const Component s_aComponents[] = { Component_ScrollBack, Component_ScrollForward, Component_Bucket };

		for ( Component eComponent : s_aComponents )
		{
			TINT iComponentWidth  = 0;
			TINT iComponentHeight = 0;

			if ( !m_pStyle->GetComponentPreferredSize( eComponent, m_pParameters, iComponentWidth, iComponentHeight ) )
				continue;

			if ( m_eArrangeMode != ArrangeMode_Horizontal )
			{
				iWidth = TMath::Max( iWidth, iComponentWidth );
				iHeight += iComponentHeight;
			}
			else
			{
				iWidth += iComponentWidth;
				iHeight = TMath::Max( iHeight, iComponentHeight );
			}
		}

		TINT iComponentWidth  = 0;
		TINT iComponentHeight = 0;

		if ( m_pStyle->GetComponentPreferredSize( Component_Background, m_pParameters, iComponentWidth, iComponentHeight ) )
		{
			iWidth  = TMath::Max( iWidth, iComponentWidth );
			iHeight = TMath::Max( iHeight, iComponentHeight );
		}
	}

	if ( m_eSizeModeHoriz != SizeMode_Fixed )
		SetWidth( iWidth );

	if ( m_eSizeModeVert != SizeMode_Fixed )
		SetHeight( iHeight );
}

// The children are the list item widgets, they're not deserialised
// $JPOG: FUNCTION PGUICore.dll+0000c3d0
TBOOL PGUIListBox::DeserialiseChildren( const PProperties* a_pProperties )
{
	return TTRUE;
}

// $JPOG: FUNCTION PGUICore.dll+0000c3e0
TBOOL PGUIListBox::SerialiseChildren( PProperties* a_pProperties )
{
	return TTRUE;
}

// $JPOG: FUNCTION PGUICore.dll+0000c6e0
void PGUIListBox::AddItem( PGUIItemData* a_pItem )
{
	AddItemImpl( a_pItem );
}

// $JPOG: FUNCTION PGUICore.dll+0000c6c0
void PGUIListBox::RemoveItem( PGUIItemData* a_pItem )
{
	RemoveItemImpl( a_pItem );
	RefreshItemList();
}

// $JPOG: FUNCTION PGUICore.dll+0000c6b0
void PGUIListBox::RemoveAllItems()
{
	RemoveAllItemsImpl();
}

// $JPOG: FUNCTION PGUICore.dll+0000c690
void PGUIListBox::SelectItem( PGUIItemData* a_pItem )
{
	SelectItemImpl( a_pItem );
	RefreshItemList();
}

// $JPOG: FUNCTION PGUICore.dll+0000c670
void PGUIListBox::EnsureVisible( PGUIItemData* a_pItem )
{
	EnsureVisibleImpl( a_pItem );
	RefreshItemList();
}

// $JPOG: FUNCTION PGUICore.dll+0000c410
TBOOL PGUIListBox::IsItemVisible( PGUIItemData* a_pItem )
{
	return FindItemWidget( a_pItem ) != TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+0000a660
void PGUIListBox::SizeToFit( TINT a_iNumItems )
{
	RefreshItemList();

	TINT iBucketX, iBucketY, iBucketWidth, iBucketHeight;
	GetComponentInteriorBBox( Component_Bucket, iBucketX, iBucketY, iBucketWidth, iBucketHeight );

	TINT iWidth, iHeight;

	if ( m_pBlankItem )
	{
		TINT iNumRows    = ( m_eArrangeMode == ArrangeMode_Vertical ) ? a_iNumItems : 1;
		TINT iNumColumns = ( m_eArrangeMode == ArrangeMode_Horizontal ) ? a_iNumItems : 1;

		iWidth  = m_iItemWidth * iNumColumns;
		iHeight = m_iItemHeight * iNumRows;
	}
	else
	{
		// The largest of the first items
		TINT          iMaxWidth  = 0;
		TINT          iMaxHeight = 0;
		TINT          iNumItems  = 0;
		PGUIListItem* pWidget    = GetFirstItemWidget();
		pWidget->SetSizeMode( SizeMode_Free, SizeMode_Free );

		for ( auto it = m_oItems.Begin(); it != m_oItems.End() && iNumItems < a_iNumItems; it++, iNumItems++ )
		{
			it->UpdateWidget( pWidget );
			pWidget->Layout( -1, -1 );
			pWidget->Pack();

			iMaxWidth  = TMath::Max( TINT( pWidget->GetWidth() ), iMaxWidth );
			iMaxHeight = TMath::Max( TINT( pWidget->GetHeight() ), iMaxHeight );
		}

		pWidget->SetFixedSize( iMaxWidth, iMaxHeight );

		if ( m_eArrangeMode == ArrangeMode_Vertical )
		{
			iWidth  = iMaxWidth;
			iHeight = iMaxHeight * iNumItems;
		}
		else
		{
			iWidth  = iMaxWidth * iNumItems;
			iHeight = iMaxHeight;
		}
	}

	SetFixedSize( m_iWidth + iWidth - iBucketWidth, m_iHeight + iHeight - iBucketHeight );
}

// $JPOG: FUNCTION PGUICore.dll+0000a870
void PGUIListBox::SizeToFit( TINT a_iNumRows, TINT a_iNumColumns )
{
	RefreshItemList();

	TINT iBucketX, iBucketY, iBucketWidth, iBucketHeight;
	GetComponentInteriorBBox( Component_Bucket, iBucketX, iBucketY, iBucketWidth, iBucketHeight );

	SetFixedSize( m_iWidth + m_iItemWidth * a_iNumColumns - iBucketWidth, m_iHeight + m_iItemHeight * a_iNumRows - iBucketHeight );
}

// $JPOG: FUNCTION PGUICore.dll+0000a470
void PGUIListBox::SetScrollBarStyle( const TPString8& a_rcName )
{
	TGUIStyle* pStyle = TGUIManager::GetSingleton()->FindStyleInstance( a_rcName );

#ifdef BARNYARD_COMMUNITY_PATCH
	// Fixes an unknown style name dereferencing TNULL
	if ( !pStyle )
		return;
#endif

	if ( pStyle->GetWidgetInfo()->GetTClass()->IsA( &TGetClass( PGUISliderBar ) ) )
		SetScrollBarStyle( pStyle );
}

// NOTE: the scroll bar isn't added to the list box (as in the original)
// $JPOG: FUNCTION PGUICore.dll+0000a4c0
void PGUIListBox::SetScrollBarStyle( TGUIStyle* a_pStyle )
{
	if ( a_pStyle == m_pScrollBarStyle )
		return;

	if ( PGUISliderBar* pScrollBar = GetScrollBar() )
	{
		RemoveWidget( pScrollBar );
		delete pScrollBar;
	}

	m_pScrollBarStyle = a_pStyle;

	if ( a_pStyle )
	{
		TGUIWidget* pScrollBar = TSTATICCAST( TGUIWidget, a_pStyle->GetWidgetInfo()->GetTClass()->CreateObject() );
		pScrollBar->Create( a_pStyle, TPString8() );
		pScrollBar->SetSizeMode( SizeMode_Free, SizeMode_Free );
		pScrollBar->SetState( State_Enabled, State_Enabled );
		pScrollBar->SetState( State_Visible, State_Visible );

		m_bHasScrollBar    = TTRUE;
		m_iFirstItemWidget = 1;
	}
	else
	{
		m_bHasScrollBar    = TFALSE;
		m_iFirstItemWidget = 0;
	}
}

// $JPOG: FUNCTION PGUICore.dll+0000a5a0
void PGUIListBox::SetListItemStyle( const TPString8& a_rcName )
{
	TGUIStyle* pStyle = TGUIManager::GetSingleton()->FindStyleInstance( a_rcName );

	if ( !pStyle || pStyle->GetWidgetInfo()->GetTClass()->IsA( &TGetClass( PGUIListItem ) ) )
		SetListItemStyle( pStyle );
}

// $JPOG: FUNCTION PGUICore.dll+0000c450
void PGUIListBox::SetListItemStyle( TGUIStyle* a_pStyle )
{
	m_pListItemStyle = a_pStyle;
	DeleteItemList();
}

// $JPOG: FUNCTION PGUICore.dll+0000a5f0
void PGUIListBox::SetOrientation( TINT a_iOrientation )
{
	if ( TUINT( a_iOrientation ) <= ArrangeMode_Grid )
		m_eArrangeMode = ArrangeMode( a_iOrientation );

	if ( PGUISliderBar* pScrollBar = GetScrollBar() )
		pScrollBar->SetOrientation( a_iOrientation );
}

// $JPOG: FUNCTION PGUICore.dll+0000c430
void PGUIListBox::SetScrollBarPosition( TINT a_iPosition )
{
	if ( TUINT( a_iPosition ) <= Placement_After )
		m_eScrollBarPosition = Placement( a_iPosition );
}

// $JPOG: FUNCTION PGUICore.dll+0000c3f0
PGUISliderBar* PGUIListBox::GetScrollBar()
{
	return m_bHasScrollBar ? TSTATICCAST( PGUISliderBar, m_oChildren.Head().Get() ) : TNULL;
}

// The child after the scroll bar, TNULL if there's none
// $JPOG: FUNCTION PGUICore.dll+0000a8e0
PGUIListItem* PGUIListBox::GetFirstItemWidget()
{
	TGUIWidget* pWidget = m_oChildren.Head().Get();

	if ( m_iFirstItemWidget <= 0 )
		return TSTATICCAST( PGUIListItem, pWidget );

	for ( TINT i = 0; pWidget != m_oChildren.End().Get(); )
	{
		i++;
		pWidget = pWidget->Next();

		if ( i >= m_iFirstItemWidget )
			return TSTATICCAST( PGUIListItem, pWidget );
	}

	return TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+0000a620
void PGUIListBox::SelectItemAtPoint( TINT a_iX, TINT a_iY )
{
	TGUIWidget*   pWidget = GetChildAtPoint( a_iX, a_iY, State_Visible, State_Visible );
	PGUIItemData* pItem   = pWidget ? FindWidgetItem( pWidget ) : TNULL;

	SelectItemImpl( pItem );
	RefreshItemList();
}

// $JPOG: FUNCTION PGUICore.dll+0000ba50
void PGUIListBox::GenerateItemList()
{
	if ( !m_pStyle || !m_pListItemStyle )
		return;

	if ( !m_oItems.IsEmpty() && !m_pSelectedItem )
		SelectItemImpl( m_oItems.Head().Get() );

	TINT iBucketX, iBucketY, iBucketWidth, iBucketHeight;
	GetComponentInteriorBBox( Component_Bucket, iBucketX, iBucketY, iBucketWidth, iBucketHeight );

	// The first list item gives the size of the items
	TGUIWidget* pFirstWidget;

	if ( m_iNumItemWidgets != 0 )
	{
		pFirstWidget = GetFirstItemWidget();
	}
	else
	{
		pFirstWidget = MakeNewItem();
		AddWidget( pFirstWidget, TFALSE );

		if ( m_pBlankItem )
			m_pBlankItem->UpdateWidget( pFirstWidget );

		m_iNumItemWidgets = 1;
	}

	pFirstWidget->Layout( -1, -1 );
	pFirstWidget->Pack();
	pFirstWidget->SetPosition( iBucketX, iBucketY );

	if ( m_eArrangeMode == ArrangeMode_Horizontal )
		pFirstWidget->SetHeight( iBucketHeight );
	else if ( m_eArrangeMode == ArrangeMode_Vertical )
		pFirstWidget->SetWidth( iBucketWidth );

	if ( pFirstWidget->GetSizeModeHoriz() == SizeMode_Free )
		pFirstWidget->SetWidth( iBucketWidth );

	if ( pFirstWidget->GetSizeModeVert() == SizeMode_Free )
		pFirstWidget->SetHeight( iBucketHeight );

	m_iItemWidth  = pFirstWidget->GetWidth();
	m_iItemHeight = pFirstWidget->GetHeight();

	if ( m_iItemWidth <= 0 || m_iItemHeight <= 0 )
		return;

	// The number of items that fit
	TINT iNumRows    = 1;
	TINT iNumColumns = 1;

	if ( m_eArrangeMode == ArrangeMode_Horizontal )
	{
		iNumColumns = iBucketWidth / m_iItemWidth;
	}
	else if ( m_eArrangeMode == ArrangeMode_Vertical )
	{
		iNumRows = iBucketHeight / m_iItemHeight;
	}
	else if ( m_eArrangeMode == ArrangeMode_Grid )
	{
		iNumColumns = iBucketWidth / m_iItemWidth;
		iNumRows    = iBucketHeight / m_iItemHeight;
	}

	TINT iNumItems = iNumColumns * iNumRows;

	if ( iNumItems != 0 )
	{
		// NOTE: the number of children counts the scroll bar (as in the original)
		for ( TINT i = TINT( m_oChildren.Count() ); i < iNumItems; i++ )
			AddWidget( MakeNewItem(), TFALSE );
	}
	else
	{
		pFirstWidget->SetState( State_Visible, 0u );
	}

	TGUIWidget* pWidget = GetFirstItemWidget();
	TINT        iY      = iBucketY;

	for ( TINT iRow = 0; iRow < iNumRows; iRow++ )
	{
		TINT iX = iBucketX;

		for ( TINT iColumn = 0; iColumn < iNumColumns; iColumn++ )
		{
			pWidget->SetPosition( iX, iY );
			pWidget->SetSize( m_iItemWidth, m_iItemHeight );
			pWidget->SetState( State_Visible, State_Visible );
			iX += m_iItemWidth;
			pWidget = pWidget->Next();
		}

		iY += m_iItemHeight;
	}

	// The remaining list items are hidden
	for ( ; pWidget != m_oChildren.End().Get(); pWidget = pWidget->Next() )
		pWidget->SetState( State_Visible, 0u );

	m_iNumItemWidgets  = iNumItems;
	m_iNumVisibleItems = iNumItems;
	m_iNumColumns      = iNumColumns;
	m_iNumRows         = iNumRows;
}

// $JPOG: FUNCTION PGUICore.dll+0000a920
void PGUIListBox::RefreshItemList()
{
	if ( !m_pStyle || !m_pListItemStyle )
		return;

	m_pSelectedItemWidget = TNULL;

	if ( m_iNumItemWidgets == 0 )
		GenerateItemList();

	TINT iBucketX, iBucketY, iBucketWidth, iBucketHeight;
	GetComponentInteriorBBox( Component_Bucket, iBucketX, iBucketY, iBucketWidth, iBucketHeight );

	TGUIWidget*   pWidget = GetFirstItemWidget();
	PGUIItemData* pItem   = m_pFirstVisibleItem;
	TINT          iIndex  = 0;

	// The visible items
	if ( pItem && pItem != GetItemsEnd() )
	{
		while ( pWidget != m_oChildren.End().Get() && iIndex < m_iNumVisibleItems )
		{
			PGUIListItem* pListItem = AsListItem( pWidget );
			pItem->UpdateWidget( pListItem );

			TUINT uiState = pItem->GetState() | State_Visible;

			if ( !( m_uiState & State_Enabled ) )
				uiState = ( pItem->GetState() & ~( State_Enabled | State_Visible ) ) | State_Visible;

			pListItem->SetState( ~TUINT( State_Focused ), uiState );
			pListItem->Pack();
			pListItem->SetData( pItem );
			pListItem->SetIndex( iIndex );

			if ( pItem == m_pSelectedItem )
			{
				pListItem->SetState( State_Focused, ( m_uiState & State_Focused ) ? TUINT( State_Focused ) : 0u );
				m_pSelectedItemWidget = pListItem;
			}
			else
			{
				pListItem->SetState( State_Focused, 0u );
			}

			pItem   = pItem->Next();
			pWidget = pWidget->Next();
			iIndex++;

			if ( pItem == GetItemsEnd() )
				break;
		}
	}

	const TBOOL bWidgetsUsed = pWidget == m_oChildren.End().Get();
	SetState( State_ScrollableForward, ( m_iNumVisibleItems > 0 && bWidgetsUsed && pItem && pItem != GetItemsEnd() ) ? TUINT( State_ScrollableForward ) : 0u );

	const TBOOL bScrollableBack = m_iNumVisibleItems > 0 && m_pFirstVisibleItem && m_pFirstVisibleItem->Prev() && m_pFirstVisibleItem->Prev() != GetItemsEnd();
	SetState( State_ScrollableBack, bScrollableBack ? TUINT( State_ScrollableBack ) : 0u );

	// The remaining list items are hidden
	for ( ; pWidget != m_oChildren.End().Get(); pWidget = pWidget->Next() )
	{
		PGUIListItem* pListItem = AsListItem( pWidget );
		pListItem->SetState( State_Visible, 0u );
		pListItem->SetData( TNULL );
		pListItem->SetIndex( -1 );
	}
}

// $JPOG: FUNCTION PGUICore.dll+0000ac70
void PGUIListBox::DeleteItemList()
{
	if ( m_iNumItemWidgets == 0 )
		return;

	for ( TGUIWidget* pWidget = GetFirstItemWidget(); pWidget != m_oChildren.End().Get(); )
	{
		TGUIWidget* pNext = pWidget->Next();
		RemoveWidget( pWidget );

		if ( pWidget )
			delete pWidget;

		pWidget = pNext;
	}

	m_iNumItemWidgets = 0;
}

// $JPOG: FUNCTION PGUICore.dll+0000aea0
void PGUIListBox::HitSelectedItem()
{
	const TBOOL bEnabled = m_pSelectedItem && ( m_pSelectedItem->GetState() & PGUIItemData::State_Enabled );

	if ( m_pSelectedItem && ( ( bEnabled && ( m_eSelectionMode & SelectionMode_Enabled ) ) || ( !bEnabled && ( m_eSelectionMode & SelectionMode_Disabled ) ) ) )
	{
		PGUIPlaySound( GetScreen(), "PGUIListBox", "select" );

		TGUIActionEvent oEvent;
		oEvent.pItem   = TNULL;
		oEvent.bAccept = TFALSE;
		m_oActionEmitter.Throw( &oEvent );
	}
	else
	{
		PGUIPlaySound( GetScreen(), "PGUIListBox", "select_fail" );
	}
}

// $JPOG: FUNCTION PGUICore.dll+0000b240
TBOOL PGUIListBox::DoButtonEvent( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	if ( !m_pSelectedItemWidget || !m_pSelectedItem )
	{
		PGUIPlaySound( GetScreen(), "PGUIListBox", "decrement_fail" );
		return TFALSE;
	}

	if ( m_eArrangeMode != ArrangeMode_Grid )
	{
		if ( ( m_eArrangeMode == ArrangeMode_Vertical && ( a_iActions & 0x200 ) ) || ( m_eArrangeMode == ArrangeMode_Horizontal && ( a_iActions & 0x1000 ) ) )
			return Navigate( a_pEvent, -1 );

		if ( ( m_eArrangeMode == ArrangeMode_Vertical && ( a_iActions & 0x800 ) ) || ( m_eArrangeMode == ArrangeMode_Horizontal && ( a_iActions & 0x400 ) ) )
			return Navigate( a_pEvent, 1 );

		// The mouse wheel
		if ( a_pEvent->GetDoodad() == 0x30009 )
			return Navigate( a_pEvent, -1 );

		if ( a_pEvent->GetDoodad() == 0x3000A )
			return Navigate( a_pEvent, 1 );

		return TFALSE;
	}

	// The grid: by columns and rows
	TINT iIndex = m_pSelectedItemWidget->GetIndex() - m_iFirstItemWidget;

	if ( a_iActions & 0x400 )
		return ( ( iIndex + 1 ) % m_iNumColumns != 0 ) && Navigate( a_pEvent, 1 );

	if ( a_iActions & 0x1000 )
		return ( iIndex % m_iNumColumns != 0 ) && Navigate( a_pEvent, -1 );

	if ( a_iActions & 0x200 )
		return iIndex >= m_iNumColumns && Navigate( a_pEvent, -m_iNumColumns );

	if ( a_iActions & 0x800 )
		return m_pSelectedItemWidget->GetIndex() < TINT( m_oItems.Count() ) - m_iNumColumns && Navigate( a_pEvent, m_iNumColumns );

	return TFALSE;
}

// $JPOG: FUNCTION PGUICore.dll+0000b4a0
TBOOL PGUIListBox::Navigate( TInputInterface::InputEvent* a_pEvent, TINT a_iDelta )
{
	PGUIItemData* pItem = m_pSelectedItem;

	if ( !pItem )
		return TFALSE;

	if ( a_iDelta == 1 )
	{
		PGUIItemData* pNext = pItem->Next();

		if ( !pNext || pNext == GetItemsEnd() )
		{
#ifdef BARNYARD_COMMUNITY_PATCH
			PGUIPlaySound( GetScreen(), "PGUIListBox", "increment_fail" );
#else
			// The original plays the decrement sound
			PGUIPlaySound( GetScreen(), "PGUIListBox", "decrement_fail" );
#endif
			return TTRUE;
		}

		PGUIPlaySound( GetScreen(), "PGUIListBox", "increment" );

		// Scrolls when the selection reaches the last visible item but one
		if ( m_iNumVisibleItems - 2 == m_pSelectedItemWidget->GetIndex() )
		{
			PGUIItemData* pAfterNext = m_pSelectedItem->Next()->Next();

			if ( pAfterNext && pAfterNext != GetItemsEnd() )
				SetFirstItem( m_pFirstVisibleItem->Next() );
		}

		SelectItemImpl( m_pSelectedItem->Next() );
		RefreshItemList();
		a_pEvent->StartRepeat( 0.25f, 0.1f );

		return TTRUE;
	}

	if ( a_iDelta == -1 )
	{
		PGUIItemData* pPrev = pItem->Prev();

		if ( !pPrev || pPrev == GetItemsEnd() )
		{
			PGUIPlaySound( GetScreen(), "PGUIListBox", "decrement_fail" );
			return TTRUE;
		}

		PGUIPlaySound( GetScreen(), "PGUIListBox", "decrement" );

		// Scrolls when the selection reaches the first visible item but one
		if ( m_pSelectedItemWidget->GetIndex() == 1 )
		{
			PGUIItemData* pBeforePrev = m_pSelectedItem->Prev()->Prev();

			if ( pBeforePrev && pBeforePrev != GetItemsEnd() )
				SetFirstItem( pBeforePrev );
		}

		SelectItemImpl( m_pSelectedItem->Prev() );
		RefreshItemList();
		a_pEvent->StartRepeat( 0.25f, 0.1f );

		return TTRUE;
	}

	if ( a_iDelta > 1 )
	{
		for ( TINT i = a_iDelta; i > 0; i-- )
		{
			PGUIItemData* pNext = pItem->Next();

			if ( !pNext || pNext == GetItemsEnd() )
				break;

			pItem = pNext;
		}

		PGUIPlaySound( GetScreen(), "PGUIListBox", "increment" );
		SelectItemImpl( pItem );
		RefreshItemList();
		a_pEvent->StartRepeat( 0.25f, 0.1f );

		return TTRUE;
	}

	if ( a_iDelta < -1 )
	{
		for ( TINT i = a_iDelta; i < 0; i++ )
		{
			PGUIItemData* pPrev = pItem->Prev();

			if ( !pPrev || pPrev == GetItemsEnd() )
				break;

			pItem = pPrev;
		}

		PGUIPlaySound( GetScreen(), "PGUIListBox", "decrement" );
		SelectItemImpl( pItem );
		RefreshItemList();
		a_pEvent->StartRepeat( 0.25f, 0.1f );

		return TTRUE;
	}

	return TFALSE;
}

// $JPOG: FUNCTION PGUICore.dll+0000bda0
TGUIWidget* PGUIListBox::MakeNewItem()
{
	if ( !m_pListItemStyle )
		return TNULL;

	TGUIWidget* pWidget = TSTATICCAST( TGUIWidget, m_pListItemStyle->GetWidgetInfo()->GetTClass()->CreateObject() );
	pWidget->Create( m_pListItemStyle, TPString8() );
	pWidget->Layout( -1, -1 );
	pWidget->Pack();
	pWidget->SetState( State_Enabled, State_Enabled );
	pWidget->SetState( State_Visible, State_Visible );

	return pWidget;
}

// $JPOG: FUNCTION PGUICore.dll+0000be40
PGUIListItem* PGUIListBox::FindItemWidget( PGUIItemData* a_pItem )
{
	TGUIWidget*   pWidget = GetFirstItemWidget();
	PGUIItemData* pItem   = m_pFirstVisibleItem;

	while ( pWidget != m_oChildren.End().Get() && pItem != GetItemsEnd() )
	{
		if ( pItem == a_pItem )
			return TSTATICCAST( PGUIListItem, pWidget );

		if ( pWidget )
			pWidget = pWidget->Next();

		if ( pItem )
			pItem = pItem->Next();
	}

	return TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+0000be90
PGUIItemData* PGUIListBox::FindWidgetItem( TGUIWidget* a_pWidget )
{
	TGUIWidget*   pWidget = GetFirstItemWidget();
	PGUIItemData* pItem   = m_pFirstVisibleItem;

	if ( pWidget == m_oChildren.End().Get() )
		return TNULL;

	while ( pItem != GetItemsEnd() )
	{
		if ( pWidget == a_pWidget )
			return pItem;

		if ( pWidget )
			pWidget = pWidget->Next();

		if ( pItem )
			pItem = pItem->Next();

		if ( pWidget == m_oChildren.End().Get() )
			return TNULL;
	}

	return TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+0000bef0
void PGUIListBox::AddItemImpl( PGUIItemData* a_pItem )
{
	m_oItems.InsertTail( a_pItem );

	if ( !m_pFirstVisibleItem )
		m_pFirstVisibleItem = a_pItem;

	if ( !m_pSelectedItem )
		SelectItemImpl( a_pItem );
}

// The selection moves to the next (or the previous) item, the first visible item too
// $JPOG: FUNCTION PGUICore.dll+0000bf50
void PGUIListBox::RemoveItemImpl( PGUIItemData* a_pItem )
{
	if ( m_pSelectedItem == a_pItem )
	{
		if ( a_pItem->Next() == GetItemsEnd() )
			SelectItemImpl( ( a_pItem->Prev() == GetItemsEnd() ) ? TNULL : a_pItem->Prev() );
		else
			SelectItemImpl( a_pItem->Next() );
	}

	if ( !( m_uiState & State_ScrollableForward ) && ( m_uiState & State_ScrollableBack ) && m_pFirstVisibleItem != a_pItem )
	{
		// Scrolled to the end: the list scrolls back by one
		m_pFirstVisibleItem = m_pFirstVisibleItem->Prev();
	}
	else if ( m_pFirstVisibleItem == a_pItem )
	{
		if ( a_pItem->Next() != GetItemsEnd() )
			m_pFirstVisibleItem = a_pItem->Next();
		else if ( a_pItem->Prev() != GetItemsEnd() )
			m_pFirstVisibleItem = a_pItem->Prev();
		else
			m_pFirstVisibleItem = TNULL;
	}

	m_oItems.Remove( a_pItem );

	if ( a_pItem )
		delete a_pItem;
}

// Updates the Selected state of the items, the actions of the list box (enabled with an enabled selection) and the
// status display of the input context
// $JPOG: FUNCTION PGUICore.dll+0000c060
void PGUIListBox::SelectItemImpl( PGUIItemData* a_pItem )
{
	if ( m_pSelectedItem == a_pItem )
		return;

	if ( a_pItem == GetItemsEnd() )
		a_pItem = TNULL;

	if ( m_pSelectedItem )
		m_pSelectedItem->RemoveState( PGUIItemData::State_Selected );

	m_pSelectedItem       = a_pItem;
	m_pSelectedItemWidget = TNULL;

	if ( a_pItem )
	{
		a_pItem->AddState( PGUIItemData::State_Selected );
		EnsureVisibleImpl( m_pSelectedItem );
	}

	for ( auto it = m_oActions.Begin(); it != m_oActions.End(); it++ )
		it->SetEnabled( m_pSelectedItem && ( m_pSelectedItem->GetState() & PGUIItemData::State_Enabled ) );

	if ( GetScreen() )
		GetScreen()->GetInputContext()->UpdateStatusDisplay();

	PGUIItemChangeEvent oEvent;
	oEvent.pItem = m_pSelectedItem;
	m_oItemChangeEmitter.Throw( &oEvent );
}

// Scrolls the list so that the item is visible (at the top if it's before the visible items, at the bottom otherwise)
// $JPOG: FUNCTION PGUICore.dll+0000c160
void PGUIListBox::EnsureVisibleImpl( PGUIItemData* a_pItem )
{
	if ( FindItemWidget( a_pItem ) )
		return;

	if ( a_pItem != GetItemsEnd() )
	{
		for ( PGUIItemData* pItem = a_pItem; pItem != GetItemsEnd(); pItem = pItem->Next() )
		{
			if ( pItem == m_pFirstVisibleItem )
			{
				m_pFirstVisibleItem = a_pItem;
				return;
			}
		}
	}

	m_pFirstVisibleItem = a_pItem;

	for ( TINT i = 0; i < m_iNumItemWidgets - 1; i++ )
		m_pFirstVisibleItem = m_pFirstVisibleItem->Prev();
}

// $JPOG: FUNCTION PGUICore.dll+0000c2d0
void PGUIListBox::RemoveAllItemsImpl()
{
	m_pSelectedItem     = TNULL;
	m_pFirstVisibleItem = TNULL;
	m_oItems.DeleteAll();
}

// $JPOG: FUNCTION PGUICore.dll+0000c360
void PGUIListBox::SetFirstItem( PGUIItemData* a_pItem )
{
	m_pFirstVisibleItem = a_pItem;
}

// $JPOG: FUNCTION PGUICore.dll+0000c4d0
void PGUIListBox::Set_orientation_property( const PPropertyValue& a_rcValue )
{
	SetOrientation( a_rcValue.GetInteger() );
}

// $JPOG: FUNCTION PGUICore.dll+0000c4b0
PPropertyValue PGUIListBox::Get_orientation_property() const
{
	return PPropertyValue( TINT( m_eArrangeMode ) );
}

// $JPOG: FUNCTION PGUICore.dll+0000a190
TBOOL PGUIListBox::Val_orientation_property( const PPropertyValue& a_rcValue )
{
	if ( !a_rcValue.CanBeType( PPropertyValue::TYPE_INT ) )
		return TFALSE;

	return a_rcValue.GetInteger() == ArrangeMode_Horizontal || a_rcValue.GetInteger() == ArrangeMode_Vertical || a_rcValue.GetInteger() == ArrangeMode_Grid;
}

// $JPOG: FUNCTION PGUICore.dll+0000c490
void PGUIListBox::Set_scrollbarpos_property( const PPropertyValue& a_rcValue )
{
	SetScrollBarPosition( a_rcValue.GetInteger() );
}

// $JPOG: FUNCTION PGUICore.dll+0000c470
PPropertyValue PGUIListBox::Get_scrollbarpos_property() const
{
	return PPropertyValue( TINT( m_eScrollBarPosition ) );
}

// $JPOG: FUNCTION PGUICore.dll+0000a1f0
TBOOL PGUIListBox::Val_scrollbarpos_property( const PPropertyValue& a_rcValue )
{
	if ( !a_rcValue.CanBeType( PPropertyValue::TYPE_INT ) )
		return TFALSE;

	return a_rcValue.GetInteger() == Placement_Before || a_rcValue.GetInteger() == Placement_After;
}

// $JPOG: FUNCTION PGUICore.dll+0000c530
void PGUIListBox::Set_selectionmode_property( const PPropertyValue& a_rcValue )
{
	m_eSelectionMode = SelectionMode( a_rcValue.GetInteger() );
}

// $JPOG: FUNCTION PGUICore.dll+0000c510
PPropertyValue PGUIListBox::Get_selectionmode_property() const
{
	return PPropertyValue( TINT( m_eSelectionMode ) );
}

// $JPOG: FUNCTION PGUICore.dll+0000c4f0
TBOOL PGUIListBox::Val_selectionmode_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_INT );
}

// A style or its name
// $JPOG: FUNCTION PGUICore.dll+0000a250
void PGUIListBox::Set_scrollbarstyle_property( const PPropertyValue& a_rcValue )
{
	if ( a_rcValue.IsType( &TGetClass( TGUIStyle ) ) )
		SetScrollBarStyle( TSTATICCAST( TGUIStyle, a_rcValue.GetTObject() ) );
	else if ( a_rcValue.IsType( PPropertyValue::TYPE_TPSTRING8 ) )
		SetScrollBarStyle( a_rcValue.GetTPString8() );
}

// The name of the style
// $JPOG: FUNCTION PGUICore.dll+0000a2c0
PPropertyValue PGUIListBox::Get_scrollbarstyle_property() const
{
	return PPropertyValue( m_pScrollBarStyle ? m_pScrollBarStyle->GetName() : TPString8() );
}

// $JPOG: FUNCTION PGUICore.dll+0000a310
TBOOL PGUIListBox::Val_scrollbarstyle_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( &TGetClass( TGUIStyle ) ) || a_rcValue.CanBeType( PPropertyValue::TYPE_TPSTRING8 );
}

// A style or its name
// $JPOG: FUNCTION PGUICore.dll+0000a360
void PGUIListBox::Set_listitemstyle_property( const PPropertyValue& a_rcValue )
{
	if ( a_rcValue.IsType( &TGetClass( TGUIStyle ) ) )
		SetListItemStyle( TSTATICCAST( TGUIStyle, a_rcValue.GetTObject() ) );
	else if ( a_rcValue.IsType( PPropertyValue::TYPE_TPSTRING8 ) )
		SetListItemStyle( a_rcValue.GetTPString8() );
}

// The name of the style
// $JPOG: FUNCTION PGUICore.dll+0000a3d0
PPropertyValue PGUIListBox::Get_listitemstyle_property() const
{
	return PPropertyValue( m_pListItemStyle ? m_pListItemStyle->GetName() : TPString8() );
}

// $JPOG: FUNCTION PGUICore.dll+0000a420
TBOOL PGUIListBox::Val_listitemstyle_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( &TGetClass( TGUIStyle ) ) || a_rcValue.CanBeType( PPropertyValue::TYPE_TPSTRING8 );
}
