#pragma once
#include "Toshi/Typedefs.h"

#include <d3d11.h>

TOSHI_NAMESPACE_START

class MODAPI T2Texture
{
public:
	T2Texture();
	~T2Texture();

	void SetData( const void* a_pData, TUINT a_uiDataSize );
	void GetData( const void*& a_rData, TUINT& a_rDataSize )
	{
		a_rData     = m_pData;
		a_rDataSize = m_uiDataSize;
	}

	HRESULT Load();
	void    Unload();

	const D3D11_TEXTURE2D_DESC& GetImageInfo() const;
	TUINT                       GetWidth() const;
	TUINT                       GetHeight() const;
	TUINT                       GetMipLevels() const;

	ID3D11ShaderResourceView* GetD3DTexture() { return m_pD3DTexture; }

protected:
	TUINT                     m_uiDataSize;
	const void*               m_pData;
	TUINT                     m_Unk1;
	TUINT                     m_Unk2;
	D3D11_TEXTURE2D_DESC      m_ImageInfo;
	ID3D11ShaderResourceView* m_pD3DTexture;
};

TOSHI_NAMESPACE_END
