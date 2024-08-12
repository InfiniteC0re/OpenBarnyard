#pragma once
#include <Render/TMaterial.h>
#include <Render/TTexture.h>

class AGUI2Material : public Toshi::TMaterial
{
public:
    friend class AGUI2Renderer;
    friend class AGUI2RendererDX8;

public:
    AGUI2Material()
    {
        m_iNumTex          = 1;
        m_pTextureResource = TNULL;
        m_eBlendState      = 1;
        m_eTextureAddress  = 1;
        m_iMipMapLODBias   = 0;
    }

    void SetBlendState( TUINT a_eBlendState )
    {
        m_eBlendState = a_eBlendState;
    }

    void SetTextureAddress( TUINT a_eTextureAddress )
    {
        m_eTextureAddress = a_eTextureAddress;
    }

private:
    Toshi::TTexture* m_pTextureResource;
    TUINT            m_eBlendState;
    TUINT            m_eTextureAddress;
    TUINT            m_iMipMapLODBias;
};
