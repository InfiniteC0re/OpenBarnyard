#include "pch.h"
#include "AModelLoader.h"
#include "AAssetLoader.h"
#include "AMaterialLibraryManager.h"

#ifdef TOSHI_SKU_WINDOWS
#  include "Platform/DX8/AWorldShader/AWorldShader_DX8.h"
#  include "Platform/DX8/ASkinShader/ASkinShader_DX8.h"
#  include "Platform/DX8/AGrassShader/AGrassShaderHAL_DX8.h"

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

// $Barnyard: FUNCTION 006116a0
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

// $Barnyard: FUNCTION 00611560
AModelLoader::~AModelLoader()
{
}

void AModelLoader::MaterialApplyFlags( TMaterial* a_pMaterial, const TCHAR* a_szMaterialName )
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

// $Barnyard: FUNCTION 00611440
void AModelLoader::MaterialApplyClamp( TMaterial* a_pMaterial, const TCHAR* a_szMaterialName, TTexture* a_pTexture )
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
				pTexture->SetAddressUState( ADDRESSINGMODE_CLAMP );

			if ( pClampedMaterial->eAddressVState > 0 )
				pTexture->SetAddressVState( ADDRESSINGMODE_CLAMP );
		}
	}
}

void AModelLoader::MaterialApplyAlphaRef( TMaterial* a_pMaterial, const TCHAR* a_szMaterialName, TTexture* a_pTexture )
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

