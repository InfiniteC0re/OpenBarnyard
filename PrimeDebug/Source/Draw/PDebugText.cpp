#include "PrimeDebugPCH.h"
#include "PDebugText.h"
#include "Util/PString.h"

#ifdef PRIMEDEBUG_DEV

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

PDebugRenderer* PDebugRenderer::ms_pRenderer;

TFLOAT       PDebugText::ms_fCursorX;
TFLOAT       PDebugText::ms_fCursorY;
TFLOAT       PDebugText::ms_fTabWidth;
TFLOAT       PDebugText::ms_fWrapWidth;
TBOOL        PDebugText::ms_bWrap;
TBOOL        PDebugText::ms_bStarted;
TUINT        PDebugText::ms_uiNumGlyphs;
TUINT        PDebugText::ms_uiMaxGlyphs;
PDebugGlyph* PDebugText::ms_pGlyphs;

// Queues a character: a_fSizeX/Y the half size, a_fX/Y the centre (y from the top)
// NOTE: Prime writes the six vertices of the character's quad in the font texture, with the glyph's UVs
// $deBlob2: FUNCTION 0080501f
void PDebugText::AddQuad( TFLOAT a_fSizeX, TFLOAT a_fSizeY, TFLOAT a_fX, TFLOAT a_fY, TCHAR a_cChar, TUINT32 a_uiColour )
{
	if ( ms_uiNumGlyphs + 1 > ms_uiMaxGlyphs )
		return;

	PDebugGlyph& rGlyph = ms_pGlyphs[ ms_uiNumGlyphs++ ];
	rGlyph.fX           = a_fX;
	rGlyph.fY           = a_fY;
	rGlyph.fHalfWidth   = a_fSizeX;
	rGlyph.fHalfHeight  = a_fSizeY;
	rGlyph.uiColour     = a_uiColour;
	rGlyph.cChar        = a_cChar;
}

// $deBlob2: FUNCTION 008050de
void PDebugText::DrawAt( const TCHAR* a_szText, const TFLOAT* a_pPosition, TUINT32 a_uiColour, TFLOAT a_fScale )
{
	Draw( a_szText, a_pPosition[ 0 ], a_pPosition[ 1 ], a_uiColour, a_fScale, 0, TNULL, TNULL );
}

// At the cursor, which then moves down by the text's height
// $deBlob2: FUNCTION 0080511a
void PDebugText::Print( const TCHAR* a_szText, TUINT32 a_uiColour, TFLOAT a_fScale )
{
	TFLOAT fHeight;
	Draw( a_szText, ms_fCursorX, ms_fCursorY, a_uiColour, a_fScale, 0, TNULL, &fHeight );
	ms_fCursorY += fHeight;
}

