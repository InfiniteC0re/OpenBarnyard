#include "ToshiPCH.h"
#include "T2Texture_DX8.h"

#include "TRenderInterface_DX8.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 00615bc0
HRESULT T2Texture::Load()
{
	TPROFILER_SCOPE();

	TASSERT( m_pData != TNULL && m_uiDataSize != 0 );

	auto pRender    = TSTATICCAST( TRenderD3DInterface, TRenderInterface::GetSingleton() );
	auto pD3DDevice = pRender->GetDirect3DDevice();

	HRESULT hRes = D3DXGetImageInfoFromFileInMemory( m_pData, m_uiDataSize, &m_ImageInfo );

	if ( hRes == S_OK )
	{
		if ( m_ImageInfo.ResourceType == D3DRTYPE_TEXTURE )
		{
			return D3DXCreateTextureFromFileInMemory(
			    pD3DDevice,
			    m_pData,
			    m_uiDataSize,
			    &m_pD3DTexture
			);
		}
		else
		{
			if ( m_ImageInfo.ResourceType == D3DRTYPE_VOLUMETEXTURE )
			{
				return D3DXCreateVolumeTextureFromFileInMemory(
				    pD3DDevice,
				    m_pData,
				    m_uiDataSize,
				    &m_pD3DVolumeTexture
				);
			}
			else if ( m_ImageInfo.ResourceType == D3DRTYPE_CUBETEXTURE )
			{
				return D3DXCreateCubeTextureFromFileInMemory(
				    pD3DDevice,
				    m_pData,
				    m_uiDataSize,
				    &m_pD3DCubeTexture
				);
			}

			return D3DERR_WRONGTEXTUREFORMAT;
		}
	}

	return hRes;
}

void T2Texture::Unload()
{
	if ( m_pD3DTexture )
	{
		m_pD3DTexture->Release();
		m_pD3DTexture = TNULL;
	}

	if ( m_pData )
	{
		delete m_pData;
		m_pData = TNULL;
	}
}

IDirect3DCubeTexture8* T2Texture::GetD3DCubeTexture()
{
	return m_pD3DCubeTexture;
}

IDirect3DVolumeTexture8* T2Texture::GetD3DVolumeTexture()
{
	return m_pD3DVolumeTexture;
}

IDirect3DTexture8* T2Texture::GetD3DTexture()
{
	return m_pD3DTexture;
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

const D3DXIMAGE_INFO& T2Texture::GetImageInfo() const
{
	return m_ImageInfo;
}

void T2Texture::SetData( void* a_pData, TUINT a_uiDataSize )
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
}

TOSHI_NAMESPACE_END
