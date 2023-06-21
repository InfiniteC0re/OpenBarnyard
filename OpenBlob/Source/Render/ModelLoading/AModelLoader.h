#pragma once
#include <Toshi/File/TTRB.h>
#include <Toshi/Render/TModel.h>
#include <Toshi/Render/TTMDWin.h>
#include <Toshi/Render/TMaterial.h>
#include <Toshi/Render/TXSMaterial.h>
#include <Toshi2/T2DList.h>

class AModelLoader :
	public Toshi::TSingleton<AModelLoader>
{
public:
	struct MaterialNode : public Toshi::T2DList<MaterialNode>::Node
	{
		Toshi::TMaterial* m_pMaterial;
		char m_Name[128];
		int m_iReferenceCount;
		Toshi::TTRB* m_pTRB;
	};

	static constexpr size_t MAX_MATERIAL_COUNT = 512;

public:
	AModelLoader();

	void GetMaterial(Toshi::TShader* pShader, const char* pName, Toshi::TMaterial*& pOutMaterial, Toshi::TXSMaterial::TextureType eTextureType);

private:
	Toshi::TMaterial* GetCreatedMaterial(const Toshi::TString8& name, Toshi::TTRB* pTRB);

private:
	// Static data
	inline static MaterialNode m_oNodesAlloc[MAX_MATERIAL_COUNT];
	inline static Toshi::T2DList<MaterialNode> ms_oFreeMaterials;
	inline static Toshi::T2DList<MaterialNode> ms_oMaterialList;

private:
	// Callbacks
	static TBOOL LoadTRBCallback2(Toshi::TModel* pModel, Toshi::TTMDWin::TTRBWinHeader* pHeader);
	
	static TBOOL AModelLoaderLoadTRBCallback(Toshi::TTRB& pTRB, Toshi::TModel* pModel);
	static TBOOL AModelLoaderLoadTRBCallback2(Toshi::TModel* pModel, Toshi::TTMDWin::TTRBWinHeader* pHeader);
};
