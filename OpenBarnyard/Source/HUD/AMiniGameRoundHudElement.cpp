#include "pch.h"
#include "AMiniGameRoundHudElement.h"
#include "GUI/AGUI2.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"

#include <GUI/T2GUIFontManager.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00456b30
AMiniGameRoundHudElement::AMiniGameRoundHudElement()
    : AHudElement( MASK_MINIGAME )
    , m_fDuration( 1.0f )
    , m_iNumPoints( 0 )
    , m_iTitleLocale( -1 )
    , m_iLocale( -1 )
    , m_fTimer( 0.0f )
    , m_iRound( -1 )
    , m_iNumRounds( -1 )
    , m_bDone( TFALSE )
    , m_bTitle( TFALSE )
    , m_iState( 0 )
{
}

// $Barnyard: FUNCTION 00456f90
// $Barnyard: FUNCTION 00456bc0
AMiniGameRoundHudElement::~AMiniGameRoundHudElement()
{
}

// $Barnyard: FUNCTION 00456fb0
TBOOL AMiniGameRoundHudElement::OnAttach()
{
	m_bDone  = TTRUE;
	m_bTitle = TFALSE;

	m_oText.Create( T2GUIFontManager::FindFont( "Rekord26" ), 0.0f );
	m_oText.SetText( m_wszText );
	m_oText.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oText.SetTranslation( 0.0f, AGUI2::GetRootElement()->GetHeight() * 0.25f );

	AGUI2::GetRootElement()->AddChildTail( m_oText );

	return TTRUE;
}

// $Barnyard: FUNCTION 00456d90
void AMiniGameRoundHudElement::OnDetach()
{
	m_oText.RemoveSelf();
}

// $Barnyard: FUNCTION 00456db0
void AMiniGameRoundHudElement::Show()
{
	m_oText.Show();
	m_oText.SetAlpha( 1.0f );
	m_oBackground.SetAlpha( 1.0f );
	m_bCurrentlyVisible = TTRUE;
}

// $Barnyard: FUNCTION 00456df0
void AMiniGameRoundHudElement::Hide()
{
	m_oText.Hide();
	m_bCurrentlyVisible = TFALSE;
}

// $Barnyard: FUNCTION 00456e00
void AMiniGameRoundHudElement::Update( TFLOAT a_fDeltaTime )
{
	TFLOAT fTime = m_fTimer - ( m_bTitle ? 1.0f : 0.0f );

	if ( m_fDuration + 4.0f < fTime )
	{
		m_bDone = TTRUE;
		return;
	}

	TFLOAT fAlpha = TFLOAT( ( m_oText.GetColour() >> 24 ) & 0xFF ) * ( 1.0f / 255.0f );
	m_fTimer += a_fDeltaTime;

	if ( fAlpha <= 0.0f )
	{
		fAlpha       = 0.0f;
		m_bDone      = TTRUE;
		m_bForceHide = TTRUE;
	}
	else if ( m_fDuration + 1.5f < fTime )
	{
		// Fade out
		fAlpha -= a_fDeltaTime * 5.0f;

		if ( fAlpha < 0.0f )
			fAlpha = 0.0f;
	}
	else if ( fTime > 0.0f && m_iState != 1 )
	{
		// The title is over, show the round
		TINT iLocale = m_bTitle ? m_iTitleLocale : m_iLocale;

		if ( iLocale > -1 )
		{
			TStringManager::String16Format( m_wszText, 64, ALocaleManager::Interface()->GetString( iLocale ) );
			m_oText.SetTextAndFitWidth( m_wszText );
		}
		else
		{
			FormatRoundText();
		}

		m_iState = 1;
	}

	m_oText.SetAlpha( fAlpha );
}

// $Barnyard: FUNCTION 00457030
void AMiniGameRoundHudElement::Start( TBOOL a_bTitle, TINT a_iRound )
{
	m_iRound     = a_iRound;
	m_iNumRounds = -1;
	m_fTimer     = 0.0f;
	m_bDone      = TFALSE;
	m_bForceHide = TFALSE;
	m_bTitle     = a_bTitle;

	if ( a_bTitle )
	{
		if ( m_iTitleLocale > -1 )
			TStringManager::String16Format( m_wszText, 64, ALocaleManager::Interface()->GetString( m_iTitleLocale ) );
		else
			TStringManager::String16Format( m_wszText, 64, L"oops (localise me!)" );

		m_oText.SetTextAndFitWidth( m_wszText );
	}
	else
	{
		FormatRoundText();
	}
}

// $Barnyard: FUNCTION 00456c30
void AMiniGameRoundHudElement::SetParent( T2GUIElement* a_pParent )
{
	if ( a_pParent )
	{
		m_oText.RemoveSelf();
		a_pParent->AddChildTail( m_oText );
	}
}

// $Barnyard: FUNCTION 00456c60
void AMiniGameRoundHudElement::FormatRoundText()
{
	if ( m_iNumRounds > -1 )
		ALocale::FormatTags( m_wszText, 64, 993, PPropertyValue( m_iNumRounds ), PPropertyValue( m_iRound ) );
	else
		ALocale::FormatTags( m_wszText, 64, 994, PPropertyValue( m_iRound ) );

	if ( m_iNumPoints != 0 )
		ALocale::FormatTags( m_wszPoints, 64, 990, PPropertyValue( m_iNumPoints ) );

	m_oText.SetTextAndFitWidth( m_wszText );
}
