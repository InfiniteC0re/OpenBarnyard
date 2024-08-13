#include "ToshiPCH.h"
#include "T2Texture_DX8.h"

#include "TRenderInterface_DX8.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

HRESULT T2Texture::Load()
{
	TASSERT( m_pData != TNULL && m_uiDataSize != 0 );

	auto pRender	= TSTATICCAST( TRenderD3DInterface, TRenderInterface::GetSingleton() );
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
				&m_pD3DTexture );
		}
		else
		{
			if ( m_ImageInfo.ResourceType == D3DRTYPE_VOLUMETEXTURE )
			{
				return D3DXCreateVolumeTextureFromFileInMemory(
					pD3DDevice,
					m_pData,
					m_uiDataSize,
					&m_pD3DVolumeTexture );
			}
			else if ( m_ImageInfo.ResourceType == D3DRTYPE_CUBETEXTURE )
			{
				return D3DXCreateCubeTextureFromFileInMemory(
					pD3DDevice,
					m_pData,
					m_uiDataSize,
					&m_pD3DCubeTexture );
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

TOSHI_NAMESPACE_END
