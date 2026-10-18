#include "ToshiPCH.h"
#include "PGUILayoutManager.h"

#include <TGUI/TGUIPropertySpecSimple.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

// NOTE: abstract, CreateObject returns TNULL in JPOG
TDEFINE_CLASS_INIT_NORUNTIME( PGUILayoutManager );

TGUIDeserialisable::PropertySet* PGUILayoutManager::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* PGUILayoutManager::ms_oPropertySetOrdered = TNULL;

// NOTE: the category of the property specs (an argument the JPOG constructors don't have) is the name of the class
// $JPOG: FUNCTION PGUICore.dll+00021e00
void PGUILayoutManager::InitialiseClass()
{
	using t_Setter = TGUIPropertySpecSimple::t_Setter;
	using t_Getter = TGUIPropertySpecSimple::t_Getter;

	CreatePropertySets( ms_oPropertySet, ms_oPropertySetOrdered, *TGUILayoutManager::ms_oPropertySet );

	TPString8 strCategory( "PGUILayoutManager" );
	TINT      iSortHint = 0;
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "padx" ), static_cast<t_Setter>( &PGUILayoutManager::Set_padx_property ), static_cast<t_Getter>( &PGUILayoutManager::Get_padx_property ), PPropertyValue::TYPE_INT, strCategory, TTRUE, &TGetClass( PGUILayoutManager ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "pady" ), static_cast<t_Setter>( &PGUILayoutManager::Set_pady_property ), static_cast<t_Getter>( &PGUILayoutManager::Get_pady_property ), PPropertyValue::TYPE_INT, strCategory, TTRUE, &TGetClass( PGUILayoutManager ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "samewidth" ), static_cast<t_Setter>( &PGUILayoutManager::Set_samewidth_property ), static_cast<t_Getter>( &PGUILayoutManager::Get_samewidth_property ), PPropertyValue::TYPE_BOOL, strCategory, TTRUE, &TGetClass( PGUILayoutManager ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "sameheight" ), static_cast<t_Setter>( &PGUILayoutManager::Set_sameheight_property ), static_cast<t_Getter>( &PGUILayoutManager::Get_sameheight_property ), PPropertyValue::TYPE_BOOL, strCategory, TTRUE, &TGetClass( PGUILayoutManager ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "fillhorizontal" ), static_cast<t_Setter>( &PGUILayoutManager::Set_fillhorizontal_property ), static_cast<t_Getter>( &PGUILayoutManager::Get_fillhorizontal_property ), PPropertyValue::TYPE_BOOL, strCategory, TTRUE, &TGetClass( PGUILayoutManager ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "fillvertical" ), static_cast<t_Setter>( &PGUILayoutManager::Set_fillvertical_property ), static_cast<t_Getter>( &PGUILayoutManager::Get_fillvertical_property ), PPropertyValue::TYPE_BOOL, strCategory, TTRUE, &TGetClass( PGUILayoutManager ), iSortHint++ ) );
}

// $JPOG: FUNCTION PGUICore.dll+00022fb0
void PGUILayoutManager::DeinitialiseClass()
{
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );
}

// $JPOG: FUNCTION PGUICore.dll+00023070
PGUILayoutManager::PGUILayoutManager()
    : m_iPadX( 0 )
    , m_iPadY( 0 )
    , m_bSameWidth( TFALSE )
    , m_bSameHeight( TFALSE )
    , m_bFillHorizontal( TTRUE )
    , m_bFillVertical( TTRUE )
{
}

// $JPOG: FUNCTION PGUICore.dll+00023130
PGUILayoutManager::PGUILayoutManager( const PGUILayoutManager& a_rcOther )
    : TGUILayoutManager( a_rcOther )
    , m_iPadX( a_rcOther.m_iPadX )
    , m_iPadY( a_rcOther.m_iPadY )
    , m_bSameWidth( a_rcOther.m_bSameWidth )
    , m_bSameHeight( a_rcOther.m_bSameHeight )
    , m_bFillHorizontal( a_rcOther.m_bFillHorizontal )
    , m_bFillVertical( a_rcOther.m_bFillVertical )
{
}

