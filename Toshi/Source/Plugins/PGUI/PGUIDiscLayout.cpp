#include "ToshiPCH.h"
#include "PGUIDiscLayout.h"

#include <TGUI/TGUIManager.h>
#include <TGUI/TGUIWidget.h>
#include <TGUI/TGUIPropertySpecSimple.h>

#include <cmath>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_INIT( PGUIDiscLayout );

TGUIDeserialisable::PropertySet* PGUIDiscLayout::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* PGUIDiscLayout::ms_oPropertySetOrdered = TNULL;
TGUILayoutInfo*                  PGUIDiscLayout::ms_pLayoutInfo         = TNULL;

const TCHAR* const PGUIDiscLayout::ms_csaFlowDirectionNameStr[ FlowDirection_NUMOF ]   = { "clockwise", "anticlockwise" };
const TCHAR* const PGUIDiscLayout::ms_csaSlotAllocationNameStr[ SlotAllocation_NUMOF ] = { "sequential", "distributed" };

// NOTE: the category of the property specs (an argument the JPOG constructors don't have) is the name of the class
// $JPOG: FUNCTION PGUICore.dll+0001d5b0
void PGUIDiscLayout::InitialiseClass()
{
	using t_Setter = TGUIPropertySpecSimple::t_Setter;
	using t_Getter = TGUIPropertySpecSimple::t_Getter;

	CreatePropertySets( ms_oPropertySet, ms_oPropertySetOrdered, *TGUILayoutManager::ms_oPropertySet );

	TPString8 strCategory( "PGUIDiscLayout" );
	TINT      iSortHint = 0;
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "samewidth" ), static_cast<t_Setter>( &PGUIDiscLayout::Set_samewidth_property ), static_cast<t_Getter>( &PGUIDiscLayout::Get_samewidth_property ), PPropertyValue::TYPE_BOOL, strCategory, TTRUE, &TGetClass( PGUIDiscLayout ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "sameheight" ), static_cast<t_Setter>( &PGUIDiscLayout::Set_sameheight_property ), static_cast<t_Getter>( &PGUIDiscLayout::Get_sameheight_property ), PPropertyValue::TYPE_BOOL, strCategory, TTRUE, &TGetClass( PGUIDiscLayout ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "fullwidth" ), static_cast<t_Setter>( &PGUIDiscLayout::Set_fullwidth_property ), static_cast<t_Getter>( &PGUIDiscLayout::Get_fullwidth_property ), PPropertyValue::TYPE_BOOL, strCategory, TTRUE, &TGetClass( PGUIDiscLayout ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "fullheight" ), static_cast<t_Setter>( &PGUIDiscLayout::Set_fullheight_property ), static_cast<t_Getter>( &PGUIDiscLayout::Get_fullheight_property ), PPropertyValue::TYPE_BOOL, strCategory, TTRUE, &TGetClass( PGUIDiscLayout ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "radiusx" ), static_cast<t_Setter>( &PGUIDiscLayout::Set_radiusx_property ), static_cast<t_Getter>( &PGUIDiscLayout::Get_radiusx_property ), PPropertyValue::TYPE_FLOAT, strCategory, TTRUE, &TGetClass( PGUIDiscLayout ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "radiusy" ), static_cast<t_Setter>( &PGUIDiscLayout::Set_radiusy_property ), static_cast<t_Getter>( &PGUIDiscLayout::Get_radiusy_property ), PPropertyValue::TYPE_FLOAT, strCategory, TTRUE, &TGetClass( PGUIDiscLayout ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "offsetangle" ), static_cast<t_Setter>( &PGUIDiscLayout::Set_offsetangle_property ), static_cast<t_Getter>( &PGUIDiscLayout::Get_offsetangle_property ), PPropertyValue::TYPE_FLOAT, strCategory, TTRUE, &TGetClass( PGUIDiscLayout ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "slotcount" ), static_cast<t_Setter>( &PGUIDiscLayout::Set_slotcount_property ), static_cast<t_Getter>( &PGUIDiscLayout::Get_slotcount_property ), PPropertyValue::TYPE_INT, strCategory, TTRUE, &TGetClass( PGUIDiscLayout ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "flowdirection" ), static_cast<t_Setter>( &PGUIDiscLayout::Set_flowdirection_property ), static_cast<t_Getter>( &PGUIDiscLayout::Get_flowdirection_property ), PPropertyValue::TYPE_INT, strCategory, TTRUE, &TGetClass( PGUIDiscLayout ), iSortHint++, FlowDirection_NUMOF, ms_csaFlowDirectionNameStr ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "slotallocation" ), static_cast<t_Setter>( &PGUIDiscLayout::Set_slotallocation_property ), static_cast<t_Getter>( &PGUIDiscLayout::Get_slotallocation_property ), PPropertyValue::TYPE_INT, strCategory, TTRUE, &TGetClass( PGUIDiscLayout ), iSortHint++, SlotAllocation_NUMOF, ms_csaSlotAllocationNameStr ) );

	ms_pLayoutInfo = new TGUILayoutInfo( TPString8( "PGUIDiscLayout" ), TPString8( "Disc Layout" ), ms_oPropertySet );
	TGUIManager::AddLayoutClass( ms_pLayoutInfo );
}

