#include "ToshiPCH.h"
#include "PGUIListItem.h"

#include <TGUI/TGUIManager.h>
#include <TGUI/TGUIWidgetInfo.h>
#include <TGUI/TGUIStyle.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_INIT( PGUIListItem );

TGUIDeserialisable::PropertySet* PGUIListItem::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* PGUIListItem::ms_oPropertySetOrdered = TNULL;
TGUIWidgetInfo*                  PGUIListItem::ms_pWidgetInfo         = TNULL;

// $JPOG: FUNCTION PGUICore.dll+0000c980
void PGUIListItem::InitialiseClass()
{
	ms_pWidgetInfo = new TGUIWidgetInfo();
	CreatePropertySets( ms_oPropertySet, ms_oPropertySetOrdered, *TGUIWidget::ms_oPropertySet );

	ms_pWidgetInfo->AddComponentName( TPString8( "background" ) );
	ms_pWidgetInfo->AddComponentName( TPString8( "foreground" ) );
	ms_pWidgetInfo->SetWidgetName( TPString8( "PGUIListItem" ) );
	ms_pWidgetInfo->SetWidgetLabel( TPString8( "List Item" ) );
	ms_pWidgetInfo->SetProperties( ms_oPropertySet, ms_oPropertySetOrdered );
	ms_pWidgetInfo->AddStateVars( TGUIWidget::ms_pWidgetInfo->GetStateVarSet() );
	ms_pWidgetInfo->SetConcrete( TTRUE );

	TGUIManager::AddWidgetClass( ms_pWidgetInfo );
}

// $JPOG: FUNCTION PGUICore.dll+0000cd80
void PGUIListItem::DeinitialiseClass()
{
	ms_pWidgetInfo->Clear();
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );

	delete ms_pWidgetInfo;
	ms_pWidgetInfo = TNULL;
}

// NOTE: the original leaves the item and the index uninitialised
// $JPOG: FUNCTION PGUICore.dll+0000d140
PGUIListItem::PGUIListItem()
    : m_pData( TNULL )
    , m_iIndex( -1 )
{
}

// $JPOG: FUNCTION PGUICore.dll+0000d0a0
PGUIListItem::PGUIListItem( const PGUIListItem& a_rcOther )
    : TGUIWidget( a_rcOther )
    , m_pData( TNULL )
    , m_iIndex( -1 )
{
}

// $JPOG: FUNCTION PGUICore.dll+0000d130
PGUIListItem::~PGUIListItem()
{
}

// $JPOG: FUNCTION PGUICore.dll+0000d0e0
TBOOL PGUIListItem::Deserialise( const PProperties* a_pProperties )
{
	return TGUIWidget::Deserialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+0000d0c0
TBOOL PGUIListItem::Serialise( PProperties* a_pProperties )
{
	return TGUIWidget::Serialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+0000d170
TGUIDeserialisable::PropertySet* PGUIListItem::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $JPOG: FUNCTION PGUICore.dll+0000d160
TGUIDeserialisable::PropertySet* PGUIListItem::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

const TGUIWidgetInfo* PGUIListItem::GetWidgetInfo() const
{
	return ms_pWidgetInfo;
}

// $JPOG: FUNCTION PGUICore.dll+0000d100
PGUIListItem* PGUIListItem::Clone() const
{
	return new PGUIListItem( *this );
}

// $JPOG: FUNCTION PGUICore.dll+0000c860
TBOOL PGUIListItem::AcceptsFocus() const
{
	return TTRUE;
}

// $JPOG: FUNCTION PGUICore.dll+0000ce40
void PGUIListItem::Pack()
{
	if ( m_pStyle )
	{
		SetComponentBBox( 0, 0, 0, m_iWidth, m_iHeight );
		SetComponentBBox( 1, 0, 0, m_iWidth, m_iHeight );
	}

	m_bValidated = TTRUE;
}

// $JPOG: FUNCTION PGUICore.dll+0000ce90
void PGUIListItem::Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight )
{
	m_bLayoutValid = TTRUE;

	if ( m_eSizeModeHoriz == SizeMode_Fixed && m_eSizeModeVert == SizeMode_Fixed )
		return;

	TINT iWidth  = 0;
	TINT iHeight = 0;

	if ( m_pStyle )
	{
		for ( TINT i = 0; i < 2; i++ )
		{
			TINT iPreferredWidth  = 0;
			TINT iPreferredHeight = 0;

			if ( m_pStyle->GetComponentPreferredSize( i, m_pParameters, iPreferredWidth, iPreferredHeight ) )
			{
				iWidth  = TMath::Max( iWidth, iPreferredWidth );
				iHeight = TMath::Max( iHeight, iPreferredHeight );
			}
		}
	}

	if ( m_eSizeModeHoriz != SizeMode_Fixed )
		SetWidth( iWidth );

	if ( m_eSizeModeVert != SizeMode_Fixed )
		SetHeight( iHeight );
}