// $Barnyard: FUNCTION 006119d0
TMaterial* AModelLoader::CreateMaterial( TShader* a_pShader, const TCHAR* a_szMaterialName )
{
	TPROFILER_SCOPE();

	char prefixName[ 32 ];

	if ( a_pShader->IsA( &TGetClass( ASkinShader ) ) )
	{
		TStringManager::String8Copy( prefixName, "ss_" );
	}
	else if ( a_pShader->IsA( &TGetClass( AWorldShader ) ) )
	{
		TStringManager::String8Copy( prefixName, "ws_" );
	}
	else if ( a_pShader->IsA( &TGetClass( AGrassShader ) ) )
	{
		TStringManager::String8Copy( prefixName, "gs_" );
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
		    AMaterialLibraryManager::GetSingleton()->FindTexture( pTMDMaterial->szTextureFile )
		);

		szTextureName = pTMDMaterial->szTextureFile;
	}
	else
	{
		TTRACE( "Couldn't find texture for material '%s'", a_szMaterialName );
		pTexture = TDYNAMICCAST(
		    TTextureResourceHAL,
		    TRenderD3DInterface::Interface()->GetInvalidTexture()
		);
		szTextureName = "invalid";
	}

	TVALIDPTR( pTexture );

	auto        eTextureFormat = pTexture->GetImageInfo().Format;
	const TBOOL bIsAlpha       = eTextureFormat == D3DFMT_A8R8G8B8 ||
	    eTextureFormat == D3DFMT_A4R4G4B4 ||
	    eTextureFormat == D3DFMT_A8R3G3B2 ||
	    eTextureFormat == D3DFMT_DXT5;

	if ( a_pShader->IsA( &TGetClass( ASkinShader ) ) )
	{
		ASkinShaderHAL* pShader   = TDYNAMICCAST( ASkinShaderHAL, a_pShader );
		ASkinMaterial*  pMaterial = pShader->CreateMaterial( TNULL );
		pMaterial->Create( bIsAlpha ? 1 : 0 );
		pMaterial->SetTexture( pTexture );

		pResultMaterial = pMaterial;
	}
	else if ( a_pShader->IsA( &TGetClass( AWorldShader ) ) )
	{
		AWorldShaderHAL* pShader   = TDYNAMICCAST( AWorldShaderHAL, a_pShader );
		AWorldMaterial*  pMaterial = pShader->CreateMaterial( TNULL );
		pMaterial->Create( bIsAlpha ? 1 : 0 );
		pMaterial->SetTexture( 0, pTexture );
		pMaterial->SetTextureNum( 1 );

		pResultMaterial = pMaterial;

		TTODO( "Setup material using properties" );
	}
	else if ( a_pShader->IsA( &TGetClass( AGrassShader ) ) )
	{
		AGrassShaderHAL* pShader   = TDYNAMICCAST( AGrassShaderHAL, a_pShader );
		AGrassMaterial*  pMaterial = pShader->CreateMaterial( TNULL );
		pMaterial->Create( 0 );
		pMaterial->SetTexture( 0, pTexture );
		pMaterial->SetTextureNum( 1 );

		pResultMaterial = pMaterial;
	}
	else
	{
		TASSERT( !"Support other shaders" );
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

// $Barnyard: FUNCTION 00611f50
void AModelLoader::DestroyMaterial( TMaterial* a_pMaterial )
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

static constexpr const TCHAR* TOSHICGROUP_NAMES[] = {
	"coll_support",
	"coll_player",
	"coll_raccoons",
	"coll_grass",
	"coll_tin",
	"coll_timber",
	"coll_stone",
	"coll_dirt",
	"coll_water",
	"coll_sand",
	"coll_mud",
	"coll_hay",
	"coll_wood",
	"coll",
	"coll_nofriction",
	"coll_liquid",
	"coll_waterwall",
	"coll_deepwater",
	"water",
	"collision_camera"
};

TSTATICASSERT( TARRAYSIZE( TOSHICGROUP_NAMES ) == TCollisionCommon::TOSHICGROUP_NUMOF );

// $Barnyard: FUNCTION 00616150
TCollisionCommon::TOSHICGROUP AModelLoader::GetCollisionGroup( const TCHAR* a_szCollGroupName )
{
	T2_FOREACH_ARRAY( TOSHICGROUP_NAMES, it )
	{
		if ( !TStringManager::String8Compare( a_szCollGroupName, TOSHICGROUP_NAMES[ it ] ) )
			return TCollisionCommon::TOSHICGROUP( it );
	}

#ifdef TOSHI_DEBUG
	if ( TStringManager::String8Compare( a_szCollGroupName, "default" ) != 0 )
		TERROR( "AModelLoader::GetCollisionGroup: Asked for an unknown collision type '%s'\n", a_szCollGroupName );
#endif

	return TCollisionCommon::TOSHICGROUP_DEFAULT;
}

// $Barnyard: FUNCTION 00613270
void AModelLoader::InitialiseStatic()
{
	InitialiseGrassLayersTextures();
}

void AModelLoader::AddMaterial( const TString8& a_rName, MaterialNode* a_pMaterialNode )
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

TMaterial* AModelLoader::FindMaterial( const TString8& a_rName )
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

TTMDBase::Material* AModelLoader::FindMaterialInModel( const TCHAR* a_szName )
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

// $Barnyard: FUNCTION 006528c0
TBOOL AModelLoader::AModelLoaderLoadTMDCallback( TModel* a_pModel )
{
	return TFALSE;
}

// $Barnyard: FUNCTION 006114d0
TBOOL AModelLoader::AModelLoaderLoadTRBCallback( TModel* a_pModel )
{
	TPROFILER_SCOPE();

	// Resolve all collision groups
	for ( TINT i = 0; i < a_pModel->GetNumCollisionMeshes(); i++ )
	{
		TModelCollisionMesh* pCollMesh = &a_pModel->GetCollisionMeshes()[ i ];
		TINT                 iNumCollTypes = pCollMesh->GetNumCollTypes();
		
		for ( TINT k = 0; k < iNumCollTypes; k++ )
		{
			TModelCollisionType& rCollType = pCollMesh->GetCollType( k );
			
			rCollType.eCollGroup = GetCollisionGroup( rCollType.strName );
		}
	}

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

		switch ( pTRBLod->m_eShader )
		{
			case TTMDWin::ST_WORLD:
				LoadWorldMeshTRB( a_pModel, i, &a_pModel->m_LODs[ i ], pTRBLod );
				break;
			case TTMDWin::ST_SKIN:
				LoadSkinLOD( a_pModel, i, &a_pModel->m_LODs[ i ], pTRBLod );
				break;
			case TTMDWin::ST_GRASS:
				LoadGrassMeshTRB( a_pModel, i, &a_pModel->m_LODs[ i ], pTRBLod );
				break;
			default:
				TASSERT( !"The model is using an unknown shader" );
				return TFALSE;
				break;
		}
	}

	return TTRUE;
}
