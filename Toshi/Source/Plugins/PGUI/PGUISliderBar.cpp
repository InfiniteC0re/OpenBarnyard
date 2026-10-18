#include "ToshiPCH.h"
#include "PGUISliderBar.h"
#include "PGUICore.h"

#include <TGUI/TGUIManager.h>
#include <TGUI/TGUIWidgetInfo.h>
#include <TGUI/TGUIStyle.h>
#include <TGUI/TGUIStyleRootComponent.h>
#include <TGUI/TGUIComponentCache.h>
#include <TGUI/TGUIScreen.h>
#include <TGUI/TGUIPropertySpecSimple.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_INIT( PGUISliderBar );

TGUIDeserialisable::PropertySet* PGUISliderBar::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* PGUISliderBar::ms_oPropertySetOrdered = TNULL;
TGUIWidgetInfo*                  PGUISliderBar::ms_pWidgetInfo         = TNULL;

// The point is in the rectangle of the component cache
static TBOOL IsInComponent( const TGUIComponentCache& a_rcCache, TINT a_iX, TINT a_iY )
{
	const TGUIRectangle& rcBBox = a_rcCache.GetBBox();
	return a_iX >= rcBBox.m_iX && a_iY >= rcBBox.m_iY && a_iX < rcBBox.m_iX + rcBBox.m_iWidth && a_iY < rcBBox.m_iY + rcBBox.m_iHeight;
}

// NOTE: the category of the property specs (an argument the JPOG constructors don't have) is the name of the class
// $JPOG: FUNCTION PGUICore.dll+00013920
void PGUISliderBar::InitialiseClass()
{
	using t_Setter = TGUIPropertySpecSimple::t_Setter;
	using t_Getter = TGUIPropertySpecSimple::t_Getter;

	ms_pWidgetInfo = new TGUIWidgetInfo();
	CreatePropertySets( ms_oPropertySet, ms_oPropertySetOrdered, *TGUIWidget::ms_oPropertySet );

	TPString8 strCategory( "PGUISliderBar" );
	TINT      iSortHint = 0;
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "valuemin" ), static_cast<t_Setter>( &PGUISliderBar::Set_valuemin_property ), static_cast<t_Getter>( &PGUISliderBar::Get_valuemin_property ), PPropertyValue::TYPE_FLOAT, strCategory, TTRUE, &TGetClass( PGUISliderBar ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "valuemax" ), static_cast<t_Setter>( &PGUISliderBar::Set_valuemax_property ), static_cast<t_Getter>( &PGUISliderBar::Get_valuemax_property ), PPropertyValue::TYPE_FLOAT, strCategory, TTRUE, &TGetClass( PGUISliderBar ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "valuecurr" ), static_cast<t_Setter>( &PGUISliderBar::Set_valuecurr_property ), static_cast<t_Getter>( &PGUISliderBar::Get_valuecurr_property ), PPropertyValue::TYPE_FLOAT, strCategory, TTRUE, &TGetClass( PGUISliderBar ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "valuestep" ), static_cast<t_Setter>( &PGUISliderBar::Set_valuestep_property ), static_cast<t_Getter>( &PGUISliderBar::Get_valuestep_property ), PPropertyValue::TYPE_FLOAT, strCategory, TTRUE, &TGetClass( PGUISliderBar ), iSortHint++ ) );
	AddPropertySpec( ms_oPropertySet, ms_oPropertySetOrdered, new TGUIPropertySpecSimple( TPString8( "orientation" ), static_cast<t_Setter>( &PGUISliderBar::Set_orientation_property ), static_cast<t_Getter>( &PGUISliderBar::Get_orientation_property ), PPropertyValue::TYPE_INT, strCategory, TTRUE, &TGetClass( PGUISliderBar ), iSortHint++, Orientation_NUMOF, ms_csaOrientationNameStr ) );

	TPString8 strStateCategory( "State" );
	DeclareStateVar( &TGetClass( PGUISliderBar ), "Dragging", State_Dragging, strStateCategory, iSortHint, ms_pWidgetInfo, ms_oPropertySet, ms_oPropertySetOrdered );
	DeclareStateVar( &TGetClass( PGUISliderBar ), "Incrementing", State_Incrementing, strStateCategory, iSortHint, ms_pWidgetInfo, ms_oPropertySet, ms_oPropertySetOrdered );
	DeclareStateVar( &TGetClass( PGUISliderBar ), "Decrementing", State_Decrementing, strStateCategory, iSortHint, ms_pWidgetInfo, ms_oPropertySet, ms_oPropertySetOrdered );
	DeclareStateVar( &TGetClass( PGUISliderBar ), "CanIncrement", State_CanIncrement, strStateCategory, iSortHint, ms_pWidgetInfo, ms_oPropertySet, ms_oPropertySetOrdered );
	DeclareStateVar( &TGetClass( PGUISliderBar ), "CanDecrement", State_CanDecrement, strStateCategory, iSortHint, ms_pWidgetInfo, ms_oPropertySet, ms_oPropertySetOrdered );

	static const TCHAR* s_aComponents[] = { "background", "track", "pretrack", "posttrack", "preicon", "posticon", "knob", "knoblabel", "tracklabel", "prelabel", "postlabel", "foreground" };

	for ( const TCHAR* szComponent : s_aComponents )
		ms_pWidgetInfo->AddComponentName( TPString8( szComponent ) );

	ms_pWidgetInfo->SetWidgetName( TPString8( "PGUISliderBar" ) );
	ms_pWidgetInfo->SetWidgetLabel( TPString8( "Slider Bar" ) );
	ms_pWidgetInfo->SetProperties( ms_oPropertySet, ms_oPropertySetOrdered );
	ms_pWidgetInfo->AddStateVars( TGUIWidget::ms_pWidgetInfo->GetStateVarSet() );
	ms_pWidgetInfo->SetConcrete( TTRUE );

	TGUIManager::AddWidgetClass( ms_pWidgetInfo );
}

