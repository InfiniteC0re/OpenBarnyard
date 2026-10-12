#include "pch.h"
#ifdef BARNYARD_LATEST
#  include "AGUI2XUIText.h"
#  include "AGUI2XUIResource.h"
#  include "Locale/ALocaleManager.h"

#  include <GUI/T2GUIFontManager.h>
#  include <Toshi/TString8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGUI2XUIText );

TCHAR AGUI2XUIText::ms_szDefaultFont[ 48 ];

// The controller button icons of wii_icons128.tga (128x128). The table is filled by the static
// initialiser of the file (8041e0b4), which also registers the class.
static constexpr TFLOAT ICON_UV = 1.0f / 128.0f;

AGUI2XUIText::Icon AGUI2XUIText::ms_aIcons[ 11 ] = {
	{ "1", 0xB1, 22.0f, 21.0f, 1.0f, 1.0f, { 0.0f * ICON_UV, 84.0f * ICON_UV }, { 22.0f * ICON_UV, 105.0f * ICON_UV } },
	{ "2", 0xB2, 22.0f, 22.0f, 1.0f, 1.0f, { 0.0f * ICON_UV, 105.0f * ICON_UV }, { 22.0f * ICON_UV, 127.0f * ICON_UV } },
	{ "A", 0xBC, 32.0f, 32.0f, 1.0f, 1.0f, { 0.0f * ICON_UV, 0.0f * ICON_UV }, { 32.0f * ICON_UV, 32.0f * ICON_UV } },
	{ "B", 0xBD, 33.0f, 45.0f, 1.0f, 1.0f, { 33.0f * ICON_UV, 5.0f * ICON_UV }, { 66.0f * ICON_UV, 50.0f * ICON_UV } },
	{ "C", 0xBE, 40.0f, 28.0f, 1.0f, 1.0f, { 83.0f * ICON_UV, 100.0f * ICON_UV }, { 123.0f * ICON_UV, 128.0f * ICON_UV } },
	{ "dpad", 0xC6, 47.0f, 47.0f, 1.0f, 1.0f, { 24.0f * ICON_UV, 80.0f * ICON_UV }, { 71.0f * ICON_UV, 127.0f * ICON_UV } },
	{ "home", 0xD8, 22.0f, 22.0f, 1.0f, 1.0f, { 0.0f * ICON_UV, 61.0f * ICON_UV }, { 22.0f * ICON_UV, 83.0f * ICON_UV } },
	{ "minus", 0xDE, 22.0f, 22.0f, 1.0f, 1.0f, { 27.0f * ICON_UV, 54.0f * ICON_UV }, { 49.0f * ICON_UV, 76.0f * ICON_UV } },
	{ "plus", 0xF0, 21.0f, 22.0f, 1.0f, 1.0f, { 48.0f * ICON_UV, 54.0f * ICON_UV }, { 69.0f * ICON_UV, 76.0f * ICON_UV } },
	{ "thumbstick", 0xF7, 57.0f, 56.0f, 1.0f, 1.0f, { 71.0f * ICON_UV, 43.0f * ICON_UV }, { 128.0f * ICON_UV, 99.0f * ICON_UV } },
	{ "Z", 0xFE, 51.0f, 41.0f, 1.0f, 1.0f, { 73.0f * ICON_UV, 1.0f * ICON_UV }, { 124.0f * ICON_UV, 42.0f * ICON_UV } },
};

// $BarnyardWiiJP: FUNCTION 8041da30
TBOOL AGUI2XUIText::RenderIcon( T2GUIRenderer* a_pRenderer, TINT a_iChar, TFLOAT* a_pOutAdvance, TFLOAT a_fLineHeight, TFLOAT a_fX, TFLOAT a_fY )
{
	for ( TINT i = 0; i < TARRAYSIZE( ms_aIcons ); i++ )
	{
		Icon& rIcon = ms_aIcons[ i ];

		if ( a_iChar == rIcon.uiChar && ms_pIconMaterial )
		{
			// Centred on the middle of the line
			TFLOAT fHeight = rIcon.fHeight * rIcon.fScaleY;
			TFLOAT fY      = ( a_fY - 0.5f * a_fLineHeight ) - 0.5f * fHeight;

			TVector2 vecPos1( a_fX, fY );
			TVector2 vecPos2( a_fX + rIcon.fWidth * rIcon.fScaleX, fY + fHeight );

			a_pRenderer->SetMaterial( ms_pIconMaterial );
			a_pRenderer->SetColour( 0xFFFFFFFF );
			a_pRenderer->RenderRectangle( vecPos1, vecPos2, rIcon.vecUV1, rIcon.vecUV2 );

			*a_pOutAdvance = rIcon.fWidth * rIcon.fScaleX;
			return TTRUE;
		}
	}

	return TFALSE;
}

// $BarnyardWiiJP: FUNCTION 8041db70
TFLOAT AGUI2XUIText::GetIconWidth( TINT a_iChar )
{
	for ( TINT i = 0; i < TARRAYSIZE( ms_aIcons ); i++ )
	{
		if ( a_iChar == ms_aIcons[ i ].uiChar )
			return ms_aIcons[ i ].fWidth * ms_aIcons[ i ].fScaleX;
	}

	return -1.0f;
}

