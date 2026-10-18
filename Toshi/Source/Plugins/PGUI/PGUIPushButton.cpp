#include "ToshiPCH.h"
#include "PGUIPushButton.h"

#include <TGUI/TGUIManager.h>
#include <TGUI/TGUIWidgetInfo.h>
#include <TGUI/TGUIStyle.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_INIT( PGUIPushButton );

TGUIDeserialisable::PropertySet* PGUIPushButton::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* PGUIPushButton::ms_oPropertySetOrdered = TNULL;
TGUIWidgetInfo*                  PGUIPushButton::ms_pWidgetInfo         = TNULL;

// $JPOG: FUNCTION PGUICore.dll+00012f60
void PGUIPushButton::InitialiseClass()
{
	ms_pWidgetInfo = new TGUIWidgetInfo();
	CreatePropertySets( ms_oPropertySet, ms_oPropertySetOrdered, *PGUIButton::ms_oPropertySet );

	ms_pWidgetInfo->AddComponentName( TPString8( "background" ) );
	ms_pWidgetInfo->AddComponentName( TPString8( "foreground" ) );
	ms_pWidgetInfo->AddComponentName( TPString8( "effect" ) );
	ms_pWidgetInfo->SetWidgetName( TPString8( "PGUIPushButton" ) );
	ms_pWidgetInfo->SetWidgetLabel( TPString8( "Push Button" ) );
	ms_pWidgetInfo->SetProperties( ms_oPropertySet, ms_oPropertySetOrdered );
	ms_pWidgetInfo->AddStateVars( PGUIButton::ms_pWidgetInfo->GetStateVarSet() );
	ms_pWidgetInfo->SetConcrete( TTRUE );

	TGUIManager::AddWidgetClass( ms_pWidgetInfo );
}

// $JPOG: FUNCTION PGUICore.dll+000133e0
void PGUIPushButton::DeinitialiseClass()
{
	ms_pWidgetInfo->Clear();
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );

	delete ms_pWidgetInfo;
	ms_pWidgetInfo = TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+00013840
PGUIPushButton::PGUIPushButton()
{
}

// $JPOG: FUNCTION PGUICore.dll+00013780
PGUIPushButton::PGUIPushButton( const PGUIPushButton& a_rcOther )
    : PGUIButton( a_rcOther )
{
}

// $JPOG: FUNCTION PGUICore.dll+00013830
PGUIPushButton::~PGUIPushButton()
{
}

// $JPOG: FUNCTION PGUICore.dll+000137d0
TBOOL PGUIPushButton::Deserialise( const PProperties* a_pProperties )
{
	return PGUIButton::Deserialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+000137a0
TBOOL PGUIPushButton::Serialise( PProperties* a_pProperties )
{
	return PGUIButton::Serialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+00013870
TGUIDeserialisable::PropertySet* PGUIPushButton::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $JPOG: FUNCTION PGUICore.dll+00013860
TGUIDeserialisable::PropertySet* PGUIPushButton::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

const TGUIWidgetInfo* PGUIPushButton::GetWidgetInfo() const
{
	return ms_pWidgetInfo;
}

// $JPOG: FUNCTION PGUICore.dll+00013800
PGUIPushButton* PGUIPushButton::Clone() const
{
	return new PGUIPushButton( *this );
}

// The three components fill the button
// $JPOG: FUNCTION PGUICore.dll+000134a0
void PGUIPushButton::Pack()
{
	if ( m_pStyle )
	{
		SetComponentBBox( 0, 0, 0, m_iWidth, m_iHeight );
		SetComponentBBox( 1, 0, 0, m_iWidth, m_iHeight );
		SetComponentBBox( 2, 0, 0, m_iWidth, m_iHeight );
	}

	m_bValidated = TTRUE;
}

// The size that isn't fixed is the largest preferred size of the background and the foreground (the layout isn't valid
// while a size depends on the size of the parent)
// $JPOG: FUNCTION PGUICore.dll+00013500
void PGUIPushButton::Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight )
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

// $JPOG: FUNCTION PGUICore.dll+00013760
void PGUIPushButton::DoButtonAction()
{
	TGUIActionEvent oEvent;
	oEvent.pItem   = TNULL;
	oEvent.bAccept = TFALSE;
	m_oActionEmitter.Throw( &oEvent );
}
