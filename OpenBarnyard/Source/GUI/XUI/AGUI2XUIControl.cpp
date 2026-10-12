#include "pch.h"
#ifdef BARNYARD_LATEST
#  include "AGUI2XUIControl.h"
#  include "AGUI2XUIVisual.h"
#  include "AGUI2XUIResource.h"
#  include "GUI/AGUI2.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGUI2XUIControl );

// $BarnyardWiiJP: FUNCTION 8041581c
AGUI2XUIControl::AGUI2XUIControl()
{
	m_pVisual          = TNULL;
	m_wszText          = TNULL;
	m_wszImagePath     = TNULL;
	m_bEnabled         = TTRUE;
	m_bUnfocussedInput = TFALSE;

	for ( TINT i = 0; i < XURXUIControlData::NAV_NUMOF; i++ )
	{
		m_awszNav[ i ] = TNULL;
		m_apNav[ i ]   = TNULL;
	}
}

// $BarnyardWiiJP: FUNCTION 8041589c
// $BarnyardWiiJP: FUNCTION 80415e48
AGUI2XUIControl::~AGUI2XUIControl()
{
	if ( m_pVisual )
	{
		m_pVisual->RemoveSelf();

		if ( m_pVisual )
			delete m_pVisual;
	}
}

// $BarnyardWiiJP: FUNCTION 80415950
TBOOL AGUI2XUIControl::Create( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	XURXUIControlData* pData = TSTATICCAST( XURXUIControlData, a_pData );

	// NOTE: controls always create their children
	AGUI2XUIElement::Create( a_rResource, a_pData, TTRUE );

	// The visual is either set explicitly or named after the XUI class (e.g. XuiButton)
	if ( pData->m_usVisual != 0 )
		m_pVisual = AGUI2::GetSingleton()->GetXUI().CreateVisual( a_rResource.GetString( pData->m_usVisual ) );
	else
		m_pVisual = AGUI2::GetSingleton()->GetXUI().CreateVisual( a_rResource.GetString( pData->GetClassIndex() ) );

	if ( m_pVisual )
		AddChildTail( *m_pVisual );

	if ( pData->m_usText != 0 )
		m_wszText = a_rResource.GetString( pData->m_usText );

	if ( pData->m_usImagePath != 0 )
		m_wszImagePath = a_rResource.GetString( pData->m_usImagePath );

	if ( m_pVisual )
		m_pVisual->SetPresenterData( m_wszText, m_wszImagePath );

	for ( TINT i = 0; i < XURXUIControlData::NAV_NUMOF; i++ )
	{
		if ( pData->m_aNav[ i ] != 0 )
			m_awszNav[ i ] = a_rResource.GetString( pData->m_aNav[ i ] );
	}

	m_bEnabled         = pData->m_bEnabled;
	m_bUnfocussedInput = pData->m_bUnfocussedInput;

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 80415ad4
void AGUI2XUIControl::Tick( TFLOAT a_fDeltaTime )
{
	AGUI2XUIElement::Tick( a_fDeltaTime );
}

// $BarnyardWiiJP: FUNCTION 80415ad8
void AGUI2XUIControl::ResolveNavigation( AGUI2XUIElement* a_pRoot )
{
	if ( !a_pRoot )
		return;

	for ( TINT i = 0; i < XURXUIControlData::NAV_NUMOF; i++ )
	{
		if ( m_awszNav[ i ] )
		{
			AGUI2XUIElement* pElement = a_pRoot->FindChildElement( m_awszNav[ i ] );

			if ( pElement )
				m_apNav[ i ] = TSTATICCAST( AGUI2XUIControl, pElement );
		}
	}
}

// $BarnyardWiiJP: FUNCTION 80415b5c
void AGUI2XUIControl::OnPress( TBOOL a_bEnded )
{
	if ( a_bEnded == TTRUE )
		SetFocus( TTRUE );
	else if ( a_bEnded == TFALSE )
		Press();
}

// $BarnyardWiiJP: FUNCTION 80415b98
void AGUI2XUIControl::Render()
{
	AGUI2XUIElement::Render();
}

// $BarnyardWiiJP: FUNCTION 80415b9c
void AGUI2XUIControl::PlayAnimation( const TWCHAR* a_wszStartFrame, const TWCHAR* a_wszEndFrame )
{
	AGUI2XUIElement::PlayAnimation( a_wszStartFrame, a_wszEndFrame );
}

// $BarnyardWiiJP: FUNCTION 80415ba0
void AGUI2XUIControl::PlayTimelineRange( TFLOAT a_fStart, TFLOAT a_fEnd )
{
	AGUI2XUIElement::PlayTimelineRange( a_fStart, a_fEnd );
}

// $BarnyardWiiJP: FUNCTION 80415ba4
void AGUI2XUIControl::UpdateTimelinePropValue( AGUI2XUIResource& a_rResource, TUINT32 a_uiProp, TUINT32 a_uiFrom, TUINT32 a_uiTo, TINT a_iIndex, TBOOL a_bKeyframeChanged, TFLOAT a_fT )
{
	// Map the element properties of the control back to the element ones
	switch ( a_uiProp )
	{
		case XURPROP_CONTROL_ELEMENT_POSITION: a_uiProp = XURPROP_ELEMENT_POSITION; break;
		case XURPROP_CONTROL_ELEMENT_SHOW: a_uiProp = XURPROP_ELEMENT_SHOW; break;
		case XURPROP_CONTROL_ELEMENT_BRUSHFLAGS: a_uiProp = XURPROP_ELEMENT_BRUSHFLAGS; break;
		case XURPROP_CONTROL_ELEMENT_COLORWRITEFLAGS: a_uiProp = XURPROP_ELEMENT_COLORWRITEFLAGS; break;
		case XURPROP_CONTROL_ELEMENT_DISABLETIMELINERECURSION: a_uiProp = XURPROP_ELEMENT_DISABLETIMELINERECURSION; break;
		case XURPROP_CONTROL_ELEMENT_BLENDMODE: a_uiProp = XURPROP_ELEMENT_BLENDMODE; break;
		case XURPROP_CONTROL_ELEMENT_ANCHOR: a_uiProp = XURPROP_ELEMENT_ANCHOR; break;
		case XURPROP_CONTROL_ELEMENT_PIVOT: a_uiProp = XURPROP_ELEMENT_PIVOT; break;
		case XURPROP_CONTROL_ELEMENT_ROTATION: a_uiProp = XURPROP_ELEMENT_ROTATION; break;
		case XURPROP_CONTROL_ELEMENT_SCALE: a_uiProp = XURPROP_ELEMENT_SCALE; break;
	}

	AGUI2XUIElement::UpdateTimelinePropValue( a_rResource, a_uiProp, a_uiFrom, a_uiTo, a_iIndex, a_bKeyframeChanged, a_fT );
}

// $BarnyardWiiJP: FUNCTION 80415d64
void AGUI2XUIControl::UpdateTimelinePropFloat( AGUI2XUIResource& a_rResource, TUINT32 a_uiProp, TINT a_iIndex, TBOOL a_bKeyframeChanged, TFLOAT a_fFrom, TFLOAT a_fTo, TFLOAT a_fT )
{
	switch ( a_uiProp )
	{
		case XURPROP_CONTROL_ELEMENT_HEIGHT: a_uiProp = XURPROP_ELEMENT_HEIGHT; break;
		case XURPROP_CONTROL_ELEMENT_OPACITY: a_uiProp = XURPROP_ELEMENT_OPACITY; break;
		case XURPROP_CONTROL_ELEMENT_WIDTH: a_uiProp = XURPROP_ELEMENT_WIDTH; break;
	}

	AGUI2XUIElement::UpdateTimelinePropFloat( a_rResource, a_uiProp, a_iIndex, a_bKeyframeChanged, a_fFrom, a_fTo, a_fT );
}

#endif // BARNYARD_LATEST
