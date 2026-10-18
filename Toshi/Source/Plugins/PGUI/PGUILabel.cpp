#include "ToshiPCH.h"
#include "PGUILabel.h"

#include <TGUI/TGUIManager.h>
#include <TGUI/TGUIWidgetInfo.h>
#include <TGUI/TGUIStyle.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_INIT( PGUILabel );

TGUIDeserialisable::PropertySet* PGUILabel::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* PGUILabel::ms_oPropertySetOrdered = TNULL;
TGUIWidgetInfo*                  PGUILabel::ms_pWidgetInfo         = TNULL;

// $JPOG: FUNCTION PGUICore.dll+00007da0
void PGUILabel::InitialiseClass()
{
	ms_pWidgetInfo = new TGUIWidgetInfo();
	CreatePropertySets( ms_oPropertySet, ms_oPropertySetOrdered, *TGUIWidget::ms_oPropertySet );

	ms_pWidgetInfo->AddComponentName( TPString8( "background" ) );
	ms_pWidgetInfo->AddComponentName( TPString8( "foreground" ) );
	ms_pWidgetInfo->SetWidgetName( TPString8( "PGUILabel" ) );
	ms_pWidgetInfo->SetWidgetLabel( TPString8( "Label" ) );
	ms_pWidgetInfo->SetProperties( ms_oPropertySet, ms_oPropertySetOrdered );
	ms_pWidgetInfo->AddStateVars( TGUIWidget::ms_pWidgetInfo->GetStateVarSet() );
	ms_pWidgetInfo->SetConcrete( TTRUE );

	TGUIManager::AddWidgetClass( ms_pWidgetInfo );
}

// $JPOG: FUNCTION PGUICore.dll+000081a0
void PGUILabel::DeinitialiseClass()
{
	ms_pWidgetInfo->Clear();
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );

	delete ms_pWidgetInfo;
	ms_pWidgetInfo = TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+000085b0
PGUILabel::PGUILabel()
{
}

// $JPOG: FUNCTION PGUICore.dll+00008510
PGUILabel::PGUILabel( const PGUILabel& a_rcOther )
    : TGUIWidget( a_rcOther )
{
}

// $JPOG: FUNCTION PGUICore.dll+000085a0
PGUILabel::~PGUILabel()
{
}

// $JPOG: FUNCTION PGUICore.dll+00008550
TBOOL PGUILabel::Deserialise( const PProperties* a_pProperties )
{
	return TGUIWidget::Deserialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+00008530
TBOOL PGUILabel::Serialise( PProperties* a_pProperties )
{
	return TGUIWidget::Serialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+000085e0
TGUIDeserialisable::PropertySet* PGUILabel::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $JPOG: FUNCTION PGUICore.dll+000085d0
TGUIDeserialisable::PropertySet* PGUILabel::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

const TGUIWidgetInfo* PGUILabel::GetWidgetInfo() const
{
	return ms_pWidgetInfo;
}

// $JPOG: FUNCTION PGUICore.dll+00008570
PGUILabel* PGUILabel::Clone() const
{
	return new PGUILabel( *this );
}

// The two components fill the label
// $JPOG: FUNCTION PGUICore.dll+00008260
void PGUILabel::Pack()
{
	if ( m_pStyle )
	{
		SetComponentBBox( 0, 0, 0, m_iWidth, m_iHeight );
		SetComponentBBox( 1, 0, 0, m_iWidth, m_iHeight );
	}

	m_bValidated = TTRUE;
}

// The size that isn't fixed is the largest preferred size of the background and the foreground (the layout isn't valid
// while a size depends on the size of the parent)
// $JPOG: FUNCTION PGUICore.dll+000082b0
void PGUILabel::Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight )
{
	if ( m_eSizeModeHoriz == SizeMode_Fixed )
		a_iMaxWidth = m_iWidth;

	if ( m_eSizeModeVert == SizeMode_Fixed )
		a_iMaxHeight = m_iHeight;

	m_bLayoutValid = TTRUE;

	if ( m_eSizeModeHoriz == SizeMode_Fixed && m_eSizeModeVert == SizeMode_Fixed )
		return;

	TINT iWidth  = 0;
	TINT iHeight = 0;

	if ( m_pStyle )
	{
		for ( TINT i = 0; i < 2; i++ )
		{
			TINT  iPreferredWidth  = 0;
			TINT  iPreferredHeight = 0;
			TBOOL bAllKnown;

			if ( m_pStyle->GetComponentPreferredSize( i, m_pParameters, iPreferredWidth, iPreferredHeight, a_iMaxWidth, a_iMaxHeight, bAllKnown ) )
			{
				if ( bAllKnown )
				{
					iWidth  = TMath::Max( iWidth, iPreferredWidth );
					iHeight = TMath::Max( iHeight, iPreferredHeight );
				}
				else
				{
					m_bLayoutValid = TFALSE;
				}
			}
		}
	}

	if ( m_eSizeModeHoriz != SizeMode_Fixed )
		SetWidth( iWidth );

	if ( m_eSizeModeVert != SizeMode_Fixed )
		SetHeight( iHeight );
}
