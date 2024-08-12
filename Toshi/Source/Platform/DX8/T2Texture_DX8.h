#pragma once
#include "Toshi/Typedefs.h"

#include <d3dx8.h>

namespace Toshi
{

class T2Texture
{
public:
    T2Texture()
    {
        m_uiDataSize  = 0;
        m_pData       = TNULL;
        m_Unk1        = 0;
        m_Unk2        = 0;
        m_pD3DTexture = TNULL;
    }

    ~T2Texture()
    {
        Unload();
    }

    void SetData( void* a_pData, TUINT a_uiDataSize )
    {
        m_pData      = a_pData;
        m_uiDataSize = a_uiDataSize;
    }

    HRESULT Load();

    void Unload();

    const D3DXIMAGE_INFO& GetImageInfo() const
    {
        return m_ImageInfo;
    }

    TUINT GetWidth() const
    {
        return m_ImageInfo.Width;
    }

    TUINT GetHeight() const
    {
        return m_ImageInfo.Height;
    }

    TUINT GetMipLevels() const
    {
        return m_ImageInfo.MipLevels;
    }

    IDirect3DTexture8* GetD3DTexture()
    {
        return m_pD3DTexture;
    }

    IDirect3DVolumeTexture8* GetD3DVolumeTexture()
    {
        return m_pD3DVolumeTexture;
    }

    IDirect3DCubeTexture8* GetD3DCubeTexture()
    {
        return m_pD3DCubeTexture;
    }

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

} // namespace Toshi