#include "pch.h"
#include "AGUI2DayOfWeek.h"
#include "AGUI2.h"

#include <GUI/T2GUIFontManager.h>
#include "Locale/ALocaleManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0043ce20
// $Barnyard: FUNCTION 0043ce70
AGUI2DayOfWeek::~AGUI2DayOfWeek()
{
}

// $Barnyard: FUNCTION 00637f30
void AGUI2DayOfWeek::Create()
{
	m_oText.Create( T2GUIFontManager::FindFont( "Rekord26" ), 0.0f );
	m_oText.T2GUIElement::Hide();
	AddChildTail( m_oText );
	m_oText.SetTransform( 0.0f, -130.0f );

	AGUI2::GetRootElement()->AddChildTail( *this );
}

// $Barnyard: FUNCTION 00637e10
void AGUI2DayOfWeek::Destroy()
{
	m_oText.RemoveSelf();
}

// $Barnyard: FUNCTION 00637e30
void AGUI2DayOfWeek::Show( TINT a_iWeekDay )
{
	const TWCHAR* aDayNames[ 7 ];
	aDayNames[ 0 ] = ALocaleManager::Interface()->GetString( 1169 );
	aDayNames[ 1 ] = ALocaleManager::Interface()->GetString( 1170 );
	aDayNames[ 2 ] = ALocaleManager::Interface()->GetString( 1171 );
	aDayNames[ 3 ] = ALocaleManager::Interface()->GetString( 1172 );
	aDayNames[ 4 ] = ALocaleManager::Interface()->GetString( 1173 );
	aDayNames[ 5 ] = ALocaleManager::Interface()->GetString( 1174 );
	aDayNames[ 6 ] = ALocaleManager::Interface()->GetString( 1168 );

	m_oText.SetTextAndFitWidth( aDayNames[ a_iWeekDay ] );
	m_oText.T2GUIElement::Show();
	m_oText.SetAlpha( 1.0f );

	m_fTimeLeft = 6.0f;
	m_fFadeTime = 2.0f;
}

// $Barnyard: FUNCTION 00637f80
void AGUI2DayOfWeek::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_fTimeLeft <= 0.0f )
	{
		m_oText.T2GUIElement::Hide();
		return;
	}

	if ( m_fTimeLeft - m_fFadeTime < 0.0f )
	{
		// Fade out
		TFLOAT fAlpha = m_fTimeLeft * 0.5f;
		m_oText.SetAlpha( ( fAlpha <= 0.0f ) ? 0.0f : fAlpha );
	}

	m_fTimeLeft -= a_fDeltaTime;
}