// $JPOG: FUNCTION PGUICore.dll+000151a0
void PGUISliderBar::DeinitialiseClass()
{
	ms_pWidgetInfo->Clear();
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );

	delete ms_pWidgetInfo;
	ms_pWidgetInfo = TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+00015260
PGUISliderBar::PGUISliderBar()
    : m_oValueChangeEmitter( this )
    , m_iDragOffsetX( 0 )
    , m_iDragOffsetY( 0 )
    , m_fMinimum( 0.0f )
    , m_fMaximum( 10.0f )
    , m_fValue( 5.0f )
    , m_fStep( 1.0f )
    , m_eOrientation( Orientation_Horizontal )
{
}

// $JPOG: FUNCTION PGUICore.dll+00016c20
PGUISliderBar::PGUISliderBar( const PGUISliderBar& a_rcOther )
    : TGUIWidget( a_rcOther )
    , m_oValueChangeEmitter( this )
    , m_iDragOffsetX( 0 )
    , m_iDragOffsetY( 0 )
    , m_fMinimum( a_rcOther.m_fMinimum )
    , m_fMaximum( a_rcOther.m_fMaximum )
    , m_fValue( a_rcOther.m_fValue )
    , m_fStep( a_rcOther.m_fStep )
    , m_eOrientation( a_rcOther.m_eOrientation )
{
}

// $JPOG: FUNCTION PGUICore.dll+00017df0
PGUISliderBar::~PGUISliderBar()
{
}

