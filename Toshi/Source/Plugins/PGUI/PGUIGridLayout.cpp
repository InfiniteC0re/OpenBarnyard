#include "ToshiPCH.h"
#include "PGUIGridLayout.h"

#include <TGUI/TGUIManager.h>
#include <TGUI/TGUIWidget.h>
#include <TGUI/TGUIPropertySpecSimple.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_INIT( PGUIGridLayout );

TGUIDeserialisable::PropertySet* PGUIGridLayout::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* PGUIGridLayout::ms_oPropertySetOrdered = TNULL;
TGUILayoutInfo*                  PGUIGridLayout::ms_pLayoutInfo         = TNULL;

// NOTE: the category of the property specs (an argument the JPOG constructors don't have) is the name of the class
// $JPOG: FUNCTION PGUICore.dll+00020510
void PGUIGridLayout::InitialiseClass()
{
	using t_Setter = TGUIPropertySpecSimple::t_Setter;
	using t_Getter = TGUIPropertySpecSimple::t_Getter;

	CreatePropertySets( ms_oPropertySet, ms_oPropertySetOrdered, *PGUILayoutManager::ms_oPropertySet );

	TPString8 strCategory( "PGUIGridLayout" );
	TINT      iSortHint = 0;
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "columns" ), static_cast<t_Setter>( &PGUIGridLayout::Set_columns_property ), static_cast<t_Getter>( &PGUIGridLayout::Get_columns_property ), PPropertyValue::TYPE_INT, strCategory, TTRUE, &TGetClass( PGUIGridLayout ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "rows" ), static_cast<t_Setter>( &PGUIGridLayout::Set_rows_property ), static_cast<t_Getter>( &PGUIGridLayout::Get_rows_property ), PPropertyValue::TYPE_INT, strCategory, TTRUE, &TGetClass( PGUIGridLayout ), iSortHint++ ) );

	ms_pLayoutInfo = new TGUILayoutInfo( TPString8( "PGUIGridLayout" ), TPString8( "Grid Layout" ), ms_oPropertySet );
	TGUIManager::AddLayoutClass( ms_pLayoutInfo );
}

// $JPOG: FUNCTION PGUICore.dll+00020c90
void PGUIGridLayout::DeinitialiseClass()
{
	delete ms_pLayoutInfo;
	ms_pLayoutInfo = TNULL;

	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );
}

// $JPOG: FUNCTION PGUICore.dll+00021c90
PGUIGridLayout::PGUIGridLayout()
    : m_iNumRows( 1 )
    , m_iNumColumns( 1 )
{
}

// $JPOG: FUNCTION PGUICore.dll+00021b90
PGUIGridLayout::PGUIGridLayout( const PGUIGridLayout& a_rcOther )
    : PGUILayoutManager( a_rcOther )
    , m_iNumRows( a_rcOther.m_iNumRows )
    , m_iNumColumns( a_rcOther.m_iNumColumns )
{
}

// $JPOG: FUNCTION PGUICore.dll+00021c80
PGUIGridLayout::~PGUIGridLayout()
{
}

