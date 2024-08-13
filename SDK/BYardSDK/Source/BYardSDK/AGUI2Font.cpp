#include "pch.h"
#include "AGUI2Font.h"
#include "AGUI2.h"
#include "SDKHooks.h"

#include <cwctype>

TBOOL AGUI2Font::Create( AGUI2FontDef* a_pFontDef )
{
	return CALL_THIS( 0x006c2cf0, AGUI2Font*, TBOOL, this, AGUI2FontDef*, a_pFontDef );
}

void AGUI2Font::DrawTextWrapped( const wchar_t* a_wszText, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fWidth, TFLOAT a_fHeight, TUINT32 a_uiColour, TFLOAT a_fScale, TextAlign a_eAlign, void* a_fnCallback /*= TNULL*/ )
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

				pTextBuffer			= TNULL;
				TFLOAT fWidth1		= 0.0f;
				TFLOAT fWidth2		= 0.0f;
				auto   pTextBuffer3 = pTextBuffer2;

				do {
					if ( pFontDef->ui16MinCharacter <= wChar && ( wChar <= pFontDef->ui16MaxCharacter ) )
					{
						TINT iCharIndex = pFontDef->pCharactersMap[ wChar - pFontDef->ui16MinCharacter ];

						if ( iCharIndex >= 0 )
						{
							auto& pCharData = pFontDef->pCharactersData[ iCharIndex ];
							fWidth1 += ( pFontDef->Unk2 + pCharData.Unk4 + pCharData.Unk3 + pCharData.ui8CharWidth ) * a_fScale;
						}
					}

					if ( wChar == L'\n' )
					{
						fWidth2		= fWidth1;
						pTextBuffer = pTextBuffer3;
						break;
					}

					if ( std::iswspace( wChar ) != 0 && *pTextBuffer3 != L'\xA0' )
					{
						fWidth2		= fWidth1;
						pTextBuffer = pTextBuffer3;
					}

					wChar = pTextBuffer3[ 1 ];
					if ( wChar == L'\0' ) break;
					pTextBuffer3++;

				} while ( fWidth1 < a_fWidth || pTextBuffer == TNULL );

				if ( pTextBuffer3[ 1 ] == L'\0' )
				{
					pTextBuffer = pTextBuffer3 + 1;
					fWidth2		= fWidth1;
				}

				TFLOAT fPosX;

				if ( a_eAlign == TextAlign_Left ) fPosX = a_fX;
				else if ( a_eAlign == TextAlign_Center ) fPosX = ( a_fWidth - fWidth2 ) * 0.5f + a_fX;
				else if ( a_eAlign == TextAlign_Right ) fPosX = ( a_fWidth - fWidth2 ) + a_fX;
				else fPosX = a_fX;

				DrawTextSingleLine( pTextBuffer2, pTextBuffer - pTextBuffer2, fPosX, a_fY, a_uiColour, a_fScale, a_fnCallback );
			}

			a_fY += ( pFontDef->uiLineHeight + pFontDef->uiLinesMargin ) * a_fScale;

		} while ( *pTextBuffer != L'\0' );
	}
}

void AGUI2Font::DrawTextSingleLine( const wchar_t* a_wszText, TINT a_iTextLength, TFLOAT a_fX, TFLOAT a_fY, TUINT32 a_uiColour, TFLOAT a_fScale, void* a_fnCallback /*= TNULL*/ )
{
	auto pRenderer = AGUI2::GetRenderer();
	pRenderer->SetColour( a_uiColour );

	TFLOAT		   fOffsetX		 = 0.0f;
	TFLOAT		   fOffsetY		 = 0.0f;
	TFLOAT		   fXToUV		 = 0.0f;
	TFLOAT		   fYToUV		 = 0.0f;
	AGUI2Material* pPrevMaterial = TNULL;

	for ( TINT i = 0; i < a_iTextLength; i++ )
	{
		auto pFontDef = m_pFontDef;
		auto wChar	  = a_wszText[ i ];

		if ( pFontDef->ui16MinCharacter <= wChar && wChar <= pFontDef->ui16MaxCharacter )
		{
			TINT iCharIndex = pFontDef->pCharactersMap[ wChar - pFontDef->ui16MinCharacter ];

			if ( iCharIndex >= 0 )
			{
				auto&  pCharData = pFontDef->pCharactersData[ iCharIndex ];
				TFLOAT fPos1X	 = pCharData.Unk3 * a_fScale + a_fX;

				if ( pCharData.ui8CharWidth != 0 && pCharData.ui8CharHeight != 0 )
				{
					AGUI2Material* pMaterial = ( pFontDef->uiNumMaterials > 1 ) ?
						m_ppMaterials[ ( wChar >> 8 ) ] :
						m_ppMaterials[ 0 ];

					if ( pMaterial != pPrevMaterial )
					{
						pPrevMaterial = pMaterial;
						pRenderer->SetMaterial( pMaterial );
						fXToUV = 1.0f / pRenderer->GetWidth( pMaterial );
						fYToUV = 1.0f / pRenderer->GetHeight( pMaterial );
					}

					TFIXME( "Call a_fnCallback(i, fOffsetX, fOffsetY)" );

					TFLOAT fPos1Y = ( ( pFontDef->uiLineHeight + pCharData.Unk5 ) - pCharData.ui8CharHeight ) * a_fScale + a_fY;
					TFLOAT fPos2Y = ( pCharData.ui8CharHeight + 1 ) * a_fScale + fPos1Y;
					TFLOAT fPos2X = ( pCharData.ui8CharWidth + 1 ) * a_fScale + fPos1X;

					pRenderer->RenderRectangle(
						{ fPos1X + fOffsetX, fPos1Y + fOffsetY },
						{ fPos2X + fOffsetX, fPos2Y + fOffsetY },
						{ ( pCharData.ui16PosX ) * fXToUV, ( pCharData.ui16PosY ) * fYToUV },
						{ ( pCharData.ui16PosX + pCharData.ui8CharWidth + 1 ) * fXToUV, ( pCharData.ui16PosY + pCharData.ui8CharHeight + 1 ) * fYToUV } );
				}

				TFLOAT fPos2X = TFLOAT( pFontDef->Unk2 + pCharData.Unk4 + pCharData.ui8CharWidth );
				a_fX		  = fPos2X * a_fScale + fPos1X;
			}
		}
	}
}