// $JPOG: FUNCTION PGUICore.dll+00017da0
TBOOL PGUISliderBar::Deserialise( const PProperties* a_pProperties )
{
	return TGUIWidget::Deserialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+00017d80
TBOOL PGUISliderBar::Serialise( PProperties* a_pProperties )
{
	return TGUIWidget::Serialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+00017e20
TGUIDeserialisable::PropertySet* PGUISliderBar::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $JPOG: FUNCTION PGUICore.dll+00017e10
TGUIDeserialisable::PropertySet* PGUISliderBar::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

const TGUIWidgetInfo* PGUISliderBar::GetWidgetInfo() const
{
	return ms_pWidgetInfo;
}

// $JPOG: FUNCTION PGUICore.dll+00017dc0
PGUISliderBar* PGUISliderBar::Clone() const
{
	return new PGUISliderBar( *this );
}

// $JPOG: FUNCTION PGUICore.dll+00007d00
TBOOL PGUISliderBar::AcceptsFocus() const
{
	return TTRUE;
}

// Losing the focus stops the increments, the decrements and the drag
// $JPOG: FUNCTION PGUICore.dll+00017a00
void PGUISliderBar::NotifyFocusLost( TGUIWidget* a_pNewFocus, TGUIWidget* a_pOldFocus )
{
	SetState( State_Incrementing, 0u );
	SetState( State_Decrementing, 0u );
	SetState( State_Dragging, 0u );
	TGUIWidget::NotifyFocusLost( a_pNewFocus, a_pOldFocus );
}

// The navigation actions along the orientation (north/east: increment, south/west: decrement) change the value by the
// step and repeat while held. A click on the knob starts dragging it, on the icons it increments/decrements (repeated
// while held over them) and on the track it moves the value there (if the style has a knob).
// $JPOG: FUNCTION PGUICore.dll+00015340
TBOOL PGUISliderBar::OnButtonPress( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	if ( TGUIWidget::OnButtonPress( a_pEvent, a_iActions ) )
		return TTRUE;

	// Increment
	if ( ( ( a_iActions & 0x200 ) && m_eOrientation == Orientation_Vertical || ( a_iActions & 0x400 ) && m_eOrientation == Orientation_Horizontal ) && !( m_uiState & State_Decrementing ) )
	{
		PGUIPlaySound( GetScreen(), "PGUISliderBar", m_fValue >= m_fMaximum ? "increment_fail" : "increment" );

		m_iRepeatDoodad = a_pEvent->GetDoodad();
		SetState( State_Incrementing, State_Incrementing );
		SetCurrentValue( m_fValue + m_fStep );
		GetScreen()->GrabFocus( this );
		a_pEvent->StartRepeat( 0.5f, 0.2f );
		ThrowValueChange();

		return TTRUE;
	}

	// Decrement
	if ( ( ( a_iActions & 0x800 ) && m_eOrientation == Orientation_Vertical || ( a_iActions & 0x1000 ) && m_eOrientation == Orientation_Horizontal ) && !( m_uiState & State_Incrementing ) )
	{
		PGUIPlaySound( GetScreen(), "PGUISliderBar", m_fValue <= m_fMinimum ? "decrement_fail" : "decrement" );

		m_iRepeatDoodad = a_pEvent->GetDoodad();
		SetState( State_Decrementing, State_Decrementing );
		SetCurrentValue( m_fValue - m_fStep );
		GetScreen()->GrabFocus( this );
		a_pEvent->StartRepeat( 0.5f, 0.2f );
		ThrowValueChange();

		return TTRUE;
	}

	if ( !( a_iActions & 0x4000 ) )
		return TFALSE;

	// A click
	TINT iX = 0;
	TINT iY = 0;
	TransformDeviceToWidget( GetScreen()->GetPointerPositionX(), GetScreen()->GetPointerPositionY(), iX, iY );

	if ( IsInComponent( m_pComponentCaches[ Component_Knob ], iX, iY ) )
	{
		PGUIPlaySound( GetScreen(), "PGUISliderBar", "knobdown" );

		const TGUIRectangle& rcKnob = m_pComponentCaches[ Component_Knob ].GetBBox();
		m_iRepeatDoodad             = a_pEvent->GetDoodad();
		SetState( State_Dragging, State_Dragging );
		m_iDragOffsetX = iX - rcKnob.m_iX - rcKnob.m_iWidth / 2;
		m_iDragOffsetY = iY - rcKnob.m_iY - rcKnob.m_iHeight / 2;
		GetScreen()->GrabPointer( this );
	}
	else if ( IsInComponent( m_pComponentCaches[ Component_PostIcon ], iX, iY ) )
	{
		PGUIPlaySound( GetScreen(), "PGUISliderBar", m_fValue >= m_fMaximum ? "increment_fail" : "increment" );

		m_iRepeatDoodad = a_pEvent->GetDoodad();
		SetState( State_Incrementing, State_Incrementing );
		SetCurrentValue( m_fValue + m_fStep );
		GetScreen()->GrabPointer( this );
		a_pEvent->StartRepeat( 0.5f, 0.2f );
		ThrowValueChange();
	}
	else if ( IsInComponent( m_pComponentCaches[ Component_PreIcon ], iX, iY ) )
	{
		PGUIPlaySound( GetScreen(), "PGUISliderBar", m_fValue <= m_fMinimum ? "decrement_fail" : "decrement" );

		m_iRepeatDoodad = a_pEvent->GetDoodad();
		SetState( State_Decrementing, State_Decrementing );
		SetCurrentValue( m_fValue - m_fStep );
		GetScreen()->GrabPointer( this );
		a_pEvent->StartRepeat( 0.5f, 0.2f );
		ThrowValueChange();
	}
	else if ( m_pStyle->GetRootComponent( Component_Knob ).GetStates().Count() > 0 && IsInComponent( m_pComponentCaches[ Component_Track ], iX, iY ) )
	{
		// The value at the click on the track
		const TGUIRectangle& rcTrack = m_pComponentCaches[ Component_Track ].GetBBox();
		TDOUBLE              dRatio;

		if ( m_eOrientation != Orientation_Horizontal )
			dRatio = TDOUBLE( iY - rcTrack.m_iY ) / TDOUBLE( rcTrack.m_iHeight );
		else
			dRatio = TDOUBLE( iX - rcTrack.m_iX ) / TDOUBLE( rcTrack.m_iWidth );

		TFLOAT fRatio = TFLOAT( dRatio );
		TFLOAT fValue = m_fMinimum + ( m_fMaximum - m_fMinimum ) * fRatio;

		if ( fValue != m_fValue )
		{
			PGUIPlaySound( GetScreen(), "PGUISliderBar", fValue >= m_fValue ? "increment" : "decrement" );
			SetCurrentValue( m_fMinimum + ( m_fMaximum - m_fMinimum ) * fRatio );
		}

		ThrowValueChange();
	}

	return TTRUE;
}

// The held increment/decrement (the click only while the pointer is over its icon) changes the value again, the repeat
// stops at the end of the range
// $JPOG: FUNCTION PGUICore.dll+000162e0
TBOOL PGUISliderBar::OnButtonRepeat( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	if ( TGUIWidget::OnButtonRepeat( a_pEvent, a_iActions ) )
		return TTRUE;

	if ( a_pEvent->GetDoodad() == m_iRepeatDoodad && ( m_uiState & State_Incrementing ) )
	{
		if ( a_iActions & 0x4000 )
		{
			TINT iX = 0;
			TINT iY = 0;
			TransformDeviceToWidget( GetScreen()->GetPointerPositionX(), GetScreen()->GetPointerPositionY(), iX, iY );

			if ( !IsInComponent( m_pComponentCaches[ Component_PostIcon ], iX, iY ) )
				return TTRUE;
		}

		if ( m_fValue >= m_fMaximum )
		{
#ifdef BARNYARD_COMMUNITY_PATCH
			PGUIPlaySound( GetScreen(), "PGUISliderBar", "increment_fail" );
#else
			// The original plays the decrement sounds when incrementing
			PGUIPlaySound( GetScreen(), "PGUISliderBar", "decrement_fail" );
#endif
			a_pEvent->GetSource()->StopRepeat( a_pEvent->GetDoodad() );
		}
		else
		{
#ifdef BARNYARD_COMMUNITY_PATCH
			PGUIPlaySound( GetScreen(), "PGUISliderBar", "increment" );
#else
			PGUIPlaySound( GetScreen(), "PGUISliderBar", "decrement" );
#endif
		}

		SetCurrentValue( m_fValue + m_fStep );
		ThrowValueChange();

		return TTRUE;
	}

	if ( a_pEvent->GetDoodad() != m_iRepeatDoodad || !( m_uiState & State_Decrementing ) )
		return TFALSE;

	if ( a_iActions & 0x4000 )
	{
		TINT iX = 0;
		TINT iY = 0;
		TransformDeviceToWidget( GetScreen()->GetPointerPositionX(), GetScreen()->GetPointerPositionY(), iX, iY );

		if ( !IsInComponent( m_pComponentCaches[ Component_PreIcon ], iX, iY ) )
			return TTRUE;
	}

	if ( m_fValue <= m_fMinimum )
	{
#ifdef BARNYARD_COMMUNITY_PATCH
		PGUIPlaySound( GetScreen(), "PGUISliderBar", "decrement_fail" );
#else
		// The original plays the increment sounds when decrementing
		PGUIPlaySound( GetScreen(), "PGUISliderBar", "increment_fail" );
#endif
		a_pEvent->GetSource()->StopRepeat( a_pEvent->GetDoodad() );
	}
	else
	{
#ifdef BARNYARD_COMMUNITY_PATCH
		PGUIPlaySound( GetScreen(), "PGUISliderBar", "decrement" );
#else
		PGUIPlaySound( GetScreen(), "PGUISliderBar", "increment" );
#endif
	}

	SetCurrentValue( m_fValue - m_fStep );
	ThrowValueChange();

	return TTRUE;
}

// A click release stops the drag and the increments/decrements (plays "knobup"), the release of the incrementing or
// decrementing doodad stops it
// $JPOG: FUNCTION PGUICore.dll+00016890
TBOOL PGUISliderBar::OnButtonRelease( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	if ( TGUIWidget::OnButtonRelease( a_pEvent, a_iActions ) )
		return TTRUE;

	if ( a_iActions & 0x4000 )
	{
		SetState( State_Dragging, 0u );
		SetState( State_Incrementing, 0u );
		SetState( State_Decrementing, 0u );
		GetScreen()->ReleasePointer( this );
		PGUIPlaySound( GetScreen(), "PGUISliderBar", "knobup" );

		return TTRUE;
	}

	if ( a_pEvent->GetDoodad() == m_iRepeatDoodad && ( m_uiState & State_Incrementing ) )
	{
		GetScreen()->ReleaseFocus( this );
		SetState( State_Incrementing, 0u );
		return TTRUE;
	}

	if ( a_pEvent->GetDoodad() == m_iRepeatDoodad && ( m_uiState & State_Decrementing ) )
	{
		GetScreen()->ReleaseFocus( this );
		SetState( State_Decrementing, 0u );
		return TTRUE;
	}

	return TFALSE;
}

// While dragging, the value follows the centre of the knob on the track (the vertical sliders go up)
// $JPOG: FUNCTION PGUICore.dll+00016a50
TBOOL PGUISliderBar::OnPointerMove( TINT a_iX, TINT a_iY )
{
	if ( !( m_uiState & State_Dragging ) )
		return TTRUE;

	const TGUIRectangle& rcTrack = m_pComponentCaches[ Component_Track ].GetBBox();
	const TGUIRectangle& rcKnob  = m_pComponentCaches[ Component_Knob ].GetBBox();

	if ( m_eOrientation == Orientation_Vertical )
	{
		TINT iRange    = rcTrack.m_iHeight - rcKnob.m_iHeight;
		TINT iPosition = TMath::Min( TMath::Max( a_iY - m_iDragOffsetY - ( rcTrack.m_iY + rcKnob.m_iHeight / 2 ), 0 ), iRange );
		m_fValue       = TFLOAT( m_fMaximum - TDOUBLE( iPosition ) / TDOUBLE( iRange ) * ( m_fMaximum - m_fMinimum ) );
	}
	else if ( m_eOrientation == Orientation_Horizontal )
	{
		TINT iRange    = rcTrack.m_iWidth - rcKnob.m_iWidth;
		TINT iPosition = TMath::Min( TMath::Max( a_iX - m_iDragOffsetX - ( rcTrack.m_iX + rcKnob.m_iWidth / 2 ), 0 ), iRange );
		m_fValue       = TFLOAT( m_fMinimum + TDOUBLE( iPosition ) / TDOUBLE( iRange ) * ( m_fMaximum - m_fMinimum ) );
	}

	UpdateKnob();
	ThrowValueChange();

	return TTRUE;
}

// $JPOG: FUNCTION PGUICore.dll+00017d70
void PGUISliderBar::Pack()
{
	PackComponents();
}

// $JPOG: FUNCTION PGUICore.dll+000152d0
void PGUISliderBar::Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight )
{
	m_bLayoutValid = TTRUE;

	if ( m_eSizeModeHoriz == SizeMode_Fixed && m_eSizeModeVert == SizeMode_Fixed )
		return;

	TINT iWidth  = 0;
	TINT iHeight = 0;
	LayoutComponents( iWidth, iHeight );

	if ( m_eSizeModeHoriz != SizeMode_Fixed )
		SetWidth( iWidth );

	if ( m_eSizeModeVert != SizeMode_Fixed )
		SetHeight( iHeight );
}

// $JPOG: FUNCTION PGUICore.dll+00007a10
void PGUISliderBar::SetStepValue( TFLOAT a_fStep )
{
	m_fStep = ( 0.0f >= a_fStep ) ? 0.0f : a_fStep;
}

// $JPOG: FUNCTION PGUICore.dll+00007a40
void PGUISliderBar::SetMaximumValue( TFLOAT a_fMaximum )
{
	m_fMaximum = ( a_fMaximum >= m_fMinimum ) ? a_fMaximum : m_fMinimum;
	m_fValue   = ( m_fMaximum >= m_fValue ) ? m_fValue : m_fMaximum;
	UpdateKnob();
}

// $JPOG: FUNCTION PGUICore.dll+00007aa0
void PGUISliderBar::SetMinimumValue( TFLOAT a_fMinimum )
{
	m_fMinimum = ( m_fMaximum >= a_fMinimum ) ? a_fMinimum : m_fMaximum;
	m_fValue   = ( m_fValue >= m_fMinimum ) ? m_fValue : m_fMinimum;
	UpdateKnob();
}

// $JPOG: FUNCTION PGUICore.dll+00007b00
void PGUISliderBar::SetCurrentValue( TFLOAT a_fValue )
{
	TFLOAT fValue = ( a_fValue < m_fMaximum ) ? a_fValue : m_fMaximum;
	m_fValue      = ( m_fMinimum >= fValue ) ? m_fMinimum : fValue;
	UpdateKnob();
}

// $JPOG: FUNCTION PGUICore.dll+00007b40
void PGUISliderBar::SetRange( TFLOAT a_fMinimum, TFLOAT a_fMaximum )
{
	m_fMinimum = a_fMinimum;
	m_fMaximum = ( a_fMinimum >= a_fMaximum ) ? a_fMinimum : a_fMaximum;

	TFLOAT fValue = ( m_fValue >= m_fMaximum ) ? m_fMaximum : m_fValue;
	m_fValue      = ( m_fMinimum >= fValue ) ? m_fMinimum : fValue;
	UpdateKnob();
}

// $JPOG: FUNCTION PGUICore.dll+00017a50
void PGUISliderBar::SetOrientation( TINT a_iOrientation )
{
	if ( TUINT( a_iOrientation ) <= Orientation_Vertical )
		m_eOrientation = Orientation( a_iOrientation );
}

// $JPOG: FUNCTION PGUICore.dll+00016ca0
void PGUISliderBar::UpdateKnob()
{
	if ( !m_pStyle )
		return;

	SetState( State_CanDecrement, ( m_fValue > m_fMinimum ) ? TUINT( State_CanDecrement ) : 0u );
	SetState( State_CanIncrement, ( m_fValue < m_fMaximum ) ? TUINT( State_CanIncrement ) : 0u );

	const TINT iWidth    = m_iWidth;
	const TINT iHeight   = m_iHeight;
	TINT       iTrackEnd = ( m_eOrientation != Orientation_Horizontal ) ? iHeight : iWidth;
	TINT       iPreSize  = 0;

	TINT iKnobWidth  = 0;
	TINT iKnobHeight = 0;
	m_pStyle->GetComponentPreferredSize( Component_Knob, m_pParameters, iKnobWidth, iKnobHeight );

	TINT iIconWidth  = 0;
	TINT iIconHeight = 0;

	if ( m_eOrientation == Orientation_Vertical )
	{
		if ( m_pStyle->GetComponentPreferredSize( Component_PreIcon, m_pParameters, iIconWidth, iIconHeight ) )
			iPreSize = iIconHeight;

		if ( m_pStyle->GetComponentPreferredSize( Component_PostIcon, m_pParameters, iIconWidth, iIconHeight ) )
			iTrackEnd = iHeight - iIconHeight;

		TINT iRange    = iTrackEnd - ( iPreSize + iKnobHeight );
		TINT iPosition = TINT( TDOUBLE( iRange ) / ( m_fMaximum - m_fMinimum ) * ( m_fValue - m_fMinimum ) );
		TINT iCentre   = iTrackEnd - iKnobHeight / 2 - iPosition;

		m_pComponentCaches[ Component_PostTrack ].Invalidate();
		m_pComponentCaches[ Component_PostTrack ].ModifyBBox( TNULL ).m_iHeight = iCentre - iPreSize;
		m_pComponentCaches[ Component_PreTrack ].Invalidate();
		m_pComponentCaches[ Component_PreTrack ].ModifyBBox( TNULL ).m_iHeight = iTrackEnd - iCentre;
		m_pComponentCaches[ Component_PreTrack ].Invalidate();
		m_pComponentCaches[ Component_PreTrack ].ModifyBBox( TNULL ).m_iY = iCentre;
		m_pComponentCaches[ Component_Knob ].Invalidate();
		m_pComponentCaches[ Component_Knob ].ModifyBBox( TNULL ).m_iY = iCentre - iKnobHeight / 2;
	}
	else if ( m_eOrientation == Orientation_Horizontal )
	{
		if ( m_pStyle->GetComponentPreferredSize( Component_PreIcon, m_pParameters, iIconWidth, iIconHeight ) )
			iPreSize = iIconWidth;

		if ( m_pStyle->GetComponentPreferredSize( Component_PostIcon, m_pParameters, iIconWidth, iIconHeight ) )
			iTrackEnd = iWidth - iIconWidth;

		TINT iRange    = iTrackEnd - ( iKnobWidth + iPreSize );
		TINT iPosition = TINT( TDOUBLE( iRange ) / ( m_fMaximum - m_fMinimum ) * ( m_fValue - m_fMinimum ) );
		TINT iCentre   = iPosition + iPreSize + iKnobWidth / 2;

		m_pComponentCaches[ Component_PreTrack ].Invalidate();
		m_pComponentCaches[ Component_PreTrack ].ModifyBBox( TNULL ).m_iWidth = iCentre - iPreSize;
		m_pComponentCaches[ Component_PostTrack ].Invalidate();
		m_pComponentCaches[ Component_PostTrack ].ModifyBBox( TNULL ).m_iWidth = iTrackEnd - iCentre;
		m_pComponentCaches[ Component_PostTrack ].Invalidate();
		m_pComponentCaches[ Component_PostTrack ].ModifyBBox( TNULL ).m_iX = iCentre;
		m_pComponentCaches[ Component_Knob ].Invalidate();
		m_pComponentCaches[ Component_Knob ].ModifyBBox( TNULL ).m_iX = iCentre - iKnobWidth / 2;
	}
}

// $JPOG: FUNCTION PGUICore.dll+00017060
void PGUISliderBar::ThrowValueChange()
{
	TGUIValueChangeEvent oEvent;
	oEvent.oValue = PPropertyValue( m_fValue );
	m_oValueChangeEmitter.Throw( &oEvent );
}

// NOTE: the label components don't get a rectangle (as in the original)
// $JPOG: FUNCTION PGUICore.dll+000170b0
void PGUISliderBar::PackComponents()
{
	if ( !m_pStyle )
		return;

	const TINT iWidth    = m_iWidth;
	const TINT iHeight   = m_iHeight;
	TINT       iTrackEnd = ( m_eOrientation != Orientation_Horizontal ) ? iHeight : iWidth;
	TINT       iPreSize  = 0;

	TINT iIconWidth  = 0;
	TINT iIconHeight = 0;
	TINT iKnobWidth  = 0;
	TINT iKnobHeight = 0;
	m_pStyle->GetComponentPreferredSize( Component_Knob, m_pParameters, iKnobWidth, iKnobHeight );

	if ( m_eOrientation == Orientation_Vertical )
	{
		// The pre icon at the bottom, the post icon at the top
		if ( m_pStyle->GetComponentPreferredSize( Component_PreIcon, m_pParameters, iIconWidth, iIconHeight ) )
		{
			SetComponentBBox( Component_PreIcon, ( iWidth - iIconWidth ) / 2, iHeight - iIconHeight, iWidth, iIconHeight );
			iTrackEnd = iHeight - iIconHeight;
		}

		if ( m_pStyle->GetComponentPreferredSize( Component_PostIcon, m_pParameters, iIconWidth, iIconHeight ) )
		{
			SetComponentBBox( Component_PostIcon, ( iWidth - iIconWidth ) / 2, 0, iWidth, iIconHeight );
			iPreSize = iIconHeight;
		}

		TINT iRange    = iTrackEnd - iKnobHeight - iPreSize;
		TINT iPosition = TINT( TDOUBLE( iRange ) / ( m_fMaximum - m_fMinimum ) * ( m_fValue - m_fMinimum ) );
		TINT iCentre   = iTrackEnd - iKnobHeight / 2 - iPosition;

		SetComponentBBox( Component_Track, 0, iPreSize, iWidth, iTrackEnd - iPreSize );
		SetComponentBBox( Component_Background, 0, 0, iWidth, iHeight );
		SetComponentBBox( Component_Knob, ( iWidth - iKnobWidth ) / 2, iCentre - iKnobHeight / 2, iKnobWidth, iKnobHeight );
		SetComponentBBox( Component_PreTrack, 0, iCentre, iWidth, iTrackEnd - iCentre );
		SetComponentBBox( Component_PostTrack, 0, iPreSize, iWidth, iCentre - iPreSize );
		SetComponentBBox( Component_Foreground, 0, 0, iWidth, iHeight );
	}
	else if ( m_eOrientation == Orientation_Horizontal )
	{
		if ( m_pStyle->GetComponentPreferredSize( Component_PreIcon, m_pParameters, iIconWidth, iIconHeight ) )
		{
			SetComponentBBox( Component_PreIcon, 0, 0, iIconWidth, iHeight );
			iPreSize = iIconWidth;
		}

		if ( m_pStyle->GetComponentPreferredSize( Component_PostIcon, m_pParameters, iIconWidth, iIconHeight ) )
		{
			SetComponentBBox( Component_PostIcon, iWidth - iIconWidth, 0, iIconWidth, iHeight );
			iTrackEnd = iWidth - iIconWidth;
		}

		TINT iRange    = iTrackEnd - ( iKnobWidth + iPreSize );
		TINT iPosition = TINT( TDOUBLE( iRange ) / ( m_fMaximum - m_fMinimum ) * ( m_fValue - m_fMinimum ) );
		TINT iCentre   = iPosition + iPreSize + iKnobWidth / 2;

		SetComponentBBox( Component_Track, iPreSize, 0, iTrackEnd - iPreSize, iHeight );
		SetComponentBBox( Component_Background, 0, 0, iWidth, iHeight );
		SetComponentBBox( Component_Knob, iCentre - iKnobWidth / 2, ( iHeight - iKnobHeight ) / 2, iKnobWidth, iKnobHeight );
		SetComponentBBox( Component_PreTrack, iPreSize, 0, iCentre - iPreSize, iHeight );
		SetComponentBBox( Component_PostTrack, iCentre, 0, iTrackEnd - iCentre, iHeight );
		SetComponentBBox( Component_Foreground, 0, 0, iWidth, iHeight );
	}
}

// $JPOG: FUNCTION PGUICore.dll+00017500
void PGUISliderBar::LayoutComponents( TINT& a_rWidth, TINT& a_rHeight )
{
	if ( !m_pStyle )
	{
		a_rWidth  = 0;
		a_rHeight = 0;
		return;
	}

	TINT iWidth           = 0;
	TINT iHeight          = 0;
	TINT iComponentWidth  = 0;
	TINT iComponentHeight = 0;

	if ( m_eOrientation != Orientation_Horizontal )
	{
		// The track and the icons on top of each other
		if ( m_pStyle->GetComponentPreferredSize( Component_Track, m_pParameters, iComponentWidth, iComponentHeight ) )
		{
			iWidth = TMath::Max( iWidth, iComponentWidth );
			iHeight += TMath::Max( iComponentHeight, 0 );
		}

		if ( m_pStyle->GetComponentPreferredSize( Component_PreIcon, m_pParameters, iComponentWidth, iComponentHeight ) )
		{
			iWidth = TMath::Max( iWidth, iComponentWidth );
			iHeight += iComponentHeight;
		}

		if ( m_pStyle->GetComponentPreferredSize( Component_PostIcon, m_pParameters, iComponentWidth, iComponentHeight ) )
		{
			iWidth = TMath::Max( iWidth, iComponentWidth );
			iHeight += iComponentHeight;
		}

		if ( m_pStyle->GetComponentPreferredSize( Component_Background, m_pParameters, iComponentWidth, iComponentHeight ) )
		{
			iWidth  = TMath::Max( iWidth, iComponentWidth );
			iHeight = TMath::Max( iHeight, iComponentHeight );
		}

		if ( m_pStyle->GetComponentPreferredSize( Component_Foreground, m_pParameters, iComponentWidth, iComponentHeight ) )
		{
			iWidth  = TMath::Max( iWidth, iComponentWidth );
			iHeight = TMath::Max( iHeight, iComponentHeight );
		}

		if ( m_pStyle->GetComponentPreferredSize( Component_Knob, m_pParameters, iComponentWidth, iComponentHeight ) )
		{
			iWidth  = TMath::Max( iWidth, iComponentWidth );
			iHeight = TMath::Max( iHeight, 3 * iComponentHeight );
		}
	}
	else
	{
		// The track and the icons side by side
		if ( m_pStyle->GetComponentPreferredSize( Component_Track, m_pParameters, iComponentWidth, iComponentHeight ) )
		{
			iHeight = TMath::Max( iHeight, iComponentHeight );
			iWidth += TMath::Max( iComponentWidth, 0 );
		}

		if ( m_pStyle->GetComponentPreferredSize( Component_PreIcon, m_pParameters, iComponentWidth, iComponentHeight ) )
		{
			iHeight = TMath::Max( iHeight, iComponentHeight );
			iWidth += iComponentWidth;
		}

		if ( m_pStyle->GetComponentPreferredSize( Component_PostIcon, m_pParameters, iComponentWidth, iComponentHeight ) )
		{
			iHeight = TMath::Max( iHeight, iComponentHeight );
			iWidth += iComponentWidth;
		}

		if ( m_pStyle->GetComponentPreferredSize( Component_Background, m_pParameters, iComponentWidth, iComponentHeight ) )
		{
			iHeight = TMath::Max( iHeight, iComponentHeight );
			iWidth  = TMath::Max( iWidth, iComponentWidth );
		}

		if ( m_pStyle->GetComponentPreferredSize( Component_Foreground, m_pParameters, iComponentWidth, iComponentHeight ) )
		{
			iHeight = TMath::Max( iHeight, iComponentHeight );
			iWidth  = TMath::Max( iWidth, iComponentWidth );
		}

		if ( m_pStyle->GetComponentPreferredSize( Component_Knob, m_pParameters, iComponentWidth, iComponentHeight ) )
		{
			iHeight = TMath::Max( iHeight, iComponentHeight );
			iWidth  = TMath::Max( iWidth, 3 * iComponentWidth );
		}
	}

	a_rWidth  = iWidth;
	a_rHeight = iHeight;
}

// $JPOG: FUNCTION PGUICore.dll+00017cf0
void PGUISliderBar::Set_valuemin_property( const PPropertyValue& a_rcValue )
{
	TFLOAT fMinimum = a_rcValue.GetFloat();
	m_fMinimum      = ( m_fMaximum >= fMinimum ) ? fMinimum : m_fMaximum;
	m_fValue        = ( m_fValue >= m_fMinimum ) ? m_fValue : m_fMinimum;
	UpdateKnob();
	Pack();
}

// $JPOG: FUNCTION PGUICore.dll+00017cd0
PPropertyValue PGUISliderBar::Get_valuemin_property() const
{
	return PPropertyValue( m_fMinimum );
}

// $JPOG: FUNCTION PGUICore.dll+00017cb0
TBOOL PGUISliderBar::Val_valuemin_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_FLOAT );
}

