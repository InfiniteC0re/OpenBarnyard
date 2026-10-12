#include "pch.h"
#ifdef BARNYARD_LATEST
#  include "AGUI2XUIButton.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGUI2XUIButton );

// $BarnyardWiiJP: FUNCTION 80415140
AGUI2XUIButton::AGUI2XUIButton()
{
}

// $BarnyardWiiJP: FUNCTION 80415184
// $BarnyardWiiJP: FUNCTION 804152e8
AGUI2XUIButton::~AGUI2XUIButton()
{
}

// $BarnyardWiiJP: FUNCTION 804151dc
TBOOL AGUI2XUIButton::Create( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	AGUI2XUIControl::Create( a_rResource, a_pData, a_bCreateChildren );
	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 80415200
void AGUI2XUIButton::Press()
{
	if ( ( m_eState & STATE_PRESSED ) == 0 )
		PlayAnimation( L"Press", L"EndPress" );

	m_eState = STATE_PRESSED;
}

#endif // BARNYARD_LATEST
