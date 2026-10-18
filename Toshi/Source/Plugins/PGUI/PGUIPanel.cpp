#include "ToshiPCH.h"
#include "PGUIPanel.h"

#include <TGUI/TGUIManager.h>
#include <TGUI/TGUIWidgetInfo.h>
#include <TGUI/TGUIStyle.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_INIT( PGUIPanel );

TGUIDeserialisable::PropertySet* PGUIPanel::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* PGUIPanel::ms_oPropertySetOrdered = TNULL;
TGUIWidgetInfo*                  PGUIPanel::ms_pWidgetInfo         = TNULL;

// $JPOG: FUNCTION PGUICore.dll+00012710
void PGUIPanel::InitialiseClass()
{
	ms_pWidgetInfo = new TGUIWidgetInfo();
	CreatePropertySets( ms_oPropertySet, ms_oPropertySetOrdered, *TGUIWidget::ms_oPropertySet );

	ms_pWidgetInfo->AddComponentName( TPString8( "background" ) );
	ms_pWidgetInfo->SetWidgetName( TPString8( "PGUIPanel" ) );
	ms_pWidgetInfo->SetWidgetLabel( TPString8( "Panel" ) );
	ms_pWidgetInfo->SetProperties( ms_oPropertySet, ms_oPropertySetOrdered );
	ms_pWidgetInfo->AddStateVars( TGUIWidget::ms_pWidgetInfo->GetStateVarSet() );
	ms_pWidgetInfo->SetConcrete( TTRUE );

	TGUIManager::AddWidgetClass( ms_pWidgetInfo );
}

// $JPOG: FUNCTION PGUICore.dll+00012aa0
void PGUIPanel::DeinitialiseClass()
{
	ms_pWidgetInfo->Clear();
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );

	delete ms_pWidgetInfo;
	ms_pWidgetInfo = TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+00012e10
PGUIPanel::PGUIPanel()
{
}

// $JPOG: FUNCTION PGUICore.dll+00012d70
PGUIPanel::PGUIPanel( const PGUIPanel& a_rcOther )
    : TGUIWidget( a_rcOther )
{
}

// $JPOG: FUNCTION PGUICore.dll+00012e00
PGUIPanel::~PGUIPanel()
{
}

// $JPOG: FUNCTION PGUICore.dll+00012db0
TBOOL PGUIPanel::Deserialise( const PProperties* a_pProperties )
{
	return TGUIWidget::Deserialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+00012d90
TBOOL PGUIPanel::Serialise( PProperties* a_pProperties )
{
	return TGUIWidget::Serialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+00012e40
TGUIDeserialisable::PropertySet* PGUIPanel::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $JPOG: FUNCTION PGUICore.dll+00012e30
TGUIDeserialisable::PropertySet* PGUIPanel::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

const TGUIWidgetInfo* PGUIPanel::GetWidgetInfo() const
{
	return ms_pWidgetInfo;
}

// $JPOG: FUNCTION PGUICore.dll+00012dd0
PGUIPanel* PGUIPanel::Clone() const
{
	return new PGUIPanel( *this );
}

// $JPOG: FUNCTION PGUICore.dll+000074f0
TBOOL PGUIPanel::AllowsChildren() const
{
	return TTRUE;
}

// $JPOG: FUNCTION PGUICore.dll+00012b60
void PGUIPanel::Pack()
{
	PackChildren();

	if ( m_pStyle )
		SetComponentBBox( 0, 0, 0, m_iWidth, m_iHeight );

	m_bValidated = TTRUE;
}

// The children are laid out first, the size that isn't fixed fits them and the background
// $JPOG: FUNCTION PGUICore.dll+00012ba0
void PGUIPanel::Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight )
{
	m_bLayoutValid = TTRUE;
	LayoutChildren( a_iMaxWidth, a_iMaxHeight );

	if ( m_eSizeModeHoriz == SizeMode_Fixed && m_eSizeModeVert == SizeMode_Fixed )
		return;

	TINT iWidth  = m_iWidth;
	TINT iHeight = m_iHeight;

	if ( m_pStyle )
	{
		TINT iPreferredWidth  = 0;
		TINT iPreferredHeight = 0;

		if ( m_pStyle->GetComponentPreferredSize( 0, m_pParameters, iPreferredWidth, iPreferredHeight ) )
		{
			iWidth  = TMath::Max( iWidth, iPreferredWidth );
			iHeight = TMath::Max( iHeight, iPreferredHeight );
		}
	}

	if ( m_eSizeModeHoriz != SizeMode_Fixed )
		SetWidth( iWidth );

	if ( m_eSizeModeVert != SizeMode_Fixed )
		SetHeight( iHeight );
}
