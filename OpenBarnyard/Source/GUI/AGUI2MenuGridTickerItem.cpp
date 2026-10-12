#include "pch.h"
#include "AGUI2MenuGridTickerItem.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AGUI2MenuGridTickerItem ) == 0x170 );

// $Barnyard: FUNCTION 0063fdc0
AGUI2MenuGridTickerItem::AGUI2MenuGridTickerItem()
{
}

// $Barnyard: FUNCTION 0063fe10
// $Barnyard: FUNCTION 0063ff40
AGUI2MenuGridTickerItem::~AGUI2MenuGridTickerItem()
{
	m_oTicker.RemoveSelf();
}

// $Barnyard: FUNCTION 0063fe80
void AGUI2MenuGridTickerItem::Create( TINT a_iScore )
{
	m_oTicker.Create( 4, TNULL, -1.0f, -1.0f );
	m_oTicker.SetScore( a_iScore );
	m_oTicker.SetSpinning( TTRUE );

	T2GUITransform& rTransform = m_oTicker.GetTransform();
	rTransform.SetIdentity();
	rTransform.Scale( ms_fScale, ms_fScale );
	rTransform.SetTranslation( 0.0f, m_oTicker.GetHeight() * 0.05f );

	SetDimensions( m_oTicker.GetWidth() * ms_fScale * 0.75f, m_oTicker.GetHeight() * ms_fScale * 0.75f );
	AddChildTail( m_oTicker );
}

// NOTE: folded with AGUI2MenuGridRectItem::SetAlpha 0063f8a0 (both forward to the element at 0x74)
void AGUI2MenuGridTickerItem::SetAlpha( TFLOAT a_fAlpha )
{
	m_oTicker.SetAlpha( a_fAlpha );
}
