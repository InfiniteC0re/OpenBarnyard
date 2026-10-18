#include "ToshiPCH.h"
#include "PGUIStackLayout.h"

#include <TGUI/TGUIManager.h>
#include <TGUI/TGUIWidget.h>
#include <TGUI/TGUIPropertySpecSimple.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_INIT( PGUIStackLayout );

TGUIDeserialisable::PropertySet* PGUIStackLayout::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* PGUIStackLayout::ms_oPropertySetOrdered = TNULL;
TGUILayoutInfo*                  PGUIStackLayout::ms_pLayoutInfo         = TNULL;

const TCHAR* const PGUIStackLayout::ms_csaFlowNameStr[ FlowDirection_NUMOF ] = { "horizontal", "vertical" };
const TCHAR* const PGUIStackLayout::ms_csaJustifyHorizNameStr[ 3 ]           = { "left", "center", "right" };
const TCHAR* const PGUIStackLayout::ms_csaJustifyVertNameStr[ 3 ]            = { "top", "center", "bottom" };

// The position of a child of size a_iSize justified in a_iSpace after a_iStart (a_iEnd: the far edge of the parent)
static TINT JustifyPosition( TGUILayoutManager::Alignment a_eAlignment, TINT a_iStart, TINT a_iEndMargin, TINT a_iParentSize, TINT a_iSpace, TINT a_iSize )
{
	if ( a_eAlignment == TGUILayoutManager::Alignment_Left )
		return a_iStart;

	if ( a_eAlignment == TGUILayoutManager::Alignment_Right )
		return a_iParentSize - a_iEndMargin - a_iSize;

	return a_iStart + ( a_iSpace - a_iSize ) / 2;
}

// NOTE: the category of the property specs (an argument the JPOG constructors don't have) is the name of the class
// $JPOG: FUNCTION PGUICore.dll+000251e0
void PGUIStackLayout::InitialiseClass()
{
	using t_Setter = TGUIPropertySpecSimple::t_Setter;
	using t_Getter = TGUIPropertySpecSimple::t_Getter;

	CreatePropertySets( ms_oPropertySet, ms_oPropertySetOrdered, *PGUILayoutManager::ms_oPropertySet );

	TPString8 strCategory( "PGUIStackLayout" );
	TINT      iSortHint = 0;
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "justifyhoriz" ), static_cast<t_Setter>( &PGUIStackLayout::Set_justifyhoriz_property ), static_cast<t_Getter>( &PGUIStackLayout::Get_justifyhoriz_property ), PPropertyValue::TYPE_INT, strCategory, TTRUE, &TGetClass( PGUIStackLayout ), iSortHint++, 3, ms_csaJustifyHorizNameStr ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "justifyvert" ), static_cast<t_Setter>( &PGUIStackLayout::Set_justifyvert_property ), static_cast<t_Getter>( &PGUIStackLayout::Get_justifyvert_property ), PPropertyValue::TYPE_INT, strCategory, TTRUE, &TGetClass( PGUIStackLayout ), iSortHint++, 3, ms_csaJustifyVertNameStr ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "flow" ), static_cast<t_Setter>( &PGUIStackLayout::Set_flow_property ), static_cast<t_Getter>( &PGUIStackLayout::Get_flow_property ), PPropertyValue::TYPE_INT, strCategory, TTRUE, &TGetClass( PGUIStackLayout ), iSortHint++, FlowDirection_NUMOF, ms_csaFlowNameStr ) );

	ms_pLayoutInfo = new TGUILayoutInfo( TPString8( "PGUIStackLayout" ), TPString8( "Stack Layout" ), ms_oPropertySet );
	TGUIManager::AddLayoutClass( ms_pLayoutInfo );
}

// $JPOG: FUNCTION PGUICore.dll+00025c20
void PGUIStackLayout::DeinitialiseClass()
{
	delete ms_pLayoutInfo;
	ms_pLayoutInfo = TNULL;

	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );
}

// $JPOG: FUNCTION PGUICore.dll+00026b60
PGUIStackLayout::PGUIStackLayout()
    : m_eFlowDirection( FlowDirection_Horizontal )
    , m_eJustifyHoriz( Alignment_Left )
    , m_eJustifyVert( Alignment_Top )
{
}

