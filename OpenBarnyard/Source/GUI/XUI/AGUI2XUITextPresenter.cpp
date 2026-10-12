#include "pch.h"
#ifdef BARNYARD_LATEST
#  include "AGUI2XUITextPresenter.h"
#  include "AGUI2XUIText.h"
#  include "AGUI2XUIResource.h"

#  include <GUI/T2GUIFontManager.h>
#  include <Toshi/TString8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGUI2XUITextPresenter );

// $BarnyardWiiJP: FUNCTION 8041e8a4
AGUI2XUITextPresenter::AGUI2XUITextPresenter()
{
#  ifdef BARNYARD_COMMUNITY_PATCH
	// The original doesn't initialise it, but Create reads it when the data has no font
	m_pFont = TNULL;
#  endif
}

// $BarnyardWiiJP: FUNCTION 8041e8f0
// $BarnyardWiiJP: FUNCTION 8041ebf0
AGUI2XUITextPresenter::~AGUI2XUITextPresenter()
{
}

// $BarnyardWiiJP: FUNCTION 8041e958
TBOOL AGUI2XUITextPresenter::Create( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	XURXUITextPresenterData* pData = TSTATICCAST( XURXUITextPresenterData, a_pData );

	AGUI2XUIElement::Create( a_rResource, a_pData, a_bCreateChildren );

	TString8 strFont;

	if ( pData->m_usFont != 0 )
	{
		strFont = a_rResource.GetString( pData->m_usFont );

		if ( strFont == "Rekord" )
			strFont = "Rekord_Black";

		TString8 strSize;
		strSize.Format( "_%d", pData->m_uiPointSize );
		strFont.Concat( strSize );

		m_pFont = T2GUIFontManager::FindFont( strFont );
	}

	if ( !m_pFont )
		m_pFont = T2GUIFontManager::FindFont( AGUI2XUIText::ms_szDefaultFont );

	m_oTextBox.Create( m_pFont, 0.0f );
	m_oTextBox.SetSize( GetWidth(), GetHeight(), TTRUE );
	m_oTextBox.SetColour( pData->m_uiTextColor );
	m_oTextBox.SetShadowColour( pData->m_uiDropShadowColor );
	m_oTextBox.SetColour( 0xFF000000 );

	if ( pData->m_uiTextStyle & AGUI2XUIText::TEXTSTYLE_LEFT )
		m_oTextBox.SetTextAlign( T2GUIFont::TextAlign_Left );
	else if ( pData->m_uiTextStyle & AGUI2XUIText::TEXTSTYLE_CENTER )
		m_oTextBox.SetTextAlign( T2GUIFont::TextAlign_Center );
	else if ( pData->m_uiTextStyle & AGUI2XUIText::TEXTSTYLE_RIGHT )
		m_oTextBox.SetTextAlign( T2GUIFont::TextAlign_Right );

	m_oTextBox.SetShadowEnabled( ( pData->m_uiTextStyle & AGUI2XUIText::TEXTSTYLE_DROPSHADOW ) != 0 );
	AddChildTail( m_oTextBox );

	return TTRUE;
}

#endif // BARNYARD_LATEST
