#include "pch.h"
#include "AGUI2FadingText.h"
#include "GUI/AGUI2.h"
#include "Locale/ALocaleManager.h"

#include <GUI/T2GUIFontManager.h>
#include <Math/TMathInline.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 006423e0
AGUI2FadingText::AGUI2FadingText()
{
	m_fUnk2     = 3.0f;
	m_fUnk3     = 0.1f;
	m_fFadeTime = 1.0f;
	m_fUnk4     = 0.5f;
	m_fUnk5     = 1.0f;
	m_fUnk1     = 0.0f;
	m_fTime     = 0.0f;
	m_fDelay    = 0.0f;
	m_vUnk6     = TVector2::VEC_ZERO;
	m_vUnk7     = TVector2::VEC_ZERO;
	m_pTextFont = TNULL;

	TStringManager::String16Copy( m_wszBuffer, L" " );
	SetAlpha( 0.0f );
	m_pTextFont = TNULL;
	SetAlpha( 1.0f );

	SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	SetTransform( 0.0f, AGUI2::GetRootElement()->GetHeight() * 0.25f );
	AGUI2::GetRootElement()->AddChildTail( *this );
}

// $Barnyard: FUNCTION 00642110
AGUI2FadingText::~AGUI2FadingText()
{
	RemoveSelf();
}

// $Barnyard: FUNCTION 00642340
void AGUI2FadingText::Create( T2GUIFont* a_pFont, T2GUIElement* a_pParent, T2GUIATTACHMENT a_eAnchor, T2GUIATTACHMENT a_ePivot )
{
	m_pTextFont = a_pFont;
	SetAlpha( 1.0f );

	if ( a_pParent )
	{
		SetTransform( 0.0f, a_pParent->GetHeight() * 0.25f );
		SetAttachment( a_eAnchor, a_ePivot );
		a_pParent->AddChildTail( *this );
	}
	else
	{
		SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
		SetTransform( 0.0f, AGUI2::GetRootElement()->GetHeight() * 0.25f );
		AGUI2::GetRootElement()->AddChildTail( *this );
	}
}

// $Barnyard: FUNCTION 00642170
void AGUI2FadingText::SetText( const TWCHAR* a_wszText, TUINT32 a_uiColour )
{
	m_fTime = 0.0f;
	Show();
	SetAlpha( 1.0f );

	if ( TStringManager::String16Compare( m_wszBuffer, a_wszText ) != 0 )
	{
		TStringManager::String16CopySafe( m_wszBuffer, a_wszText, MAX_TEXT_LENGTH );

		T2GUIFont* pFont = m_pTextFont ? m_pTextFont : T2GUIFontManager::FindFont( "Rekord26" );
		AGUI2TextBox::Create( pFont, pFont->GetTextWidth( a_wszText ) );
		AGUI2TextBox::SetText( m_wszBuffer );
		SetColour( 0xFFFFFFFF );
	}
}

// $Barnyard: FUNCTION 00642200
void AGUI2FadingText::SetLocaleText( TINT a_iLocaleId, TINT a_iUnused )
{
	Show();
	m_fTime = 0.0f;
	TStringManager::String16CopySafe( m_wszBuffer, ALocaleManager::Interface()->GetString( a_iLocaleId ), MAX_TEXT_LENGTH );

	T2GUIFont* pFont = m_pTextFont ? m_pTextFont : T2GUIFontManager::FindFont( "Rekord26" );
	AGUI2TextBox::Create( pFont, pFont->GetTextWidth( m_wszBuffer ) );
	AGUI2TextBox::SetText( m_wszBuffer );
	SetColour( 0xFFFFFFFF );
	Show();
	SetAlpha( 1.0f );
}

// $Barnyard: FUNCTION 00642290
void AGUI2FadingText::Update( TFLOAT a_fDeltaTime )
{
	m_fDelay -= a_fDeltaTime;

	if ( m_fDelay <= 0.0f )
		m_fTime += a_fDeltaTime;
	else
		m_fTime = 0.0f;

	if ( IsVisible() && m_fTime > m_fFadeTime )
	{
		TFLOAT fAlpha = TUINT8( m_uiColour >> 24 ) * ( 1.0f / 255.0f ) - a_fDeltaTime * 5.0f;
		TMath::Clip( fAlpha, 0.0f, 1.0f );
		SetAlpha( fAlpha );
	}
}