// Queues the text from (a_fX, a_fY), wrapping it at the wrap width when wrapping is on. a_pWidth gets the widest
// line, a_pHeight the height of all the lines.
// NOTE: a space is measured as a word of no characters, so text only wraps at a space past the wrap width (or at
// any character further than that)
// $deBlob2: FUNCTION 00805175
void PDebugText::Draw( const TCHAR* a_szText, TFLOAT a_fX, TFLOAT a_fY, TUINT32 a_uiColour, TFLOAT a_fScale, TUINT8 a_uiFlags, TFLOAT* a_pWidth, TFLOAT* a_pHeight )
{
	TFLOAT fLineHeight = a_fScale * LINE_HEIGHT;

	if ( a_pWidth )
		*a_pWidth = 0.0f;

	TINT         iIndex = 0;
	const TCHAR* pChar  = a_szText;
	TFLOAT       fX     = a_fX;
	TFLOAT       fY     = a_fY;
	TFLOAT       fCurX  = a_fX;

	for ( ;; )
	{
		TUINT8 uiChar = TUINT8( *pChar );

		if ( uiChar == 0 )
		{
			if ( a_pHeight )
				*a_pHeight = ( fY - a_fY ) + fLineHeight;

			return;
		}

		TBOOL bDraw = TFALSE;

		if ( uiChar == ' ' )
		{
			const TCHAR* pSpace = strchr( pChar, ' ' );
			fCurX               = fX;

			if ( pSpace )
			{
				if ( !ms_bWrap )
				{
					bDraw = TTRUE;
				}
				else if ( ms_fWrapWidth + a_fX < TFLOAT( pSpace - pChar ) * CHAR_WIDTH * a_fScale + fX )
				{
					fY += fLineHeight;
					fX    = a_fX;
					fCurX = a_fX;
				}
			}
		}

		if ( !bDraw )
		{
			if ( ms_bWrap && fCurX > ms_fWrapWidth + a_fX )
			{
				// Past the wrap width: a new line, and the character again
				fY += fLineHeight;
				iIndex--;
				fX    = a_fX;
				pChar = pChar - 1;
				fCurX = a_fX;
			}
			else if ( uiChar == '\n' )
			{
				fY += fLineHeight;
				fCurX = a_fX;
				fX    = a_fX;
			}
			else if ( uiChar == '\t' )
			{
				fCurX = TFLOAT( TINT( ( fCurX - a_fX ) / ( ms_fTabWidth * a_fScale ) ) + 1 ) * ms_fTabWidth * a_fScale + a_fX;
				fX    = fCurX;
			}
			else if ( uiChar == '[' )
			{
				if ( a_szText[ iIndex + 1 ] == 'c' && a_szText[ iIndex + 3 ] == ']' )
				{
					switch ( a_szText[ iIndex + 2 ] )
					{
						case '0': a_uiColour = COLOUR_WHITE; break;
						case '1': a_uiColour = COLOUR_RED; break;
						case '2': a_uiColour = COLOUR_GREEN; break;
						case '3': a_uiColour = COLOUR_BLUE; break;
						case '4': a_uiColour = COLOUR_CYAN; break;
						case '5': a_uiColour = COLOUR_YELLOW; break;
						case '6': a_uiColour = COLOUR_MAGENTA; break;
						case '7': a_uiColour = COLOUR_BLACK; break;
					}

					iIndex += 3;
					pChar += 3;
				}
				else
				{
					bDraw = TTRUE;
				}
			}
			else if ( TUINT8( uiChar - ' ' ) < 0x81 )
			{
				bDraw = TTRUE;
			}
		}

		if ( bDraw )
		{
			if ( !( a_uiFlags & FLAGS_MEASUREONLY ) )
				AddQuad( a_fScale * 0.0055f, a_fScale * 0.013750001f, fX, fY, TCHAR( uiChar ), a_uiColour );

			fCurX = a_fScale * CHAR_WIDTH + fX;
			fX    = fCurX;

			if ( a_pWidth && *a_pWidth <= fCurX - a_fX )
				*a_pWidth = fCurX - a_fX;
		}

		iIndex++;
		pChar++;
	}
}

// $deBlob2: FUNCTION 008054a1
void PDebugText::PrintfScaled( TUINT32 a_uiColour, TFLOAT a_fScale, const TCHAR* a_szFormat, ... )
{
	TCHAR   szText[ FORMAT_SIZE ];
	va_list oArgs;
	va_start( oArgs, a_szFormat );
	PString::FormatV( szText, sizeof( szText ), a_szFormat, oArgs );
	va_end( oArgs );
	Print( szText, a_uiColour, a_fScale );
}

// $deBlob2: FUNCTION 008054e5
void PDebugText::PrintfColour( TUINT32 a_uiColour, const TCHAR* a_szFormat, ... )
{
	TCHAR   szText[ FORMAT_SIZE ];
	va_list oArgs;
	va_start( oArgs, a_szFormat );
	PString::FormatV( szText, sizeof( szText ), a_szFormat, oArgs );
	va_end( oArgs );
	Print( szText, a_uiColour, 1.0f );
}

// $deBlob2: FUNCTION 00805526
void PDebugText::PrintfAtScaled( TFLOAT a_fX, TFLOAT a_fY, TUINT32 a_uiColour, TFLOAT a_fScale, const TCHAR* a_szFormat, ... )
{
	TCHAR   szText[ FORMAT_SIZE ];
	va_list oArgs;
	va_start( oArgs, a_szFormat );
	PString::FormatV( szText, sizeof( szText ), a_szFormat, oArgs );
	va_end( oArgs );
	Draw( szText, a_fX, a_fY, a_uiColour, a_fScale, 0, TNULL, TNULL );
}

// $deBlob2: FUNCTION 00805587
void PDebugText::PrintfAtColour( TFLOAT a_fX, TFLOAT a_fY, TUINT32 a_uiColour, const TCHAR* a_szFormat, ... )
{
	TCHAR   szText[ FORMAT_SIZE ];
	va_list oArgs;
	va_start( oArgs, a_szFormat );
	PString::FormatV( szText, sizeof( szText ), a_szFormat, oArgs );
	va_end( oArgs );
	Draw( szText, a_fX, a_fY, a_uiColour, 1.0f, 0, TNULL, TNULL );
}

