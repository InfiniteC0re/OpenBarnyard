#include "ToshiPCH.h"
#include "T2Texture_DX11.h"

#include "TRenderInterface_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

HRESULT T2Texture::Load()
{
	TPROFILER_SCOPE();

	TASSERT( m_pData != TNULL && m_uiDataSize != 0 );

	m_pD3DTexture = TRenderD3DInterface::Interface()->CreateTextureFromFileInMemory( m_pData, m_uiDataSize, &m_ImageInfo );

	return m_pD3DTexture ? S_OK : E_FAIL;
}

void T2Texture::Unload()
{
	if ( m_pD3DTexture )
	{
		if ( TRenderD3DInterface* pRender = TRenderD3DInterface::Interface() )
			pRender->InvalidateShaderResource( m_pD3DTexture );

		m_pD3DTexture->Release();
		m_pD3DTexture = TNULL;
	}

	if ( m_pData )
	{
		delete m_pData;
		m_pData = TNULL;
	}
}

TUINT T2Texture::GetMipLevels() const
{
	return m_ImageInfo.MipLevels;
}

TUINT T2Texture::GetHeight() const
{
	return m_ImageInfo.Height;
}

TUINT T2Texture::GetWidth() const
{
	return m_ImageInfo.Width;
}

const D3D11_TEXTURE2D_DESC& T2Texture::GetImageInfo() const
{
	return m_ImageInfo;
}

void T2Texture::SetData( const void* a_pData, TUINT a_uiDataSize )
{
	m_pData      = a_pData;
	m_uiDataSize = a_uiDataSize;
}

T2Texture::~T2Texture()
{
	Unload();
}

T2Texture::T2Texture()
{
	m_uiDataSize  = 0;
	m_pData       = TNULL;
	m_Unk1        = 0;
	m_Unk2        = 0;
	m_pD3DTexture = TNULL;
	TUtil::MemClear( &m_ImageInfo, sizeof( m_ImageInfo ) );
}

TOSHI_NAMESPACE_END
