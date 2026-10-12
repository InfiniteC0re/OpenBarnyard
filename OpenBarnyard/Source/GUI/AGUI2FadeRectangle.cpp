#include "pch.h"
#include "AGUI2FadeRectangle.h"
#include "AGUI2.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00638e20
AGUI2FadeRectangle::AGUI2FadeRectangle()
    : m_bFading( TFALSE )
{
	TFLOAT fWidth;
	TFLOAT fHeight;
	AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );
	SetDimensions( fWidth, fHeight );
}

// $Barnyard: FUNCTION 00638d00
void AGUI2FadeRectangle::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_bFading != TTRUE )
		return;

	TINT iA = TINT( TMath::LERPClamped( TFLOAT( m_iFromA ), TFLOAT( m_iToA ), m_fTime / m_fDuration ) );
	TINT iR = TINT( TMath::LERPClamped( TFLOAT( m_iFromR ), TFLOAT( m_iToR ), m_fTime / m_fDuration ) );
	TINT iG = TINT( TMath::LERPClamped( TFLOAT( m_iFromG ), TFLOAT( m_iToG ), m_fTime / m_fDuration ) );
	TINT iB = TINT( TMath::LERPClamped( TFLOAT( m_iFromB ), TFLOAT( m_iToB ), m_fTime / m_fDuration ) );
	SetColour( TUINT32( iB | ( ( iG | ( ( iR | ( iA << 8 ) ) << 8 ) ) << 8 ) ) );

	m_fTime += a_fDeltaTime;

	if ( m_fTime > m_fDuration )
	{
		m_bFading = TFALSE;
		RemoveSelf();
	}
}

// $Barnyard: FUNCTION 00638eb0
void AGUI2FadeRectangle::Start( TUINT32 a_uiFromColour, TUINT32 a_uiToColour, TFLOAT a_fDuration )
{
	SetColour( a_uiFromColour );

	m_iFromR    = ( a_uiFromColour >> 16 ) & 0xff;
	m_iFromB    = a_uiFromColour & 0xff;
	m_iFromG    = ( a_uiFromColour >> 8 ) & 0xff;
	m_iToB      = a_uiToColour & 0xff;
	m_iToG      = ( a_uiToColour >> 8 ) & 0xff;
	m_iFromA    = ( a_uiFromColour >> 24 ) & 0xff;
	m_iToR      = ( a_uiToColour >> 16 ) & 0xff;
	m_iToA      = ( a_uiToColour >> 24 ) & 0xff;
	m_fDuration = a_fDuration;
	m_fTime     = 0.0f;
	m_bFading   = TTRUE;

	AGUI2::GetRootElement()->AddChildTail( *this );
}
