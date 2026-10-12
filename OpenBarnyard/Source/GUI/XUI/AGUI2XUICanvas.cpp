#include "pch.h"
#ifdef BARNYARD_LATEST
#  include "AGUI2XUICanvas.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGUI2XUICanvas );

// $BarnyardWiiJP: FUNCTION 80415388
AGUI2XUICanvas::AGUI2XUICanvas()
    : m_oTimelinePlayer( &m_oTimelineControl )
{
}

// $BarnyardWiiJP: FUNCTION 804153e0
// $BarnyardWiiJP: FUNCTION 80415698
AGUI2XUICanvas::~AGUI2XUICanvas()
{
}

// $BarnyardWiiJP: FUNCTION 80415454
TBOOL AGUI2XUICanvas::Create( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	AGUI2XUIElement::Create( a_rResource, a_pData, a_bCreateChildren );
	m_oTimelineControl.Create( this, a_rResource, a_pData );

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 804154b0
void AGUI2XUICanvas::Tick( TFLOAT a_fDeltaTime )
{
	if ( IsVisible() )
		m_oTimelinePlayer.Tick( a_fDeltaTime );

	AGUI2XUIElement::Tick( a_fDeltaTime );
}

// $BarnyardWiiJP: FUNCTION 8041552c
void AGUI2XUICanvas::PlayAnimation( const TWCHAR* a_wszStartFrame, const TWCHAR* a_wszEndFrame )
{
	m_oTimelinePlayer.PlayAnimation( a_wszStartFrame, a_wszEndFrame );
	AGUI2XUIElement::PlayAnimation( a_wszStartFrame, a_wszEndFrame );
}

// $BarnyardWiiJP: FUNCTION 80415584
void AGUI2XUICanvas::StopAnimation()
{
	m_oTimelinePlayer.Stop();
	AGUI2XUIElement::StopAnimation();
}

// $BarnyardWiiJP: FUNCTION 804155bc
void AGUI2XUICanvas::AddXUIChild( AGUI2XUIElement* a_pElement )
{
	AddChildTail( *a_pElement );

	// Let the timeline that animates this element know about it
	AGUI2XUITimeline* pTimeline = m_oTimelineControl.FindTimeline( a_pElement->GetId() );

	if ( pTimeline )
		pTimeline->SetElement( a_pElement );
}

#endif // BARNYARD_LATEST