// $JPOG: FUNCTION PGUICore.dll+000233f0
PGUILayoutManager::~PGUILayoutManager()
{
}

// $JPOG: FUNCTION PGUICore.dll+00023410
TGUIDeserialisable::PropertySet* PGUILayoutManager::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $JPOG: FUNCTION PGUICore.dll+00023400
TGUIDeserialisable::PropertySet* PGUILayoutManager::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

// $JPOG: FUNCTION PGUICore.dll+000233d0
void PGUILayoutManager::Set_padx_property( const PPropertyValue& a_rcValue )
{
	m_iPadX = TINT16( a_rcValue.GetInteger() );
}

// $JPOG: FUNCTION PGUICore.dll+000233b0
PPropertyValue PGUILayoutManager::Get_padx_property() const
{
	return PPropertyValue( TINT( m_iPadX ) );
}

// $JPOG: FUNCTION PGUICore.dll+000230b0
TBOOL PGUILayoutManager::Val_padx_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_INT ) && a_rcValue.GetInteger() >= 0;
}

// $JPOG: FUNCTION PGUICore.dll+00023390
void PGUILayoutManager::Set_pady_property( const PPropertyValue& a_rcValue )
{
	m_iPadY = TINT16( a_rcValue.GetInteger() );
}

// $JPOG: FUNCTION PGUICore.dll+00023370
PPropertyValue PGUILayoutManager::Get_pady_property() const
{
	return PPropertyValue( TINT( m_iPadY ) );
}

// $JPOG: FUNCTION PGUICore.dll+000230f0
TBOOL PGUILayoutManager::Val_pady_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_INT ) && a_rcValue.GetInteger() >= 0;
}

// $JPOG: FUNCTION PGUICore.dll+00023350
void PGUILayoutManager::Set_samewidth_property( const PPropertyValue& a_rcValue )
{
	m_bSameWidth = a_rcValue.GetBoolean();
}

// $JPOG: FUNCTION PGUICore.dll+00023330
PPropertyValue PGUILayoutManager::Get_samewidth_property() const
{
	return PPropertyValue( m_bSameWidth );
}

// $JPOG: FUNCTION PGUICore.dll+00023310
TBOOL PGUILayoutManager::Val_samewidth_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_BOOL );
}

// $JPOG: FUNCTION PGUICore.dll+000232f0
void PGUILayoutManager::Set_sameheight_property( const PPropertyValue& a_rcValue )
{
	m_bSameHeight = a_rcValue.GetBoolean();
}

// $JPOG: FUNCTION PGUICore.dll+000232d0
PPropertyValue PGUILayoutManager::Get_sameheight_property() const
{
	return PPropertyValue( m_bSameHeight );
}

// $JPOG: FUNCTION PGUICore.dll+000232b0
TBOOL PGUILayoutManager::Val_sameheight_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_BOOL );
}

// $JPOG: FUNCTION PGUICore.dll+00023290
void PGUILayoutManager::Set_fillhorizontal_property( const PPropertyValue& a_rcValue )
{
	m_bFillHorizontal = a_rcValue.GetBoolean();
}

// $JPOG: FUNCTION PGUICore.dll+00023270
PPropertyValue PGUILayoutManager::Get_fillhorizontal_property() const
{
	return PPropertyValue( m_bFillHorizontal );
}

// $JPOG: FUNCTION PGUICore.dll+00023250
TBOOL PGUILayoutManager::Val_fillhorizontal_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_BOOL );
}

// $JPOG: FUNCTION PGUICore.dll+00023230
void PGUILayoutManager::Set_fillvertical_property( const PPropertyValue& a_rcValue )
{
	m_bFillVertical = a_rcValue.GetBoolean();
}

// $JPOG: FUNCTION PGUICore.dll+00023210
PPropertyValue PGUILayoutManager::Get_fillvertical_property() const
{
	return PPropertyValue( m_bFillVertical );
}

// $JPOG: FUNCTION PGUICore.dll+000231f0
TBOOL PGUILayoutManager::Val_fillvertical_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_BOOL );
}
