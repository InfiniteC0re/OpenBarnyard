#include "pch.h"
#include "AModelLoader.h"
#include "AAssetLoader.h"
#include "AMaterialLibraryManager.h"

#ifdef TOSHI_SKU_WINDOWS
#  include "Platform/DX8/AWorldShader/AWorldShader_DX8.h"
#  include "Platform/DX8/ASkinShader/ASkinShader_DX8.h"

#  include <Render/TTMDWin.h>
#  include <Platform/DX8/TRenderInterface_DX8.h>
#  include <Platform/DX8/TTextureResourceHAL_DX8.h>
#endif // TOSHI_SKU_WINDOWS


//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TDEFINE_CLASS( AModelLoader );

TOSHI_NAMESPACE_USING

AModelLoader::AModelLoader()
{
	for ( TUINT i = 0; i < TARRAYSIZE( ms_oNodesAlloc ); i++ )
	{
		ms_oFreeMaterials.PushBack( &ms_oNodesAlloc[ i ] );
		ms_oNodesAlloc[ i ].iId = TUINT16( i );
	}

	// Load material properties
	const TBOOL bAssetPackLoaded = AAssetLoader::GetAssetTRB( AAssetType_AssetPack ) != TNULL;

	if ( bAssetPackLoaded )
	{
		// Use asset pack
		ms_pWorldEndRender       = AAssetLoader::GetSymbolAddress( "materialproperties", "WorldEndRender", AAssetType_AssetPack );
		ms_pWorldStartRender     = AAssetLoader::GetSymbolAddress( "materialproperties", "WorldStartRender", AAssetType_AssetPack );
		ms_pWorldUVOffsetsPerSec = AAssetLoader::GetSymbolAddress( "materialproperties", "WorldUVOffsetsPerSec", AAssetType_AssetPack );
		ms_pAlphaRefMaterials    = AAssetLoader::CastSymbol<AlphaRefMaterial>( "materialproperties", "AlphaRefMaterials", AAssetType_AssetPack );
		ms_pClampedMaterials     = AAssetLoader::CastSymbol<ClampedMaterial>( "materialproperties", "ClampedMaterials", AAssetType_AssetPack );
		ms_pGlowMaterials        = AAssetLoader::CastSymbol<const TCHAR*>( "materialproperties", "GlowMaterials", AAssetType_AssetPack );
	}
	else
	{
		// Load from file
		m_oTRB.Load( "Data\\MiscTextures\\MaterialProperties.trb" );
		ms_pWorldEndRender       = m_oTRB.GetSymbolAddress( "WorldEndRender" );
		ms_pWorldStartRender     = m_oTRB.GetSymbolAddress( "WorldStartRender" );
		ms_pWorldUVOffsetsPerSec = m_oTRB.GetSymbolAddress( "WorldUVOffsetsPerSec" );
		ms_pAlphaRefMaterials    = m_oTRB.CastSymbol<AlphaRefMaterial>( "AlphaRefMaterials" );
		ms_pClampedMaterials     = m_oTRB.CastSymbol<ClampedMaterial>( "ClampedMaterials" );
		ms_pGlowMaterials        = m_oTRB.CastSymbol<const TCHAR*>( "GlowMaterials" );
	}

	TModel::SetLoaderTMDCallback( AModelLoaderLoadTMDCallback );
	TModel::SetLoaderTRBCallback( AModelLoaderLoadTRBCallback );

	// Count glow materials
	ms_iNumGlowMaterials = 0;
	auto pGlowMaterial   = ms_pGlowMaterials;

	while ( *pGlowMaterial )
	{
		ms_iNumGlowMaterials += 1;
		pGlowMaterial++;
	}

	// Load double sided materials
	ms_pDoubleSidedMaterials = ( bAssetPackLoaded ) ?
	    AAssetLoader::CastSymbol<const TCHAR*>( "materialproperties", "DoubleSidedMaterials", AAssetType_AssetPack ) :
	    m_oTRB.CastSymbol<const TCHAR*>( "DoubleSidedMaterials" );

	// Count double sided materials
	ms_iNumDoubleSidedMaterials = 0;
	auto pDoubleSidedMaterial   = ms_pDoubleSidedMaterials;

	while ( *pDoubleSidedMaterial )
	{
		ms_iNumDoubleSidedMaterials += 1;
		pDoubleSidedMaterial++;
	}

	// Symbol table is not needed anymore
	if ( !bAssetPackLoaded )
		m_oTRB.DeleteSymbolTable();

	if ( TNULL == ms_pDefaultSkinMaterial )
	{
		ASkinMaterial* pSkinMaterial = ASkinShader::GetSingleton()->CreateMaterial( TNULL );
		pSkinMaterial->Create( 7 );
		pSkinMaterial->SetFlags( TMaterial::FLAGS_NO_CULL, TFALSE );
		pSkinMaterial->SetTexture( TNULL );
		
		ms_pDefaultSkinMaterial = pSkinMaterial;
	}

	if ( TNULL == ms_pDefaultWorldMaterial )
	{
		AWorldMaterial* pWorldMaterial = AWorldShader::GetSingleton()->CreateMaterial( TNULL );
		pWorldMaterial->Create( 6 );
		pWorldMaterial->SetFlags( TMaterial::FLAGS_NO_CULL, TFALSE );
		pWorldMaterial->SetTexture( 0, TNULL );
		pWorldMaterial->SetTextureNum( 1 );

		ms_pDefaultWorldMaterial = pWorldMaterial;
	}
}

 AModelLoader::~AModelLoader()
{
}

