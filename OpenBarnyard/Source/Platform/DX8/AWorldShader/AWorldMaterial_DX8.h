#pragma once
#include "Render/AWorldShader/AWorldMaterial.h"

#include <Render/TOrderTable.h>

class AWorldMaterialHAL : public AWorldMaterial
{
public:
	TDECLARE_CLASS(AWorldMaterial);

public:
	AWorldMaterialHAL();
	~AWorldMaterialHAL();

	//-----------------------------------------------------------------------------
	// Toshi::TMaterial
	//-----------------------------------------------------------------------------
	virtual void OnDestroy() override;
	virtual void PreRender() override;
	virtual void PostRender() override;

	//-----------------------------------------------------------------------------
	// AWorldMaterial
	//-----------------------------------------------------------------------------
	virtual TBOOL Create(BLENDMODE a_eBlendMode) override;
	virtual void SetBlendMode(BLENDMODE a_eBlendMode) override;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void CopyToOtherMaterial();

	void SetOrderTable(Toshi::TOrderTable* a_pOrderTable);

private:
	void SetupRenderer();

private:
	AWorldMaterialHAL* m_pOtherMaterial;
	Toshi::TOrderTable* m_pAssignedOrderTable;
	TBOOL m_aHasUVOffsets[MAX_TEXTURES];
	TFLOAT m_aUVOffsetsX[MAX_TEXTURES];
	TFLOAT m_aUVOffsetsY[MAX_TEXTURES];
};