// $JPOG: FUNCTION PGUICore.dll+0001f1d0
void PGUIDiscLayout::DeinitialiseClass()
{
	delete ms_pLayoutInfo;
	ms_pLayoutInfo = TNULL;

	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );
}

// $JPOG: FUNCTION PGUICore.dll+0001f290
PGUIDiscLayout::PGUIDiscLayout()
    : m_bSameWidth( TFALSE )
    , m_bSameHeight( TFALSE )
    , m_bFullWidth( TFALSE )
    , m_bFullHeight( TFALSE )
    , m_fRadiusX( 100.0f )
    , m_fRadiusY( 100.0f )
    , m_fOffsetAngle( 0.0f )
    , m_iSlotCount( 0 )
    , m_eFlowDirection( FlowDirection_Clockwise )
    , m_eSlotAllocation( SlotAllocation_Sequential )
{
}

// $JPOG: FUNCTION PGUICore.dll+0001f840
PGUIDiscLayout::PGUIDiscLayout( const PGUIDiscLayout& a_rcOther )
    : TGUILayoutManager( a_rcOther )
    , m_bSameWidth( a_rcOther.m_bSameWidth )
    , m_bSameHeight( a_rcOther.m_bSameHeight )
    , m_bFullWidth( a_rcOther.m_bFullWidth )
    , m_bFullHeight( a_rcOther.m_bFullHeight )
    , m_fRadiusX( a_rcOther.m_fRadiusX )
    , m_fRadiusY( a_rcOther.m_fRadiusY )
    , m_fOffsetAngle( a_rcOther.m_fOffsetAngle )
    , m_iSlotCount( a_rcOther.m_iSlotCount )
    , m_eFlowDirection( a_rcOther.m_eFlowDirection )
    , m_eSlotAllocation( a_rcOther.m_eSlotAllocation )
{
}

// $JPOG: FUNCTION PGUICore.dll+00020400
PGUIDiscLayout::~PGUIDiscLayout()
{
}

