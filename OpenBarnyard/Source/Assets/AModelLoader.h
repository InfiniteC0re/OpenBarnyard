#pragma once
#include "Render/AWorldShader/AWorldMaterial.h"
#include "Render/ASkinShader/ASkinMaterial.h"
#include "Render/AStaticInstanceShader/AStaticInstanceMaterial.h"
#include "Assets/AMaterialLibrary.h"

#include <Toshi/T2DList.h>
#include <Render/TTMDBase.h>
#include <Render/TModel.h>
#include <Render/TTexture.h>
#include <Render/TCollisionCommon.h>
#include <File/TTRB.h>

#ifdef TOSHI_SKU_WINDOWS
#  include <Render/TTMDWin.h>
#  include <Platform/DX8/T2Texture_DX8.h>
#endif

class AModelLoader
    : public Toshi::TObject
    , public Toshi::TSingleton<AModelLoader>
{
public:
	TDECLARE_CLASS( AModelLoader, Toshi::TObject );

	struct MaterialNode : public Toshi::T2DList<MaterialNode>::Node
	{
		Toshi::TMaterial* pMaterial;
		TCHAR             szName[ 64 ];
		TUINT16           iNumRefs;
		TUINT16           iId;
		TCHAR             szTextureName[ 32 ];
	};

	struct ClampedMaterial
	{
		const TCHAR* szName;
		TUINT16      eAddressUState;
		TUINT16      eAddressVState;
	};

	struct AlphaRefMaterial
	{
		const TCHAR* szName;
		void*        Unknown;
	};

	static constexpr TUINT MAX_NUM_ALLOCATED_MATERIALS = 512;
	static constexpr TUINT MAX_NUM_MODEL_MATERIALS     = 150;

	friend class AModelInstance;

public:
	AModelLoader();
	~AModelLoader();

	static Toshi::TMaterial* CreateMaterial( Toshi::TShader* a_pShader, const TCHAR* a_szMaterialName );
	static void              DestroyMaterial( Toshi::TMaterial* a_pMaterial );

	static Toshi::TCollisionCommon::TOSHICGROUP GetCollisionGroup( const TCHAR* a_szCollGroupName );

	static void InitialiseStatic();

private:
	static void              AddMaterial( const Toshi::TString8& a_rName, MaterialNode* a_pMaterialNode );
	static Toshi::TMaterial* FindMaterial( const Toshi::TString8& a_rName );

	static Toshi::TTMDBase::Material* FindMaterialInModel( const TCHAR* a_szName );

	static void MaterialApplyFlags( Toshi::TMaterial* a_pMaterial, const TCHAR* a_szMaterialName );
	static void MaterialApplyClamp( Toshi::TMaterial* a_pMaterial, const TCHAR* a_szMaterialName, Toshi::TTexture* a_pTexture );
	static void MaterialApplyAlphaRef( Toshi::TMaterial* a_pMaterial, const TCHAR* a_szMaterialName, Toshi::TTexture* a_pTexture );

	static void LoadGrassMeshTRB( Toshi::TModel* a_pModel, TINT a_iLODIndex, Toshi::TModelLOD* a_pLOD, Toshi::TTMDWin::TRBLODHeader* a_pLODHeader );
	static void LoadWorldMeshTRB( Toshi::TModel* a_pModel, TINT a_iLODIndex, Toshi::TModelLOD* a_pLOD, Toshi::TTMDWin::TRBLODHeader* a_pLODHeader );
	static void LoadSkinLOD( Toshi::TModel* a_pModel, TINT a_iLODIndex, Toshi::TModelLOD* a_pLOD, Toshi::TTMDWin::TRBLODHeader* a_pLODHeader );

	static void InitialiseGrassLayersTextures();

private:
	static TBOOL AModelLoaderLoadTMDCallback( Toshi::TModel* a_pModel );
	static TBOOL AModelLoaderLoadTRBCallback( Toshi::TModel* a_pModel );

	inline static MaterialNode                 ms_oNodesAlloc[ MAX_NUM_ALLOCATED_MATERIALS ];
	inline static Toshi::T2DList<MaterialNode> ms_oFreeMaterials;
	inline static Toshi::T2DList<MaterialNode> ms_oUsedMaterials;

	inline static Toshi::TTMDBase::MaterialsHeader ms_oCurrentModelMaterialsHeader;
	inline static Toshi::TTMDBase::Material        ms_oCurrentModelMaterials[ MAX_NUM_MODEL_MATERIALS ];
	inline static TINT16                           ms_oMaterialIds[ MAX_NUM_ALLOCATED_MATERIALS ];
	inline static void*                            ms_pWorldEndRender;
	inline static void*                            ms_pWorldStartRender;
	inline static void*                            ms_pWorldUVOffsetsPerSec;
	inline static const TCHAR**                    ms_pGlowMaterials;
	inline static const TCHAR**                    ms_pDoubleSidedMaterials;
	inline static AlphaRefMaterial*                ms_pAlphaRefMaterials;
	inline static ClampedMaterial*                 ms_pClampedMaterials;
	inline static TINT                             ms_iNumGlowMaterials;
	inline static TINT                             ms_iNumDoubleSidedMaterials;
	inline static TINT                             ms_iNumCreatedMaterials;
	inline static ASkinMaterial*                   ms_pDefaultSkinMaterial;
	inline static AWorldMaterial*                  ms_pDefaultWorldMaterial;
	inline static AStaticInstanceMaterial*         ms_pDefaultStaticInstanceMaterial;

private:
	Toshi::TTRB m_oTRB;
};
