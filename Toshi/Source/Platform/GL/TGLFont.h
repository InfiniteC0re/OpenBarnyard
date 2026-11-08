#pragma once
#include "TRenderInterface_GL.h"

#include "Platform/GL/TGLHeaders.h"

TOSHI_NAMESPACE_START

// NOTE: CD3DFont of the DirectX 8 SDK samples (d3dfont.cpp), that retail links in for its debug text.
// Draws text from a texture that has every printable ASCII character.
class TGLFont
{
public:
	// Font creation flags
	enum
	{
		GLFONT_BOLD    = 0x0001,
		GLFONT_ITALIC  = 0x0002,
		GLFONT_ZENABLE = 0x0004,
	};

	// Font rendering flags
	enum
	{
		GLFONT_FILTERED = 0x0004,
	};

	static constexpr TUINT MAX_NUM_VERTICES = 50 * 6;

		struct FONT2DVERTEX
	{
		TFLOAT  fX, fY, fZ, fRHW;
		TUINT32 uiColour;
		TFLOAT  fTU, fTV;
	};

public:
	TGLFont( const TCHAR* a_szFontName, TUINT a_dwHeight, TUINT a_dwFlags );
	~TGLFont();

	TBOOL DrawText( TFLOAT a_fX, TFLOAT a_fY, TUINT32 a_dwColour, const TCHAR* a_szText, TUINT a_dwFlags );
	TBOOL GetTextExtent( const TCHAR* a_szText, TINT* a_pWidth, TINT* a_pHeight );

	TBOOL InitDeviceObjects( TRenderGLInterface* a_pRenderer );
	TBOOL RestoreDeviceObjects();
	TBOOL InvalidateDeviceObjects();
	TBOOL DeleteDeviceObjects();

private:
	void DrawTriangles( TUINT a_dwNumTriangles );

#ifdef TOSHI_SKU_WASM
	TBOOL BuildBuiltInAtlas();
#endif

private:
	TCHAR                          m_szFontName[ 80 ];
	TUINT                          m_dwFontHeight;
	TUINT                          m_dwFontFlags;
	TRenderGLInterface*            m_pRenderer;
	GLuint                         m_uiFontTexture;
	GLuint                         m_uiProgram;
	FONT2DVERTEX                   m_aVertices[ MAX_NUM_VERTICES ];
	TUINT                          m_dwTexWidth;
	TUINT                          m_dwTexHeight;
	TFLOAT                         m_fTextScale;
	TFLOAT                         m_afTexCoords[ 128 - 32 ][ 4 ];
	TRenderGLInterface::STATEBLOCK m_oSavedStateBlock;
	TRenderGLInterface::STATEBLOCK m_oDrawTextStateBlock;
	TBOOL                          m_bStateBlocksRecorded;
};

TOSHI_NAMESPACE_END