// $JPOG: FUNCTION PGUICore.dll+000203b0
TBOOL PGUIDiscLayout::Deserialise( const PProperties* a_pProperties )
{
	return TGUILayoutManager::Deserialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+00020390
TBOOL PGUIDiscLayout::Serialise( PProperties* a_pProperties )
{
	return TGUILayoutManager::Serialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+00020420
TGUIDeserialisable::PropertySet* PGUIDiscLayout::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $JPOG: FUNCTION PGUICore.dll+00020410
TGUIDeserialisable::PropertySet* PGUIDiscLayout::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

// $JPOG: FUNCTION PGUICore.dll+000203d0
PGUIDiscLayout* PGUIDiscLayout::Clone() const
{
	return new PGUIDiscLayout( *this );
}

// $JPOG: FUNCTION PGUICore.dll+0001f2e0
void PGUIDiscLayout::DoLayout( TGUIWidget* a_pWidget, TINT& a_rWidth, TINT& a_rHeight )
{
	if ( a_pWidget->GetChildCount() == 0 )
	{
		a_rWidth  = 0;
		a_rHeight = 0;
		return;
	}

	TGUIRectangle oRect( 0, 0, 0, 0 );
	TINT          iMaxWidth    = 0;
	TINT          iMaxHeight   = 0;
	TINT          iNumChildren = 0;
	CalculateLayoutSizes( a_pWidget, oRect, iMaxWidth, iMaxHeight, iNumChildren );

	a_rWidth  = oRect.m_iWidth;
	a_rHeight = oRect.m_iHeight;
}

// Centres the managed children on their slots (relative to the bounding box of the disc) and packs all of them
// $JPOG: FUNCTION PGUICore.dll+0001f360
void PGUIDiscLayout::DoPack( TGUIWidget* a_pWidget )
{
	if ( a_pWidget->GetChildCount() == 0 )
		return;

	TINT iMarginLeft, iMarginRight, iMarginTop, iMarginBottom;
	a_pWidget->GetMargins( iMarginLeft, iMarginRight, iMarginTop, iMarginBottom );

	TINT iInteriorWidth, iInteriorHeight;
	a_pWidget->GetInteriorSize( iInteriorWidth, iInteriorHeight );

	TINT          iMaxWidth    = 0;
	TINT          iMaxHeight   = 0;
	TINT          iNumChildren = 0;
	TGUIRectangle oRect( 0, 0, 0, 0 );
	CalculateLayoutSizes( a_pWidget, oRect, iMaxWidth, iMaxHeight, iNumChildren );

#ifdef BARNYARD_COMMUNITY_PATCH
	// Fixes the managed children being counted twice (the slots are halved without a slot count)
	iNumChildren = 0;
#endif

	for ( auto it = a_pWidget->GetChildren().Begin(); it != a_pWidget->GetChildren().End(); it++ )
	{
		if ( it->GetPlaceMode() == TGUIWidget::PlaceMode_Fixed )
			continue;

		iMaxWidth  = TMath::Max( TINT( it->GetWidth() ), iMaxWidth );
		iMaxHeight = TMath::Max( TINT( it->GetHeight() ), iMaxHeight );

		if ( ++iNumChildren > m_iSlotCount && m_iSlotCount > 0 )
			break;
	}

	TFLOAT fSlotAngle = TMath::TWO_PI / TFLOAT( ( m_iSlotCount > 0 ) ? m_iSlotCount : iNumChildren );

	if ( m_eFlowDirection == FlowDirection_AntiClockwise )
		fSlotAngle = -fSlotAngle;

	TINT iSlot = 0;

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

		if ( bFreeWidth )
		{
			if ( m_bSameWidth )
				iChildWidth = iMaxWidth;
			else if ( iChildWidth > iInteriorWidth )
				iChildWidth = iInteriorWidth;
		}

		if ( it->GetSizeModeVert() == TGUIWidget::SizeMode_Free )
		{
			if ( m_bSameHeight )
				iChildHeight = iMaxHeight;
			else if ( iChildHeight > iInteriorHeight )
				iChildHeight = iInteriorHeight;
		}

		TDOUBLE dAngle = TDOUBLE( iSlot ) * fSlotAngle + m_fOffsetAngle;
		TDOUBLE dX     = TDOUBLE( iChildWidth ) * -0.5f + TDOUBLE( -oRect.m_iX ) + cos( dAngle ) * m_fRadiusX;
		TDOUBLE dY     = sin( dAngle ) * m_fRadiusY + TDOUBLE( -oRect.m_iY ) + -0.5f * TDOUBLE( iChildHeight );

		if ( bFreeWidth )
			it->SetWidth( iChildWidth );

		if ( it->GetSizeModeVert() == TGUIWidget::SizeMode_Free )
			it->SetHeight( iChildHeight );

		it->SetPosition( TINT( dX ), TINT( dY ) );
		it->Pack();
		iSlot++;
	}
}

// NOTE: the children that aren't managed are packed here, the count of the managed children stops one after the slot
// count (as in the original)
// $JPOG: FUNCTION PGUICore.dll+0001f8b0
void PGUIDiscLayout::CalculateLayoutSizes( TGUIWidget* a_pWidget, TGUIRectangle& a_rRect, TINT& a_rMaxWidth, TINT& a_rMaxHeight, TINT& a_rNumChildren ) const
{
	TINT iMarginLeft, iMarginRight, iMarginTop, iMarginBottom;
	a_pWidget->GetMargins( iMarginLeft, iMarginRight, iMarginTop, iMarginBottom );

	TINT iMaxWidth    = 0;
	TINT iMaxHeight   = 0;
	TINT iNumChildren = 0;

	for ( auto it = a_pWidget->GetChildren().Begin(); it != a_pWidget->GetChildren().End(); it++ )
	{
		if ( it->GetPlaceMode() == TGUIWidget::PlaceMode_Fixed )
			continue;

		iMaxWidth  = TMath::Max( TINT( it->GetWidth() ), iMaxWidth );
		iMaxHeight = TMath::Max( TINT( it->GetHeight() ), iMaxHeight );

		if ( ++iNumChildren > m_iSlotCount && m_iSlotCount > 0 )
			break;
	}

	TFLOAT fSlotAngle = TMath::TWO_PI / TFLOAT( ( m_iSlotCount > 0 ) ? m_iSlotCount : iNumChildren );

	if ( m_eFlowDirection == FlowDirection_AntiClockwise )
		fSlotAngle = -fSlotAngle;

	// The bounding box of the children centred on their slots
	TFLOAT fMinX  = TMath::MAXFLOAT;
	TFLOAT fMaxX  = -TMath::MAXFLOAT;
	TFLOAT fMinY  = TMath::MAXFLOAT;
	TFLOAT fMaxY  = -TMath::MAXFLOAT;
	TINT   iSlot  = 0;

	for ( auto it = a_pWidget->GetChildren().Begin(); it != a_pWidget->GetChildren().End(); it++ )
	{
		if ( it->GetPlaceMode() == TGUIWidget::PlaceMode_Fixed )
		{
			it->Pack();
			continue;
		}

		TINT iChildWidth  = it->GetWidth();
		TINT iChildHeight = it->GetHeight();

		if ( it->GetSizeModeHoriz() == TGUIWidget::SizeMode_Free && m_bSameWidth )
			iChildWidth = iMaxWidth;

		if ( it->GetSizeModeVert() == TGUIWidget::SizeMode_Free && m_bSameHeight )
			iChildHeight = iMaxHeight;

		TDOUBLE dAngle   = TDOUBLE( iSlot ) * fSlotAngle + m_fOffsetAngle;
		TDOUBLE dSin     = sin( dAngle );
		TFLOAT  fCentreX = TFLOAT( cos( dAngle ) * m_fRadiusX );
		TFLOAT  fCentreY = TFLOAT( dSin * m_fRadiusY );

		fMinX = TMath::Min( fMinX, TFLOAT( -0.5f * TDOUBLE( iChildWidth ) + fCentreX ) );
		fMaxX = TMath::Max( fMaxX, TFLOAT( fCentreX + TDOUBLE( iChildWidth ) * 0.5f ) );
		fMinY = TMath::Min( fMinY, TFLOAT( -0.5f * TDOUBLE( iChildHeight ) + fCentreY ) );
		fMaxY = TMath::Max( fMaxY, TFLOAT( fCentreY + TDOUBLE( iChildHeight ) * 0.5f ) );
		iSlot++;
	}

	// The empty slots with the full size options
	for ( ; iSlot < m_iSlotCount; iSlot++ )
	{
		TDOUBLE dAngle   = TDOUBLE( iSlot ) * fSlotAngle + m_fOffsetAngle;
		TFLOAT  fCentreX = TFLOAT( cos( dAngle ) * m_fRadiusX );
		TFLOAT  fCentreY = TFLOAT( sin( dAngle ) * m_fRadiusY );

		if ( m_bFullWidth )
		{
			TFLOAT fHalfWidth = TFLOAT( TDOUBLE( iMaxWidth ) * 0.5f );
			fMinX             = TMath::Min( fMinX, fCentreX - fHalfWidth );
			fMaxX             = TMath::Max( fMaxX, fCentreX + fHalfWidth );
		}

		if ( m_bFullHeight )
		{
			TFLOAT fHalfHeight = TFLOAT( TDOUBLE( iMaxHeight ) * 0.5f );
			fMinY              = TMath::Min( fMinY, fCentreY - fHalfHeight );
			fMaxY              = TMath::Max( fMaxY, fCentreY + fHalfHeight );
		}
	}

	a_rRect.m_iX      = TINT( fMinX ) - iMarginLeft;
	a_rRect.m_iY      = TINT( fMinY ) - iMarginTop;
	a_rRect.m_iWidth  = TINT( fMaxX - fMinX ) + iMarginRight + iMarginLeft;
	a_rRect.m_iHeight = TINT( fMaxY - fMinY ) + iMarginBottom + iMarginTop;
	a_rMaxWidth       = iMaxWidth;
	a_rMaxHeight      = iMaxHeight;
	a_rNumChildren    = iNumChildren;
}

// $JPOG: FUNCTION PGUICore.dll+00020370
void PGUIDiscLayout::Set_samewidth_property( const PPropertyValue& a_rcValue )
{
	m_bSameWidth = a_rcValue.GetBoolean();
}

// $JPOG: FUNCTION PGUICore.dll+00020350
PPropertyValue PGUIDiscLayout::Get_samewidth_property() const
{
	return PPropertyValue( m_bSameWidth );
}

// $JPOG: FUNCTION PGUICore.dll+00020330
TBOOL PGUIDiscLayout::Val_samewidth_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_BOOL );
}

