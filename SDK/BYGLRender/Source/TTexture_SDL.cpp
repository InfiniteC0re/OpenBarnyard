#include "pch.h"
#include "TTexture_SDL.h"
#include "TRender_SDL.h"

#include "SOIL2/SOIL2.h"

namespace Toshi
{

void TTextureSDL::Init()
{
	TASSERT( 0 < m_DataSize );
	TRenderSDL* pRender = TRenderSDL::Interface();

	int            width, height, channels;
	unsigned char* texData = SOIL_load_image_from_memory( m_TexData, m_DataSize, &width, &height, &channels, 4 );
	m_TexInfo->Handle      = TRenderSDL::Interface()->CreateTexture( width, height, GL_RGBA, TFALSE, texData );

	// FIXME: use settings of the texture here
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	SOIL_free_image_data( texData );

	TTextureManagerSDL::GetSingleton()->AddTexture( this );
	SelectSamplerId();

#ifdef TOSHI_DEBUG
	TTextureManagerSDL::s_NumTextures += 1;
#endif
}

void TTextureSDL::Deinit()
{
	glDeleteTextures( 1, &m_TexInfo->Handle );

	if ( m_pPrevTexture )
		m_pPrevTexture->m_pNextTexture = m_pNextTexture;

	if ( m_pNextTexture )
		m_pNextTexture->m_pPrevTexture = m_pPrevTexture;

	auto pTexManager = TTextureManagerSDL::GetSingleton();

	if ( pTexManager->GetLastTexture() == this )
		pTexManager->SetLastTexture( m_pPrevTexture );

	m_pNextTexture = TNULL;
	m_pPrevTexture = TNULL;

#ifdef TOSHI_DEBUG
	TASSERT( TTextureManagerSDL::s_NumTextures > 0 );
	TTextureManagerSDL::s_NumTextures -= 1;
#endif
}

void TTextureSDL::Bind( GLenum a_eTarget )
{
	glBindTexture( a_eTarget, m_TexInfo->Handle );
}

TTextureSDL* TTextureSDL::InitRunTime( Info* pTextureInfo )
{
	TRenderSDL* pRender = TRenderSDL::Interface();

	TTextureSDL* pTexture = new TTextureSDL;
	TASSERT( pTexture != TNULL );

	pTexture->m_TexName = "runtime";
	pTexture->m_TexInfo = pTextureInfo;
	pTexture->m_TexData = TNULL;
	TTextureManagerSDL::GetSingleton()->AddTexture( pTexture );

#ifdef TOSHI_DEBUG
	TTextureManagerSDL::s_NumTextures += 1;
#endif

	return pTexture;
}

TTextureSDL* TTextureSDL::InitRunTime( TEXTURE_FORMAT format, UINT width, UINT height, const void* srcData )
{
	TRenderSDL* pRender = TRenderSDL::Interface();

	TTextureSDL* pTexture = new TTextureSDL;
	TASSERT( pTexture != TNULL );

	pTexture->m_TexName = "runtime";

	pTexture->m_TexInfo = new TTextureSDL::Info;
	TASSERT( pTexture->m_TexInfo != TNULL );

	GLenum glFormat =
	    ( format == TEXTURE_FORMAT_R8G8B8A8_UNORM ) ? GL_RGBA :
	    ( format == TEXTURE_FORMAT_R8G8_UNORM )     ? GL_RG :
	                                                  GL_R;

	pTexture->m_TexInfo->Handle      = pRender->CreateTexture( width, height, glFormat, TFALSE, srcData );
	pTexture->m_TexInfo->Width       = width;
	pTexture->m_TexInfo->Height      = height;
	pTexture->m_TexInfo->MipMapCount = 1;
	pTexture->m_TexInfo->Format      = format;

	TTextureManagerSDL::GetSingleton()->AddTexture( pTexture );

#ifdef TOSHI_DEBUG
	TTextureManagerSDL::s_NumTextures += 1;
#endif

	return pTexture;
}

void TTextureSDL::SelectSamplerId()
{
	if ( m_eAddressU == TEXTURE_ADDRESS_CLAMP && m_eAddressV == TEXTURE_ADDRESS_CLAMP )
	{
		if ( m_eFilter == TEXTURE_FILTER_MIN_MAG_MIP_POINT )
			m_SamplerId = 0;
		else if ( m_eFilter == TEXTURE_FILTER_MIN_MAG_MIP_LINEAR || m_eFilter == TEXTURE_FILTER_MIN_POINT_MAG_MIP_LINEAR )
			m_SamplerId = 1;
		else if ( m_eFilter == TEXTURE_FILTER_MIN_MAG_LINEAR_MIP_POINT )
			m_SamplerId = 5;
		else
			TASSERT( TFALSE );
	}
	else if ( m_eAddressU == TEXTURE_ADDRESS_WRAP && m_eAddressV == TEXTURE_ADDRESS_WRAP )
	{
		if ( m_eFilter == TEXTURE_FILTER_MIN_MAG_MIP_POINT )
			m_SamplerId = 2;
		else if ( m_eFilter == TEXTURE_FILTER_MIN_MAG_MIP_LINEAR )
			m_SamplerId = 3;
		else if ( m_eFilter == TEXTURE_FILTER_MIN_MAG_LINEAR_MIP_POINT )
			m_SamplerId = 6;
		else
			TASSERT( TFALSE );
	}
	else
	{
		if ( m_eAddressU == TEXTURE_ADDRESS_CLAMP && m_eAddressV == TEXTURE_ADDRESS_WRAP )
		{
			if ( m_eFilter == TEXTURE_FILTER_MIN_MAG_MIP_POINT )
				m_SamplerId = 9;
			else if ( m_eFilter == TEXTURE_FILTER_MIN_MAG_MIP_LINEAR )
				m_SamplerId = 10;
			else if ( m_eFilter == TEXTURE_FILTER_MIN_MAG_LINEAR_MIP_POINT )
				m_SamplerId = 0xb;

			TASSERT( TFALSE );
		}
		else
		{
			TASSERT( TFALSE );

			m_eAddressU = TEXTURE_ADDRESS_CLAMP;
			m_eAddressV = TEXTURE_ADDRESS_CLAMP;
			m_eFilter   = TEXTURE_FILTER_MIN_MAG_MIP_LINEAR;
			m_SamplerId = 3;
		}
	}
}

void TTextureSDL::SetWrap( TEXTURE_ADDRESS_MODE eAddressU, TEXTURE_ADDRESS_MODE eAddressV )
{
	m_eAddressU = eAddressU;
	m_eAddressV = eAddressV;
	SelectSamplerId();
}

TTextureManagerSDL::TTextureManagerSDL() :
    m_pLastTexture( TNULL )
{
	uint8_t* srcData;

	// White texture
	constexpr int WhiteTextureWidth  = 8;
	constexpr int WhiteTextureHeight = 8;
	constexpr int WhiteTextureSize   = WhiteTextureWidth * WhiteTextureHeight * 4;

	srcData = (uint8_t*)TMemalign( WhiteTextureSize, 16 );
	TUtil::MemSet( srcData, 0xFF, WhiteTextureSize );
	m_pWhiteTexture = TTextureSDL::InitRunTime( TEXTURE_FORMAT_R8G8B8A8_UNORM, WhiteTextureWidth, WhiteTextureHeight, srcData );
	m_pWhiteTexture->SetName( "white" );
	m_pWhiteTexture->m_pNextTexture = TNULL;
	m_pWhiteTexture->m_pPrevTexture = TNULL;
	TFree( srcData );

	// Invalid texture
	constexpr int InvalidTextureWidth  = 128;
	constexpr int InvalidTextureHeight = 128;
	constexpr int InvalidTextureSize   = InvalidTextureWidth * InvalidTextureHeight * 4;

	srcData = (uint8_t*)TMemalign( InvalidTextureSize, 16 );

	for ( uint8_t* pos = srcData; pos < srcData + InvalidTextureSize; pos += 4 )
	{
		*( pos )     = rand() % 256;
		*( pos + 1 ) = rand() % 256;
		*( pos + 2 ) = rand() % 256;
		*( pos + 3 ) = rand() % 256;
	}

	m_pInvalidTexture = TTextureSDL::InitRunTime( TEXTURE_FORMAT_R8G8B8A8_UNORM, 8, 8, srcData );
	m_pInvalidTexture->SetName( "invalid" );
	m_pInvalidTexture->m_pNextTexture = TNULL;
	m_pInvalidTexture->m_pPrevTexture = TNULL;
	TFree( srcData );
}

TTextureSDL* TTextureManagerSDL::FindTexture( const char* texName )
{
	TTextureSDL* pCurrentTex = m_pLastTexture;

	while ( TTRUE )
	{
		if ( pCurrentTex == TNULL )
		{
			return m_pInvalidTexture;
		}

		if ( T2String8::CompareNoCase( pCurrentTex->GetName(), texName ) == 0 )
		{
			break;
		}

		pCurrentTex = pCurrentTex->m_pPrevTexture;
	}

	return pCurrentTex;
}

void TTextureManagerSDL::AddTexture( TTextureSDL* pTexture )
{
	pTexture->m_pPrevTexture = GetLastTexture();
	pTexture->m_pNextTexture = TNULL;

	if ( pTexture->m_pPrevTexture != TNULL )
	{
		pTexture->m_pPrevTexture->m_pNextTexture = pTexture;
	}

	SetLastTexture( pTexture );
}

} // namespace Toshi
