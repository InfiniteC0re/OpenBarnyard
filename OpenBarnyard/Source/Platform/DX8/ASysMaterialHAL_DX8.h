#pragma once
#include "ASysShaderHAL_DX8.h"
#include "Render/Shader/ASysMaterial.h"

#include <Toshi/Render/TOrderTable.h>

TOBJECT(ASysMaterialHAL, ASysMaterial, TTRUE)
{
public:
	ASysMaterialHAL();
	~ASysMaterialHAL();

	virtual void PreRender() override;
	virtual void PostRender() override;
	virtual void SetBlendMode(BLENDMODE a_eBlendMode) override;

	void SetOrderTable(Toshi::TOrderTable* a_pOrderTable);

	ASysShaderHAL* GetSysShaderHAL() const
	{
		return TSTATICCAST(ASysShaderHAL*, GetShader());
	}

private:
	Toshi::TOrderTable* m_pAssignedOrderTable;
};
