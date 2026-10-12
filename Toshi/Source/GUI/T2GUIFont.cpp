#include "ToshiPCH.h"
#include "T2GUIFont.h"
#include "T2GUI.h"

#include <cwctype>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006c3340
T2GUIFont::~T2GUIFont()
{
	Destroy();
}

// $Barnyard: FUNCTION 006c2d60
void T2GUIFont::Destroy()
{
	if ( m_bFlag )
	{
		// A clone, only the definition is owned
		TFree( m_pFontDef );
		m_pFontDef = TNULL;
		return;
	}

	if ( m_pFontDef )
	{
		for ( TINT i = 0; i < m_pFontDef->uiNumMaterials; i++ )
			T2GUI::GetRenderer()->DestroyMaterial( m_ppMaterials[ i ] );

		TFree( m_ppMaterials );
		m_ppMaterials = TNULL;
		m_pFontDef    = TNULL;
	}
}

// $Barnyard: FUNCTION 006c2f80
T2GUIFont* T2GUIFont::Clone()
{
	T2GUIFont* pFont = new T2GUIFont();

	pFont->m_pFontDef = TSTATICCAST( T2GUIFontDef, TMalloc( sizeof( T2GUIFontDef ) ) );
	TUtil::MemCopy( pFont->m_pFontDef, m_pFontDef, sizeof( T2GUIFontDef ) );
	pFont->m_ppMaterials = m_ppMaterials;
	pFont->m_bFlag       = TTRUE;


	return pFont;
}

