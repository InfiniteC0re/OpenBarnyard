#include "pch.h"
#include "ASkinLightingManager.h"
#include "Assets/AMaterialLibraryManager.h"
#include "Assets/AAssetLoader.h"

#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0060dd70
ASkinLightingManager::ASkinLightingManager()
{
	m_iUnk = 0;
	
	// Load the matlib that contains all lighting textures
	m_pSkinLightMatLib = AMaterialLibraryManager::List::GetSingleton()->CreateLibraryFromAsset(
	    "Data\\MiscTextures\\SkinLighttex.ttl",
	    AAssetLoader::GetAssetTRB( AAssetType_Startup )
	);

	// Get texture factory to actually create TTexture objects (not sure why duplicates are needed)
	TTextureFactory* pTexFactory = TRenderInterface::GetSingleton()->GetSystemResource<TTextureFactory>(
		SYSRESOURCE_TEXTUREFACTORY
	);

	TTRB::ERROR eError = m_oTRB.Load( "Data\\MiscTextures\\SkinLighting.trb" );
	TASSERT( eError == TTRB::ERROR_OK );

	m_pSkinLightHeader = m_oTRB.CastSymbol<SkinLightTRBHeader>( "skinlight" );
	TASSERT( m_pSkinLightHeader != TNULL );

	for ( TINT i = 0; i < m_pSkinLightHeader->iNumLights; i++ )
	{
		SkinLight& skinLight = m_pSkinLightHeader->pSkinLights[ i ];

		// Get texture indices
		TINT iTex1 = m_pSkinLightMatLib->FindTextureIndex( skinLight.szTex1 );
		TINT iTex2 = m_pSkinLightMatLib->FindTextureIndex( skinLight.szTex2 );
		TINT iTex3 = m_pSkinLightMatLib->FindTextureIndex( skinLight.szTex3 );
		TINT iTex4 = m_pSkinLightMatLib->FindTextureIndex( skinLight.szTex4 );

		if ( iTex4 < 0 )
			iTex4 = iTex2;

		// Create the textures

		if ( iTex1 > -1 )
			skinLight.pTex1 = pTexFactory->CreateFromT2Texture( m_pSkinLightMatLib->GetTexture( iTex1 )->pTexture );

		if ( iTex2 > -1 )
			skinLight.pTex2 = pTexFactory->CreateFromT2Texture( m_pSkinLightMatLib->GetTexture( iTex2 )->pTexture );

		if ( iTex3 > -1 )
			skinLight.pTex3 = pTexFactory->CreateFromT2Texture( m_pSkinLightMatLib->GetTexture( iTex3 )->pTexture );

		if ( iTex4 > -1 )
			skinLight.pTex4 = pTexFactory->CreateFromT2Texture( m_pSkinLightMatLib->GetTexture( iTex4 )->pTexture );
	}

	m_oTRB.DeleteSymbolTable();
}

// $Barnyard: FUNCTION 0060dd70
ASkinLightingManager::~ASkinLightingManager()
{
	AMaterialLibraryManager::List::GetSingleton()->DestroyLibrary( m_pSkinLightMatLib );
}