// $deBlob2: FUNCTION 008055e5
void PDebugText::PrintfAt( TFLOAT a_fX, TFLOAT a_fY, const TCHAR* a_szFormat, ... )
{
	TCHAR   szText[ FORMAT_SIZE ];
	va_list oArgs;
	va_start( oArgs, a_szFormat );
	PString::FormatV( szText, sizeof( szText ), a_szFormat, oArgs );
	va_end( oArgs );
	Draw( szText, a_fX, a_fY, COLOUR_WHITE, 1.0f, 0, TNULL, TNULL );
}

// $deBlob2: FUNCTION 00805642
void PDebugText::Printf( const TCHAR* a_szFormat, ... )
{
	TCHAR   szText[ FORMAT_SIZE ];
	va_list oArgs;
	va_start( oArgs, a_szFormat );
	PString::FormatV( szText, sizeof( szText ), a_szFormat, oArgs );
	va_end( oArgs );
	Print( szText, COLOUR_WHITE, 1.0f );
}

// Draws what was queued and puts the cursor back at the top left
// $deBlob2: FUNCTION 0080568a
void PDebugText::Flush()
{
	if ( !ms_uiNumGlyphs || !ms_bStarted )
		return;

	if ( ms_uiNumGlyphs > ms_uiMaxGlyphs )
	{
		ms_uiNumGlyphs = 0;
		Print( "MAXIMUM DEBUG TEXT CHARACTER COUNT EXCEEDED", COLOUR_WHITE, 1.0f );
	}

	PDebugRenderer* pRenderer = PDebugRenderer::GetRenderer();

	if ( !pRenderer )
	{
		ms_uiNumGlyphs = 0;
		return;
	}

	ms_fCursorX = MARGIN;
	ms_fCursorY = MARGIN;

	pRenderer->DrawGlyphs( ms_pGlyphs, TINT( ms_uiNumGlyphs ) );
	ms_uiNumGlyphs = 0;
}

// $deBlob2: FUNCTION 00805832
TFLOAT PDebugText::GetCursorX()
{
	return ms_fCursorX;
}

// $deBlob2: FUNCTION 00805839
TFLOAT PDebugText::GetCursorY()
{
	return ms_fCursorY;
}

// $deBlob2: FUNCTION 00805840
void PDebugText::Measure( const TCHAR* a_szText, TFLOAT* a_pWidth, TFLOAT* a_pHeight, TFLOAT a_fScale )
{
	Draw( a_szText, 0.0f, 0.0f, 0, a_fScale, FLAGS_MEASUREONLY, a_pWidth, a_pHeight );
}

// $deBlob2: FUNCTION 00805870
TFLOAT PDebugText::GetLineHeight()
{
	return LINE_HEIGHT;
}

// $deBlob2: FUNCTION 00805877
TFLOAT PDebugText::GetCharWidth()
{
	return CHAR_WIDTH;
}

// $deBlob2: FUNCTION 00805891
void PDebugText::SetCursorX( TFLOAT a_fX )
{
	ms_fCursorX = a_fX;
}

// $deBlob2: FUNCTION 008058a3
void PDebugText::SetCursorY( TFLOAT a_fY )
{
	ms_fCursorY = a_fY;
}

// $deBlob2: FUNCTION 008058b5
void PDebugText::SetTabSize( TINT a_iCharacters )
{
	ms_fTabWidth = TFLOAT( a_iCharacters ) * CHAR_WIDTH;
}

// $deBlob2: FUNCTION 008058e5
void PDebugText::SetWrap( TBOOL a_bWrap )
{
	ms_bWrap = a_bWrap;
}

// $deBlob2: FUNCTION 008058f2
void PDebugText::Shutdown()
{
	ms_bStarted = TFALSE;
	delete[] ms_pGlyphs;
	ms_pGlyphs     = TNULL;
	ms_uiNumGlyphs = 0;
}

// $deBlob2: FUNCTION 0080594a
void PDebugText::Startup( TUINT a_uiMaxVertices )
{
	ms_uiMaxGlyphs = a_uiMaxVertices / 6;
	ms_bStarted    = TTRUE;
	ms_fCursorX    = MARGIN;
	ms_fCursorY    = MARGIN;
	ms_fTabWidth   = 8 * CHAR_WIDTH;
	ms_fWrapWidth  = 0.9f;
	ms_pGlyphs     = new PDebugGlyph[ ms_uiMaxGlyphs ];
	ms_uiNumGlyphs = 0;
	ms_bWrap       = TTRUE;
}

#endif // PRIMEDEBUG_DEV
