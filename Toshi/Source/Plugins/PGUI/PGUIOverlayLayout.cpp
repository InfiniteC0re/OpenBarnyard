#include "ToshiPCH.h"
#include "PGUIOverlayLayout.h"

#include <TGUI/TGUIManager.h>
#include <TGUI/TGUIWidget.h>
#include <TGUI/TGUIPropertySpecSimple.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_INIT( PGUIOverlayLayout );

TGUIDeserialisable::PropertySet* PGUIOverlayLayout::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* PGUIOverlayLayout::ms_oPropertySetOrdered = TNULL;
TGUILayoutInfo*                  PGUIOverlayLayout::ms_pLayoutInfo         = TNULL;

const TCHAR* const PGUIOverlayLayout::ms_csaJustifyHorizNameStr[ 3 ] = { "left", "center", "right" };
const TCHAR* const PGUIOverlayLayout::ms_csaJustifyVertNameStr[ 3 ]  = { "top", "center", "bottom" };

// NOTE: the category of the property specs (an argument the JPOG constructors don't have) is the name of the class
// $JPOG: FUNCTION PGUICore.dll+000234d0
void PGUIOverlayLayout::InitialiseClass()
{
	using t_Setter = TGUIPropertySpecSimple::t_Setter;
	using t_Getter = TGUIPropertySpecSimple::t_Getter;

	CreatePropertySets( ms_oPropertySet, ms_oPropertySetOrdered, *TGUILayoutManager::ms_oPropertySet );

	TPString8 strCategory( "PGUIOverlayLayout" );
	TINT      iSortHint = 0;
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "samewidth" ), static_cast<t_Setter>( &PGUIOverlayLayout::Set_samewidth_property ), static_cast<t_Getter>( &PGUIOverlayLayout::Get_samewidth_property ), PPropertyValue::TYPE_BOOL, strCategory, TTRUE, &TGetClass( PGUIOverlayLayout ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "sameheight" ), static_cast<t_Setter>( &PGUIOverlayLayout::Set_sameheight_property ), static_cast<t_Getter>( &PGUIOverlayLayout::Get_sameheight_property ), PPropertyValue::TYPE_BOOL, strCategory, TTRUE, &TGetClass( PGUIOverlayLayout ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "fillhorizontal" ), static_cast<t_Setter>( &PGUIOverlayLayout::Set_fillhorizontal_property ), static_cast<t_Getter>( &PGUIOverlayLayout::Get_fillhorizontal_property ), PPropertyValue::TYPE_BOOL, strCategory, TTRUE, &TGetClass( PGUIOverlayLayout ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "fillvertical" ), static_cast<t_Setter>( &PGUIOverlayLayout::Set_fillvertical_property ), static_cast<t_Getter>( &PGUIOverlayLayout::Get_fillvertical_property ), PPropertyValue::TYPE_BOOL, strCategory, TTRUE, &TGetClass( PGUIOverlayLayout ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "justifyhoriz" ), static_cast<t_Setter>( &PGUIOverlayLayout::Set_justifyhoriz_property ), static_cast<t_Getter>( &PGUIOverlayLayout::Get_justifyhoriz_property ), PPropertyValue::TYPE_INT, strCategory, TTRUE, &TGetClass( PGUIOverlayLayout ), iSortHint++, 3, ms_csaJustifyHorizNameStr ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "justifyvert" ), static_cast<t_Setter>( &PGUIOverlayLayout::Set_justifyvert_property ), static_cast<t_Getter>( &PGUIOverlayLayout::Get_justifyvert_property ), PPropertyValue::TYPE_INT, strCategory, TTRUE, &TGetClass( PGUIOverlayLayout ), iSortHint++, 3, ms_csaJustifyVertNameStr ) );

	ms_pLayoutInfo = new TGUILayoutInfo( TPString8( "PGUIOverlayLayout" ), TPString8( "Overlay Layout" ), ms_oPropertySet );
	TGUIManager::AddLayoutClass( ms_pLayoutInfo );
}

// $JPOG: FUNCTION PGUICore.dll+000246a0
void PGUIOverlayLayout::DeinitialiseClass()
{
	delete ms_pLayoutInfo;
	ms_pLayoutInfo = TNULL;

	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );
}

// $JPOG: FUNCTION PGUICore.dll+00024760
PGUIOverlayLayout::PGUIOverlayLayout()
    : m_bSameWidth( TFALSE )
    , m_bSameHeight( TFALSE )
    , m_bFillHorizontal( TTRUE )
    , m_bFillVertical( TTRUE )
    , m_eJustifyHoriz( Alignment_Left )
    , m_eJustifyVert( Alignment_Top )
{
}

// $JPOG: FUNCTION PGUICore.dll+00024d00
PGUIOverlayLayout::PGUIOverlayLayout( const PGUIOverlayLayout& a_rcOther )
    : TGUILayoutManager( a_rcOther )
    , m_bSameWidth( a_rcOther.m_bSameWidth )
    , m_bSameHeight( a_rcOther.m_bSameHeight )
    , m_bFillHorizontal( a_rcOther.m_bFillHorizontal )
    , m_bFillVertical( a_rcOther.m_bFillVertical )
    , m_eJustifyHoriz( a_rcOther.m_eJustifyHoriz )
    , m_eJustifyVert( a_rcOther.m_eJustifyVert )
{
}