void AModelLoader::MaterialApplyFlags( Toshi::TMaterial* a_pMaterial, const TCHAR* a_szMaterialName )
{
	// Check if material is glowing
	for ( TINT i = 0; i < ms_iNumGlowMaterials; i++ )
	{
		if ( TStringManager::String8Compare( ms_pGlowMaterials[ i ], a_szMaterialName ) == 0 )
		{
			a_pMaterial->SetFlags( TMaterial::FLAGS_GLOW, TTRUE );
			break;
		}
	}

	// Check if material is double sided
	for ( TINT i = 0; i < ms_iNumDoubleSidedMaterials; i++ )
	{
		if ( TStringManager::String8Compare( ms_pDoubleSidedMaterials[ i ], a_szMaterialName ) == 0 )
		{
			a_pMaterial->SetFlags( TMaterial::FLAGS_NO_CULL, TTRUE );
			break;
		}
	}
}

void AModelLoader::MaterialApplyClamp( Toshi::TMaterial* a_pMaterial, const TCHAR* a_szMaterialName, Toshi::TTexture* a_pTexture )
{
	auto pClampedMaterial = ms_pClampedMaterials;

	for ( ;; )
	{
		if ( pClampedMaterial->szName == TNULL ) return;
		if ( TStringManager::String8Compare( pClampedMaterial->szName, a_szMaterialName ) == 0 ) break;

		pClampedMaterial += 1;
	}

	for ( TUINT i = 0; i < a_pMaterial->GetTextureNum(); i++ )
	{
		if ( a_pTexture )
		{
			auto pTexture = TDYNAMICCAST( TTextureResourceHAL, a_pTexture );

			if ( pClampedMaterial->eAddressUState > 0 )
				pTexture->SetAddressUState( 2 );

			if ( pClampedMaterial->eAddressVState > 0 )
				pTexture->SetAddressVState( 2 );
		}
	}
}

void AModelLoader::MaterialApplyAlphaRef( Toshi::TMaterial* a_pMaterial, const TCHAR* a_szMaterialName, Toshi::TTexture* a_pTexture )
{
	auto pAlphaRefMaterial = ms_pAlphaRefMaterials;

	for ( ;; )
	{
		if ( pAlphaRefMaterial->szName == TNULL ) return;
		if ( TStringManager::String8Compare( pAlphaRefMaterial->szName, a_szMaterialName ) == 0 ) break;

		pAlphaRefMaterial += 1;
	}

	// Does nothing with the material in the original game
}

