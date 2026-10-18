#include "ToshiPCH.h"
#include "PGUIVerticalSplit.h"

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

TDEFINE_CLASS_INIT( PGUIVerticalSplit );

TGUIDeserialisable::PropertySet* PGUIVerticalSplit::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* PGUIVerticalSplit::ms_oPropertySetOrdered = TNULL;
TGUIWidgetInfo*                  PGUIVerticalSplit::ms_pWidgetInfo         = TNULL;

// NOTE: the category of the property specs (an argument the JPOG constructors don't have) is the name of the class
// $JPOG: FUNCTION PGUICore.dll+0001c460
void PGUIVerticalSplit::InitialiseClass()
{
	using t_Setter = TGUIPropertySpecSimple::t_Setter;
	using t_Getter = TGUIPropertySpecSimple::t_Getter;

	ms_pWidgetInfo = new TGUIWidgetInfo();
	CreatePropertySets( ms_oPropertySet, ms_oPropertySetOrdered, *PGUIPanel::ms_oPropertySet );

	TPString8 strCategory( "PGUIVerticalSplit" );
	TINT      iSortHint = 0;
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "stretchleft" ), static_cast<t_Setter>( &PGUIVerticalSplit::Set_stretchleft_property ), static_cast<t_Getter>( &PGUIVerticalSplit::Get_stretchleft_property ), PPropertyValue::TYPE_FLOAT, strCategory, TTRUE, &TGetClass( PGUIVerticalSplit ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "stretchright" ), static_cast<t_Setter>( &PGUIVerticalSplit::Set_stretchright_property ), static_cast<t_Getter>( &PGUIVerticalSplit::Get_stretchright_property ), PPropertyValue::TYPE_FLOAT, strCategory, TTRUE, &TGetClass( PGUIVerticalSplit ), iSortHint++ ) );

	ms_pWidgetInfo->AddComponentName( TPString8( "background" ) );
	ms_pWidgetInfo->AddComponentName( TPString8( "splitter" ) );
	ms_pWidgetInfo->SetWidgetName( TPString8( "PGUIVerticalSplit" ) );
	ms_pWidgetInfo->SetWidgetLabel( TPString8( "Vertical Splitter" ) );
	ms_pWidgetInfo->SetProperties( ms_oPropertySet, ms_oPropertySetOrdered );
	ms_pWidgetInfo->AddStateVars( PGUIPanel::ms_pWidgetInfo->GetStateVarSet() );
	ms_pWidgetInfo->SetConcrete( TTRUE );

	TGUIManager::AddWidgetClass( ms_pWidgetInfo );
}

// $JPOG: FUNCTION PGUICore.dll+0001cd10
void PGUIVerticalSplit::DeinitialiseClass()
{
	ms_pWidgetInfo->Clear();
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );

	delete ms_pWidgetInfo;
	ms_pWidgetInfo = TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+0001d400
PGUIVerticalSplit::PGUIVerticalSplit()
    : m_fStretchLeft( 0.0f )
    , m_fStretchRight( 0.0f )
{
}

// $JPOG: FUNCTION PGUICore.dll+0001d2b0
PGUIVerticalSplit::PGUIVerticalSplit( const PGUIVerticalSplit& a_rcOther )
    : PGUIPanel( a_rcOther )
    , m_fStretchLeft( a_rcOther.m_fStretchLeft )
    , m_fStretchRight( a_rcOther.m_fStretchRight )
{
}

// $JPOG: FUNCTION PGUICore.dll+0001d3f0
PGUIVerticalSplit::~PGUIVerticalSplit()
{
}

