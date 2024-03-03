#pragma once
#include "Render/ASkinShader/ASkinMaterial.h"

#include <Render/TOrderTable.h>
#include <Platform/DX8/TTextureResourceHAL_DX8.h>

class ASkinMaterialHAL : public ASkinMaterial
{
public:
	TDECLARE_CLASS(ASkinMaterial);

public:
	ASkinMaterialHAL();
	~ASkinMaterialHAL();

	//-----------------------------------------------------------------------------
	// Toshi::TMaterial
	//-----------------------------------------------------------------------------
	virtual void OnDestroy() override;
	virtual void PreRender() override;
	virtual void PostRender() override;

	//-----------------------------------------------------------------------------
	// ASkinMaterial
	//-----------------------------------------------------------------------------
	virtual TBOOL Create(BLENDMODE a_eBlendMode) override;
	virtual void SetBlendMode(BLENDMODE a_eBlendMode) override;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void CopyToAlphaBlendMaterial();

	void SetOrderTable(Toshi::TOrderTable* a_pOrderTable);

	ASkinMaterialHAL* GetAlphaBlendMaterial() const { return m_pAlphaBlendMaterial; }
	void SetAlphaBlendMaterial(ASkinMaterialHAL* val) { m_pAlphaBlendMaterial = val; }

	TBOOL IsSkin() const { return m_bIsSkin; }
	TBOOL HasLighting1Tex() const { return m_bHasLighting1Tex; }
	TBOOL HasLighting2Tex() const { return m_bHasLighting2Tex; }

	Toshi::TTextureResourceHAL* GetLightingTexture(ELightingTexture a_eTexture) const;
	Toshi::TTextureResourceHAL* GetSomeTexture() const;

	static ASkinMaterialHAL* Upcast(Toshi::TMaterial* a_pMaterial)
	{
		TASSERT(a_pMaterial->IsA(&ms_oClass));
		return TSTATICCAST(ASkinMaterialHAL*, a_pMaterial);
	}

private:
	ASkinMaterialHAL* m_pAlphaBlendMaterial;
	Toshi::TOrderTable* m_pAssignedOrderTable;
	TBOOL m_bIsSkin;
	TBOOL m_bHasLighting1Tex;
	TBOOL m_bHasLighting2Tex;
	Toshi::TTexture* m_pSomeTexture;
};