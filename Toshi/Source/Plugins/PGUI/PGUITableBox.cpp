#include "ToshiPCH.h"
#include "PGUITableBox.h"
#include "PGUICore.h"

#include <TGUI/TGUIManager.h>
#include <TGUI/TGUIWidgetInfo.h>
#include <TGUI/TGUIStyle.h>
#include <TGUI/TGUIScreen.h>
#include <TGUI/TGUIInputContext.h>
#include <TGUI/TGUIComponentCache.h>
#include <TGUI/TGUIDisplayContext.h>
#include <TGUI/TGUIPropertySpecSimple.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_INIT( PGUITableBox );

TGUIDeserialisable::PropertySet* PGUITableBox::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* PGUITableBox::ms_oPropertySetOrdered = TNULL;
TGUIWidgetInfo*                  PGUITableBox::ms_pWidgetInfo         = TNULL;

const TCHAR* const PGUITableBox::ms_csaSelectionModeNameStr[ SelectionMode_NUMOF ] = { "None", "Enabled", "Disabled", "Any" };

static TBOOL IsInRect( const TGUIRectangle& a_rcRect, TINT a_iX, TINT a_iY )
{
	return a_iX >= a_rcRect.m_iX && a_iY >= a_rcRect.m_iY && a_iX < a_rcRect.m_iX + a_rcRect.m_iWidth && a_iY < a_rcRect.m_iY + a_rcRect.m_iHeight;
}

// NOTE: the category of the property specs (an argument the JPOG constructors don't have) is the name of the class
// $JPOG: FUNCTION PGUICore.dll+00018140
void PGUITableBox::InitialiseClass()
{
	using t_Setter = TGUIPropertySpecSimple::t_Setter;
	using t_Getter = TGUIPropertySpecSimple::t_Getter;

	ms_pWidgetInfo = new TGUIWidgetInfo();
	CreatePropertySets( ms_oPropertySet, ms_oPropertySetOrdered, *PGUIPanel::ms_oPropertySet );

	TPString8 strCategory( "PGUITableBox" );
	TINT      iSortHint = 0;
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "columns" ), static_cast<t_Setter>( &PGUITableBox::Set_columns_property ), static_cast<t_Getter>( &PGUITableBox::Get_columns_property ), PPropertyValue::TYPE_INT, strCategory, TTRUE, &TGetClass( PGUITableBox ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "selectionmode" ), static_cast<t_Setter>( &PGUITableBox::Set_selectionmode_property ), static_cast<t_Getter>( &PGUITableBox::Get_selectionmode_property ), PPropertyValue::TYPE_INT, strCategory, TTRUE, &TGetClass( PGUITableBox ), iSortHint++, SelectionMode_NUMOF, ms_csaSelectionModeNameStr ) );

	ms_pWidgetInfo->AddComponentName( TPString8( "background" ) );
	ms_pWidgetInfo->AddComponentName( TPString8( "bucket" ) );
	ms_pWidgetInfo->AddComponentName( TPString8( "scrollback" ) );
	ms_pWidgetInfo->AddComponentName( TPString8( "scrollforward" ) );

	TPString8 strStateCategory( "State" );
	DeclareStateVar( &TGetClass( PGUITableBox ), "scrollableback", State_ScrollableBack, strStateCategory, iSortHint, ms_pWidgetInfo, ms_oPropertySet, ms_oPropertySetOrdered );
	DeclareStateVar( &TGetClass( PGUITableBox ), "scrollableforward", State_ScrollableForward, strStateCategory, iSortHint, ms_pWidgetInfo, ms_oPropertySet, ms_oPropertySetOrdered );

	ms_pWidgetInfo->SetWidgetName( TPString8( "PGUITableBox" ) );
	ms_pWidgetInfo->SetWidgetLabel( TPString8( "Table Box" ) );
	ms_pWidgetInfo->SetProperties( ms_oPropertySet, ms_oPropertySetOrdered );
	ms_pWidgetInfo->AddStateVars( PGUIPanel::ms_pWidgetInfo->GetStateVarSet() );
	ms_pWidgetInfo->SetConcrete( TTRUE );

	TGUIManager::AddWidgetClass( ms_pWidgetInfo );
}

// $JPOG: FUNCTION PGUICore.dll+00018cd0
void PGUITableBox::DeinitialiseClass()
{
	ms_pWidgetInfo->Clear();
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );

	delete ms_pWidgetInfo;
	ms_pWidgetInfo = TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+00018d90
