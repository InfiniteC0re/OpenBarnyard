#include "ToshiPCH.h"
#include "TGUIColour.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $JPOG: FUNCTION TGuiInterface.dll+00008b40
TGUIColour::TGUIColour( TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue, TFLOAT a_fAlpha )
{
	m_uiAlpha = TUINT8( TINT64( a_fAlpha * 255.0f ) );
	m_uiRed   = TUINT8( TINT64( a_fRed * 255.0f ) );
	m_uiGreen = TUINT8( TINT64( a_fGreen * 255.0f ) );
	m_uiBlue  = TUINT8( TINT64( a_fBlue * 255.0f ) );
}

// $JPOG: FUNCTION TGuiInterface.dll+00008ae0
TGUIColour::TGUIColour( TUINT a_uiColour, FormatARGB a_eFormat )
{
	m_uiAlpha = TUINT8( a_uiColour >> 24 );
	m_uiRed   = TUINT8( a_uiColour >> 16 );
	m_uiGreen = TUINT8( a_uiColour >> 8 );
	m_uiBlue  = TUINT8( a_uiColour );
}

// $JPOG: FUNCTION TGuiInterface.dll+00008b10
TGUIColour::TGUIColour( TUINT a_uiColour, FormatABGR a_eFormat )
{
	m_uiAlpha = TUINT8( a_uiColour >> 24 );
	m_uiRed   = TUINT8( a_uiColour );
	m_uiGreen = TUINT8( a_uiColour >> 8 );
	m_uiBlue  = TUINT8( a_uiColour >> 16 );
}

// $JPOG: FUNCTION TGuiInterface.dll+00008910
TGUIColour& TGUIColour::operator*=( const TGUIColour& a_rcOther )
{
	m_uiAlpha = TUINT8( TUINT16( m_uiAlpha * a_rcOther.m_uiAlpha ) >> 8 );
	m_uiRed   = TUINT8( ( m_uiRed * a_rcOther.m_uiRed ) >> 8 );
	m_uiGreen = TUINT8( TUINT16( m_uiGreen * a_rcOther.m_uiGreen ) >> 8 );
	m_uiBlue  = TUINT8( TUINT16( m_uiBlue * a_rcOther.m_uiBlue ) >> 8 );
	return *this;
}

// $JPOG: FUNCTION TGuiInterface.dll+000089a0
void TGUIColour::SetAlpha( TFLOAT a_fAlpha )
{
	m_uiAlpha = TUINT8( TINT64( a_fAlpha * 255.0f ) );
}

// $JPOG: FUNCTION TGuiInterface.dll+00008a40
void TGUIColour::Get( TFLOAT& a_rfAlpha, TFLOAT& a_rfRed, TFLOAT& a_rfGreen, TFLOAT& a_rfBlue ) const
{
	const TFLOAT fScale = 1.0f / 255.0f;

	a_rfAlpha = m_uiAlpha * fScale;
	a_rfRed   = m_uiRed * fScale;
	a_rfGreen = m_uiGreen * fScale;
	a_rfBlue  = m_uiBlue * fScale;
}

TOSHI_NAMESPACE_END