TFLOAT AGUI2Font::GetTextHeightWrapped( const wchar_t* a_wszText, TFLOAT a_fMaxWidth, TFLOAT a_fScale )
{
	auto pFontDef = m_pFontDef;

	if ( a_wszText && a_wszText[ 0 ] != '\0' && pFontDef )
	{
		TFLOAT fHeight	   = 0.0f;
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

				pTextBuffer			= TNULL;
				TFLOAT fWidth1		= 0.0f;
				TFLOAT fWidth2		= 0.0f;
				auto   pTextBuffer3 = pTextBuffer2;

				do {
					if ( pFontDef->ui16MinCharacter <= wChar && ( wChar <= pFontDef->ui16MaxCharacter ) )
					{
						TINT iCharIndex = pFontDef->pCharactersMap[ wChar - pFontDef->ui16MinCharacter ];

						if ( iCharIndex >= 0 )
						{
							auto& pCharData = pFontDef->pCharactersData[ iCharIndex ];
							fWidth1 += ( pFontDef->Unk2 + pCharData.Unk4 + pCharData.Unk3 + pCharData.ui8CharWidth ) * a_fScale;
						}
					}

					if ( wChar == L'\n' )
					{
						fWidth2		= fWidth1;
						pTextBuffer = pTextBuffer3;
						break;
					}

					if ( std::iswspace( wChar ) != 0 && *pTextBuffer3 != L'\xA0' )
					{
						fWidth2		= fWidth1;
						pTextBuffer = pTextBuffer3;
					}

					wChar = pTextBuffer3[ 1 ];
					if ( wChar == L'\0' ) break;
					pTextBuffer3++;

				} while ( fWidth1 < a_fMaxWidth || pTextBuffer == TNULL );

				if ( pTextBuffer3[ 1 ] == L'\0' )
				{
					pTextBuffer = pTextBuffer3 + 1;
					fWidth2		= fWidth1;
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

TFLOAT AGUI2Font::GetTextWidth( const wchar_t* a_wszText, TFLOAT a_fScale )
{
	if ( a_wszText && m_pFontDef )
	{
		return GetTextWidth(
			a_wszText,
			Toshi::TStringManager::String16Length( a_wszText ),
			a_fScale );
	}

	return 0.0f;
}

TFLOAT AGUI2Font::GetTextWidth( const wchar_t* a_wszText, TINT a_iTextLength, TFLOAT a_fScale )
{
	TINT iWidth = 0;

	for ( TINT i = 0; i < a_iTextLength; i++ )
	{
		auto pFontDef = m_pFontDef;
		auto wChar	  = a_wszText[ i ];

		if ( pFontDef->ui16MinCharacter <= wChar && wChar <= pFontDef->ui16MaxCharacter )
		{
			TINT iCharIndex = pFontDef->pCharactersMap[ wChar - pFontDef->ui16MinCharacter ];

			if ( iCharIndex >= 0 )
			{
				auto& pCharData = pFontDef->pCharactersData[ iCharIndex ];
				iWidth += pFontDef->Unk2 + pCharData.Unk4 + pCharData.Unk3 + pCharData.ui8CharWidth;
			}
		}
	}

	return iWidth * a_fScale;
}
