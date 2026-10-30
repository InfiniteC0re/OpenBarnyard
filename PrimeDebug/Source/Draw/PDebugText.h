#pragma once
#include "PDebugRenderer.h"

#ifdef PRIMEDEBUG_DEV

//-----------------------------------------------------------------------------
// Purpose: Prime's debug text (geDebugText): monospaced text over the screen,
// queued and drawn by Flush. Positions and sizes are fractions of the screen;
// a line is 0.0275 high and a character 0.011 wide at a scale of 1, text at
// (x, y) has its first character centred there. "[cN]" in a text switches its
// colour (0 white, 1 red, 2 green, 3 blue, 4 cyan, 5 yellow, 6 magenta,
// 7 black). Print draws at the cursor and moves it down; Flush puts it back
// at the top left.
// NOTE: Prime queues the quads of its "debugfont" texture; here the
// characters go to PDebugRenderer::DrawGlyphs.
//-----------------------------------------------------------------------------
class PDebugText
{
public:
	static constexpr TFLOAT LINE_HEIGHT = 0.0275f;
	static constexpr TFLOAT CHAR_WIDTH  = 0.011f;
	static constexpr TFLOAT MARGIN      = 0.05f;
	static constexpr TINT   FORMAT_SIZE = 0x400;

	enum FLAGS_ : TUINT8
	{
		FLAGS_NONE        = 0,
		FLAGS_MEASUREONLY = BITFLAG( 0 ),
	};

	enum COLOUR : TUINT32
	{
		COLOUR_WHITE   = 0xFFFFFFFF,
		COLOUR_RED     = 0xFFFF0000,
		COLOUR_GREEN   = 0xFF008000,
		COLOUR_BLUE    = 0xFF0000FF,
		COLOUR_CYAN    = 0xFF00FFFF,
		COLOUR_YELLOW  = 0xFFFFFF00,
		COLOUR_MAGENTA = 0xFFFF00FF,
		COLOUR_BLACK   = 0xFF000000,
	};

public:
	// a_uiMaxVertices: Prime's vertex budget, 6 a character
	static void Startup( TUINT a_uiMaxVertices );
	static void Shutdown();
	static void Flush();

	static void Draw( const TCHAR* a_szText, TFLOAT a_fX, TFLOAT a_fY, TUINT32 a_uiColour, TFLOAT a_fScale, TUINT8 a_uiFlags, TFLOAT* a_pWidth, TFLOAT* a_pHeight );
	static void DrawAt( const TCHAR* a_szText, const TFLOAT* a_pPosition, TUINT32 a_uiColour, TFLOAT a_fScale );
	static void Print( const TCHAR* a_szText, TUINT32 a_uiColour, TFLOAT a_fScale );
	static void PrintfScaled( TUINT32 a_uiColour, TFLOAT a_fScale, const TCHAR* a_szFormat, ... );
	static void PrintfColour( TUINT32 a_uiColour, const TCHAR* a_szFormat, ... );
	static void Printf( const TCHAR* a_szFormat, ... );
	static void PrintfAtScaled( TFLOAT a_fX, TFLOAT a_fY, TUINT32 a_uiColour, TFLOAT a_fScale, const TCHAR* a_szFormat, ... );
	static void PrintfAtColour( TFLOAT a_fX, TFLOAT a_fY, TUINT32 a_uiColour, const TCHAR* a_szFormat, ... );
	static void PrintfAt( TFLOAT a_fX, TFLOAT a_fY, const TCHAR* a_szFormat, ... );
	static void Measure( const TCHAR* a_szText, TFLOAT* a_pWidth, TFLOAT* a_pHeight, TFLOAT a_fScale );

	static TFLOAT GetCursorX();
	static TFLOAT GetCursorY();
	static void   SetCursorX( TFLOAT a_fX );
	static void   SetCursorY( TFLOAT a_fY );
	static TFLOAT GetLineHeight();
	static TFLOAT GetCharWidth();
	static void   SetTabSize( TINT a_iCharacters );
	static void   SetWrap( TBOOL a_bWrap );

private:
	static void AddQuad( TFLOAT a_fSizeX, TFLOAT a_fSizeY, TFLOAT a_fX, TFLOAT a_fY, TCHAR a_cChar, TUINT32 a_uiColour );

private:
	static TFLOAT       ms_fCursorX;
	static TFLOAT       ms_fCursorY;
	static TFLOAT       ms_fTabWidth;
	static TFLOAT       ms_fWrapWidth;
	static TBOOL        ms_bWrap;
	static TBOOL        ms_bStarted;
	static TUINT        ms_uiNumGlyphs;
	static TUINT        ms_uiMaxGlyphs;
	static PDebugGlyph* ms_pGlyphs;
};

#endif // PRIMEDEBUG_DEV
