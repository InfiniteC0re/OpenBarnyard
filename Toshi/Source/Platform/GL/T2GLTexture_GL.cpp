#include "ToshiPCH.h"
#include "T2GLTexture_GL.h"
#include "T2Render_GL.h"

TOSHI_NAMESPACE_START

T2GLTexture::~T2GLTexture()
{
	if ( m_pHandle != 0 )
		Destroy();
}

void T2GLTexture::Destroy()
{
	T2TextureManager::GetSingleton()->RemoveTexture( this );
	T2Render::DestroyTexture( m_pHandle );

	m_pHandle = 0;
}

void T2GLTexture::Create( TEXTURE_FORMAT a_eFormat, UINT a_uiWidth, UINT a_uiHeight, const void* a_pData )
{
	if ( m_pHandle != 0 )
		Destroy();

	m_pchTexName = "runtime";

	GLenum glFormat =
	    ( a_eFormat == TEXTURE_FORMAT_R8G8B8A8_UNORM ) ? GL_RGBA :
	    ( a_eFormat == TEXTURE_FORMAT_R8G8_UNORM )     ? GL_RG :
	                                                     GL_R;

	m_pHandle       = T2Render::CreateTexture( a_uiWidth, a_uiHeight, glFormat, glFormat, GL_UNSIGNED_BYTE, TFALSE, a_pData );
	m_uiWidth       = a_uiWidth;
	m_uiHeight      = a_uiHeight;
	m_uiMipMapCount = 1;
	m_eFormat       = a_eFormat;

	T2TextureManager::GetSingleton()->AddTexture( this );
}

void T2GLTexture::SetWrap( TEXTURE_ADDRESS_MODE eAddressU, TEXTURE_ADDRESS_MODE eAddressV )
{
	m_eAddressU = eAddressU;
	m_eAddressV = eAddressV;
}

T2TextureManager::T2TextureManager()
    : m_pLastTexture( TNULL )
{
	{
		// White texture
		constexpr int WhiteTextureWidth  = 8;
		constexpr int WhiteTextureHeight = 8;
		constexpr int WhiteTextureSize   = WhiteTextureWidth * WhiteTextureHeight * 4;

		TUINT8* srcData = (TUINT8*)TMemalign( WhiteTextureSize, 16 );
		TUtil::MemSet( srcData, 0xFF, WhiteTextureSize );

		m_pWhiteTexture = new T2GLTexture;
		m_pWhiteTexture->Create( TEXTURE_FORMAT_R8G8B8A8_UNORM, WhiteTextureWidth, WhiteTextureHeight, srcData );
		m_pWhiteTexture->SetName( "white" );
		m_pWhiteTexture->m_pNextTexture = TNULL;
		m_pWhiteTexture->m_pPrevTexture = TNULL;
		TFree( srcData );
	}

	{
		// Invalid texture
		constexpr int InvalidTextureWidth  = 128;
		constexpr int InvalidTextureHeight = 128;
		constexpr int InvalidTextureSize   = InvalidTextureWidth * InvalidTextureHeight * 4;

		TUINT8* srcData = (TUINT8*)TMemalign( InvalidTextureSize, 16 );

		for ( TUINT8* pos = srcData; pos < srcData + InvalidTextureSize; pos += 4 )
		{
			*( pos )     = rand() % 256;
			*( pos + 1 ) = rand() % 256;
			*( pos + 2 ) = rand() % 256;
			*( pos + 3 ) = rand() % 256;
		}

		m_pInvalidTexture = new T2GLTexture;
		m_pInvalidTexture->Create( TEXTURE_FORMAT_R8G8B8A8_UNORM, 8, 8, srcData );
		m_pInvalidTexture->SetName( "invalid" );
		m_pInvalidTexture->m_pNextTexture = TNULL;
		m_pInvalidTexture->m_pPrevTexture = TNULL;
		TFree( srcData );
	}
}

T2GLTexture* T2TextureManager::FindTexture( const TCHAR* a_pchTexName )
{
	T2GLTexture* pCurrentTex = m_pLastTexture;

	while ( TTRUE )
	{
		if ( pCurrentTex == TNULL )
			return m_pInvalidTexture;

		if ( !T2String8::CompareNoCase( pCurrentTex->GetName(), a_pchTexName ) )
			break;

		pCurrentTex = pCurrentTex->m_pPrevTexture;
	}

	return pCurrentTex;
}

void T2TextureManager::AddTexture( T2GLTexture* a_pTexture )
{
	a_pTexture->m_pPrevTexture = GetLastTexture();
	a_pTexture->m_pNextTexture = TNULL;

	if ( a_pTexture->m_pPrevTexture != TNULL )
		a_pTexture->m_pPrevTexture->m_pNextTexture = a_pTexture;

	m_pLastTexture = a_pTexture;
}

void T2TextureManager::RemoveTexture( T2GLTexture* a_pTexture )
{
	if ( a_pTexture->m_pPrevTexture )
		a_pTexture->m_pPrevTexture->m_pNextTexture = a_pTexture->m_pNextTexture;

	if ( a_pTexture->m_pNextTexture )
		a_pTexture->m_pNextTexture->m_pPrevTexture = a_pTexture->m_pPrevTexture;

	if ( m_pLastTexture == a_pTexture )
		m_pLastTexture = a_pTexture->m_pPrevTexture;

	a_pTexture->m_pNextTexture = TNULL;
	a_pTexture->m_pPrevTexture = TNULL;
}

TOSHI_NAMESPACE_END
