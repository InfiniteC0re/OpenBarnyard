#pragma once
#include "TRenderInterface_DX11.h"

#include <d3d11.h>

TOSHI_NAMESPACE_START

// NOTE: this is CD3DFont from the DirectX 8 SDK samples (d3dfont.cpp) that retail links in for TDebugD3DText.
// Draws text from a texture that has every printable ASCII character rendered with GDI.
class CD3DFont
{
public:
	// Font creation flags
	enum
	{
		D3DFONT_BOLD    = 0x0001,
		D3DFONT_ITALIC  = 0x0002,
		D3DFONT_ZENABLE = 0x0004,
	};

	// Font rendering flags
	enum
	{
		D3DFONT_FILTERED = 0x0004,
	};

	static constexpr TUINT MAX_NUM_VERTICES = 50 * 6;

	// D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1
	struct FONT2DVERTEX
	{
		TFLOAT  fX, fY, fZ, fRHW;
		TUINT32 uiColour;
		TFLOAT  fTU, fTV;
	};

public:
	CD3DFont( const TCHAR* a_szFontName, DWORD a_dwHeight, DWORD a_dwFlags );
	~CD3DFont();

	HRESULT DrawText( TFLOAT a_fX, TFLOAT a_fY, DWORD a_dwColour, const TCHAR* a_szText, DWORD a_dwFlags );
	HRESULT GetTextExtent( const TCHAR* a_szText, SIZE* a_pSize );

	HRESULT InitDeviceObjects( TRenderD3DInterface* a_pRenderer );
	HRESULT RestoreDeviceObjects();
	HRESULT InvalidateDeviceObjects();
	HRESULT DeleteDeviceObjects();

private:
	void DrawTriangles( DWORD a_dwNumTriangles );

private:
	TCHAR                           m_szFontName[ 80 ];
	DWORD                           m_dwFontHeight;
	DWORD                           m_dwFontFlags;
	TRenderD3DInterface*            m_pRenderer;
	ID3D11ShaderResourceView*       m_pTexture;
	ID3D11Buffer*                   m_pVB;
	ID3D11VertexShader*             m_pVertexShader;
	ID3D11PixelShader*              m_pPixelShader;
	ID3D11InputLayout*              m_pInputLayout;
	DWORD                           m_dwTexWidth;
	DWORD                           m_dwTexHeight;
	TFLOAT                          m_fTextScale;
	TFLOAT                          m_afTexCoords[ 128 - 32 ][ 4 ];
	TRenderD3DInterface::STATEBLOCK m_oSavedStateBlock;
	TRenderD3DInterface::STATEBLOCK m_oDrawTextStateBlock;
	TBOOL                           m_bStateBlocksRecorded;
};

TOSHI_NAMESPACE_END
