#include "pch.h"
#include "AGUI2MenuGridRectItem.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AGUI2MenuGridRectItem ) == 0xec );

// $Barnyard: FUNCTION 0063f760
AGUI2MenuGridRectItem::AGUI2MenuGridRectItem()
{
}

// $Barnyard: FUNCTION 0063f7b0
// $Barnyard: FUNCTION 0063f8b0
AGUI2MenuGridRectItem::~AGUI2MenuGridRectItem()
{
	m_oRectangle.RemoveSelf();
}

// $Barnyard: FUNCTION 0063f820
void AGUI2MenuGridRectItem::Create( T2GUITextureSection* a_pTexSec, TFLOAT a_fSize, TFLOAT a_fHeight )
{
	m_oRectangle.SetTextureSection( *a_pTexSec );

	TFLOAT fAspect = TMath::Abs( TFLOAT( a_pTexSec->GetPoint2X() - a_pTexSec->GetPoint1X() ) ) / TMath::Abs( TFLOAT( a_pTexSec->GetPoint2Y() - a_pTexSec->GetPoint1Y() ) );
	TFLOAT fWidth  = a_fSize / fAspect;

	m_oRectangle.SetDimensions( fWidth, a_fHeight );
	SetDimensions( fWidth, a_fHeight );
	AddChildTail( m_oRectangle );
}

// $Barnyard: FUNCTION 0063f8a0
void AGUI2MenuGridRectItem::SetAlpha( TFLOAT a_fAlpha )
{
	// NOTE: folded with AGUI2MenuGridTickerItem::SetAlpha (both forward to the element at 0x74)
	m_oRectangle.SetAlpha( a_fAlpha );
}