// $JPOG: FUNCTION PGUICore.dll+0001d3a0
TBOOL PGUIVerticalSplit::Deserialise( const PProperties* a_pProperties )
{
	return TGUIWidget::Deserialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+0001d370
TBOOL PGUIVerticalSplit::Serialise( PProperties* a_pProperties )
{
	return PGUIPanel::Serialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+0001d450
TGUIDeserialisable::PropertySet* PGUIVerticalSplit::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $JPOG: FUNCTION PGUICore.dll+0001d440
TGUIDeserialisable::PropertySet* PGUIVerticalSplit::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

const TGUIWidgetInfo* PGUIVerticalSplit::GetWidgetInfo() const
{
	return ms_pWidgetInfo;
}

// $JPOG: FUNCTION PGUICore.dll+0001d3c0
PGUIVerticalSplit* PGUIVerticalSplit::Clone() const
{
	return new PGUIVerticalSplit( *this );
}

// The width left (the widths of the children are the minimum, 64 for a missing one) is shared by the stretch factors
// $JPOG: FUNCTION PGUICore.dll+0001cdd0
void PGUIVerticalSplit::Pack()
{
	TGUIWidget* pLeft  = GetLeftWidget();
	TGUIWidget* pRight = GetRightWidget();

	TINT iLeftWidth  = pLeft ? pLeft->GetWidth() : 64;
	TINT iRightWidth = pRight ? pRight->GetWidth() : 64;

	TINT iSplitterWidth  = 0;
	TINT iSplitterHeight = 0;

	if ( m_pStyle )
		m_pStyle->GetComponentPreferredSize( 1, m_pParameters, iSplitterWidth, iSplitterHeight );

	TINT   iSpace        = m_iWidth - ( iRightWidth + iSplitterWidth + iLeftWidth );
	TFLOAT fTotalStretch = m_fStretchLeft + m_fStretchRight;
	TINT   iSplit;

	if ( fTotalStretch == 0.0f )
		iSplit = iSpace / 2 + iLeftWidth;
	else
		iSplit = TINT( TDOUBLE( TMath::FloorToInt( TFLOAT( m_fStretchLeft / fTotalStretch * TDOUBLE( iSpace ) + TDOUBLE( iLeftWidth ) ) ) ) );

	TINT iRightSize = m_iWidth - iSplitterWidth - iSplit;

	if ( pLeft )
	{
		pLeft->SetSize( iSplit, m_iHeight );
		pLeft->SetPosition( 0, 0 );
	}

	if ( pRight )
	{
		pRight->SetSize( iRightSize, m_iHeight );
		pRight->SetPosition( iSplitterWidth + iSplit, 0 );
	}

	if ( m_pStyle )
	{
		SetComponentBBox( 0, 0, 0, m_iWidth, m_iHeight );
		SetComponentBBox( 1, iSplit, 0, iSplitterWidth, m_iHeight );
	}

	if ( pLeft )
		pLeft->Pack();

	if ( pRight )
		pRight->Pack();
}

// The widths of the children and of the splitter, the height of the higher child (64 for a missing one)
// $JPOG: FUNCTION PGUICore.dll+0001cfb0
void PGUIVerticalSplit::Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight )
{
	m_bLayoutValid = TTRUE;

	TGUIWidget* pLeft  = GetLeftWidget();
	TGUIWidget* pRight = GetRightWidget();

	TINT iLeftWidth   = 64;
	TINT iRightWidth  = 64;
	TINT iLeftHeight  = 64;
	TINT iRightHeight = 64;

	if ( pLeft )
	{
		pLeft->Layout( a_iMaxWidth, a_iMaxHeight );
		iLeftWidth  = pLeft->GetWidth();
		iLeftHeight = pLeft->GetHeight();
	}

	if ( pRight )
	{
		pRight->Layout( a_iMaxWidth, a_iMaxHeight );
		iRightWidth  = pRight->GetWidth();
		iRightHeight = pRight->GetHeight();
	}

	TINT iSplitterWidth  = 0;
	TINT iSplitterHeight = 0;

	if ( m_pStyle )
		m_pStyle->GetComponentPreferredSize( 1, m_pParameters, iSplitterWidth, iSplitterHeight );

	TINT iWidth  = iRightWidth + iLeftWidth + iSplitterWidth;
	TINT iHeight = TMath::Max( iLeftHeight, iRightHeight );

	if ( pLeft )
	{
		pLeft->SetHeight( iHeight );
		pLeft->SetPosition( 0, 0 );
	}

	if ( pRight )
	{
		pRight->SetHeight( iHeight );
		pRight->SetPosition( iSplitterWidth + iLeftWidth, 0 );
	}

	SetSize( iWidth, iHeight );

	if ( m_pStyle )
	{
		SetComponentBBox( 0, 0, 0, iWidth, iHeight );
		SetComponentBBox( 1, iLeftWidth, 0, iSplitterWidth, iHeight );
	}
}

// $JPOG: FUNCTION PGUICore.dll+0001d510
TGUIWidget* PGUIVerticalSplit::GetLeftWidget()
{
	return ( m_oChildren.Count() > 0 ) ? m_oChildren.Head().Get() : TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+0001d4f0
TGUIWidget* PGUIVerticalSplit::GetRightWidget()
{
	return ( m_oChildren.Count() >= 2 ) ? m_oChildren.Head()->Next() : TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+0001d350
void PGUIVerticalSplit::Set_stretchleft_property( const PPropertyValue& a_rcValue )
{
	m_fStretchLeft = a_rcValue.GetFloat();
}

// $JPOG: FUNCTION PGUICore.dll+0001d330
PPropertyValue PGUIVerticalSplit::Get_stretchleft_property() const
{
	return PPropertyValue( m_fStretchLeft );
}

// $JPOG: FUNCTION PGUICore.dll+0001d120
TBOOL PGUIVerticalSplit::Val_stretchleft_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_FLOAT ) && a_rcValue.GetFloat() >= 0.0f;
}

// $JPOG: FUNCTION PGUICore.dll+0001d310
void PGUIVerticalSplit::Set_stretchright_property( const PPropertyValue& a_rcValue )
{
	m_fStretchRight = a_rcValue.GetFloat();
}

// $JPOG: FUNCTION PGUICore.dll+0001d2f0
PPropertyValue PGUIVerticalSplit::Get_stretchright_property() const
{
	return PPropertyValue( m_fStretchRight );
}

// $JPOG: FUNCTION PGUICore.dll+0001d170
TBOOL PGUIVerticalSplit::Val_stretchright_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_FLOAT ) && a_rcValue.GetFloat() >= 0.0f;
}