PGUITableBox::PGUITableBox()
    : m_oItemChangeEmitter( this )
    , m_oActionEmitter( this )
    , m_iNumColumns( 0 )
    , m_ppStampWidgets( TNULL )
    , m_piColumnWidths( TNULL )
    , m_pFirstVisibleRow( TNULL )
    , m_pLastVisibleRow( TNULL )
    , m_pSelectedRow( TNULL )
    , m_iNumVisibleRows( 0 )
    , m_eOrientation( Orientation_Vertical )
    , m_bHoverSelectMode( TFALSE )
    , m_eSelectionMode( SelectionMode_Enabled )
    , m_aRowRects( 10, 0 )
{
}

// The rows and the stamp widgets are cloned
// NOTE: the widths of the columns aren't copied (as in the original), they're uninitialised until the next Pack
// $JPOG: FUNCTION PGUICore.dll+000199c0
PGUITableBox::PGUITableBox( const PGUITableBox& a_rcOther )
    : PGUIPanel( a_rcOther )
    , m_oItemChangeEmitter( this )
    , m_oActionEmitter( this )
    , m_iNumColumns( a_rcOther.m_iNumColumns )
    , m_ppStampWidgets( TNULL )
    , m_piColumnWidths( TNULL )
    , m_pFirstVisibleRow( TNULL )
    , m_pLastVisibleRow( TNULL )
    , m_pSelectedRow( TNULL )
    , m_iNumVisibleRows( 0 )
    , m_eOrientation( a_rcOther.m_eOrientation )
    , m_bHoverSelectMode( a_rcOther.m_bHoverSelectMode )
    , m_eSelectionMode( a_rcOther.m_eSelectionMode )
    , m_aRowRects( 10, 0 )
{
	for ( auto it = a_rcOther.m_oRows.Begin(); it != a_rcOther.m_oRows.End(); it++ )
		m_oRows.InsertTail( it->Clone() );

	PGUIRowData* pFirstRow = m_oRows.IsEmpty() ? TNULL : m_oRows.Head().Get();
	m_pFirstVisibleRow     = pFirstRow;
	m_pSelectedRow         = pFirstRow;

	if ( a_rcOther.m_ppStampWidgets )
	{
		m_ppStampWidgets = new TGUIWidget*[ m_iNumColumns ];
		m_piColumnWidths = new TINT[ m_iNumColumns ];

		for ( TINT i = 0; i < m_iNumColumns; i++ )
			m_ppStampWidgets[ i ] = a_rcOther.m_ppStampWidgets[ i ]->Clone();
	}

	SelectItemImpl( pFirstRow );
}

// NOTE: the rows are removed without being deleted and the stamp widgets aren't deleted (as in the original)
// $JPOG: FUNCTION PGUICore.dll+00018e40
PGUITableBox::~PGUITableBox()
{
	m_oRows.RemoveAll();
}

