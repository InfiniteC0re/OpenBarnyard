#pragma once
#include <PrimeDebug.h>

#ifdef PRIMEDEBUG_DEV
#  include <Draw/PDebugRenderer.h>
#  include <Util/PArray.h>

#  include <Render/TBackend.h>

#  if defined( TRENDERINTERFACE_DX8 ) || defined( TRENDERINTERFACE_DX11 ) || defined( TRENDERINTERFACE_GL )
#	define APRIMEDEBUG_D3DFONT // the debug text font of TOSHI's D3D renderers (CD3DFont)
#  endif

namespace Toshi
{
class T2GUIFont;
class TDebugFontHAL;
} // namespace Toshi

class APrimeDebugRenderer : public PDebugRenderer
{
public:
	// The fraction of the distance to the camera the wires of the solid shapes are moved towards it
	static constexpr TFLOAT WIRE_NUDGE = 0.004f;

public:
	APrimeDebugRenderer();
	~APrimeDebugRenderer();

	// The screen part is drawn between these (an AGUI2 renderer scene)
	TBOOL Begin2D();
	void  End2D();

	//-----------------------------------------------------------------------------
	// PDebugRenderer
	//-----------------------------------------------------------------------------
	virtual void  DrawRect( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fWidth, TFLOAT a_fHeight, TUINT32 a_uiColour ) override;
	virtual void  DrawGlyphs( const PDebugGlyph* a_pGlyphs, TINT a_iNumGlyphs ) override;
	virtual void  DrawLines2D( const PDebugVertex* a_pVertices, TINT a_iNumVertices ) override;
	virtual void  DrawLines3D( const PDebugVertex* a_pVertices, TINT a_iNumVertices, TBOOL a_bDepth, TBOOL a_bOverSolid ) override;
	virtual void  DrawTriangles3D( const PDebugVertex* a_pVertices, TINT a_iNumVertices, TBOOL a_bDepth ) override;
	virtual TINT  GetScreenWidth() override;
	virtual TINT  GetScreenHeight() override;
	virtual TBOOL ProjectToScreen( const Toshi::TVector4& a_rcPosition, TFLOAT& a_rfX, TFLOAT& a_rfY ) override;

private:
	// A position on the screen (fractions) in the coordinates of the AGUI2 root element
	void ToGUI( TFLOAT a_fX, TFLOAT a_fY, TFLOAT& a_rfX, TFLOAT& a_rfY );

#  ifdef APRIMEDEBUG_D3DFONT
	// The debug text font for characters a_iPixels high (created once for each height)
	Toshi::TDebugFontHAL* GetFont( TINT a_iPixels, TFLOAT& a_rfCharWidth );
#  endif

	// Lines (a_bLines) or triangles of the world with the line shader
	void Draw3D( const PDebugVertex* a_pVertices, TINT a_iNumVertices, TBOOL a_bDepth, TBOOL a_bLines, TBOOL a_bOverSolid );

private:
#  ifdef APRIMEDEBUG_D3DFONT
	static constexpr TINT MAX_FONTS = 8;

	struct Font
	{
		TINT             iPixels;
		TFLOAT           fCharWidth;
		Toshi::TDebugFontHAL* pFont;
	};

	Font                m_aFonts[ MAX_FONTS ];
	TINT                m_iNumFonts;
	PArray<PDebugGlyph> m_oGlyphs; // drawn after the AGUI2 scene
#  endif

	Toshi::T2GUIFont* m_pFont;
	TBOOL             m_bIn2D;
	TFLOAT            m_fRootWidth;
	TFLOAT            m_fRootHeight;
};

#endif // PRIMEDEBUG_DEV
