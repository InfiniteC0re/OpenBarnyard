#include "ToshiPCH.h"
#include "T2Texture_GL.h"

#include "TRenderInterface_GL.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TBOOL T2Texture::Load()
{
	TPROFILER_SCOPE();

	TASSERT( m_pData != TNULL && m_uiDataSize != 0 );

	m_uiTexture = TRenderGLInterface::Interface()->CreateTextureFromFileInMemory( m_pData, m_uiDataSize, &m_ImageInfo );

	return m_uiTexture != 0;
}

void T2Texture::Unload()
{
	if ( m_uiTexture )
	{
		if ( TRenderGLInterface* pRender = TRenderGLInterface::Interface() )
			pRender->DestroyTexture( m_uiTexture );

		m_uiTexture = 0;
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

const TGLTextureInfo& T2Texture::GetImageInfo() const
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
	m_uiTexture = 0;
	TUtil::MemClear( &m_ImageInfo, sizeof( m_ImageInfo ) );
}

TOSHI_NAMESPACE_END