Toshi::TMaterial* AModelLoader::CreateMaterial( Toshi::TShader* a_pShader, const TCHAR* a_szMaterialName )
{
	TPROFILER_SCOPE();

	char prefixName[ 32 ];

	if ( a_pShader->IsA( &TGetClass( AWorldShader ) ) )
	{
		TStringManager::String8Copy( prefixName, "ws_" );
	}
	else
	{
		TASSERT( !"Unknown shader" );
		return TNULL;
	}

	TString8 materialName = prefixName;
	materialName += a_szMaterialName;

	auto pResultMaterial = FindMaterial( materialName );
	if ( pResultMaterial ) return pResultMaterial;

	TTextureResourceHAL* pTexture;
	const char*          szTextureName;
	auto                 pTMDMaterial = FindMaterialInModel( a_szMaterialName );

	if ( pTMDMaterial )
	{
		pTexture = TDYNAMICCAST(
		    TTextureResourceHAL,
		    AMaterialLibraryManager::GetSingleton()->FindTexture( pTMDMaterial->szTextureFile ) );

		szTextureName = pTMDMaterial->szTextureFile;
	}
	else
	{
		TTRACE( "Couldn't find texture for material '%s'", a_szMaterialName );
		pTexture = TDYNAMICCAST(
		    TTextureResourceHAL,
		    TRenderD3DInterface::Interface()->GetInvalidTexture() );
		szTextureName = "invalid";
	}

	TVALIDPTR( pTexture );

	auto        eTextureFormat = pTexture->GetImageInfo().Format;
	const TBOOL bIsAlpha       = eTextureFormat == D3DFMT_A8R8G8B8 ||
	    eTextureFormat == D3DFMT_A4R4G4B4 ||
	    eTextureFormat == D3DFMT_A8R3G3B2 ||
	    eTextureFormat == D3DFMT_DXT5;

	if ( a_pShader->IsA( &TGetClass( AWorldShader ) ) )
	{
		auto pShader   = TDYNAMICCAST( AWorldShaderHAL, a_pShader );
		auto pMaterial = pShader->CreateMaterial( TNULL );
		pMaterial->Create( bIsAlpha ? 1 : 0 );
		pMaterial->SetTexture( 0, pTexture );
		pMaterial->SetTextureNum( 1 );

		pResultMaterial = pMaterial;

		TTODO( "Setup material using properties" );
	}
	else
	{
		TTODO( "Support other shaders" );
	}

	MaterialApplyFlags( pResultMaterial, a_szMaterialName );
	MaterialApplyClamp( pResultMaterial, a_szMaterialName, pTexture );
	MaterialApplyAlphaRef( pResultMaterial, a_szMaterialName, pTexture );

	TASSERT( !ms_oFreeMaterials.IsEmpty() );
	auto pMatNode       = ms_oFreeMaterials.PopFront();
	pMatNode->pMaterial = pResultMaterial;
	pMatNode->iNumRefs  = 1;

	TStringManager::String8Copy( pMatNode->szTextureName, szTextureName, TARRAYSIZE( pMatNode->szTextureName ) );
	TStringManager::String8Copy( pMatNode->szName, materialName, TARRAYSIZE( pMatNode->szName ) );

	// Truncate material name so it fits the limit
	if ( materialName.Length() > TMaterial::NAMESIZELIMIT )
	{
		materialName[ TMaterial::NAMESIZELIMIT ] = '\0';
	}

	pResultMaterial->SetName( materialName );
	ms_oUsedMaterials.PushFront( pMatNode );

	AddMaterial( materialName, pMatNode );
	return pResultMaterial;
}

void AModelLoader::DestroyMaterial( Toshi::TMaterial* a_pMaterial )
{
	TPROFILER_SCOPE();

	TINT   iId = 0;
	TINT16 iIndex;

	if ( 0 < ms_iNumCreatedMaterials )
	{
		while ( true )
		{
			iIndex = ms_oMaterialIds[ iId ];
			if ( a_pMaterial == ms_oNodesAlloc[ iIndex ].pMaterial ) break;

			if ( ms_iNumCreatedMaterials <= ++iId )
			{
				TASSERT( !"Trying to destroy some unknown material" );
				return;
			}
		}

		ms_oNodesAlloc[ iIndex ].iNumRefs -= 1;

		if ( ms_oNodesAlloc[ iIndex ].iNumRefs < 1 )
		{
			ms_oNodesAlloc[ iIndex ].Remove();
			ms_oNodesAlloc[ iIndex ].pMaterial->Destroy();
			ms_oNodesAlloc[ iIndex ].pMaterial = TNULL;

			ms_oFreeMaterials.PushFront( &ms_oNodesAlloc[ iIndex ] );
			ms_iNumCreatedMaterials -= 1;

			if ( iId < ms_iNumCreatedMaterials )
			{
				TINT16* pNext = ms_oMaterialIds + iId + 1;
				TINT16* pCurr = ms_oMaterialIds + iId;

				for ( TINT i = ms_iNumCreatedMaterials - iId; i != 0; i-- )
				{
					*( pCurr++ ) = *( pNext++ );
				}
			}
		}
	}
}

void AModelLoader::AddMaterial( const Toshi::TString8& a_rName, MaterialNode* a_pMaterialNode )
{
	TPROFILER_SCOPE();

	TINT iMaxIndex      = ms_iNumCreatedMaterials;
	TINT iMaterialIndex = ms_iNumCreatedMaterials / 2;

	TINT iOffset    = 0;
	TINT iCmpResult = 0;

	while ( iOffset <= iMaterialIndex )
	{
		if ( iMaxIndex <= iMaterialIndex ) break;

		auto pMatNode = &ms_oNodesAlloc[ ms_oMaterialIds[ iMaterialIndex ] ];
		iCmpResult    = TStringManager::String8Compare( a_rName, pMatNode->szName );

		if ( -1 < iCmpResult )
		{
			iOffset        = iMaterialIndex + 1;
			iMaterialIndex = iMaxIndex;
		}

		iMaxIndex      = iMaterialIndex;
		iMaterialIndex = iOffset + ( iMaterialIndex - iOffset ) / 2;
	}

	if ( iCmpResult < 1 && ( iOffset = iMaterialIndex, iCmpResult < 0 ) )
	{
		iOffset = iMaxIndex;
	}

	for ( TINT i = ms_iNumCreatedMaterials; iOffset < i; i-- )
	{
		ms_oMaterialIds[ i ] = ms_oMaterialIds[ i - 1 ];
	}

	ms_oMaterialIds[ iOffset ] = a_pMaterialNode->iId;
	ms_iNumCreatedMaterials += 1;
}

