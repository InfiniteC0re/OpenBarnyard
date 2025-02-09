#include "pch.h"
#include "AEnhancedTexture.h"
#include "SOIL2/SOIL2.h"
#include "HookHelpers.h"

#include <BYardSDK/AMaterialLibrary.h>

#include <Platform/GL/T2Render_GL.h>
#include <Platform/GL/T2GLTexture_GL.h>
#include <Platform/DX8/T2Texture_DX8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

MEMBER_HOOK( 0x00615bc0, T2Texture, T2Texture_Load, HRESULT )
{
	HRESULT hRes = CallOriginal();

	const void* pData;
	TUINT       uiSize;
	GetData( pData, uiSize );

	T2GLTexture* pTexture = AEnhancedTextureManager::GetSingleton()->CreateTexture(
		GetD3DTexture(),
		pData,
		uiSize
	);

	return hRes;
}

MEMBER_HOOK( 0x00615f60, AMaterialLibrary, AMaterialLibrary_DestroyTextures, void )
{
	for ( TINT i = 0; i < m_iNumTextures; i++ )
	{
		T2GLTexture* pGLTexture = AEnhancedTextureManager::GetAssociation( m_pTextures[ i ].pTexture->GetD3DTexture() );
		AEnhancedTextureManager::GetSingleton()->DestroyTexture( pGLTexture );
	}

	CallOriginal();
}

AEnhancedTextureManager::AEnhancedTextureManager()
{
	TASSERT( TTRUE == T2TextureManager::IsSingletonCreated() );

	InstallHook<T2Texture_Load>();
	InstallHook<AMaterialLibrary_DestroyTextures>();
}

AEnhancedTextureManager::~AEnhancedTextureManager()
{
}

T2GLTexture* AEnhancedTextureManager::CreateTexture( IDirect3DTexture8* a_pD3DTexture, const void* a_pData, TSIZE a_uiDataSize )
{
	TASSERT( a_uiDataSize > 0 );

	int            width, height, channels;
	unsigned char* texData = SOIL_load_image_from_memory( (const unsigned char*)a_pData, a_uiDataSize, &width, &height, &channels, 4 );
	
	T2GLTexture* pTexture = new T2GLTexture();
	pTexture->Create( TEXTURE_FORMAT_R8G8B8A8_UNORM, width, height, texData );

	// FIXME: use settings of the texture here
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	SOIL_free_image_data( texData );

	AddAssociation( a_pD3DTexture, pTexture );

	return pTexture;
}

void AEnhancedTextureManager::DestroyTexture( T2GLTexture* a_pTexture )
{
	RemoveAssociation( a_pTexture );
	delete a_pTexture;
}

void AEnhancedTextureManager::AddAssociation( IDirect3DTexture8* a_pD3DTexture, T2GLTexture* a_pGLTexture )
{
	TASSERT( m_mapAssociations[ a_pD3DTexture ] == m_mapAssociations.End() );
	m_mapAssociations.Insert( a_pD3DTexture, a_pGLTexture );
}

void AEnhancedTextureManager::RemoveAssociation( IDirect3DTexture8* a_pD3DTexture )
{
	auto it = m_mapAssociations[ a_pD3DTexture ];

	if ( it != m_mapAssociations.End() )
		m_mapAssociations.Remove( it );
}

void AEnhancedTextureManager::RemoveAssociation( Toshi::T2GLTexture* a_pGLTexture )
{
	auto it = m_mapAssociations.FindByValue( a_pGLTexture );

	if ( it != m_mapAssociations.End() )
		m_mapAssociations.Remove( it );
}

T2GLTexture* AEnhancedTextureManager::GetAssociation( IDirect3DTexture8* a_pD3DTexture )
{
	AEnhancedTextureManager* pTexMngr = AEnhancedTextureManager::GetSingleton();

	auto it = pTexMngr->m_mapAssociations[ a_pD3DTexture ];
	return ( it != pTexMngr->m_mapAssociations.End() ) ? it->GetSecond() : T2TextureManager::GetSingleton()->GetInvalidTexture();
}