// $JPOG: FUNCTION PGUICore.dll+00020310
void PGUIDiscLayout::Set_sameheight_property( const PPropertyValue& a_rcValue )
{
	m_bSameHeight = a_rcValue.GetBoolean();
}

// $JPOG: FUNCTION PGUICore.dll+000202f0
PPropertyValue PGUIDiscLayout::Get_sameheight_property() const
{
	return PPropertyValue( m_bSameHeight );
}

// $JPOG: FUNCTION PGUICore.dll+000202d0
TBOOL PGUIDiscLayout::Val_sameheight_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_BOOL );
}

// $JPOG: FUNCTION PGUICore.dll+000202b0
void PGUIDiscLayout::Set_fullwidth_property( const PPropertyValue& a_rcValue )
{
	m_bFullWidth = a_rcValue.GetBoolean();
}

// $JPOG: FUNCTION PGUICore.dll+00020290
PPropertyValue PGUIDiscLayout::Get_fullwidth_property() const
{
	return PPropertyValue( m_bFullWidth );
}

// $JPOG: FUNCTION PGUICore.dll+00020270
TBOOL PGUIDiscLayout::Val_fullwidth_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_BOOL );
}

// $JPOG: FUNCTION PGUICore.dll+00020250
void PGUIDiscLayout::Set_fullheight_property( const PPropertyValue& a_rcValue )
{
	m_bFullHeight = a_rcValue.GetBoolean();
}