// $JPOG: FUNCTION PGUICore.dll+00025050
PGUIOverlayLayout::~PGUIOverlayLayout()
{
}

// $JPOG: FUNCTION PGUICore.dll+00025000
TBOOL PGUIOverlayLayout::Deserialise( const PProperties* a_pProperties )
{
	return TGUILayoutManager::Deserialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+00024fe0
TBOOL PGUIOverlayLayout::Serialise( PProperties* a_pProperties )
{
	return TGUILayoutManager::Serialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+00025070
TGUIDeserialisable::PropertySet* PGUIOverlayLayout::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $JPOG: FUNCTION PGUICore.dll+00025060
TGUIDeserialisable::PropertySet* PGUIOverlayLayout::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

// $JPOG: FUNCTION PGUICore.dll+00025020
PGUIOverlayLayout* PGUIOverlayLayout::Clone() const
{
	return new PGUIOverlayLayout( *this );
}

// The size of the largest managed child (with the margins of the widget)
// NOTE: the original computes the largest size twice, the same size options don't change the result
// $JPOG: FUNCTION PGUICore.dll+000247a0
void PGUIOverlayLayout::DoLayout( TGUIWidget* a_pWidget, TINT& a_rWidth, TINT& a_rHeight )
{
	if ( a_pWidget->GetChildCount() == 0 )
	{
		a_rWidth  = 0;
		a_rHeight = 0;
		return;
	}

	TINT iMarginLeft, iMarginRight, iMarginTop, iMarginBottom;
	a_pWidget->GetMargins( iMarginLeft, iMarginRight, iMarginTop, iMarginBottom );

	TINT iMaxWidth  = 0;
	TINT iMaxHeight = 0;
	TINT iWidth     = 0;
	TINT iHeight    = 0;

	for ( auto it = a_pWidget->GetChildren().Begin(); it != a_pWidget->GetChildren().End(); it++ )
	{
		if ( it->GetPlaceMode() == TGUIWidget::PlaceMode_Fixed )
			continue;

		TINT iChildWidth  = it->GetWidth();
		TINT iChildHeight = it->GetHeight();

		if ( iChildWidth < 0 || iChildHeight < 0 )
			continue;

		iMaxWidth  = TMath::Max( iMaxWidth, iChildWidth );
		iMaxHeight = TMath::Max( iMaxHeight, iChildHeight );
		iHeight    = TMath::Max( iHeight, iChildHeight );
		iWidth     = TMath::Max( iWidth, iChildWidth );
	}

	if ( m_bSameWidth )
		iWidth = iMaxWidth;

	if ( m_bSameHeight )
		iHeight = iMaxHeight;

	a_rWidth  = iMarginRight + iMarginLeft + iWidth;
	a_rHeight = iMarginBottom + iMarginTop + iHeight;
}

// $JPOG: FUNCTION PGUICore.dll+00024940
void PGUIOverlayLayout::DoPack( TGUIWidget* a_pWidget )
{
	if ( a_pWidget->GetChildCount() == 0 )
		return;

	TINT iMarginLeft, iMarginRight, iMarginTop, iMarginBottom;
	a_pWidget->GetMargins( iMarginLeft, iMarginRight, iMarginTop, iMarginBottom );

	const TINT iWidgetWidth  = a_pWidget->GetWidth();
	const TINT iWidgetHeight = a_pWidget->GetHeight();

	TINT iInteriorWidth, iInteriorHeight;
	a_pWidget->GetInteriorSize( iInteriorWidth, iInteriorHeight );

	// The largest managed child, the others are packed
	TINT iMaxWidth  = 0;
	TINT iMaxHeight = 0;

	for ( auto it = a_pWidget->GetChildren().Begin(); it != a_pWidget->GetChildren().End(); it++ )
	{
		if ( it->GetPlaceMode() == TGUIWidget::PlaceMode_Fixed )
		{
			it->Pack();
			continue;
		}

		iMaxWidth  = TMath::Max( TINT( it->GetWidth() ), iMaxWidth );
		iMaxHeight = TMath::Max( TINT( it->GetHeight() ), iMaxHeight );
	}

	// NOTE: the children that aren't managed are packed again (as in the original)
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

		if ( bFreeHeight )
		{
			if ( m_bSameHeight )
				iChildHeight = iMaxHeight;
			else if ( m_bFillVertical )
				iChildHeight = iInteriorHeight;
			else if ( iChildHeight > iInteriorHeight )
				iChildHeight = iInteriorHeight;
		}

		TINT iY;

		if ( m_eJustifyVert == Alignment_Top )
			iY = iMarginTop;
		else if ( m_eJustifyVert == Alignment_Bottom )
			iY = iWidgetHeight - iMarginBottom - iChildHeight;
		else
			iY = iMarginTop + ( iInteriorHeight - iChildHeight ) / 2;

		if ( bFreeWidth )
		{
			if ( m_bSameWidth )
				iChildWidth = iMaxWidth;
			else if ( m_bFillHorizontal )
				iChildWidth = iInteriorWidth;
			else if ( iChildWidth > iInteriorWidth )
				iChildWidth = iInteriorWidth;
		}

		TINT iX;

		if ( m_eJustifyHoriz == Alignment_Left )
			iX = iMarginLeft;
		else if ( m_eJustifyHoriz == Alignment_Right )
			iX = iWidgetWidth - iMarginRight - iChildWidth;
		else
			iX = iMarginLeft + ( iInteriorWidth - iChildWidth ) / 2;

		if ( bFreeWidth )
			it->SetWidth( iChildWidth );

		if ( bFreeHeight )
			it->SetHeight( iChildHeight );

		it->SetPosition( iX, iY );
		it->Pack();
	}
}

// $JPOG: FUNCTION PGUICore.dll+00024fc0
void PGUIOverlayLayout::Set_samewidth_property( const PPropertyValue& a_rcValue )
{
	m_bSameWidth = a_rcValue.GetBoolean();
}

// $JPOG: FUNCTION PGUICore.dll+00024fa0
PPropertyValue PGUIOverlayLayout::Get_samewidth_property() const
{
	return PPropertyValue( m_bSameWidth );
}

// $JPOG: FUNCTION PGUICore.dll+00024f80
TBOOL PGUIOverlayLayout::Val_samewidth_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_BOOL );
}

