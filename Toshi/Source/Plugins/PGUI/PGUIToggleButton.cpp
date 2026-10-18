#include "ToshiPCH.h"
#include "PGUIToggleButton.h"

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

TDEFINE_CLASS_INIT( PGUIToggleButton );

TGUIDeserialisable::PropertySet* PGUIToggleButton::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* PGUIToggleButton::ms_oPropertySetOrdered = TNULL;
TGUIWidgetInfo*                  PGUIToggleButton::ms_pWidgetInfo         = TNULL;

// NOTE: the category of the property specs (an argument the JPOG constructors don't have) is the name of the class
// $JPOG: FUNCTION PGUICore.dll+0001b550
void PGUIToggleButton::InitialiseClass()
{
	using t_Setter = TGUIPropertySpecSimple::t_Setter;
	using t_Getter = TGUIPropertySpecSimple::t_Getter;

	ms_pWidgetInfo = new TGUIWidgetInfo();
	CreatePropertySets( ms_oPropertySet, ms_oPropertySetOrdered, *PGUIButton::ms_oPropertySet );

	TPString8 strCategory( "PGUIToggleButton" );
	TINT      iSortHint = 0;
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "test" ), static_cast<t_Setter>( &PGUIToggleButton::Set_test_property ), static_cast<t_Getter>( &PGUIToggleButton::Get_test_property ), PPropertyValue::TYPE_BOOL, strCategory, TTRUE, &TGetClass( PGUIToggleButton ), iSortHint++ ) );

	DeclareStateVar( &TGetClass( PGUIToggleButton ), "Toggled", State_Toggled, TPString8( "State" ), iSortHint, ms_pWidgetInfo, ms_oPropertySet, ms_oPropertySetOrdered );

	ms_pWidgetInfo->AddComponentName( TPString8( "background" ) );
	ms_pWidgetInfo->AddComponentName( TPString8( "foreground" ) );
	ms_pWidgetInfo->AddComponentName( TPString8( "iconleft" ) );
	ms_pWidgetInfo->AddComponentName( TPString8( "iconright" ) );
	ms_pWidgetInfo->SetWidgetName( TPString8( "PGUIToggleButton" ) );
	ms_pWidgetInfo->SetWidgetLabel( TPString8( "Toggle Button" ) );
	ms_pWidgetInfo->SetProperties( ms_oPropertySet, ms_oPropertySetOrdered );
	ms_pWidgetInfo->AddStateVars( PGUIButton::ms_pWidgetInfo->GetStateVarSet() );
	ms_pWidgetInfo->SetConcrete( TTRUE );

	TGUIManager::AddWidgetClass( ms_pWidgetInfo );
}

// $JPOG: FUNCTION PGUICore.dll+0001bd90
void PGUIToggleButton::DeinitialiseClass()
{
	ms_pWidgetInfo->Clear();
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );

	delete ms_pWidgetInfo;
	ms_pWidgetInfo = TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+0001c2d0
PGUIToggleButton::PGUIToggleButton()
{
}

// $JPOG: FUNCTION PGUICore.dll+0001c1d0
PGUIToggleButton::PGUIToggleButton( const PGUIToggleButton& a_rcOther )
    : PGUIButton( a_rcOther )
{
}

// $JPOG: FUNCTION PGUICore.dll+0001c2c0
PGUIToggleButton::~PGUIToggleButton()
{
}

