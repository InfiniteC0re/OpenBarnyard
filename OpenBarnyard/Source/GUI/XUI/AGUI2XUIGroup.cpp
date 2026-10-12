#include "pch.h"
#ifdef BARNYARD_LATEST
#  include "AGUI2XUIGroup.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGUI2XUIGroup );

// $BarnyardWiiJP: FUNCTION 8041986c
AGUI2XUIGroup::AGUI2XUIGroup()
    : m_oTimelinePlayer( &m_oTimelineControl )
{
}

// $BarnyardWiiJP: FUNCTION 804198c4
// $BarnyardWiiJP: FUNCTION 80419b88
AGUI2XUIGroup::~AGUI2XUIGroup()
{
}

// $BarnyardWiiJP: FUNCTION 80419938
TBOOL AGUI2XUIGroup::Create( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	AGUI2XUIElement::Create( a_rResource, a_pData, TTRUE );
	m_oTimelineControl.Create( this, a_rResource, a_pData );

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 80419998
void AGUI2XUIGroup::Tick( TFLOAT a_fDeltaTime )
{
	if ( IsVisible() )
		m_oTimelinePlayer.Tick( a_fDeltaTime );

	AGUI2XUIElement::Tick( a_fDeltaTime );
}

// $BarnyardWiiJP: FUNCTION 80419a14
void AGUI2XUIGroup::PlayAnimation( const TWCHAR* a_wszStartFrame, const TWCHAR* a_wszEndFrame )
{
	m_oTimelinePlayer.PlayAnimation( a_wszStartFrame, a_wszEndFrame );
	AGUI2XUIElement::PlayAnimation( a_wszStartFrame, a_wszEndFrame );
}

// $BarnyardWiiJP: FUNCTION 80419a6c
void AGUI2XUIGroup::PlayTimelineRange( TFLOAT a_fStart, TFLOAT a_fEnd )
{
	m_oTimelinePlayer.PlayTimelineRange( a_fStart, a_fEnd );
	AGUI2XUIElement::PlayTimelineRange( a_fStart, a_fEnd );
}

// $BarnyardWiiJP: FUNCTION 80419ac4
void AGUI2XUIGroup::StopAnimation()
{
	m_oTimelinePlayer.Stop();
	AGUI2XUIElement::StopAnimation();
}

#endif // BARNYARD_LATEST
