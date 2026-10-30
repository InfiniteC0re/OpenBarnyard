#pragma once
#include "PrimeDebug.h"

#ifdef PRIMEDEBUG_DEV
#  include <Math/TVector4.h>

// A vertex of the debug lines and triangles: on screen x, y are fractions of the screen (0..1, y down), in the world
// they're world coordinates
struct PDebugVertex
{
	TFLOAT  fX;       // 0x0
	TFLOAT  fY;       // 0x4
	TFLOAT  fZ;       // 0x8
	TUINT32 uiColour; // 0xC
};

// A character of the debug text: the centre and the half size of its cell, in fractions of the screen
struct PDebugGlyph
{
	TFLOAT  fX;
	TFLOAT  fY;
	TFLOAT  fHalfWidth;
	TFLOAT  fHalfHeight;
	TUINT32 uiColour;
	TCHAR   cChar;
};

//-----------------------------------------------------------------------------
// Purpose: what the debug tools draw with, implemented by the game with its
// renderer (Prime draws with its own shaders and vertex buffers: geDebugDraw,
// the "debugfont" texture, the debugline/primitives shaders).
// Colours are 0xAARRGGBB. 2D positions and sizes are fractions of the screen.
//-----------------------------------------------------------------------------
class PDebugRenderer
{
public:
	virtual ~PDebugRenderer() = default;

	// A filled rectangle, a_fX/a_fY its top left corner
	virtual void DrawRect( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fWidth, TFLOAT a_fHeight, TUINT32 a_uiColour ) = 0;

	// Characters, each drawn in its cell
	virtual void DrawGlyphs( const PDebugGlyph* a_pGlyphs, TINT a_iNumGlyphs ) = 0;

	// Pairs of vertices on the screen
	virtual void DrawLines2D( const PDebugVertex* a_pVertices, TINT a_iNumVertices ) = 0;

	virtual void DrawLines3D( const PDebugVertex* a_pVertices, TINT a_iNumVertices, TBOOL a_bDepth, TBOOL a_bOverSolid ) = 0;
	virtual void DrawTriangles3D( const PDebugVertex* a_pVertices, TINT a_iNumVertices, TBOOL a_bDepth )                 = 0;

	// The size of the screen (the render target) in pixels
	virtual TINT GetScreenWidth()  = 0;
	virtual TINT GetScreenHeight() = 0;

	// The position on the screen (fractions, y down) of a point in the world; TFALSE if it's behind the camera
	virtual TBOOL ProjectToScreen( const Toshi::TVector4& a_rcPosition, TFLOAT& a_rfX, TFLOAT& a_rfY ) = 0;

public:
	static void            SetRenderer( PDebugRenderer* a_pRenderer ) { ms_pRenderer = a_pRenderer; }
	static PDebugRenderer* GetRenderer() { return ms_pRenderer; }

private:
	static PDebugRenderer* ms_pRenderer;
};

#endif // PRIMEDEBUG_DEV