// $JPOG: FUNCTION PGUICore.dll+00020230
PPropertyValue PGUIDiscLayout::Get_fullheight_property() const
{
	return PPropertyValue( m_bFullHeight );
}

// $JPOG: FUNCTION PGUICore.dll+00020210
TBOOL PGUIDiscLayout::Val_fullheight_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_BOOL );
}

// $JPOG: FUNCTION PGUICore.dll+000201f0
void PGUIDiscLayout::Set_radiusx_property( const PPropertyValue& a_rcValue )
{
	m_fRadiusX = a_rcValue.GetFloat();
}

// $JPOG: FUNCTION PGUICore.dll+000201d0
PPropertyValue PGUIDiscLayout::Get_radiusx_property() const
{
	return PPropertyValue( m_fRadiusX );
}

// $JPOG: FUNCTION PGUICore.dll+000201b0
TBOOL PGUIDiscLayout::Val_radiusx_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_FLOAT );
}

// $JPOG: FUNCTION PGUICore.dll+00020190
void PGUIDiscLayout::Set_radiusy_property( const PPropertyValue& a_rcValue )
{
	m_fRadiusY = a_rcValue.GetFloat();
}

// $JPOG: FUNCTION PGUICore.dll+00020170
PPropertyValue PGUIDiscLayout::Get_radiusy_property() const
{
	return PPropertyValue( m_fRadiusY );
}