Toshi::TMaterial* AModelLoader::FindMaterial( const Toshi::TString8& a_rName )
{
	TPROFILER_SCOPE();

	TINT iMaxIndex      = ms_iNumCreatedMaterials;
	TINT iMaterialIndex = ms_iNumCreatedMaterials / 2;

	TINT iOffset = 0;

	while ( iOffset <= iMaterialIndex )
	{
		if ( iMaxIndex <= iMaterialIndex ) break;

		auto pMatNode   = &ms_oNodesAlloc[ ms_oMaterialIds[ iMaterialIndex ] ];
		TINT iCmpResult = TStringManager::String8Compare( a_rName, pMatNode->szName );

		if ( iCmpResult == 0 )
		{
			pMatNode->iNumRefs++;

			if ( pMatNode->pMaterial )
				return pMatNode->pMaterial;

			break;
		}

		if ( iCmpResult > -1 )
		{
			iOffset        = iMaterialIndex + 1;
			iMaterialIndex = iMaxIndex;
		}

		iMaxIndex      = iMaterialIndex;
		iMaterialIndex = iOffset + ( iMaterialIndex - iOffset ) / 2;
	}

	return TNULL;
}

Toshi::TTMDBase::Material* AModelLoader::FindMaterialInModel( const TCHAR* a_szName )
{
	for ( TINT i = 0; i < ms_oCurrentModelMaterialsHeader.iNumMaterials; i++ )
	{
		if ( TStringManager::String8Compare( ms_oCurrentModelMaterials[ i ].szMatName, a_szName ) == 0 )
		{
			return &ms_oCurrentModelMaterials[ i ];
		}
	}

	return TNULL;
}

TBOOL AModelLoader::AModelLoaderLoadTMDCallback( Toshi::TModel* a_pModel )
{
	return TFALSE;
}

TBOOL AModelLoader::AModelLoaderLoadTRBCallback( Toshi::TModel* a_pModel )
{
	TPROFILER_SCOPE();

	TTODO( "Load collision data" );

	TTMDWin::TRBWinHeader*     pHeader    = a_pModel->CastSymbol<TTMDWin::TRBWinHeader>( "Header" );
	TTMDBase::MaterialsHeader* pMaterials = a_pModel->CastSymbol<TTMDBase::MaterialsHeader>( "Materials" );

	// Copy info about materials of the current model
	TUtil::MemCopy( ms_oCurrentModelMaterials, pMaterials + 1, pMaterials->uiSectionSize );
	ms_oCurrentModelMaterialsHeader = *pMaterials;

	a_pModel->m_iLODCount    = pHeader->m_iNumLODs;
	a_pModel->m_fLODDistance = pHeader->m_fLODDistance;

	for ( TINT i = 0; i < pHeader->m_iNumLODs; i++ )
	{
		auto pTRBLod = pHeader->GetLOD( i );

		a_pModel->m_LODs[ i ].iNumMeshes     = pTRBLod->m_iMeshCount1 + pTRBLod->m_iMeshCount2;
		a_pModel->m_LODs[ i ].ppMeshes       = new TMesh*[ a_pModel->m_LODs[ i ].iNumMeshes ];
		a_pModel->m_LODs[ i ].BoundingSphere = pTRBLod->m_RenderVolume;

		if ( pTRBLod->m_eShader == TTMDWin::ST_WORLD )
		{
		LoadAsWorldModel:
			LoadWorldMeshTRB( a_pModel, i, &a_pModel->m_LODs[ i ] );
			continue;
		}
		else if ( pTRBLod->m_eShader == TTMDWin::ST_SKIN )
		{
			LoadSkinLOD( a_pModel, i, &a_pModel->m_LODs[ i ], pTRBLod );
			continue;
		}
		else if ( pTRBLod->m_eShader == TTMDWin::ST_GRASS )
		{
			TTRACE( "Loading grass models is not yet implemented, loading as world model\n" );
			goto LoadAsWorldModel;
		}

		TASSERT( !"The model is using an unknown shader" );
		return TFALSE;
	}

	return TTRUE;
}