// $JPOG: FUNCTION PGUICore.dll+00017c30
void PGUISliderBar::Set_valuemax_property( const PPropertyValue& a_rcValue )
{
	TFLOAT fMaximum = a_rcValue.GetFloat();
	m_fMaximum      = ( fMaximum >= m_fMinimum ) ? fMaximum : m_fMinimum;
	m_fValue        = ( m_fMaximum >= m_fValue ) ? m_fValue : m_fMaximum;
	UpdateKnob();
	Pack();
}

// $JPOG: FUNCTION PGUICore.dll+00017c10
PPropertyValue PGUISliderBar::Get_valuemax_property() const
{
	return PPropertyValue( m_fMaximum );
}

// $JPOG: FUNCTION PGUICore.dll+00017bf0
TBOOL PGUISliderBar::Val_valuemax_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_FLOAT );
}

// $JPOG: FUNCTION PGUICore.dll+00017b80
void PGUISliderBar::Set_valuecurr_property( const PPropertyValue& a_rcValue )
{
	TFLOAT fValue = a_rcValue.GetFloat();
	fValue        = ( fValue < m_fMaximum ) ? fValue : m_fMaximum;
	m_fValue      = ( m_fMinimum >= fValue ) ? m_fMinimum : fValue;
	UpdateKnob();
	Pack();
}

