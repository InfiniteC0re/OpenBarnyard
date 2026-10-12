#pragma once
#include <d3d8.h>

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

	struct FONT2DVERTEX
	{
		TFLOAT  fX, fY, fZ, fRHW;
		TUINT32 uiColour;
		TFLOAT  fTU, fTV;
	};

	static constexpr DWORD D3DFVF_FONT2DVERTEX = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;

public:
	CD3DFont( const TCHAR* a_szFontName, DWORD a_dwHeight, DWORD a_dwFlags );
	~CD3DFont();

	HRESULT DrawText( TFLOAT a_fX, TFLOAT a_fY, DWORD a_dwColour, const TCHAR* a_szText, DWORD a_dwFlags );
	HRESULT GetTextExtent( const TCHAR* a_szText, SIZE* a_pSize );

	HRESULT InitDeviceObjects( IDirect3DDevice8* a_pDevice );
	HRESULT RestoreDeviceObjects();
	HRESULT InvalidateDeviceObjects();
	HRESULT DeleteDeviceObjects();

private:
	TCHAR                   m_szFontName[ 80 ];             // 0x00
	DWORD                   m_dwFontHeight;                 // 0x50
	DWORD                   m_dwFontFlags;                  // 0x54
	IDirect3DDevice8*       m_pDevice;                      // 0x58
	IDirect3DTexture8*      m_pTexture;                     // 0x5C
	IDirect3DVertexBuffer8* m_pVB;                          // 0x60
	DWORD                   m_dwTexWidth;                   // 0x64
	DWORD                   m_dwTexHeight;                  // 0x68
	TFLOAT                  m_fTextScale;                   // 0x6C
	TFLOAT                  m_afTexCoords[ 128 - 32 ][ 4 ]; // 0x70
	DWORD                   m_dwSavedStateBlock;            // 0x670
	DWORD                   m_dwDrawTextStateBlock;         // 0x674
};

TOSHI_NAMESPACE_END