// $JPOG: FUNCTION PGUICore.dll+00024f60
void PGUIOverlayLayout::Set_sameheight_property( const PPropertyValue& a_rcValue )
{
	m_bSameHeight = a_rcValue.GetBoolean();
}

// $JPOG: FUNCTION PGUICore.dll+00024f40
PPropertyValue PGUIOverlayLayout::Get_sameheight_property() const
{
	return PPropertyValue( m_bSameHeight );
}

// $JPOG: FUNCTION PGUICore.dll+00024f20
TBOOL PGUIOverlayLayout::Val_sameheight_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_BOOL );
}

// $JPOG: FUNCTION PGUICore.dll+00024f00
void PGUIOverlayLayout::Set_fillhorizontal_property( const PPropertyValue& a_rcValue )
{
	m_bFillHorizontal = a_rcValue.GetBoolean();
}

// $JPOG: FUNCTION PGUICore.dll+00024ee0
PPropertyValue PGUIOverlayLayout::Get_fillhorizontal_property() const
{
	return PPropertyValue( m_bFillHorizontal );
}

// $JPOG: FUNCTION PGUICore.dll+00024ec0
TBOOL PGUIOverlayLayout::Val_fillhorizontal_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_BOOL );
}

// $JPOG: FUNCTION PGUICore.dll+00024ea0
void PGUIOverlayLayout::Set_fillvertical_property( const PPropertyValue& a_rcValue )
{
	m_bFillVertical = a_rcValue.GetBoolean();
}

// $JPOG: FUNCTION PGUICore.dll+00024e80
PPropertyValue PGUIOverlayLayout::Get_fillvertical_property() const
{
	return PPropertyValue( m_bFillVertical );
}

// $JPOG: FUNCTION PGUICore.dll+00024e60
TBOOL PGUIOverlayLayout::Val_fillvertical_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_BOOL );
}

// $JPOG: FUNCTION PGUICore.dll+00024e40
void PGUIOverlayLayout::Set_justifyhoriz_property( const PPropertyValue& a_rcValue )
{
	m_eJustifyHoriz = Alignment( a_rcValue.GetInteger() );
}

// $JPOG: FUNCTION PGUICore.dll+00024e20
PPropertyValue PGUIOverlayLayout::Get_justifyhoriz_property() const
{
	return PPropertyValue( TINT( m_eJustifyHoriz ) );
}

// $JPOG: FUNCTION PGUICore.dll+00024c40
TBOOL PGUIOverlayLayout::Val_justifyhoriz_property( const PPropertyValue& a_rcValue )
{
	if ( !a_rcValue.CanBeType( PPropertyValue::TYPE_INT ) )
		return TFALSE;

	return a_rcValue.GetInteger() == 0 || a_rcValue.GetInteger() == 1 || a_rcValue.GetInteger() == 2;
}

// $JPOG: FUNCTION PGUICore.dll+00024e00
void PGUIOverlayLayout::Set_justifyvert_property( const PPropertyValue& a_rcValue )
{
	m_eJustifyVert = Alignment( a_rcValue.GetInteger() );
}

// $JPOG: FUNCTION PGUICore.dll+00024de0
PPropertyValue PGUIOverlayLayout::Get_justifyvert_property() const
{
	return PPropertyValue( TINT( m_eJustifyVert ) );
}

// $JPOG: FUNCTION PGUICore.dll+00024ca0
TBOOL PGUIOverlayLayout::Val_justifyvert_property( const PPropertyValue& a_rcValue )
{
	if ( !a_rcValue.CanBeType( PPropertyValue::TYPE_INT ) )
		return TFALSE;

	return a_rcValue.GetInteger() == 0 || a_rcValue.GetInteger() == 1 || a_rcValue.GetInteger() == 2;
}