// $JPOG: FUNCTION PGUICore.dll+00017b60
PPropertyValue PGUISliderBar::Get_valuecurr_property() const
{
	return PPropertyValue( m_fValue );
}

// $JPOG: FUNCTION PGUICore.dll+00017b40
TBOOL PGUISliderBar::Val_valuecurr_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_FLOAT );
}

// $JPOG: FUNCTION PGUICore.dll+00017af0
void PGUISliderBar::Set_valuestep_property( const PPropertyValue& a_rcValue )
{
	TFLOAT fStep = a_rcValue.GetFloat();
	m_fStep      = ( 0.0f >= fStep ) ? 0.0f : fStep;
}

// $JPOG: FUNCTION PGUICore.dll+00017ad0
PPropertyValue PGUISliderBar::Get_valuestep_property() const
{
	return PPropertyValue( m_fStep );
}

// $JPOG: FUNCTION PGUICore.dll+00017ab0
TBOOL PGUISliderBar::Val_valuestep_property( const PPropertyValue& a_rcValue )
{
	return a_rcValue.CanBeType( PPropertyValue::TYPE_FLOAT );
}

// $JPOG: FUNCTION PGUICore.dll+00017a90
void PGUISliderBar::Set_orientation_property( const PPropertyValue& a_rcValue )
{
	SetOrientation( a_rcValue.GetInteger() );
}

// $JPOG: FUNCTION PGUICore.dll+00017a70
PPropertyValue PGUISliderBar::Get_orientation_property() const
{
	return PPropertyValue( TINT( m_eOrientation ) );
}

// $JPOG: FUNCTION PGUICore.dll+00016bc0
TBOOL PGUISliderBar::Val_orientation_property( const PPropertyValue& a_rcValue )
{
	if ( !a_rcValue.CanBeType( PPropertyValue::TYPE_INT ) )
		return TFALSE;

	return a_rcValue.GetInteger() == Orientation_Horizontal || a_rcValue.GetInteger() == Orientation_Vertical;
}