// $JPOG: FUNCTION PGUICore.dll+0001c260
TBOOL PGUIToggleButton::Deserialise( const PProperties* a_pProperties )
{
	return PGUIButton::Deserialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+0001c230
TBOOL PGUIToggleButton::Serialise( PProperties* a_pProperties )
{
	return PGUIButton::Serialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+0001c300
TGUIDeserialisable::PropertySet* PGUIToggleButton::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $JPOG: FUNCTION PGUICore.dll+0001c2f0
TGUIDeserialisable::PropertySet* PGUIToggleButton::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

const TGUIWidgetInfo* PGUIToggleButton::GetWidgetInfo() const
{
	return ms_pWidgetInfo;
}

// $JPOG: FUNCTION PGUICore.dll+0001c290
PGUIToggleButton* PGUIToggleButton::Clone() const
{
	return new PGUIToggleButton( *this );
}

// The icons at the sides (their preferred widths), the foreground between them, the background over the whole button
// $JPOG: FUNCTION PGUICore.dll+0001be50
void PGUIToggleButton::Pack()
{
	if ( m_pStyle )
	{
		TINT iLeftWidth  = 0;
		TINT iRightWidth = 0;
		TINT iHeight;
		m_pStyle->GetComponentPreferredSize( 2, m_pParameters, iLeftWidth, iHeight );
		m_pStyle->GetComponentPreferredSize( 3, m_pParameters, iRightWidth, iHeight );

		SetComponentBBox( 0, 0, 0, m_iWidth, m_iHeight );
		SetComponentBBox( 1, iLeftWidth, 0, m_iWidth - ( iLeftWidth + iRightWidth ), m_iHeight );
		SetComponentBBox( 2, 0, 0, iLeftWidth, m_iHeight );
		SetComponentBBox( 3, m_iWidth - iRightWidth, 0, iRightWidth, m_iHeight );
	}

	m_bValidated = TTRUE;
}

// The width is the larger of the background and of the icons and foreground side by side, the height the largest of all
// $JPOG: FUNCTION PGUICore.dll+0001bf20
void PGUIToggleButton::Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight )
{
	m_bLayoutValid = TTRUE;

	if ( m_eSizeModeHoriz == SizeMode_Fixed && m_eSizeModeVert == SizeMode_Fixed )
		return;

	TINT iWidth  = 0;
	TINT iHeight = 0;

	if ( m_pStyle )
	{
		TINT iComponentHeight = 0;
		m_pStyle->GetComponentPreferredSize( 0, m_pParameters, iWidth, iHeight );

		TINT iForegroundWidth = 0;
		TINT iLeftWidth       = 0;
		TINT iRightWidth      = 0;

		if ( m_pStyle->GetComponentPreferredSize( 1, m_pParameters, iForegroundWidth, iComponentHeight ) )
			iHeight = TMath::Max( iComponentHeight, iHeight );

		if ( m_pStyle->GetComponentPreferredSize( 2, m_pParameters, iLeftWidth, iComponentHeight ) )
			iHeight = TMath::Max( iComponentHeight, iHeight );

#ifdef BARNYARD_COMMUNITY_PATCH
		// Fixes the size of iconright being read from iconleft
		if ( m_pStyle->GetComponentPreferredSize( 3, m_pParameters, iRightWidth, iComponentHeight ) )
#else
		if ( m_pStyle->GetComponentPreferredSize( 2, m_pParameters, iRightWidth, iComponentHeight ) )
#endif
			iHeight = TMath::Max( iComponentHeight, iHeight );

		iWidth = TMath::Max( iWidth, iRightWidth + iForegroundWidth + iLeftWidth );
	}

	if ( m_eSizeModeHoriz != SizeMode_Fixed )
		SetWidth( iWidth );

	if ( m_eSizeModeVert != SizeMode_Fixed )
		SetHeight( iHeight );
}

// $JPOG: FUNCTION PGUICore.dll+0001c090
void PGUIToggleButton::DoButtonAction()
{
	SetState( State_Toggled, ( m_uiState & State_Toggled ) ? 0u : TUINT( State_Toggled ) );

	TGUIActionEvent oEvent;
	oEvent.pItem   = TNULL;
	oEvent.bAccept = TFALSE;
	m_oActionEmitter.Throw( &oEvent );
}

// $JPOG: FUNCTION PGUICore.dll+0001c220
void PGUIToggleButton::Set_test_property( const PPropertyValue& a_rcValue )
{
}

// $JPOG: FUNCTION PGUICore.dll+0001c200
PPropertyValue PGUIToggleButton::Get_test_property() const
{
	return PPropertyValue( TTRUE );
}

// $JPOG: FUNCTION PGUICore.dll+0001c1f0
TBOOL PGUIToggleButton::Val_test_property( const PPropertyValue& a_rcValue )
{
	return TTRUE;
}