// $JPOG: FUNCTION PGUICore.dll+00021c30
TBOOL PGUIGridLayout::Deserialise( const PProperties* a_pProperties )
{
	return TGUILayoutManager::Deserialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+00021c10
TBOOL PGUIGridLayout::Serialise( PProperties* a_pProperties )
{
	return TGUILayoutManager::Serialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+00021cd0
TGUIDeserialisable::PropertySet* PGUIGridLayout::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $JPOG: FUNCTION PGUICore.dll+00021cc0
TGUIDeserialisable::PropertySet* PGUIGridLayout::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

// $JPOG: FUNCTION PGUICore.dll+00021c50
PGUIGridLayout* PGUIGridLayout::Clone() const
{
	return new PGUIGridLayout( *this );
}

// The size of the grid: the sum of the rows and the columns (the largest child for all of them with the same size
// options), with the padding and the margins of the widget
// $JPOG: FUNCTION PGUICore.dll+00020d50
void PGUIGridLayout::DoLayout( TGUIWidget* a_pWidget, TINT& a_rWidth, TINT& a_rHeight )
{
	if ( a_pWidget->GetChildCount() == 0 )
		return;

	TINT iMarginLeft, iMarginRight, iMarginTop, iMarginBottom;
	a_pWidget->GetMargins( iMarginLeft, iMarginRight, iMarginTop, iMarginBottom );

	TINT16 aRowHeights[ MAX_ROWS ];
	TINT16 aColumnWidths[ MAX_COLUMNS ];

	for ( TINT i = 0; i < m_iNumRows; i++ )
		aRowHeights[ i ] = 0;

	for ( TINT i = 0; i < m_iNumColumns; i++ )
		aColumnWidths[ i ] = 0;

	TINT iMaxWidth  = 0;
	TINT iMaxHeight = 0;
	TINT iRow       = 0;
	TINT iColumn    = 0;

	for ( auto it = a_pWidget->GetChildren().Begin(); it != a_pWidget->GetChildren().End(); it++ )
	{
		if ( it->GetPlaceMode() == TGUIWidget::PlaceMode_Fixed )
			continue;

		if ( it->IsLayoutValid() == TTRUE )
		{
			TINT iChildWidth  = it->GetWidth();
			TINT iChildHeight = it->GetHeight();

			aRowHeights[ iRow ]      = TINT16( TMath::Max( TINT( aRowHeights[ iRow ] ), iChildHeight ) );
			aColumnWidths[ iColumn ] = TINT16( TMath::Max( TINT( aColumnWidths[ iColumn ] ), iChildWidth ) );
			iMaxHeight               = TMath::Max( iMaxHeight, iChildHeight );
			iMaxWidth                = TMath::Max( iMaxWidth, iChildWidth );
		}

		if ( ++iColumn >= m_iNumColumns )
		{
			iColumn = 0;

			if ( ++iRow >= m_iNumRows )
				break;
		}
	}

	TINT iWidth  = 0;
	TINT iHeight = 0;

	if ( m_bSameWidth )
	{
		iWidth = iMaxWidth * m_iNumColumns;
	}
	else
	{
		for ( TINT i = 0; i < m_iNumColumns; i++ )
			iWidth += aColumnWidths[ i ];
	}

	if ( m_bSameHeight )
	{
		iHeight = iMaxHeight * m_iNumRows;
	}
	else
	{
		for ( TINT i = 0; i < m_iNumRows; i++ )
			iHeight += aRowHeights[ i ];
	}

	a_rWidth  = iWidth + iMarginRight + iMarginLeft + ( m_iNumColumns - 1 ) * m_iPadX;
	a_rHeight = iHeight + iMarginBottom + iMarginTop + ( m_iNumRows - 1 ) * m_iPadY;
}

// Shares the interior of the widget between the rows and the columns, then places, lays out (if needed) and packs the
// children in their cells
// $JPOG: FUNCTION PGUICore.dll+000210a0
void PGUIGridLayout::DoPack( TGUIWidget* a_pWidget )
{
	if ( a_pWidget->GetChildCount() == 0 || m_iNumRows == 0 || m_iNumColumns == 0 )
		return;

	TINT iMarginLeft, iMarginRight, iMarginTop, iMarginBottom;
	a_pWidget->GetMargins( iMarginLeft, iMarginRight, iMarginTop, iMarginBottom );

	Cell aRows[ MAX_ROWS ];
	Cell aColumns[ MAX_COLUMNS ];

	for ( TINT i = 0; i < m_iNumRows; i++ )
	{
		aRows[ i ].iSize     = 0;
		aRows[ i ].iSizeMode = TGUIWidget::SizeMode_Preferred;
	}

	for ( TINT i = 0; i < m_iNumColumns; i++ )
	{
		aColumns[ i ].iSize     = 0;
		aColumns[ i ].iSizeMode = TGUIWidget::SizeMode_Preferred;
	}

	// The sizes of the rows and the columns, a fixed child fixes its row/column, a free one makes it free (unless fixed)
	TINT iMaxWidth  = 0;
	TINT iMaxHeight = 0;
	TINT iRow       = 0;
	TINT iColumn    = 0;

	for ( auto it = a_pWidget->GetChildren().Begin(); it != a_pWidget->GetChildren().End(); it++ )
	{
		if ( it->GetPlaceMode() == TGUIWidget::PlaceMode_Fixed )
			continue;

		if ( it->IsLayoutValid() == TTRUE )
		{
			TINT iChildWidth  = it->GetWidth();
			TINT iChildHeight = it->GetHeight();

			aRows[ iRow ].iSize       = TINT16( TMath::Max( TINT( aRows[ iRow ].iSize ), iChildHeight ) );
			aColumns[ iColumn ].iSize = TINT16( TMath::Max( TINT( aColumns[ iColumn ].iSize ), iChildWidth ) );

			TGUIWidget::SizeMode eSizeModeHoriz = it->GetSizeModeHoriz();
			TGUIWidget::SizeMode eSizeModeVert  = it->GetSizeModeVert();

			if ( aRows[ iRow ].iSizeMode != eSizeModeVert )
			{
				if ( eSizeModeVert == TGUIWidget::SizeMode_Fixed )
					aRows[ iRow ].iSizeMode = TGUIWidget::SizeMode_Fixed;
				else if ( eSizeModeVert == TGUIWidget::SizeMode_Free && aRows[ iRow ].iSizeMode != TGUIWidget::SizeMode_Fixed )
					aRows[ iRow ].iSizeMode = TGUIWidget::SizeMode_Free;
			}

			if ( aColumns[ iColumn ].iSizeMode != eSizeModeHoriz )
			{
				if ( eSizeModeHoriz == TGUIWidget::SizeMode_Fixed )
					aColumns[ iColumn ].iSizeMode = TGUIWidget::SizeMode_Fixed;
				else if ( eSizeModeHoriz == TGUIWidget::SizeMode_Free && aColumns[ iColumn ].iSizeMode != TGUIWidget::SizeMode_Fixed )
					aColumns[ iColumn ].iSizeMode = TGUIWidget::SizeMode_Free;
			}

			iMaxHeight = TMath::Max( iMaxHeight, iChildHeight );
			iMaxWidth  = TMath::Max( iMaxWidth, iChildWidth );
		}

		if ( ++iColumn >= m_iNumColumns )
		{
			iColumn = 0;

			if ( ++iRow >= m_iNumRows )
				break;
		}
	}

	TINT iWidth  = 0;
	TINT iHeight = 0;

	if ( m_bSameWidth )
	{
		iWidth = m_iNumColumns * iMaxWidth;

		for ( TINT i = 0; i < m_iNumColumns; i++ )
			aColumns[ i ].iSize = TINT16( iMaxWidth );
	}
	else
	{
		for ( TINT i = 0; i < m_iNumColumns; i++ )
			iWidth += aColumns[ i ].iSize;
	}

	if ( m_bSameHeight )
	{
		iHeight = m_iNumRows * iMaxHeight;

		for ( TINT i = 0; i < m_iNumRows; i++ )
			aRows[ i ].iSize = TINT16( iMaxHeight );
	}
	else
	{
		for ( TINT i = 0; i < m_iNumRows; i++ )
			iHeight += aRows[ i ].iSize;
	}

	if ( iWidth <= 0 || iHeight <= 0 )
		return;

	TINT iSpaceWidth  = a_pWidget->GetInteriorWidth() - m_iPadX * ( m_iNumColumns - 1 );
	TINT iSpaceHeight = a_pWidget->GetInteriorHeight() - ( m_iNumRows - 1 ) * m_iPadY;

	// The rows: the free ones share the space left, or all shrink proportionally (the last one gets the rest)
	if ( m_bFillVertical == TTRUE && iSpaceHeight > iHeight )
	{
		TINT iNumFree = 0;

		for ( TINT i = 0; i < m_iNumRows; i++ )
			if ( aRows[ i ].iSizeMode == TGUIWidget::SizeMode_Free ) iNumFree++;

		if ( iNumFree > 0 )
		{
			TINT iExtra = ( iSpaceHeight - iHeight ) / iNumFree;

			for ( TINT i = 0; i < m_iNumRows; i++ )
				if ( aRows[ i ].iSizeMode == TGUIWidget::SizeMode_Free ) aRows[ i ].iSize += TINT16( iExtra );
		}
	}
	else if ( iSpaceHeight < iHeight )
	{
		TINT    iTotal  = 0;
		TDOUBLE dFactor = TDOUBLE( iSpaceHeight ) / TDOUBLE( iHeight );

		for ( TINT i = 0; i < m_iNumRows - 1; i++ )
		{
			TINT iSize = TINT( TDOUBLE( aRows[ i ].iSize ) * dFactor );
			iTotal += iSize;
			aRows[ i ].iSize = TINT16( iSize );
		}

		aRows[ m_iNumRows - 1 ].iSize = TINT16( iSpaceHeight - iTotal );
	}

	// The columns
	if ( m_bFillHorizontal == TTRUE && iSpaceWidth > iWidth )
	{
		TINT iNumFree = 0;

		for ( TINT i = 0; i < m_iNumColumns; i++ )
			if ( aColumns[ i ].iSizeMode == TGUIWidget::SizeMode_Free ) iNumFree++;

		if ( iNumFree > 0 )
		{
			for ( TINT i = 0; i < m_iNumColumns; i++ )
				if ( aColumns[ i ].iSizeMode == TGUIWidget::SizeMode_Free ) aColumns[ i ].iSize += TINT16( ( iSpaceWidth - iWidth ) / iNumFree );
		}
	}
	else if ( iSpaceWidth < iWidth )
	{
		TINT    iTotal  = 0;
		TDOUBLE dFactor = TDOUBLE( iSpaceWidth ) / TDOUBLE( iWidth );

		for ( TINT i = 0; i < m_iNumColumns - 1; i++ )
		{
			TINT iSize = TINT( TDOUBLE( aColumns[ i ].iSize ) * dFactor );
			iTotal += iSize;
			aColumns[ i ].iSize = TINT16( iSize );
		}

		aColumns[ m_iNumColumns - 1 ].iSize = TINT16( iSpaceWidth - iTotal );
	}

	// The cells
	TINT iX = iMarginLeft;
	TINT iY = iMarginTop;
	iRow    = 0;
	iColumn = 0;

	for ( auto it = a_pWidget->GetChildren().Begin(); it != a_pWidget->GetChildren().End(); it++ )
	{
		if ( it->GetPlaceMode() == TGUIWidget::PlaceMode_Fixed )
		{
			it->Pack();
			continue;
		}

		if ( it->GetSizeModeHoriz() == TGUIWidget::SizeMode_Free )
			it->SetWidth( aColumns[ iColumn ].iSize );

		if ( it->GetSizeModeVert() == TGUIWidget::SizeMode_Free )
			it->SetHeight( aRows[ iRow ].iSize );

		it->SetPosition( iX, iY );

		if ( !it->IsLayoutValid() )
			it->Layout( aColumns[ iColumn ].iSize, aRows[ iRow ].iSize );

		it->Pack();

		iX += aColumns[ iColumn ].iSize + m_iPadX;

		if ( ++iColumn >= m_iNumColumns )
		{
			iX = iMarginLeft;
			iY += aRows[ iRow ].iSize + m_iPadY;
			iColumn = 0;

			if ( ++iRow >= m_iNumRows )
				return;
		}
	}
}

// Clamped to 1-16
// $JPOG: FUNCTION PGUICore.dll+000219e0
void PGUIGridLayout::Set_columns_property( const PPropertyValue& a_rcValue )
{
	m_iNumColumns = TINT16( TMath::Min( TMath::Max( a_rcValue.GetInteger(), 1 ), MAX_COLUMNS ) );
}

// $JPOG: FUNCTION PGUICore.dll+00021bf0
PPropertyValue PGUIGridLayout::Get_columns_property() const
{
	return PPropertyValue( TINT( m_iNumColumns ) );
}

// $JPOG: FUNCTION PGUICore.dll+00021a30
TBOOL PGUIGridLayout::Val_columns_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_INT ) && a_rcValue.GetInteger() > 0;
}

// Clamped to 1-64
// $JPOG: FUNCTION PGUICore.dll+00021a70
void PGUIGridLayout::Set_rows_property( const PPropertyValue& a_rcValue )
{
	m_iNumRows = TINT16( TMath::Min( TMath::Max( a_rcValue.GetInteger(), 1 ), MAX_ROWS ) );
}

// $JPOG: FUNCTION PGUICore.dll+00021bd0
PPropertyValue PGUIGridLayout::Get_rows_property() const
{
	return PPropertyValue( TINT( m_iNumRows ) );
}

// $JPOG: FUNCTION PGUICore.dll+00021ac0
TBOOL PGUIGridLayout::Val_rows_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_INT ) && a_rcValue.GetInteger() > 0;
}
