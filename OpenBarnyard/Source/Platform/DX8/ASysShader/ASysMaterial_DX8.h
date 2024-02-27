#pragma once
#include "ASysShader_DX8.h"
#include "Render/ASysShader/ASysMaterial.h"

#include <Render/TOrderTable.h>

class ASysMaterialHAL : public ASysMaterial
{
public:
	TDECLARE_CLASS(ASysMaterial);

public:
	ASysMaterialHAL();
	~ASysMaterialHAL();

	virtual void PreRender() override;
	virtual void PostRender() override;
	virtual TBOOL Create(BLENDMODE a_eBlendMode) override;
	virtual void SetBlendMode(BLENDMODE a_eBlendMode) override;

	void SetOrderTable(Toshi::TOrderTable* a_pOrderTable);

	ASysShaderHAL* GetSysShaderHAL() const
	{
		return TSTATICCAST(ASysShaderHAL*, GetShader());
	}

private:
	Toshi::TOrderTable* m_pAssignedOrderTable;
};
