#include "ToshiPCH.h"
#include "PGUIHorizontalSplit.h"

#include <TGUI/TGUIManager.h>
#include <TGUI/TGUIWidgetInfo.h>
#include <TGUI/TGUIStyle.h>
#include <TGUI/TGUIPropertySpecSimple.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_INIT( PGUIHorizontalSplit );

TGUIDeserialisable::PropertySet* PGUIHorizontalSplit::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* PGUIHorizontalSplit::ms_oPropertySetOrdered = TNULL;
TGUIWidgetInfo*                  PGUIHorizontalSplit::ms_pWidgetInfo         = TNULL;

// NOTE: the category of the property specs (an argument the JPOG constructors don't have) is the name of the class
// $JPOG: FUNCTION PGUICore.dll+00006390
void PGUIHorizontalSplit::InitialiseClass()
{
	using t_Setter = TGUIPropertySpecSimple::t_Setter;
	using t_Getter = TGUIPropertySpecSimple::t_Getter;

	ms_pWidgetInfo = new TGUIWidgetInfo();
	CreatePropertySets( ms_oPropertySet, ms_oPropertySetOrdered, *PGUIPanel::ms_oPropertySet );

	TPString8 strCategory( "PGUIHorizontalSplit" );
	TINT      iSortHint = 0;
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "stretchtop" ), static_cast<t_Setter>( &PGUIHorizontalSplit::Set_stretchtop_property ), static_cast<t_Getter>( &PGUIHorizontalSplit::Get_stretchtop_property ), PPropertyValue::TYPE_FLOAT, strCategory, TTRUE, &TGetClass( PGUIHorizontalSplit ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "stretchbottom" ), static_cast<t_Setter>( &PGUIHorizontalSplit::Set_stretchbottom_property ), static_cast<t_Getter>( &PGUIHorizontalSplit::Get_stretchbottom_property ), PPropertyValue::TYPE_FLOAT, strCategory, TTRUE, &TGetClass( PGUIHorizontalSplit ), iSortHint++ ) );

	ms_pWidgetInfo->AddComponentName( TPString8( "background" ) );
	ms_pWidgetInfo->AddComponentName( TPString8( "splitter" ) );
	ms_pWidgetInfo->SetWidgetName( TPString8( "PGUIHorizontalSplit" ) );
	ms_pWidgetInfo->SetWidgetLabel( TPString8( "Horizontal Splitter" ) );
	ms_pWidgetInfo->SetProperties( ms_oPropertySet, ms_oPropertySetOrdered );
	ms_pWidgetInfo->AddStateVars( PGUIPanel::ms_pWidgetInfo->GetStateVarSet() );
	ms_pWidgetInfo->SetConcrete( TTRUE );

	TGUIManager::AddWidgetClass( ms_pWidgetInfo );
}

// $JPOG: FUNCTION PGUICore.dll+00006c30
void PGUIHorizontalSplit::DeinitialiseClass()
{
	ms_pWidgetInfo->Clear();
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );

	delete ms_pWidgetInfo;
	ms_pWidgetInfo = TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+00007390
PGUIHorizontalSplit::PGUIHorizontalSplit()
    : m_fStretchTop( 0.0f )
    , m_fStretchBottom( 0.0f )
{
}

// $JPOG: FUNCTION PGUICore.dll+00007230
PGUIHorizontalSplit::PGUIHorizontalSplit( const PGUIHorizontalSplit& a_rcOther )
    : PGUIPanel( a_rcOther )
    , m_fStretchTop( a_rcOther.m_fStretchTop )
    , m_fStretchBottom( a_rcOther.m_fStretchBottom )
{
}

// $JPOG: FUNCTION PGUICore.dll+00007380
PGUIHorizontalSplit::~PGUIHorizontalSplit()
{
}

