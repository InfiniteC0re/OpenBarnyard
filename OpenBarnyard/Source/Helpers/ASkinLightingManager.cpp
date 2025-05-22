#include "pch.h"
#include "ASkinLightingManager.h"
#include "Assets/AMaterialLibraryManager.h"
#include "Assets/AAssetLoader.h"
#include "Render/ASkinShader/ASkinMaterial.h"

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

// $Barnyard: FUNCTION 0060df20
ASkinLightingManager::~ASkinLightingManager()
{
	AMaterialLibraryManager::List::GetSingleton()->DestroyLibrary( m_pSkinLightMatLib );
}

// $Barnyard: FUNCTION 0060dcc0
void ASkinLightingManager::ApplySkinLight( Toshi::TManagedModel* a_pModel, const TCHAR* a_szMaterialName, SkinLight* a_pLight )
{
	TModel* pModel = a_pModel->GetModel();

	if ( pModel->GetNumLODs() != 0 )
	{
		TModelLOD& lod = pModel->GetLOD( 0 );

		for ( TINT i = 0; i < lod.iNumMeshes; i++ )
		// Apply skin light for each mesh of the first LOD
		{
			if ( !lod.ppMeshes[ i ]->GetMaterial()->GetClass()->IsA( &TGetClass( ASkinMaterial ) ) )
				continue;

			// This is certainly a mesh with some skin material
			ASkinMaterial* pMaterial = TSTATICCAST( ASkinMaterial, lod.ppMeshes[ i ]->GetMaterial() );

			SkinLight* pLight = a_pLight;
			if ( !a_szMaterialName || TStringManager::String8CompareNoCase( a_szMaterialName, pMaterial->GetName() ) != 0 || !pLight )
			{
				pLight = FindSkinLight( pMaterial->GetName() );
			}

			TVALIDPTR( pLight );
			pMaterial->SetLightingTexture( ASkinMaterial::LT_0, pLight->pTex2 );
			pMaterial->SetLightingTexture( ASkinMaterial::LT_1, pLight->pTex1 );
			pMaterial->SetLightingTexture( ASkinMaterial::LT_2, pLight->pTex4 );
			pMaterial->SetLightingTexture( ASkinMaterial::LT_3, pLight->pTex3 );
			pMaterial->SetTextureNum( 5 );
		}
	}
}

// $Barnyard: FUNCTION 0060dc40
ASkinLightingManager::SkinLight* ASkinLightingManager::FindSkinLight( const TCHAR* a_szModelName )
{
	TVALIDPTR( m_pSkinLightHeader );

	for ( TINT i = 0; i < m_pSkinLightHeader->iNumLights; i++ )
	{
		SkinLight* pSkinLight = &m_pSkinLightHeader->pSkinLights[ i ];

		for ( TINT k = 0; k < pSkinLight->iNumModels; k++ )
		{
			if ( !TStringManager::String8CompareNoCase( pSkinLight->pszModels[ k ], a_szModelName ) )
				return pSkinLight;
		}
	}

	TASSERT( m_pSkinLightHeader->iNumLights != 0 );
	return &m_pSkinLightHeader->pSkinLights[ 0 ];
}