// $JPOG: FUNCTION PGUICore.dll+00020150
TBOOL PGUIDiscLayout::Val_radiusy_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_FLOAT );
}

// $JPOG: FUNCTION PGUICore.dll+00020120
void PGUIDiscLayout::Set_offsetangle_property( const PPropertyValue& a_rcValue )
{
	m_fOffsetAngle = a_rcValue.GetFloat() * ( TMath::PI * ( 1.0f / 180.0f ) );
}

// $JPOG: FUNCTION PGUICore.dll+000200f0
PPropertyValue PGUIDiscLayout::Get_offsetangle_property() const
{
	return PPropertyValue( TFLOAT( 180.0f / TMath::PI * m_fOffsetAngle ) );
}

// $JPOG: FUNCTION PGUICore.dll+000200d0
TBOOL PGUIDiscLayout::Val_offsetangle_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_FLOAT );
}

// $JPOG: FUNCTION PGUICore.dll+00020030
void PGUIDiscLayout::Set_slotcount_property( const PPropertyValue& a_rcValue )
{
	m_iSlotCount = a_rcValue.GetInteger();
}

// $JPOG: FUNCTION PGUICore.dll+00020010
PPropertyValue PGUIDiscLayout::Get_slotcount_property() const
{
	return PPropertyValue( m_iSlotCount );
}

// $JPOG: FUNCTION PGUICore.dll+0001fff0
TBOOL PGUIDiscLayout::Val_slotcount_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_INT );
}

// $JPOG: FUNCTION PGUICore.dll+000200b0
void PGUIDiscLayout::Set_flowdirection_property( const PPropertyValue& a_rcValue )
{
	m_eFlowDirection = FlowDirection( a_rcValue.GetInteger() );
}

// $JPOG: FUNCTION PGUICore.dll+00020090
PPropertyValue PGUIDiscLayout::Get_flowdirection_property() const
{
	return PPropertyValue( TINT( m_eFlowDirection ) );
}

// $JPOG: FUNCTION PGUICore.dll+0001f780
TBOOL PGUIDiscLayout::Val_flowdirection_property( const PPropertyValue& a_rcValue )
{
	if ( !a_rcValue.CanBeType( PPropertyValue::TYPE_INT ) )
		return TFALSE;

	return a_rcValue.GetInteger() == FlowDirection_Clockwise || a_rcValue.GetInteger() == FlowDirection_AntiClockwise;
}

// $JPOG: FUNCTION PGUICore.dll+00020070
void PGUIDiscLayout::Set_slotallocation_property( const PPropertyValue& a_rcValue )
{
	m_eSlotAllocation = SlotAllocation( a_rcValue.GetInteger() );
}

// $JPOG: FUNCTION PGUICore.dll+00020050
PPropertyValue PGUIDiscLayout::Get_slotallocation_property() const
{
	return PPropertyValue( TINT( m_eSlotAllocation ) );
}

// $JPOG: FUNCTION PGUICore.dll+0001f7e0
TBOOL PGUIDiscLayout::Val_slotallocation_property( const PPropertyValue& a_rcValue )
{
	if ( !a_rcValue.CanBeType( PPropertyValue::TYPE_INT ) )
		return TFALSE;

	return a_rcValue.GetInteger() == SlotAllocation_Sequential || a_rcValue.GetInteger() == SlotAllocation_Distributed;
}
