#pragma once
#include "Toshi/Typedefs.h"

#include <d3dx8.h>

TOSHI_NAMESPACE_START

class T2Texture
{
public:
	T2Texture();
	~T2Texture();

	void SetData( void* a_pData, TUINT a_uiDataSize );

	HRESULT Load();
	void Unload();

	const D3DXIMAGE_INFO& GetImageInfo() const;
	TUINT GetWidth() const;
	TUINT GetHeight() const;
	TUINT GetMipLevels() const;

	IDirect3DTexture8* GetD3DTexture();
	IDirect3DVolumeTexture8* GetD3DVolumeTexture();
	IDirect3DCubeTexture8* GetD3DCubeTexture();

private:
	TUINT          m_uiDataSize;
	void*          m_pData;
	TUINT          m_Unk1;
	TUINT          m_Unk2;
	D3DXIMAGE_INFO m_ImageInfo;
	union
	{
		IDirect3DTexture8*       m_pD3DTexture;
		IDirect3DVolumeTexture8* m_pD3DVolumeTexture;
		IDirect3DCubeTexture8*   m_pD3DCubeTexture;
	};
};

TOSHI_NAMESPACE_END