// $JPOG: FUNCTION PGUICore.dll+00007320
TBOOL PGUIHorizontalSplit::Deserialise( const PProperties* a_pProperties )
{
	return PGUIPanel::Deserialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+000072f0
TBOOL PGUIHorizontalSplit::Serialise( PProperties* a_pProperties )
{
	return PGUIPanel::Serialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+000073e0
TGUIDeserialisable::PropertySet* PGUIHorizontalSplit::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $JPOG: FUNCTION PGUICore.dll+000073d0
TGUIDeserialisable::PropertySet* PGUIHorizontalSplit::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

const TGUIWidgetInfo* PGUIHorizontalSplit::GetWidgetInfo() const
{
	return ms_pWidgetInfo;
}

// $JPOG: FUNCTION PGUICore.dll+00007350
PGUIHorizontalSplit* PGUIHorizontalSplit::Clone() const
{
	return new PGUIHorizontalSplit( *this );
}

// The height left (the heights of the children are the minimum, 64 for a missing one) is shared by the stretch factors
// $JPOG: FUNCTION PGUICore.dll+00006cf0
void PGUIHorizontalSplit::Pack()
{
	TGUIWidget* pTop    = GetTopWidget();
	TGUIWidget* pBottom = GetBottomWidget();

	TINT iTopHeight    = pTop ? pTop->GetHeight() : 64;
	TINT iBottomHeight = pBottom ? pBottom->GetHeight() : 64;

	TINT iSplitterWidth  = 0;
	TINT iSplitterHeight = 0;

	if ( m_pStyle )
		m_pStyle->GetComponentPreferredSize( 1, m_pParameters, iSplitterWidth, iSplitterHeight );

	TINT   iSpace       = m_iHeight - ( iBottomHeight + iSplitterHeight + iTopHeight );
	TFLOAT fTotalStretch = m_fStretchTop + m_fStretchBottom;
	TINT   iSplit;

	if ( fTotalStretch == 0.0f )
		iSplit = iSpace / 2 + iTopHeight;
	else
		iSplit = TINT( TDOUBLE( TMath::FloorToInt( TFLOAT( m_fStretchTop / fTotalStretch * TDOUBLE( iSpace ) + TDOUBLE( iTopHeight ) ) ) ) );

	TINT iBottomSize = m_iHeight - iSplitterHeight - iSplit;

	if ( pTop )
	{
		pTop->SetSize( m_iWidth, iSplit );
		pTop->SetPosition( 0, 0 );
	}

	if ( pBottom )
	{
		pBottom->SetSize( m_iWidth, iBottomSize );
		pBottom->SetPosition( 0, iSplitterHeight + iSplit );
	}

	if ( m_pStyle )
	{
		SetComponentBBox( 0, 0, 0, m_iWidth, m_iHeight );
		SetComponentBBox( 1, 0, iSplit, m_iWidth, iSplitterHeight );
	}

	if ( pTop )
		pTop->Pack();

	if ( pBottom )
		pBottom->Pack();
}

// The width of the wider child, the heights of the children and of the splitter (0 while a child isn't laid out)
// $JPOG: FUNCTION PGUICore.dll+00006ed0
void PGUIHorizontalSplit::Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight )
{
	m_bLayoutValid = TTRUE;

	TGUIWidget* pTop    = GetTopWidget();
	TGUIWidget* pBottom = GetBottomWidget();

	TINT iTopWidth     = 0;
	TINT iBottomWidth  = 0;
	TINT iTopHeight    = 0;
	TINT iBottomHeight = 0;

	if ( pTop )
	{
		pTop->Layout( a_iMaxWidth, -1 );

		if ( pTop->IsLayoutValid() )
		{
			iTopWidth  = pTop->GetWidth();
			iTopHeight = pTop->GetHeight();
		}
		else
		{
			m_bLayoutValid = TFALSE;
		}
	}

	if ( pBottom )
	{
		pBottom->Layout( a_iMaxWidth, -1 );

		if ( pBottom->IsLayoutValid() )
		{
			iBottomWidth  = pBottom->GetWidth();
			iBottomHeight = pBottom->GetHeight();
		}
		else
		{
			m_bLayoutValid = TFALSE;
		}
	}

	if ( !m_bLayoutValid )
	{
		if ( m_eSizeModeHoriz != SizeMode_Fixed )
			SetWidth( 0 );

		if ( m_eSizeModeVert != SizeMode_Fixed )
			SetHeight( 0 );

		return;
	}

	TINT iSplitterWidth  = 0;
	TINT iSplitterHeight = 0;

	if ( m_pStyle )
		m_pStyle->GetComponentPreferredSize( 1, m_pParameters, iSplitterWidth, iSplitterHeight );

	TINT iWidth  = TMath::Max( iTopWidth, iBottomWidth );
	TINT iHeight = iBottomHeight + iTopHeight + iSplitterHeight;

	if ( pTop )
	{
		pTop->SetWidth( iWidth );
		pTop->SetPosition( 0, 0 );
	}

	if ( pBottom )
	{
		pBottom->SetWidth( iWidth );
		pBottom->SetPosition( 0, iSplitterHeight + iTopHeight );
	}

	SetSize( iWidth, iHeight );

	if ( m_pStyle )
	{
		SetComponentBBox( 0, 0, 0, iWidth, iHeight );
		SetComponentBBox( 1, 0, iTopHeight, iWidth, iSplitterHeight );
	}
}

// $JPOG: FUNCTION PGUICore.dll+000074a0
TGUIWidget* PGUIHorizontalSplit::GetTopWidget()
{
	return ( m_oChildren.Count() > 0 ) ? m_oChildren.Head().Get() : TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+00007480
TGUIWidget* PGUIHorizontalSplit::GetBottomWidget()
{
	return ( m_oChildren.Count() >= 2 ) ? m_oChildren.Head()->Next() : TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+000072d0
void PGUIHorizontalSplit::Set_stretchtop_property( const PPropertyValue& a_rcValue )
{
	m_fStretchTop = a_rcValue.GetFloat();
}

// $JPOG: FUNCTION PGUICore.dll+000072b0
PPropertyValue PGUIHorizontalSplit::Get_stretchtop_property() const
{
	return PPropertyValue( m_fStretchTop );
}

// $JPOG: FUNCTION PGUICore.dll+000070a0
TBOOL PGUIHorizontalSplit::Val_stretchtop_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_FLOAT ) && a_rcValue.GetFloat() >= 0.0f;
}

// $JPOG: FUNCTION PGUICore.dll+00007290
void PGUIHorizontalSplit::Set_stretchbottom_property( const PPropertyValue& a_rcValue )
{
	m_fStretchBottom = a_rcValue.GetFloat();
}

// $JPOG: FUNCTION PGUICore.dll+00007270
PPropertyValue PGUIHorizontalSplit::Get_stretchbottom_property() const
{
	return PPropertyValue( m_fStretchBottom );
}

// $JPOG: FUNCTION PGUICore.dll+000070f0
TBOOL PGUIHorizontalSplit::Val_stretchbottom_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_FLOAT ) && a_rcValue.GetFloat() >= 0.0f;
}