// The PC retail font for a XUI point size and the scale that gives the text that size
const TCHAR* AGUI2XUIText::GetPCFont( TINT a_iPointSize, TFLOAT& a_rOutScale )
{
	if ( a_iPointSize >= 22 )
	{
		a_rOutScale = a_iPointSize / 26.0f;
		return "Rekord26";
	}

	a_rOutScale = a_iPointSize / 18.0f;
	return "Rekord18";
}

// Finds a text in the strings of the PC locale. Returns the localised string or TNULL
const TWCHAR* AGUI2XUIText::FindLocaleString( const TWCHAR* a_wszText )
{
	if ( !a_wszText || !ALocaleManager::Interface() )
		return TNULL;

	ALocaleManager* pLocale = ALocaleManager::Interface();

	for ( TINT i = 0; i < pLocale->GetNumStrings(); i++ )
	{
		const TWCHAR* wszString = pLocale->GetString( i );

		if ( wszString && TStringManager::String16Compare( wszString, a_wszText ) == 0 )
			return wszString;
	}

	return TNULL;
}

// $BarnyardWiiJP: FUNCTION 8041dbc4
AGUI2XUIText::AGUI2XUIText()
{
	m_pFont = TNULL;
	m_bWrap = TFALSE;
}

// $BarnyardWiiJP: FUNCTION 8041dc24
// $BarnyardWiiJP: FUNCTION 8041e804
AGUI2XUIText::~AGUI2XUIText()
{
	T2GUIFontManager::ReleaseFont( m_pFont );
}

// $BarnyardWiiJP: FUNCTION 8041dca8
TBOOL AGUI2XUIText::Create( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	XURXUITextData* pData = TSTATICCAST( XURXUITextData, a_pData );

	AGUI2XUIElement::Create( a_rResource, a_pData, a_bCreateChildren );

	// NOTE: the Wii fonts aren't supported, so XUI texts use the PC retail fonts (data/gui/fonts.trb
	// has Rekord18 and Rekord26). The Wii build turns the font name into "<name>_<point size>"
	// ('-' and ' ' become '_') and maps every font to its Wii one:
	//   Rekord -> Rekord_Black -> HGPSoeiKakupoptai
	//   CCThatsAllFolks, CCBiffBamBoom -> GlassJaw_BB -> HGPSoeiKakupoptai
	// and takes ms_szDefaultFont ("Arial_14") for texts without a font. Here every font maps to the
	// PC font closest to its point size, scaled to that size (see GetPCFont).
	TFLOAT       fScale;
	const TCHAR* szFont = GetPCFont( ( pData->m_usFont != 0 ) ? TINT( pData->m_uiPointSize ) : 14, fScale );

	m_pFont = T2GUIFontManager::FindFont( szFont );
	m_oTextBox.Create( m_pFont, 0.0f );
	m_oTextBox.SetScale( fScale );
	m_oTextBox.SetIncludeNewLine( TTRUE );

	if ( pData->m_usText != 0 )
	{
		a_rResource.PushID( L"Text" );

		// NOTE: the string tables of the Wii build have texts that aren't supported.
		// The English text of the scene is used, or the same text of the PC locale (the translation
		// when the PC locale isn't English) if it has one.
		const TWCHAR* wszText = FindLocaleString( a_rResource.GetString( pData->m_usText ) );

		if ( wszText )
			m_oTextBox.SetTextNoResize( wszText );
		else
			m_oTextBox.SetTextNoResize( a_rResource.GetString( pData->m_usText ) );

		m_oTextBox.SetMaxChars( -1 );

		a_rResource.PopID();
	}

	m_oTextBox.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );

	TFLOAT fHeight = 1.05f * GetHeight();
	m_oTextBox.SetSize( GetWidth(), fHeight, TTRUE );
	m_oTextBox.SetColour( pData->m_uiTextColor );
	m_oTextBox.SetShadowColour( pData->m_uiDropShadowColor );

	if ( pData->m_uiTextStyle & TEXTSTYLE_LEFT )
		m_oTextBox.SetTextAlign( T2GUIFont::TextAlign_Left );
	else if ( pData->m_uiTextStyle & TEXTSTYLE_CENTER )
		m_oTextBox.SetTextAlign( T2GUIFont::TextAlign_Center );
	else if ( pData->m_uiTextStyle & TEXTSTYLE_RIGHT )
		m_oTextBox.SetTextAlign( T2GUIFont::TextAlign_Right );
	else
		m_oTextBox.SetTextAlign( T2GUIFont::TextAlign_Left );

	m_oTextBox.SetShadowEnabled( ( pData->m_uiTextStyle & TEXTSTYLE_DROPSHADOW ) != 0 );
	m_bWrap = ( pData->m_uiTextStyle & TEXTSTYLE_NOWRAP ) == 0;

	m_oTextBox.SetIconRenderCallback( RenderIcon );
	m_oTextBox.SetIconWidthCallback( GetIconWidth );

	AddChildTail( m_oTextBox );

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 8041e030
void AGUI2XUIText::Render()
{
	// Texts that don't wrap draw only their first line
	TBOOL bMultipleLines         = T2GUIFont::ms_bMultipleLines;
	T2GUIFont::ms_bMultipleLines = m_bWrap;

	T2GUIElement::Render();

	T2GUIFont::ms_bMultipleLines = bMultipleLines;
}

// $BarnyardWiiJP: FUNCTION 8041e068
void AGUI2XUIText::SetWidth( TFLOAT a_fWidth )
{
	m_oTextBox.SetWidth( a_fWidth );
	m_fWidth = a_fWidth;
}

#endif // BARNYARD_LATEST
