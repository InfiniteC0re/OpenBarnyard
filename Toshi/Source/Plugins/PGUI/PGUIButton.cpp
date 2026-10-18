#include "ToshiPCH.h"
#include "PGUIButton.h"

#include <TGUI/TGUIManager.h>
#include <TGUI/TGUIWidgetInfo.h>
#include <TGUI/TGUIScreen.h>
#include "PGUICore.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

// NOTE: abstract, CreateObject returns TNULL in JPOG
TDEFINE_CLASS_INIT_NORUNTIME( PGUIButton );

TGUIDeserialisable::PropertySet* PGUIButton::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* PGUIButton::ms_oPropertySetOrdered = TNULL;
TGUIWidgetInfo*                  PGUIButton::ms_pWidgetInfo         = TNULL;

// $JPOG: FUNCTION PGUICore.dll+00003080
void PGUIButton::InitialiseClass()
{
	ms_pWidgetInfo = new TGUIWidgetInfo();
	CreatePropertySets( ms_oPropertySet, ms_oPropertySetOrdered, *TGUIWidget::ms_oPropertySet );

	TINT iSortHint = 0;
	DeclareStateVar( &TGetClass( PGUIButton ), "Pressed", State_Pressed, TPString8( "State" ), iSortHint, ms_pWidgetInfo, ms_oPropertySet, ms_oPropertySetOrdered );

	ms_pWidgetInfo->SetWidgetName( TPString8( "PGUIButton" ) );
	ms_pWidgetInfo->SetWidgetLabel( TPString8( "Abstract Button" ) );
	ms_pWidgetInfo->SetProperties( ms_oPropertySet, ms_oPropertySetOrdered );
	ms_pWidgetInfo->AddStateVars( TGUIWidget::ms_pWidgetInfo->GetStateVarSet() );
	ms_pWidgetInfo->SetConcrete( TFALSE );

	TGUIManager::AddWidgetClass( ms_pWidgetInfo );
}

// $JPOG: FUNCTION PGUICore.dll+00003440
void PGUIButton::DeinitialiseClass()
{
	ms_pWidgetInfo->Clear();
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );

	delete ms_pWidgetInfo;
	ms_pWidgetInfo = TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+00003ac0
PGUIButton::PGUIButton()
    : m_oActionEmitter( this )
{
}

// $JPOG: FUNCTION PGUICore.dll+00003a30
PGUIButton::PGUIButton( const PGUIButton& a_rcOther )
    : TGUIWidget( a_rcOther )
    , m_oActionEmitter( this )
{
}

// $JPOG: FUNCTION PGUICore.dll+00003aa0
PGUIButton::~PGUIButton()
{
}

// $JPOG: FUNCTION PGUICore.dll+00003a80
TBOOL PGUIButton::Deserialise( const PProperties* a_pProperties )
{
	return TGUIWidget::Deserialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+00003a60
TBOOL PGUIButton::Serialise( PProperties* a_pProperties )
{
	return TGUIWidget::Serialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+00003b00
TGUIDeserialisable::PropertySet* PGUIButton::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $JPOG: FUNCTION PGUICore.dll+00003af0
TGUIDeserialisable::PropertySet* PGUIButton::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

const TGUIWidgetInfo* PGUIButton::GetWidgetInfo() const
{
	return ms_pWidgetInfo;
}

// $JPOG: FUNCTION PGUICore.dll+000043f0
TBOOL PGUIButton::AcceptsFocus() const
{
	return TTRUE;
}

// The select action presses the button (a click only when the pointer is over it, otherwise it does the action at once)
// $JPOG: FUNCTION PGUICore.dll+00003500
TBOOL PGUIButton::OnButtonPress( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	if ( TGUIWidget::OnButtonPress( a_pEvent, a_iActions ) )
		return TTRUE;

	if ( !( a_iActions & PRESS_ACTIONS ) )
		return TFALSE;

	if ( ( a_iActions & 1 ) || ( m_uiState & State_Rollover ) )
	{
		PGUIPlaySound( GetScreen(), "PGUIButton", "down" );

		SetState( State_Pressed, State_Pressed );
		GetScreen()->GrabPointer( this );
		GetScreen()->GrabFocus( this );
	}
	else
	{
		DoButtonAction();
	}

	return TTRUE;
}

// Releasing a pressed button does its action (a click only when the pointer is still over it)
// $JPOG: FUNCTION PGUICore.dll+00003660
TBOOL PGUIButton::OnButtonRelease( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	if ( TGUIWidget::OnButtonRelease( a_pEvent, a_iActions ) )
		return TTRUE;

	if ( !( a_iActions & PRESS_ACTIONS ) )
		return TFALSE;

	if ( m_uiState & State_Pressed )
	{
		PGUIPlaySound( GetScreen(), "PGUIButton", "up" );

		SetState( State_Pressed, 0u );
		GetScreen()->ReleaseFocus( this );
		GetScreen()->ReleasePointer( this );

		if ( ( a_iActions & 1 ) || ( m_uiState & State_Rollover ) )
			DoButtonAction();
	}

	return TTRUE;
}