// $JPOG: FUNCTION PGUICore.dll+00026790
PGUIStackLayout::PGUIStackLayout( const PGUIStackLayout& a_rcOther )
    : PGUILayoutManager( a_rcOther )
    , m_eFlowDirection( a_rcOther.m_eFlowDirection )
    , m_eJustifyHoriz( a_rcOther.m_eJustifyHoriz )
    , m_eJustifyVert( a_rcOther.m_eJustifyVert )
{
}

// $JPOG: FUNCTION PGUICore.dll+00026b50
PGUIStackLayout::~PGUIStackLayout()
{
}

// $JPOG: FUNCTION PGUICore.dll+00026b00
TBOOL PGUIStackLayout::Deserialise( const PProperties* a_pProperties )
{
	return TGUILayoutManager::Deserialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+00026ae0
TBOOL PGUIStackLayout::Serialise( PProperties* a_pProperties )
{
	return TGUILayoutManager::Serialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+00026ba0
TGUIDeserialisable::PropertySet* PGUIStackLayout::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $JPOG: FUNCTION PGUICore.dll+00026b90
TGUIDeserialisable::PropertySet* PGUIStackLayout::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

// $JPOG: FUNCTION PGUICore.dll+00026b20
PGUIStackLayout* PGUIStackLayout::Clone() const
{
	return new PGUIStackLayout( *this );
}

// The size of the managed children side by side (with the padding and the margins of the widget)
// $JPOG: FUNCTION PGUICore.dll+00025ce0
void PGUIStackLayout::DoLayout( TGUIWidget* a_pWidget, TINT& a_rWidth, TINT& a_rHeight )
{
	if ( a_pWidget->GetChildCount() == 0 )
	{
		a_rWidth  = 0;
		a_rHeight = 0;
		return;
	}

	TINT iMarginLeft, iMarginRight, iMarginTop, iMarginBottom;
	a_pWidget->GetMargins( iMarginLeft, iMarginRight, iMarginTop, iMarginBottom );

	TINT iWidth      = 0;
	TINT iHeight     = 0;
	TINT iNumManaged = 0;
	TINT iMaxWidth   = 0;
	TINT iMaxHeight  = 0;

	for ( auto it = a_pWidget->GetChildren().Begin(); it != a_pWidget->GetChildren().End(); it++ )
	{
		if ( it->GetPlaceMode() == TGUIWidget::PlaceMode_Fixed )
			continue;

		iNumManaged++;

		TINT iChildWidth  = it->GetWidth();
		TINT iChildHeight = it->GetHeight();

		if ( iChildWidth < 0 || iChildHeight < 0 )
			continue;

		iMaxWidth  = TMath::Max( iMaxWidth, iChildWidth );
		iMaxHeight = TMath::Max( iMaxHeight, iChildHeight );

		if ( m_eFlowDirection != FlowDirection_Horizontal )
		{
			iWidth = TMath::Max( iWidth, iChildWidth );
			iHeight += iChildHeight;
		}
		else
		{
			iHeight = TMath::Max( iHeight, iChildHeight );
			iWidth += iChildWidth;
		}
	}

	if ( m_bSameWidth && m_eFlowDirection == FlowDirection_Horizontal )
		iWidth = iNumManaged * iMaxWidth;
	else if ( m_bSameHeight && m_eFlowDirection == FlowDirection_Vertical )
		iHeight = iNumManaged * iMaxHeight;

	a_rWidth  = iMarginRight + iMarginLeft + iWidth;
	a_rHeight = iMarginBottom + iMarginTop + iHeight;

	if ( m_eFlowDirection != FlowDirection_Horizontal )
		a_rHeight += m_iPadY * ( iNumManaged - 1 );
	else
		a_rWidth += m_iPadX * ( iNumManaged - 1 );
}

// Lays out the children that need the size of the widget (twice at most, with the space left shared between them),
// then places the managed children one after the other and packs all of them
// $JPOG: FUNCTION PGUICore.dll+00025f10
void PGUIStackLayout::DoPack( TGUIWidget* a_pWidget )
{
	if ( a_pWidget->GetChildCount() == 0 )
		return;

	TINT iMarginLeft, iMarginRight, iMarginTop, iMarginBottom;
	a_pWidget->GetMargins( iMarginLeft, iMarginRight, iMarginTop, iMarginBottom );

	const TINT  iWidgetWidth  = a_pWidget->GetWidth();
	const TINT  iWidgetHeight = a_pWidget->GetHeight();
	const TBOOL bSameSize     = ( m_bSameWidth && m_eFlowDirection == FlowDirection_Horizontal ) || ( m_bSameHeight && m_eFlowDirection == FlowDirection_Vertical );

	TINT iMaxWidth = 0, iMaxHeight = 0, iFixedWidth = 0, iFixedHeight = 0, iFreeWidth = 0, iFreeHeight = 0;
	TINT iNumFree = 0, iNumManaged = 0, iNumInvalid = 0;
	TINT iInteriorWidth, iInteriorHeight;
	TINT iSpaceWidth, iSpaceHeight;
	TINT iFreeExtraWidth  = 0;
	TINT iFreeExtraHeight = 0;

	// NOTE: the number of gaps counts the children that aren't managed too (as in the original)
	const TINT iNumGaps = TINT( a_pWidget->GetChildCount() ) - 1;

	for ( TINT iPass = 1;; iPass++ )
	{
		CalculateLayoutSizes( a_pWidget, bSameSize, iMaxWidth, iMaxHeight, iFixedWidth, iFixedHeight, iFreeWidth, iFreeHeight, iNumFree, iNumManaged, iNumInvalid );
		a_pWidget->GetInteriorSize( iInteriorWidth, iInteriorHeight );

		if ( iInteriorWidth < 0 ) iInteriorWidth = 0;
		if ( iInteriorHeight < 0 ) iInteriorHeight = 0;

		if ( m_eFlowDirection != FlowDirection_Horizontal )
			iInteriorHeight += iNumGaps * -m_iPadY;
		else
			iInteriorWidth += iNumGaps * -m_iPadX;

		// The size the children that need the size of the widget get
		TINT iInvalidSize = 0;
		iSpaceWidth       = iInteriorWidth - iFixedWidth - iFreeWidth;
		iSpaceHeight      = iInteriorHeight - iFixedHeight - iFreeHeight;

		if ( m_eFlowDirection != FlowDirection_Horizontal )
		{
			if ( iInteriorHeight >= iFixedHeight )
			{
				if ( iNumInvalid > 0 && iSpaceHeight > 0 )
				{
					iInvalidSize = iSpaceHeight / iNumInvalid;
					iSpaceHeight = 0;
				}
				else if ( m_bFillVertical == TTRUE && iSpaceHeight != 0 && iNumFree > 0 )
				{
					iFreeExtraHeight = iSpaceHeight / iNumFree;
					iSpaceHeight     = 0;
				}
			}

			if ( m_bSameHeight )
			{
#ifdef BARNYARD_COMMUNITY_PATCH
				// Fixes the same height being shared from the width
				iMaxHeight = iInteriorHeight / iNumManaged;
#else
				iMaxHeight = iInteriorWidth / iNumManaged;
#endif
			}
		}
		else
		{
			if ( iInteriorWidth >= iFixedWidth )
			{
				if ( iNumInvalid > 0 && iSpaceWidth > 0 )
				{
					iInvalidSize = iSpaceWidth / iNumInvalid;
					iSpaceWidth  = 0;
				}
				else if ( m_bFillHorizontal == TTRUE && iSpaceWidth != 0 && iNumFree > 0 )
				{
					iFreeExtraWidth = iSpaceWidth / iNumFree;
					iSpaceWidth     = 0;
				}
			}

			if ( m_bSameWidth )
				iMaxWidth = iInteriorWidth / iNumManaged;
		}

		if ( iNumInvalid == 0 )
			break;

		for ( auto it = a_pWidget->GetChildren().Begin(); it != a_pWidget->GetChildren().End(); it++ )
		{
			if ( it->GetPlaceMode() == TGUIWidget::PlaceMode_Fixed || it->IsLayoutValid() )
				continue;

			if ( m_eFlowDirection == FlowDirection_Horizontal && it->GetSizeModeHoriz() != TGUIWidget::SizeMode_Fixed )
				it->Layout( TINT16( iInvalidSize ), TINT16( iInteriorHeight ) );

			if ( m_eFlowDirection == FlowDirection_Vertical && it->GetSizeModeVert() != TGUIWidget::SizeMode_Fixed )
				it->Layout( TINT16( iInteriorWidth ), TINT16( iInvalidSize ) );
		}

		TINT iWidth, iHeight;
		DoLayout( a_pWidget, iWidth, iHeight );

		if ( a_pWidget->GetSizeModeHoriz() != TGUIWidget::SizeMode_Fixed )
			a_pWidget->SetWidth( iWidth );

		if ( a_pWidget->GetSizeModeVert() != TGUIWidget::SizeMode_Fixed )
			a_pWidget->SetHeight( iHeight );

		if ( iPass >= 2 )
			break;
	}

	// The start of the flow
	TINT iX = 0;
	TINT iY = 0;

	if ( m_eFlowDirection != FlowDirection_Horizontal )
	{
		iY = iMarginTop;

		if ( iSpaceHeight > 0 && m_eJustifyVert == Alignment_Bottom )
			iY += iSpaceHeight;
		else if ( iSpaceHeight > 0 && m_eJustifyVert != Alignment_Top )
			iY += iSpaceHeight / 2;
	}
	else
	{
		iX = iMarginLeft;

		if ( iSpaceWidth > 0 && m_eJustifyHoriz == Alignment_Right )
			iX += iSpaceWidth;
		else if ( iSpaceWidth > 0 && m_eJustifyHoriz != Alignment_Left )
			iX += iSpaceWidth / 2;
	}

	for ( auto it = a_pWidget->GetChildren().Begin(); it != a_pWidget->GetChildren().End(); it++ )
	{
		if ( it->GetPlaceMode() == TGUIWidget::PlaceMode_Fixed )
		{
			it->Pack();
			continue;
		}

		TINT        iChildWidth  = it->GetWidth();
		TINT        iChildHeight = it->GetHeight();
		const TBOOL bFreeWidth   = it->GetSizeModeHoriz() == TGUIWidget::SizeMode_Free;
		const TBOOL bFreeHeight  = it->GetSizeModeVert() == TGUIWidget::SizeMode_Free;

		if ( m_eFlowDirection != FlowDirection_Horizontal )
		{
			if ( bFreeHeight )
				iChildHeight = m_bSameHeight ? iMaxHeight : iChildHeight + iFreeExtraHeight;

			if ( bFreeWidth )
				iChildWidth = m_bSameWidth ? iMaxWidth : iInteriorWidth;

			iX = JustifyPosition( m_eJustifyHoriz, iMarginLeft, iMarginRight, iWidgetWidth, iInteriorWidth, iChildWidth );
		}
		else
		{
			if ( bFreeWidth )
				iChildWidth = m_bSameWidth ? iMaxWidth : iChildWidth + iFreeExtraWidth;

			if ( bFreeHeight )
				iChildHeight = m_bSameHeight ? iMaxHeight : iInteriorHeight;

			iY = JustifyPosition( m_eJustifyVert, iMarginTop, iMarginBottom, iWidgetHeight, iInteriorHeight, iChildHeight );
		}

		if ( bFreeWidth )
			it->SetWidth( iChildWidth );

		if ( bFreeHeight )
			it->SetHeight( iChildHeight );

		it->SetPosition( iX, iY );
		it->Pack();

		if ( m_eFlowDirection != FlowDirection_Horizontal )
			iY += m_iPadY + iChildHeight;
		else
			iX += m_iPadX + iChildWidth;
	}
}

// $JPOG: FUNCTION PGUICore.dll+000267d0
void PGUIStackLayout::CalculateLayoutSizes( TGUIWidget* a_pWidget, TBOOL a_bSameSize, TINT& a_rMaxWidth, TINT& a_rMaxHeight, TINT& a_rFixedWidth, TINT& a_rFixedHeight, TINT& a_rFreeWidth, TINT& a_rFreeHeight, TINT& a_rNumFree, TINT& a_rNumManaged, TINT& a_rNumInvalid )
{
	a_rMaxWidth     = 0;
	a_rMaxHeight    = 0;
	a_rFixedWidth   = 0;
	a_rFixedHeight  = 0;
	a_rFreeWidth    = 0;
	a_rFreeHeight   = 0;
	a_rNumFree      = 0;
	a_rNumManaged   = 0;
	a_rNumInvalid   = 0;

	for ( auto it = a_pWidget->GetChildren().Begin(); it != a_pWidget->GetChildren().End(); it++ )
	{
		if ( it->GetPlaceMode() == TGUIWidget::PlaceMode_Fixed )
			continue;

		a_rNumManaged++;

		TINT iChildWidth  = it->GetWidth();
		TINT iChildHeight = it->GetHeight();

		if ( !it->IsLayoutValid() )
			a_rNumInvalid++;

		a_rMaxWidth  = TMath::Max( iChildWidth, a_rMaxWidth );
		a_rMaxHeight = TMath::Max( iChildHeight, a_rMaxHeight );

		if ( a_bSameSize )
			continue;

		if ( m_eFlowDirection != FlowDirection_Horizontal )
		{
			if ( it->GetSizeModeVert() == TGUIWidget::SizeMode_Free )
			{
				a_rFreeHeight += iChildHeight;
				a_rNumFree++;
			}
			else
			{
				a_rFixedHeight += iChildHeight;
			}
		}
		else if ( it->GetSizeModeHoriz() == TGUIWidget::SizeMode_Free )
		{
			a_rFreeWidth += iChildWidth;
			a_rNumFree++;
		}
		else
		{
			a_rFixedWidth += iChildWidth;
		}
	}

	if ( a_bSameSize )
	{
		if ( m_eFlowDirection != FlowDirection_Horizontal )
			a_rFixedHeight = a_rMaxHeight * a_rNumManaged;
		else
			a_rFixedWidth = a_rMaxWidth * a_rNumManaged;
	}
}

// $JPOG: FUNCTION PGUICore.dll+00026a40
void PGUIStackLayout::Set_flow_property( const PPropertyValue& a_rcValue )
{
	m_eFlowDirection = FlowDirection( a_rcValue.GetInteger() );
}

// $JPOG: FUNCTION PGUICore.dll+00026a20
PPropertyValue PGUIStackLayout::Get_flow_property() const
{
	return PPropertyValue( TINT( m_eFlowDirection ) );
}

// $JPOG: FUNCTION PGUICore.dll+00026730
TBOOL PGUIStackLayout::Val_flow_property( const PPropertyValue& a_rcValue )
{
	if ( !a_rcValue.CanBeType( PPropertyValue::TYPE_INT ) )
		return TFALSE;

	return a_rcValue.GetInteger() == FlowDirection_Horizontal || a_rcValue.GetInteger() == FlowDirection_Vertical;
}

// $JPOG: FUNCTION PGUICore.dll+00026ac0
void PGUIStackLayout::Set_justifyhoriz_property( const PPropertyValue& a_rcValue )
{
	m_eJustifyHoriz = Alignment( a_rcValue.GetInteger() );
}

// $JPOG: FUNCTION PGUICore.dll+00026aa0
PPropertyValue PGUIStackLayout::Get_justifyhoriz_property() const
{
	return PPropertyValue( TINT( m_eJustifyHoriz ) );
}

// $JPOG: FUNCTION PGUICore.dll+00026670
TBOOL PGUIStackLayout::Val_justifyhoriz_property( const PPropertyValue& a_rcValue )
{
	if ( !a_rcValue.CanBeType( PPropertyValue::TYPE_INT ) )
		return TFALSE;

	return a_rcValue.GetInteger() == 0 || a_rcValue.GetInteger() == 1 || a_rcValue.GetInteger() == 2;
}

// $JPOG: FUNCTION PGUICore.dll+00026a80
void PGUIStackLayout::Set_justifyvert_property( const PPropertyValue& a_rcValue )
{
	m_eJustifyVert = Alignment( a_rcValue.GetInteger() );
}

// $JPOG: FUNCTION PGUICore.dll+00026a60
PPropertyValue PGUIStackLayout::Get_justifyvert_property() const
{
	return PPropertyValue( TINT( m_eJustifyVert ) );
}

// $JPOG: FUNCTION PGUICore.dll+000266d0
TBOOL PGUIStackLayout::Val_justifyvert_property( const PPropertyValue& a_rcValue )
{
	if ( !a_rcValue.CanBeType( PPropertyValue::TYPE_INT ) )
		return TFALSE;

	return a_rcValue.GetInteger() == 0 || a_rcValue.GetInteger() == 1 || a_rcValue.GetInteger() == 2;
}