// $JPOG: FUNCTION PGUICore.dll+0001b0a0
TBOOL PGUITableBox::Deserialise( const PProperties* a_pProperties )
{
	return PGUIPanel::Deserialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+0001b070
TBOOL PGUITableBox::Serialise( PProperties* a_pProperties )
{
	return PGUIPanel::Serialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+0001b110
TGUIDeserialisable::PropertySet* PGUITableBox::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $JPOG: FUNCTION PGUICore.dll+0001b100
TGUIDeserialisable::PropertySet* PGUITableBox::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

const TGUIWidgetInfo* PGUITableBox::GetWidgetInfo() const
{
	return ms_pWidgetInfo;
}

// $JPOG: FUNCTION PGUICore.dll+0001b0d0
PGUITableBox* PGUITableBox::Clone() const
{
	return new PGUITableBox( *this );
}

// $JPOG: FUNCTION PGUICore.dll+0001b4b0
TBOOL PGUITableBox::AcceptsFocus() const
{
	return TTRUE;
}

// $JPOG: FUNCTION PGUICore.dll+0001af70
void PGUITableBox::NotifyFocusLost( TGUIWidget* a_pNewFocus, TGUIWidget* a_pOldFocus )
{
	TGUIWidget::NotifyFocusLost( a_pNewFocus, a_pOldFocus );
}

// $JPOG: FUNCTION PGUICore.dll+0001af50
void PGUITableBox::NotifyFocusGained( TGUIWidget* a_pNewFocus, TGUIWidget* a_pOldFocus )
{
	TGUIWidget::NotifyFocusGained( a_pNewFocus, a_pOldFocus );
}

// Select throws the action event for the selected row if the selection mode allows its state
// $JPOG: FUNCTION PGUICore.dll+00019d50
TBOOL PGUITableBox::OnButtonPress( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	if ( TGUIWidget::OnButtonPress( a_pEvent, a_iActions ) || DoButtonEvent( a_pEvent, a_iActions ) )
		return TTRUE;

	if ( !( a_iActions & 1 ) )
		return TFALSE;

	if ( m_pSelectedRow )
	{
		const TBOOL bEnabled = m_pSelectedRow->GetState( 0 ) & 1;

		if ( ( bEnabled && ( m_eSelectionMode & SelectionMode_Enabled ) ) || ( !bEnabled && ( m_eSelectionMode & SelectionMode_Disabled ) ) )
		{
			PGUIPlaySound( GetScreen(), "PGUITableBox", "select" );

			ActionEvent oEvent;
			oEvent.pRow    = m_pSelectedRow;
			oEvent.iColumn = -1;
			oEvent.iSource = 2;
			m_oActionEmitter.Throw( &oEvent );
		}
	}
	else
	{
		PGUIPlaySound( GetScreen(), "PGUITableBox", "select_fail" );
	}

	return TTRUE;
}

// $JPOG: FUNCTION PGUICore.dll+00019f90
TBOOL PGUITableBox::OnButtonRepeat( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	return TGUIWidget::OnButtonRepeat( a_pEvent, a_iActions ) || DoButtonEvent( a_pEvent, a_iActions );
}

// $JPOG: FUNCTION PGUICore.dll+0001af90
TBOOL PGUITableBox::OnPointerMove( TINT a_iX, TINT a_iY )
{
	if ( m_bHoverSelectMode )
		SelectItemAtPoint( a_iX, a_iY );

	return TFALSE;
}

// Paints the cells of the visible rows with the stamp widgets under the headings
// $JPOG: FUNCTION PGUICore.dll+00019240
TBOOL PGUITableBox::OnPaint( TGUIDisplayContext* a_pDisplayContext )
{
	TBOOL bResult = TGUIWidget::OnPaint( a_pDisplayContext );

	if ( !m_ppStampWidgets || m_oRows.IsEmpty() || !m_pFirstVisibleRow )
		return bResult;

	TINT iBucketX, iBucketY, iBucketWidth, iBucketHeight;
	GetComponentInteriorBBox( Component_Bucket, iBucketX, iBucketY, iBucketWidth, iBucketHeight );

	TINT         iStartX = m_pStyle ? m_pStyle->GetMarginLeft() : 0;
	TGUIWidget*  pHeader = m_oChildren.Head().Get();
	TINT         iY      = pHeader->GetYPos() + pHeader->GetHeight();
	PGUIRowData* pRow    = m_pFirstVisibleRow;

	for ( TINT iRow = 0; iRow < m_iNumVisibleRows; iRow++ )
	{
		TINT iRowHeight = UpdateWidgets( pRow );
		TINT iX         = iStartX;

		for ( TINT iColumn = 0; iColumn < m_iNumColumns; iColumn++ )
		{
			TGUIWidget* pStamp  = m_ppStampWidgets[ iColumn ];
			TUINT       uiState = pRow->GetState( iColumn );

			if ( pRow == m_pSelectedRow )
				uiState |= ( ( m_uiState & State_Focused ) ? TUINT( State_Focused ) : 0u ) | State_Selected;

			if ( !( m_uiState & State_Enabled ) )
				uiState &= ~State_Enabled;

			pStamp->SetPosition( iX, iY );
			pStamp->SetState( State_Enabled | State_Focused | State_Visible | State_Selected, uiState );

			a_pDisplayContext->PushClip( pStamp->GetXPos(), pStamp->GetYPos(), pStamp->GetWidth(), pStamp->GetHeight() );
			pStamp->OnPaint( a_pDisplayContext );
			a_pDisplayContext->PopClip();

			iX += pStamp->GetWidth();
		}

		iY += iRowHeight;

		if ( pRow )
			pRow = pRow->Next();
	}

	return bResult;
}

// $JPOG: FUNCTION PGUICore.dll+00018f80
void PGUITableBox::Pack()
{
	if ( m_pStyle )
	{
		TINT iBackWidth     = 0;
		TINT iBackHeight    = 0;
		TINT iForwardWidth  = 0;
		TINT iForwardHeight = 0;
		m_pStyle->GetComponentPreferredSize( Component_ScrollBack, m_pParameters, iBackWidth, iBackHeight );
		m_pStyle->GetComponentPreferredSize( Component_ScrollForward, m_pParameters, iForwardWidth, iForwardHeight );

		SetComponentBBox( Component_Background, 0, 0, m_iWidth, m_iHeight );

		if ( m_eOrientation != Orientation_Horizontal )
		{
			SetComponentBBox( Component_ScrollBack, 0, 0, m_iWidth, iBackHeight );
			SetComponentBBox( Component_ScrollForward, 0, m_iHeight - iForwardHeight, m_iWidth, iForwardHeight );
			SetComponentBBox( Component_Bucket, 0, iBackHeight, m_iWidth, m_iHeight - ( iForwardHeight + iBackHeight ) );
		}
		else
		{
			SetComponentBBox( Component_ScrollBack, 0, 0, iBackWidth, m_iHeight );
			SetComponentBBox( Component_ScrollForward, m_iWidth - iForwardWidth, 0, iForwardWidth, m_iHeight );
			SetComponentBBox( Component_Bucket, iBackWidth, 0, m_iWidth - ( iForwardWidth + iBackWidth ), m_iHeight );
		}
	}

	PGUIPanel::Pack();
	UpdateWidgetWidths();
	RefreshItemList();

	m_bValidated = TTRUE;
}

// As a panel, with the scroll components along the orientation
// $JPOG: FUNCTION PGUICore.dll+000190d0
void PGUITableBox::Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight )
{
	m_bLayoutValid = TTRUE;
	PGUIPanel::Layout( a_iMaxWidth, a_iMaxHeight );

	if ( m_eSizeModeHoriz == SizeMode_Fixed && m_eSizeModeVert == SizeMode_Fixed )
		return;

	TINT iWidth  = m_iWidth;
	TINT iHeight = m_iHeight;

	if ( m_pStyle )
	{
		for ( TINT iComponent = Component_ScrollBack; iComponent <= Component_ScrollForward; iComponent++ )
		{
			TINT iComponentWidth  = 0;
			TINT iComponentHeight = 0;

			if ( !m_pStyle->GetComponentPreferredSize( iComponent, m_pParameters, iComponentWidth, iComponentHeight ) )
				continue;

			if ( m_eOrientation != Orientation_Horizontal )
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
	}

	if ( m_eSizeModeHoriz != SizeMode_Fixed )
		SetWidth( iWidth );

	if ( m_eSizeModeVert != SizeMode_Fixed )
		SetHeight( iHeight );
}

// $JPOG: FUNCTION PGUICore.dll+00019440
void PGUITableBox::Flush()
{
	if ( m_ppStampWidgets )
	{
		for ( TINT i = 0; i < m_iNumColumns; i++ )
		{
			TGUIWidget* pStamp = m_ppStampWidgets[ i ];

			if ( TGUIStyle* pStyle = pStamp->GetStyle() )
			{
				for ( TINT j = 0; j < pStyle->GetParameterCount(); j++ )
					pStamp->SetParameter( j, PPropertyValue() );
			}

			pStamp->InvalidateComponentCache( TFALSE );
		}

		InsertStampWidgets();

		delete[] m_ppStampWidgets;
		delete[] m_piColumnWidths;
		m_ppStampWidgets = TNULL;
		m_piColumnWidths = TNULL;
	}

	m_aRowRects.FreeData();
}

// $JPOG: FUNCTION PGUICore.dll+00019540
void PGUITableBox::Initialise()
{
	if ( m_ppStampWidgets )
		return;

	m_ppStampWidgets = new TGUIWidget*[ m_iNumColumns ];
	m_piColumnWidths = new TINT[ m_iNumColumns ];
	RemoveStampWidgets();
}

// $JPOG: FUNCTION PGUICore.dll+0001b3d0
void PGUITableBox::AddItem( PGUIRowData* a_pRow )
{
	AddItemImpl( a_pRow );
}

// $JPOG: FUNCTION PGUICore.dll+0001b3b0
void PGUITableBox::RemoveItem( PGUIRowData* a_pRow )
{
	RemoveItemImpl( a_pRow );
	RefreshItemList();
}

// $JPOG: FUNCTION PGUICore.dll+0001b390
void PGUITableBox::RemoveAllItems()
{
	RemoveAllItemsImpl();
	RefreshItemList();
}

// $JPOG: FUNCTION PGUICore.dll+0001b370
void PGUITableBox::SelectItem( PGUIRowData* a_pRow )
{
	SelectItemImpl( a_pRow );
	RefreshItemList();
}

// $JPOG: FUNCTION PGUICore.dll+0001b350
void PGUITableBox::EnsureVisible( PGUIRowData* a_pRow )
{
	EnsureVisibleImpl( a_pRow );
	RefreshItemList();
}

// $JPOG: FUNCTION PGUICore.dll+00019590
TBOOL PGUITableBox::IsItemVisible( PGUIRowData* a_pRow )
{
	PGUIRowData* pRow = m_pFirstVisibleRow;

	if ( !pRow || pRow == GetRowsEnd() )
		return TFALSE;

	for ( TINT i = 0; i < m_iNumVisibleRows; i++ )
	{
		if ( pRow == a_pRow )
			return TTRUE;

		if ( pRow )
			pRow = pRow->Next();

		if ( pRow == GetRowsEnd() )
			return TFALSE;
	}

	return TFALSE;
}

// $JPOG: FUNCTION PGUICore.dll+000196c0
TBOOL PGUITableBox::FindItemAtPoint( TINT a_iX, TINT a_iY, PGUIRowData*& a_rpRow, TINT& a_riColumn )
{
	PGUIRowData* pRow = m_pFirstVisibleRow;

	for ( TINT iRow = 0; iRow < m_iNumVisibleRows; iRow++ )
	{
		if ( IsInRect( m_aRowRects[ iRow ], a_iX, a_iY ) )
		{
			// The column
			TINT iBucketX, iBucketY, iBucketWidth, iBucketHeight;
			GetComponentInteriorBBox( Component_Bucket, iBucketX, iBucketY, iBucketWidth, iBucketHeight );

			TINT iX = a_iX - iBucketX;

			for ( TINT iColumn = 0; iColumn < m_iNumColumns; iColumn++ )
			{
				iX -= m_piColumnWidths[ iColumn ];

				if ( iX < 0 )
				{
					a_rpRow    = pRow;
					a_riColumn = iColumn;
					return TTRUE;
				}
			}

			return TFALSE;
		}

		if ( pRow )
			pRow = pRow->Next();
	}

	return TFALSE;
}

// $JPOG: FUNCTION PGUICore.dll+000195f0
void PGUITableBox::SelectItemAtPoint( TINT a_iX, TINT a_iY )
{
	PGUIRowData* pFoundRow = TNULL;
	PGUIRowData* pRow      = m_pFirstVisibleRow;

	for ( TINT iRow = 0; iRow < m_iNumVisibleRows; iRow++ )
	{
		if ( IsInRect( m_aRowRects[ iRow ], a_iX, a_iY ) )
		{
			pFoundRow = pRow;
			break;
		}

		if ( pRow )
			pRow = pRow->Next();
	}

	SelectItemImpl( pFoundRow );
	RefreshItemList();
}

// NOTE: the bottom of the rows is compared with the height of the bucket (as in the original)
// $JPOG: FUNCTION PGUICore.dll+000197e0
void PGUITableBox::RefreshItemList()
{
	m_iNumVisibleRows = 0;

	if ( m_oRows.IsEmpty() || !m_pFirstVisibleRow )
	{
		SetState( State_ScrollableBack, 0u );
		SetState( State_ScrollableForward, 0u );
		return;
	}

	TINT iBucketX, iBucketY, iBucketWidth, iBucketHeight;
	GetComponentInteriorBBox( Component_Bucket, iBucketX, iBucketY, iBucketWidth, iBucketHeight );

	TGUIWidget*  pHeader      = m_oChildren.Head().Get();
	TINT         iY           = pHeader->GetHeight() + pHeader->GetYPos();
	PGUIRowData* pRow         = m_pFirstVisibleRow;
	TINT         iNumRows     = 0;
	TBOOL        bMoreRows    = TFALSE;

	if ( pRow != GetRowsEnd() )
	{
		while ( TTRUE )
		{
			TINT iRowHeight = UpdateWidgets( pRow );

			if ( iRowHeight + iY > iBucketHeight )
			{
				bMoreRows = pRow != GetRowsEnd();
				break;
			}

			if ( iNumRows + 1 > m_aRowRects.Size() )
				m_aRowRects.Push( TGUIRectangle() );

			m_aRowRects[ iNumRows ] = TGUIRectangle( iBucketX, iY, iBucketWidth, iRowHeight );
			iY += iRowHeight;
			m_pLastVisibleRow = pRow;
			iNumRows++;

			if ( pRow )
				pRow = pRow->Next();

			if ( pRow == GetRowsEnd() )
			{
				bMoreRows = TFALSE;
				break;
			}
		}
	}

	m_iNumVisibleRows = iNumRows;
	SetState( State_ScrollableBack, ( m_pFirstVisibleRow != m_oRows.Head().Get() ) ? TUINT( State_ScrollableBack ) : 0u );
	SetState( State_ScrollableForward, bMoreRows ? TUINT( State_ScrollableForward ) : 0u );
}

// $JPOG: FUNCTION PGUICore.dll+00019bc0
TINT PGUITableBox::UpdateWidgets( PGUIRowData* a_pRow )
{
	TINT iHeight = 0;

	for ( TINT iColumn = 0; iColumn < m_iNumColumns; iColumn++ )
	{
		TGUIWidget* pStamp = m_ppStampWidgets[ iColumn ];

		if ( TGUIStyle* pStyle = pStamp->GetStyle() )
		{
			PPropertyValue* pValues = a_pRow->GetParameterValues( iColumn );

			for ( TINT i = 0; i < pStyle->GetParameterCount(); i++ )
				pStamp->SetParameter( i, pValues[ i ] );
		}

		pStamp->SetState( ~TUINT( State_Focused | State_Visible ), a_pRow->GetState( iColumn ) );
		pStamp->SetFixedWidth( m_piColumnWidths[ iColumn ] );
		pStamp->SetState( State_Visible, State_Visible );
		pStamp->Layout( -1, -1 );
		pStamp->Pack();

		iHeight = TMath::Max( TINT( pStamp->GetHeight() ), iHeight );
	}

	return iHeight;
}

// $JPOG: FUNCTION PGUICore.dll+00019ce0
void PGUITableBox::UpdateWidgetWidths()
{
	if ( m_iNumColumns == 0 || !m_ppStampWidgets || !m_piColumnWidths )
		return;

	TGUIWidget* pHeader = m_oChildren.Head().Get();

	for ( TINT i = 0; i < m_iNumColumns; i++ )
	{
		m_piColumnWidths[ i ] = pHeader->GetWidth();
		pHeader               = pHeader->Next();
	}
}

// The navigation along the orientation, the mouse wheel, the clicks on the scroll components and on the cells
// $JPOG: FUNCTION PGUICore.dll+00019fe0
TBOOL PGUITableBox::DoButtonEvent( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	if ( ( m_eOrientation == Orientation_Vertical && ( a_iActions & 0x200 ) ) || ( m_eOrientation == Orientation_Horizontal && ( a_iActions & 0x1000 ) ) )
		return Navigate( a_pEvent, -1 );

	if ( ( m_eOrientation == Orientation_Vertical && ( a_iActions & 0x800 ) ) || ( m_eOrientation == Orientation_Horizontal && ( a_iActions & 0x400 ) ) )
		return Navigate( a_pEvent, 1 );

	if ( a_pEvent->GetDoodad() == 0x30009 )
		return Navigate( a_pEvent, -1 );

	if ( a_pEvent->GetDoodad() == 0x3000A )
		return Navigate( a_pEvent, 1 );

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
		PGUIRowData* pRow;
		TINT         iColumn;

		if ( IsInRect( m_pComponentCaches[ Component_Bucket ].GetBBox(), iX, iY ) && FindItemAtPoint( iX, iY, pRow, iColumn ) )
		{
			SelectItemImpl( pRow );
			RefreshItemList();
			PGUIPlaySound( GetScreen(), "PGUITableBox", "select" );

			ActionEvent oEvent;
			oEvent.pRow    = pRow;
			oEvent.iColumn = iColumn;
			oEvent.iSource = 0;
			m_oActionEmitter.Throw( &oEvent );
		}
	}

	return TTRUE;
}

// Moves the selection by one row (scrolling at the ends of the visible rows), or scrolls without selection
// $JPOG: FUNCTION PGUICore.dll+0001a2d0
TBOOL PGUITableBox::Navigate( TInputInterface::InputEvent* a_pEvent, TINT a_iDelta )
{
	if ( a_iDelta == -1 )
	{
		if ( m_pSelectedRow && m_pSelectedRow->Prev() && m_pSelectedRow->Prev() != GetRowsEnd() )
		{
			PGUIPlaySound( GetScreen(), "PGUITableBox", "decrement" );

			PGUIRowData* pPrev = m_pSelectedRow->Prev();

			if ( pPrev == m_pFirstVisibleRow && pPrev->Prev() && pPrev->Prev() != GetRowsEnd() )
				SetFirstItem( pPrev->Prev() );

			SelectItemImpl( m_pSelectedRow->Prev() );
			RefreshItemList();
			a_pEvent->StartRepeat( 0.25f, 0.1f );
		}
		else if ( m_pFirstVisibleRow && m_pFirstVisibleRow->Prev() && m_pFirstVisibleRow->Prev() != GetRowsEnd() )
		{
			PGUIPlaySound( GetScreen(), "PGUITableBox", "decrement" );
			SetFirstItem( m_pFirstVisibleRow->Prev() );
			RefreshItemList();
			a_pEvent->StartRepeat( 0.25f, 0.1f );
		}
		else
		{
			PGUIPlaySound( GetScreen(), "PGUITableBox", "decrement_fail" );
		}
	}
	else if ( a_iDelta == 1 )
	{
		if ( m_pSelectedRow && m_pSelectedRow->Next() && m_pSelectedRow->Next() != GetRowsEnd() )
		{
			PGUIPlaySound( GetScreen(), "PGUITableBox", "increment" );

			PGUIRowData* pNext = m_pSelectedRow->Next();

			if ( pNext == m_pLastVisibleRow && pNext->Next() && pNext->Next() != GetRowsEnd() )
				SetFirstItem( m_pFirstVisibleRow->Next() );

			SelectItemImpl( m_pSelectedRow->Next() );
			RefreshItemList();
			a_pEvent->StartRepeat( 0.25f, 0.1f );
		}
		else if ( m_pFirstVisibleRow && m_pLastVisibleRow->Next() && m_pLastVisibleRow->Next() != GetRowsEnd() )
		{
#ifdef BARNYARD_COMMUNITY_PATCH
			PGUIPlaySound( GetScreen(), "PGUITableBox", "increment" );
#else
			// The original plays the decrement sound when scrolling forward
			PGUIPlaySound( GetScreen(), "PGUITableBox", "decrement" );
#endif
			SetFirstItem( m_pFirstVisibleRow->Next() );
			RefreshItemList();
			a_pEvent->StartRepeat( 0.25f, 0.1f );
		}
		else
		{
			PGUIPlaySound( GetScreen(), "PGUITableBox", "increment_fail" );
		}
	}

	return TTRUE;
}

// $JPOG: FUNCTION PGUICore.dll+0001a990
void PGUITableBox::AddItemImpl( PGUIRowData* a_pRow )
{
	m_oRows.InsertTail( a_pRow );

	if ( !m_pFirstVisibleRow )
		m_pFirstVisibleRow = a_pRow;

	if ( !m_pSelectedRow )
		SelectItemImpl( a_pRow );
}

// $JPOG: FUNCTION PGUICore.dll+0001a9f0
void PGUITableBox::RemoveItemImpl( PGUIRowData* a_pRow )
{
	if ( m_pSelectedRow == a_pRow )
	{
		if ( a_pRow->Next() == GetRowsEnd() )
			SelectItemImpl( ( a_pRow->Prev() == GetRowsEnd() ) ? TNULL : a_pRow->Prev() );
		else
			SelectItemImpl( a_pRow->Next() );
	}

	if ( !( m_uiState & State_ScrollableForward ) && ( m_uiState & State_ScrollableBack ) && m_pFirstVisibleRow != a_pRow )
	{
		// Scrolled to the end: the table scrolls back by one
		m_pFirstVisibleRow = m_pFirstVisibleRow->Prev();
	}
	else if ( m_pFirstVisibleRow == a_pRow )
	{
		if ( a_pRow->Next() != GetRowsEnd() )
			m_pFirstVisibleRow = a_pRow->Next();
		else if ( a_pRow->Prev() != GetRowsEnd() )
			m_pFirstVisibleRow = a_pRow->Prev();
		else
			m_pFirstVisibleRow = TNULL;
	}

	m_oRows.Remove( a_pRow );

	if ( a_pRow )
		delete a_pRow;
}

// Updates the Selected state of the rows and the actions of the table box (enabled with a selection)
// $JPOG: FUNCTION PGUICore.dll+0001ab00
void PGUITableBox::SelectItemImpl( PGUIRowData* a_pRow )
{
	if ( m_pSelectedRow == a_pRow )
		return;

	if ( a_pRow == GetRowsEnd() )
		a_pRow = TNULL;

	if ( m_pSelectedRow )
		m_pSelectedRow->RemoveState( State_Selected );

	m_pSelectedRow = a_pRow;

	if ( a_pRow )
	{
		a_pRow->AddState( State_Selected );
		EnsureVisibleImpl( m_pSelectedRow );
	}

	for ( auto it = m_oActions.Begin(); it != m_oActions.End(); it++ )
		it->SetEnabled( m_pSelectedRow != TNULL );

	ItemChangeEvent oEvent;
	oEvent.pRow = m_pSelectedRow;
	m_oItemChangeEmitter.Throw( &oEvent );

	if ( TGUIScreen* pScreen = GetScreen() )
		pScreen->GetInputContext()->UpdateStatusDisplay();
}

// $JPOG: FUNCTION PGUICore.dll+0001abc0
void PGUITableBox::EnsureVisibleImpl( PGUIRowData* a_pRow )
{
	if ( IsItemVisible( a_pRow ) )
		return;

	if ( a_pRow != GetRowsEnd() )
	{
		for ( PGUIRowData* pRow = a_pRow; pRow != GetRowsEnd(); pRow = pRow->Next() )
		{
			if ( pRow == m_pFirstVisibleRow )
			{
				m_pFirstVisibleRow = a_pRow;
				return;
			}
		}
	}

	m_pFirstVisibleRow = a_pRow;

	for ( TINT i = 0; i < m_iNumVisibleRows - 1; i++ )
		m_pFirstVisibleRow = m_pFirstVisibleRow->Prev();
}

// $JPOG: FUNCTION PGUICore.dll+0001aea0
void PGUITableBox::RemoveAllItemsImpl()
{
	m_pFirstVisibleRow = TNULL;
	m_pLastVisibleRow  = TNULL;
	m_pSelectedRow     = TNULL;
	m_oRows.DeleteAll();

	SelectItemImpl( TNULL );
}

// $JPOG: FUNCTION PGUICore.dll+0001ac50
void PGUITableBox::InsertStampWidgets()
{
	if ( !m_ppStampWidgets )
		return;

	for ( TINT i = 0; i < m_iNumColumns; i++ )
		AddWidget( m_ppStampWidgets[ i ], TFALSE );
}

// The children after the headings become the stamp widgets; without them, clones of the headings are used (the
// headings are hidden)
// NOTE: with fewer children than columns, the original reads past the children
// $JPOG: FUNCTION PGUICore.dll+0001aca0
void PGUITableBox::RemoveStampWidgets()
{
	if ( !m_ppStampWidgets )
		return;

	TGUIWidget* pWidget = m_oChildren.Head().Get();

	if ( m_iNumColumns >= TINT( m_oChildren.Count() ) )
	{
		for ( TINT i = 0; i < m_iNumColumns; i++ )
		{
			m_ppStampWidgets[ i ] = pWidget->Clone();
			pWidget->SetFixedHeight( 1 );
			pWidget->SetState( State_Visible, 0u );
			pWidget = pWidget->Next();
		}
	}
	else
	{
		for ( TINT i = 0; i < m_iNumColumns; i++ )
		{
			m_piColumnWidths[ i ] = pWidget->GetWidth();
			pWidget               = pWidget->Next();
		}

		TGUIWidget* pLastHeader = pWidget->Prev();

		for ( TINT i = 0; i < m_iNumColumns; i++ )
		{
			TGUIWidget* pStamp = pLastHeader->Next();
			RemoveWidget( pStamp );
			m_ppStampWidgets[ i ] = pStamp;
		}
	}
}

// $JPOG: FUNCTION PGUICore.dll+0001af40
void PGUITableBox::SetFirstItem( PGUIRowData* a_pRow )
{
	m_pFirstVisibleRow = a_pRow;
}

// $JPOG: FUNCTION PGUICore.dll+0001b050
void PGUITableBox::Set_columns_property( const PPropertyValue& a_rcValue )
{
	m_iNumColumns = a_rcValue.GetInteger();
}

// $JPOG: FUNCTION PGUICore.dll+0001b030
PPropertyValue PGUITableBox::Get_columns_property() const
{
	return PPropertyValue( m_iNumColumns );
}

// $JPOG: FUNCTION PGUICore.dll+0001b010
TBOOL PGUITableBox::Val_columns_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_INT );
}

// $JPOG: FUNCTION PGUICore.dll+0001aff0
void PGUITableBox::Set_selectionmode_property( const PPropertyValue& a_rcValue )
{
	m_eSelectionMode = SelectionMode( a_rcValue.GetInteger() );
}

// $JPOG: FUNCTION PGUICore.dll+0001afd0
PPropertyValue PGUITableBox::Get_selectionmode_property() const
{
	return PPropertyValue( TINT( m_eSelectionMode ) );
}

// $JPOG: FUNCTION PGUICore.dll+0001afb0
TBOOL PGUITableBox::Val_selectionmode_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_INT );
}