// $Barnyard: FUNCTION 006c2cf0
TBOOL T2GUIFont::Create( T2GUIFontDef* a_pFontDef )
{
	TASSERT( a_pFontDef->uiNumMaterials != 0 );

	m_pFontDef    = a_pFontDef;
	m_ppMaterials = new T2GUIMaterial*[ a_pFontDef->uiNumMaterials ];

	auto pRenderer    = T2GUI::GetRenderer();
	auto pTextureName = a_pFontDef->szTextureNames;

	for ( TUINT8 i = 0; i < a_pFontDef->uiNumMaterials; i++ )
	{
		m_ppMaterials[ i ] = pRenderer->CreateMaterial( pTextureName );

		while ( *pTextureName != '\0' ) pTextureName++;
		pTextureName++;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 006c3410
void T2GUIFont::DrawTextWrapped( const TWCHAR* a_wszText, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fWidth, TFLOAT a_fHeight, TUINT32 a_uiColour, TFLOAT a_fScale, TextAlign a_eAlign, void* a_fnCallback /*= TNULL*/ )
{
	auto pFontDef = m_pFontDef;

	if ( a_wszText && a_wszText[ 0 ] != '\0' && pFontDef )
	{
		auto pTextBuffer = a_wszText;

		do
		{
			auto pTextBuffer2 = pTextBuffer;

			while ( *pTextBuffer2 != L'\0' && std::iswspace( *pTextBuffer2 ) != 0 )
			{
				if ( *pTextBuffer2 == L'\n' ) break;
				pTextBuffer2++;
			}

			auto wChar = *pTextBuffer2;

			if ( wChar == L'\n' )
			{
				pTextBuffer = pTextBuffer2 + 1;
			}
			else
			{
				if ( wChar == L'\0' ) return;

				pTextBuffer         = TNULL;
				TFLOAT fWidth1      = 0.0f;
				TFLOAT fWidth2      = 0.0f;
				auto   pTextBuffer3 = pTextBuffer2;

				do {
					if ( auto pCharData = FindCharData( wChar ) )
						fWidth1 += ( pFontDef->iLetterSpacing + pCharData->iRightPadding + pCharData->iLeftPadding + pCharData->ui8CharWidth ) * a_fScale;

					if ( wChar == L'\n' )
					{
						fWidth2     = fWidth1;
						pTextBuffer = pTextBuffer3;
						break;
					}

					if ( std::iswspace( wChar ) != 0 && *pTextBuffer3 != L'\xA0' )
					{
						fWidth2     = fWidth1;
						pTextBuffer = pTextBuffer3;
					}

					wChar = pTextBuffer3[ 1 ];
					if ( wChar == L'\0' ) break;
					pTextBuffer3++;

				} while ( fWidth1 < a_fWidth || pTextBuffer == TNULL );

				if ( pTextBuffer3[ 1 ] == L'\0' )
				{
					pTextBuffer = pTextBuffer3 + 1;
					fWidth2     = fWidth1;
				}

				TFLOAT fPosX;

				if ( a_eAlign == TextAlign_Left )
					fPosX = a_fX;
				else if ( a_eAlign == TextAlign_Center )
					fPosX = ( a_fWidth - fWidth2 ) * 0.5f + a_fX;
				else if ( a_eAlign == TextAlign_Right )
					fPosX = ( a_fWidth - fWidth2 ) + a_fX;
				else
					fPosX = a_fX;

				DrawTextSingleLine( pTextBuffer2, pTextBuffer - pTextBuffer2, fPosX, a_fY, a_uiColour, a_fScale, a_fnCallback );
			}

			a_fY += ( pFontDef->uiLineHeight + pFontDef->uiLinesMargin ) * a_fScale;

		} while ( *pTextBuffer != L'\0' );
	}
}

// $Barnyard: FUNCTION 006c2fe0
void T2GUIFont::DrawTextSingleLine( const TWCHAR* a_wszText, TINT a_iTextLength, TFLOAT a_fX, TFLOAT a_fY, TUINT32 a_uiColour, TFLOAT a_fScale, void* a_fnCallback /*= TNULL*/ )
{
	auto pRenderer = T2GUI::GetRenderer();
	pRenderer->SetColour( a_uiColour );

	TFLOAT         fOffsetX      = 0.0f;
	TFLOAT         fOffsetY      = 0.0f;
	TFLOAT         fXToUV        = 0.0f;
	TFLOAT         fYToUV        = 0.0f;
	T2GUIMaterial* pPrevMaterial = TNULL;

	for ( TINT i = 0; i < a_iTextLength; i++ )
	{
		auto pFontDef = m_pFontDef;
		auto wChar    = a_wszText[ i ];

		if ( auto pCharDataPtr = FindCharData( wChar ) )
		{
			{
				auto&  pCharData = *pCharDataPtr;
				TFLOAT fPos1X    = pCharData.iLeftPadding * a_fScale + a_fX;

				if ( pCharData.ui8CharWidth != 0 && pCharData.ui8CharHeight != 0 )
				{
					T2GUIMaterial* pMaterial = GetCharMaterial( wChar, pCharDataPtr );

					if ( pMaterial != pPrevMaterial )
					{
						pPrevMaterial = pMaterial;
						pRenderer->SetMaterial( pMaterial );
						fXToUV = 1.0f / pRenderer->GetWidth( pMaterial );
						fYToUV = 1.0f / pRenderer->GetHeight( pMaterial );
					}

					if ( a_fnCallback )
						TREINTERPRETCAST( CharCallback, a_fnCallback )
					( i, &fOffsetX, &fOffsetY );

					TFLOAT fPos1Y = ( ( pFontDef->uiLineHeight + pCharData.iYOffset ) - pCharData.ui8CharHeight ) * a_fScale + a_fY;
					TFLOAT fPos2Y = fPos1Y + ( pCharData.ui8CharHeight + 1 ) * a_fScale;
					TFLOAT fPos2X = fPos1X + ( pCharData.ui8CharWidth + 1 ) * a_fScale;

					pRenderer->RenderRectangle(
					    { fPos1X + fOffsetX, fPos1Y + fOffsetY },
					    { fPos2X + fOffsetX, fPos2Y + fOffsetY },
					    { ( pCharData.ui16PosX ) * fXToUV, ( pCharData.ui16PosY ) * fYToUV },
					    { ( pCharData.ui16PosX + pCharData.ui8CharWidth + 1 ) * fXToUV, ( pCharData.ui16PosY + pCharData.ui8CharHeight + 1 ) * fYToUV }
					);
				}

				TFLOAT fPos2X = TFLOAT( pFontDef->iLetterSpacing + pCharData.iRightPadding + pCharData.ui8CharWidth );
				a_fX          = fPos2X * a_fScale + fPos1X;
			}
		}
	}
}

// $Barnyard: FUNCTION 006c2e10
TFLOAT T2GUIFont::GetTextHeightWrapped( const TWCHAR* a_wszText, TFLOAT a_fMaxWidth, TFLOAT a_fScale )
{
	auto pFontDef = m_pFontDef;

	if ( a_wszText && a_wszText[ 0 ] != '\0' && pFontDef )
	{
		TFLOAT fHeight     = 0.0f;
		auto   pTextBuffer = a_wszText;

		do
		{
			auto pTextBuffer2 = pTextBuffer;

			while ( *pTextBuffer2 != L'\0' && std::iswspace( *pTextBuffer2 ) != 0 )
			{
				if ( *pTextBuffer2 == L'\n' ) break;
				pTextBuffer2++;
			}

			auto wChar = *pTextBuffer2;

			if ( wChar == L'\n' )
			{
				pTextBuffer = pTextBuffer2 + 1;
			}
			else
			{
				if ( wChar == L'\0' ) break;

				pTextBuffer         = TNULL;
				TFLOAT fWidth1      = 0.0f;
				TFLOAT fWidth2      = 0.0f;
				auto   pTextBuffer3 = pTextBuffer2;

				do {
					if ( auto pCharData = FindCharData( wChar ) )
						fWidth1 += ( pFontDef->iLetterSpacing + pCharData->iRightPadding + pCharData->iLeftPadding + pCharData->ui8CharWidth ) * a_fScale;

					if ( wChar == L'\n' )
					{
						fWidth2     = fWidth1;
						pTextBuffer = pTextBuffer3;
						break;
					}

					if ( std::iswspace( wChar ) != 0 && *pTextBuffer3 != L'\xA0' )
					{
						fWidth2     = fWidth1;
						pTextBuffer = pTextBuffer3;
					}

					wChar = pTextBuffer3[ 1 ];
					if ( wChar == L'\0' ) break;
					pTextBuffer3++;

				} while ( fWidth1 < a_fMaxWidth || pTextBuffer == TNULL );

				if ( pTextBuffer3[ 1 ] == L'\0' )
				{
					pTextBuffer = pTextBuffer3 + 1;
					fWidth2     = fWidth1;
				}
			}

			fHeight += ( pFontDef->uiLineHeight + pFontDef->uiLinesMargin ) * a_fScale;

		} while ( *pTextBuffer != L'\0' );

		return fHeight;
	}
	else
	{
		return 0.0f;
	}
}

// $Barnyard: FUNCTION 006c3380
TFLOAT T2GUIFont::GetTextWidth( const TWCHAR* a_wszText, TFLOAT a_fScale )
{
	if ( a_wszText && m_pFontDef )
	{
		return GetTextWidth(
		    a_wszText,
		    Toshi::TStringManager::String16Length( a_wszText ),
		    a_fScale
		);
	}

	return 0.0f;
}

// $Barnyard: FUNCTION 006c33c0
void T2GUIFont::DrawTextSingleLine( const TWCHAR* a_wszText, TFLOAT a_fX, TFLOAT a_fY, TUINT32 a_uiColour, TFLOAT a_fScale )
{
	if ( a_wszText && a_wszText[ 0 ] != L'\0' && m_pFontDef )
		DrawTextSingleLine( a_wszText, TStringManager::String16Length( a_wszText ), a_fX, a_fY, a_uiColour, a_fScale, TNULL );
}

// $Barnyard: FUNCTION 006c3350
TFLOAT T2GUIFont::GetCharWidth( TWCHAR a_wcChar, TFLOAT a_fScale )
{
	if ( m_pFontDef )
		return GetTextWidth( &a_wcChar, 1, a_fScale );

	return 0.0f;
}

// $Barnyard: FUNCTION 006c32b0
TFLOAT T2GUIFont::GetTextWidth( const TWCHAR* a_wszText, TINT a_iTextLength, TFLOAT a_fScale )
{
	TINT iWidth = 0;

	for ( TINT i = 0; i < a_iTextLength; i++ )
	{
		auto pFontDef = m_pFontDef;
		auto wChar    = a_wszText[ i ];

		if ( auto pCharData = FindCharData( wChar ) )
			iWidth += pFontDef->iLetterSpacing + pCharData->iRightPadding + pCharData->iLeftPadding + pCharData->ui8CharWidth;
	}

	return iWidth * a_fScale;
}

// $Barnyard: FUNCTION 006c2dd0
TFLOAT T2GUIFont::GetLineHeight()
{
	if ( m_pFontDef )
		return TFLOAT( m_pFontDef->uiLineHeight );

	return 0.0f;
}

// $Barnyard: FUNCTION 006c2df0
TFLOAT T2GUIFont::GetLinesMargin()
{
	if ( m_pFontDef )
		return TFLOAT( m_pFontDef->uiLinesMargin );

	return 0.0f;
}

#ifdef BARNYARD_LATEST

//-----------------------------------------------------------------------------
// Wii T2GUIFont
// NOTE: the Wii functions look the characters up in the 256 character pages of the Wii
// fonts. Paged fonts aren't supported (the PC retail fonts are used), so they use the PC lookup
// (FindCharData) and the PC material of a character instead. For reference, the Wii lookup
// (804a1358, T2GUIFontDef +0xC = TUINT16 min/max[ 256 ][ 2 ], +0x14 = TINT32* maps[ 256 ]):
//
//   TUINT uiPage = a_wcChar >> 8;
//   if ( !pDef->ppPageMaps[ uiPage ] ) return TNULL;
//   if ( a_wcChar < pDef->pPageRanges[ uiPage ].min || a_wcChar > pDef->pPageRanges[ uiPage ].max ) return TNULL;
//   TINT iIndex = pDef->ppPageMaps[ uiPage ][ a_wcChar - pDef->pPageRanges[ uiPage ].min ];
//   return ( iIndex < 0 ) ? TNULL : &pDef->pCharactersData[ iIndex ];
//
// and the material of a character is pCharData->Unk6 (+0xB).
//-----------------------------------------------------------------------------

// The CodeWarrior ctype classes the Wii build tests (characters below 0x100, C locale)
static TBOOL IsSpaceMSL( TWCHAR a_wcChar )
{
	return a_wcChar < 0x80 && std::iswspace( a_wcChar ) != 0;
}

static TBOOL IsAlnumMSL( TWCHAR a_wcChar )
{
	return a_wcChar < 0x80 && std::iswalnum( a_wcChar ) != 0;
}

// Characters a line can't end with (opening brackets, currency signs), sorted
static const TUINT16 s_aNoBreakAfter[] = {
	0x0024, 0x0028, 0x005b, 0x007b, 0x00a3, 0x00a5, 0x2018, 0x201b, 0x201c, 0x201f, 0x2116,
	0x3008, 0x300a, 0x300c, 0x300e, 0x3010, 0x3014, 0x3016, 0x3018, 0x301a, 0x301d
};

// Characters a line can't start with (closing punctuation, small kana, prolonged sound marks), sorted
static const TUINT16 s_aNoBreakBefore[] = {
	0x0021, 0x0025, 0x0029, 0x002c, 0x002e, 0x003a, 0x003b, 0x003f, 0x005d, 0x007d, 0x00a2, 0x00b0,
	0x2014, 0x2019, 0x201a, 0x201d, 0x201e, 0x2024, 0x2025, 0x2026, 0x2030, 0x2031, 0x2032, 0x2033,
	0x203c, 0x2044, 0x3001, 0x3002, 0x3009, 0x300b, 0x300d, 0x300f, 0x3011, 0x3015, 0x3017, 0x3019,
	0x301b, 0x301c, 0x301e, 0x301f, 0x3041, 0x3043, 0x3045, 0x3047, 0x3049, 0x3063, 0x3083, 0x3085,
	0x3087, 0x308e, 0x309d, 0x309e, 0x30a1, 0x30a3, 0x30a5, 0x30a7, 0x30a9, 0x30c3, 0x30e3, 0x30e5,
	0x30e7, 0x30ee, 0x30f5, 0x30f6, 0x30fb, 0x30fc, 0x30fd, 0x30fe
};

TSTATICASSERT( TARRAYSIZE( s_aNoBreakAfter ) == 21 );
TSTATICASSERT( TARRAYSIZE( s_aNoBreakBefore ) == 68 );

static TBOOL IsInSortedTable( const TUINT16* a_pTable, TINT a_iSize, TUINT a_uiChar )
{
	TINT iLow  = 0;
	TINT iHigh = a_iSize;

	while ( iLow < iHigh )
	{
		TINT iMid = iLow + ( iHigh - iLow ) / 2;

		if ( a_uiChar < a_pTable[ iMid ] )
			iHigh = iMid;
		else if ( a_uiChar > a_pTable[ iMid ] )
			iLow = iMid + 1;
		else
			return TTRUE;
	}

	return TFALSE;
}

// $BarnyardWiiJP: FUNCTION 804a13c4
void T2GUIFont::ResetDrawnMaterials()
{
	for ( TINT i = 0; i < TARRAYSIZE( ms_auiDrawnMaterials ); i++ )
		ms_auiDrawnMaterials[ i ] = 0;
}

// $BarnyardWiiJP: FUNCTION 804a0388
TBOOL T2GUIFont::CanBreakLine( TWCHAR a_wcChar, TWCHAR a_wcNextChar )
{
	// Never inside a word
	if ( IsAlnumMSL( a_wcChar ) && IsAlnumMSL( a_wcNextChar ) )
		return TFALSE;

	if ( IsInSortedTable( s_aNoBreakAfter, TARRAYSIZE( s_aNoBreakAfter ), a_wcChar ) )
		return TFALSE;

	if ( IsInSortedTable( s_aNoBreakBefore, TARRAYSIZE( s_aNoBreakBefore ), a_wcNextChar ) )
		return TFALSE;

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 804a1090
TINT T2GUIFont::GetLineLength( const TWCHAR** a_ppText, TFLOAT* a_pOutWidth, IconWidthCallback a_fnIconWidth, TBOOL a_bIncludeNewLine, TFLOAT a_fMaxWidth, TFLOAT a_fScale )
{
	const TWCHAR* pText = *a_ppText;

	if ( !pText || *pText == L'\0' || !m_pFontDef )
		return 0;

	// Skip the spaces the line starts with, but not a new line
	TWCHAR wcChar;

	while ( TTRUE )
	{
		wcChar = *pText;

		if ( wcChar == L'\0' || !IsSpaceMSL( wcChar ) || wcChar == L'\n' )
			break;

		pText++;
	}

	if ( wcChar == L'\n' )
	{
		// An empty line
		*a_ppText    = pText;
		*a_pOutWidth = 0.0f;
		return 1;
	}

	*a_ppText = pText;

	if ( *pText == L'\0' )
		return 0;

	TFLOAT        fBreakWidth = 0.0f;
	const TWCHAR* pBreak      = TNULL;
	*a_pOutWidth              = 0.0f;

	while ( TTRUE )
	{
		wcChar = *pText;

		if ( wcChar == L'\r' )
		{
			pText++;
		}
		else if ( wcChar == L'\n' )
		{
			if ( a_bIncludeNewLine )
				pText++;

			fBreakWidth = *a_pOutWidth;
			pBreak      = pText;

			if ( !a_bIncludeNewLine )
				pText++;

			break;
		}
		else
		{
			TFLOAT fIconWidth;

			if ( !a_fnIconWidth || ( fIconWidth = a_fnIconWidth( wcChar ), fIconWidth == -1.0f ) )
			{
				if ( auto pCharData = FindCharData( *pText ) )
					*a_pOutWidth += a_fScale * ( pCharData->iLeftPadding + pCharData->ui8CharWidth + m_pFontDef->iLetterSpacing + pCharData->iRightPadding );
			}
			else
			{
				*a_pOutWidth += fIconWidth * a_fScale;
			}

			if ( ( *a_pOutWidth <= a_fMaxWidth || !pBreak ) && CanBreakLine( pText[ 0 ], pText[ 1 ] ) )
			{
				pBreak      = pText + 1;
				fBreakWidth = *a_pOutWidth;
			}

			pText++;
		}

		if ( *pText == L'\0' || ( *a_pOutWidth > a_fMaxWidth && pBreak ) )
			break;
	}

	if ( *pText != L'\0' )
		pText = pBreak;
	else
		fBreakWidth = *a_pOutWidth;

	*a_pOutWidth = fBreakWidth;
	return TINT( pText - *a_ppText );
}

// $BarnyardWiiJP: FUNCTION 804a0574
void T2GUIFont::DrawTextWrapped( const TWCHAR* a_wszText, TUINT32 a_uiColour, TextAlign a_eAlign, CharCallback a_fnCallback, IconWidthCallback a_fnIconWidth, IconRenderCallback a_fnIconRender, TBOOL a_bIncludeNewLine, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fWidth, TFLOAT a_fHeight, TFLOAT a_fScale, TINT a_iMaxChars )
{
	if ( !a_wszText || *a_wszText == L'\0' || !m_pFontDef )
		return;

	TFLOAT fLineAdvance = a_fScale * ( m_pFontDef->uiLineHeight + TINT8( m_pFontDef->uiLinesMargin ) );
	TINT   iCharsLeft   = 0;

	if ( a_iMaxChars >= 0 )
		iCharsLeft = a_iMaxChars;

	const TWCHAR* pLine = a_wszText;
	TFLOAT        fY    = a_fY;

	while ( TTRUE )
	{
		TFLOAT fLineWidth;
		TINT   iLength = GetLineLength( &pLine, &fLineWidth, a_fnIconWidth, a_bIncludeNewLine, a_fWidth, a_fScale );

		if ( iLength == 0 )
			break;

		TFLOAT fX;

		if ( a_eAlign == TextAlign_Center )
			fX = a_fX + 0.5f * ( a_fWidth - fLineWidth );
		else if ( a_eAlign == TextAlign_Right )
			fX = a_fX + ( a_fWidth - fLineWidth );
		else
			fX = a_fX;

		TINT iDrawLength = iLength;

		if ( a_iMaxChars >= 0 )
		{
			iDrawLength = TMath::Min( iCharsLeft, iLength );
			iCharsLeft -= iDrawLength;
		}

		DrawTextSingleLine( pLine, iDrawLength, a_uiColour, a_fnCallback, a_fnIconWidth, a_fnIconRender, fX, fY, a_fScale );

		if ( a_iMaxChars >= 0 && iCharsLeft <= 0 )
			break;

		fY += fLineAdvance;
		pLine += iLength;

		if ( *pLine == L'\0' || fLineAdvance + TMath::Abs( fY - a_fY ) > a_fHeight || ms_bMultipleLines != TTRUE )
			break;
	}
}

// $BarnyardWiiJP: FUNCTION 804a0898
void T2GUIFont::DrawTextSingleLine( const TWCHAR* a_wszText, TINT a_iTextLength, TUINT32 a_uiColour, CharCallback a_fnCallback, IconWidthCallback a_fnIconWidth, IconRenderCallback a_fnIconRender, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fScale )
{
	TFLOAT         fOffsetX  = 0.0f;
	TFLOAT         fOffsetY  = 0.0f;
	T2GUIRenderer* pRenderer = T2GUI::GetRenderer();

	ResetDrawnMaterials();

	// Index of the last character of the next material pass
	TINT iLastChar = -1;

	if ( a_fnIconRender )
	{
		// The icons are drawn first, in one go
		TFLOAT fX = a_fX;

		for ( TINT i = 0; i < a_iTextLength; i++ )
		{
			if ( a_fnCallback )
				a_fnCallback( i, &fOffsetX, &fOffsetY );

			TFLOAT fAdvance    = 0.0f;
			TFLOAT fLineHeight = TFLOAT( m_pFontDef->uiLineHeight );

			if ( a_fnIconRender( pRenderer, a_wszText[ i ], &fAdvance, fLineHeight, fX + fOffsetX, fOffsetY + ( a_fY + fLineHeight ) ) )
			{
				fX += a_fScale * ( fAdvance + m_pFontDef->iLetterSpacing );
			}
			else if ( auto pCharData = FindCharData( a_wszText[ i ] ) )
			{
				iLastChar = TMath::Max( iLastChar, i );
				fX += a_fScale * ( m_pFontDef->iLetterSpacing + pCharData->iRightPadding + pCharData->iLeftPadding + pCharData->ui8CharWidth );
			}
		}
	}
	else
	{
		iLastChar = a_iTextLength - 1;

		while ( iLastChar >= 0 && !FindCharData( a_wszText[ iLastChar ] ) )
			iLastChar--;
	}

	pRenderer->SetColour( a_uiColour );

	// One pass per material (texture page), starting with the material of the last character
	while ( iLastChar >= 0 )
	{
		TFLOAT fX = a_fX;

		TUINT uiMaterial = GetCharMaterialIndex( a_wszText[ iLastChar ], FindCharData( a_wszText[ iLastChar ] ) );
		ms_auiDrawnMaterials[ uiMaterial >> 5 ] |= 1 << ( uiMaterial & 31 );

		T2GUIMaterial* pMaterial = m_ppMaterials[ uiMaterial ];
		pRenderer->SetMaterial( pMaterial );
		TFLOAT fXToUV = 1.0f / pRenderer->GetWidth( pMaterial );
		TFLOAT fYToUV = 1.0f / pRenderer->GetHeight( pMaterial );

		TINT iPassLastChar = iLastChar;
		iLastChar          = -1;

		for ( TINT i = 0; i <= iPassLastChar; i++ )
		{
			TFLOAT fIconWidth;

			if ( a_fnIconRender && ( fIconWidth = a_fnIconWidth( a_wszText[ i ] ), fIconWidth > 0.0f ) )
			{
				fX += a_fScale * ( fIconWidth + m_pFontDef->iLetterSpacing );
			}
			else if ( auto pCharData = FindCharData( a_wszText[ i ] ) )
			{
				TFLOAT fPos1X = fX + pCharData->iLeftPadding * a_fScale;

				if ( pCharData->ui8CharWidth != 0 && pCharData->ui8CharHeight != 0 )
				{
					TUINT uiCharMaterial = GetCharMaterialIndex( a_wszText[ i ], pCharData );

					if ( uiCharMaterial == uiMaterial )
					{
						TFLOAT fPos1Y = a_fY + a_fScale * ( pCharData->iYOffset + m_pFontDef->uiLineHeight - pCharData->ui8CharHeight );
						TFLOAT fPos2X = fPos1X + ( pCharData->ui8CharWidth + 1 ) * a_fScale;
						TFLOAT fPos2Y = fPos1Y + ( pCharData->ui8CharHeight + 1 ) * a_fScale;

						if ( a_fnCallback )
							a_fnCallback( i, &fOffsetX, &fOffsetY );

						pRenderer->RenderRectangle(
						    { fPos1X + fOffsetX, fPos1Y + fOffsetY },
						    { fPos2X + fOffsetX, fPos2Y + fOffsetY },
						    { pCharData->ui16PosX * fXToUV, pCharData->ui16PosY * fYToUV },
						    { ( pCharData->ui16PosX + pCharData->ui8CharWidth + 1 ) * fXToUV, ( pCharData->ui16PosY + pCharData->ui8CharHeight + 1 ) * fYToUV }
						);
					}
					else if ( ( ms_auiDrawnMaterials[ uiCharMaterial >> 5 ] & ( 1 << ( uiCharMaterial & 31 ) ) ) == 0 && i > iLastChar )
					{
						// Drawn by a later pass
						iLastChar = i;
					}
				}

				fX = fPos1X + a_fScale * ( pCharData->iRightPadding + m_pFontDef->iLetterSpacing + pCharData->ui8CharWidth );
			}
		}
	}
}

// $BarnyardWiiJP: FUNCTION 804a0238
TFLOAT T2GUIFont::GetTextHeightWrapped( const TWCHAR* a_wszText, IconWidthCallback a_fnIconWidth, TBOOL a_bIncludeNewLine, TINT a_iMaxChars, TFLOAT a_fMaxWidth, TFLOAT a_fScale )
{
	TFLOAT fHeight = 0.0f;

	if ( a_wszText && *a_wszText != L'\0' && m_pFontDef )
	{
		TFLOAT        fLineAdvance = a_fScale * ( m_pFontDef->uiLineHeight + TINT8( m_pFontDef->uiLinesMargin ) );
		TINT          iCharsLeft   = ( a_iMaxChars >= 0 ) ? a_iMaxChars : 0;
		const TWCHAR* pLine        = a_wszText;

		do
		{
			TFLOAT fLineWidth;
			TINT   iLength = GetLineLength( &pLine, &fLineWidth, a_fnIconWidth, a_bIncludeNewLine, a_fMaxWidth, a_fScale );

			if ( iLength == 0 )
				break;

			fHeight += fLineAdvance;
			pLine += iLength;

			if ( a_iMaxChars >= 0 )
			{
				iCharsLeft -= iLength;

				if ( iCharsLeft <= 0 )
					break;
			}

		} while ( *pLine != L'\0' );
	}

	return fHeight;
}

// $BarnyardWiiJP: FUNCTION 804a0f68
TFLOAT T2GUIFont::GetTextWidth( const TWCHAR* a_wszText, TINT a_iTextLength, IconWidthCallback a_fnIconWidth, TFLOAT a_fScale )
{
	TINT iWidth = 0;

	for ( TINT i = 0; i < a_iTextLength; i++ )
	{
		TFLOAT fIconWidth;

		if ( !a_fnIconWidth || ( fIconWidth = a_fnIconWidth( a_wszText[ i ] ), fIconWidth == -1.0f ) )
		{
			if ( auto pCharData = FindCharData( a_wszText[ i ] ) )
				iWidth += pCharData->ui8CharWidth + pCharData->iLeftPadding + m_pFontDef->iLetterSpacing + pCharData->iRightPadding;
		}
		else
		{
			iWidth += m_pFontDef->iLetterSpacing + TINT( fIconWidth );
		}
	}

	return iWidth * a_fScale;
}

// $BarnyardWiiJP: FUNCTION 804a01a8
TFLOAT T2GUIFont::GetTextWidth( const TWCHAR* a_wszText, IconWidthCallback a_fnIconWidth, TFLOAT a_fScale )
{
	if ( a_wszText && m_pFontDef )
		return GetTextWidth( a_wszText, TStringManager::String16Length( a_wszText ), a_fnIconWidth, a_fScale );

	return 0.0f;
}

// $BarnyardWiiJP: FUNCTION 804a0164
TFLOAT T2GUIFont::GetCharWidth( TWCHAR a_wcChar, IconWidthCallback a_fnIconWidth, TFLOAT a_fScale )
{
	if ( m_pFontDef )
		return GetTextWidth( &a_wcChar, 1, a_fnIconWidth, a_fScale );

	return 0.0f;
}

#endif // BARNYARD_LATEST

TOSHI_NAMESPACE_END
