#pragma once
#include "Render/Shader/ASysShader.h"

#include <Toshi/Render/TOrderTable.h>

TOBJECT(ASysShaderHAL, ASysShader, TTRUE)
{
public:
	static constexpr TUINT NUM_ORDER_TABLES = 4;

public:
	virtual void Flush() override;
	virtual void StartFlush() override;
	virtual void EndFlush() override;
	virtual TBOOL Create() override;
	virtual TBOOL Validate() override;
	virtual void Invalidate() override;
	virtual TBOOL TryInvalidate() override;
	virtual TBOOL TryValidate() override;
	virtual void Render(Toshi::TRenderPacket* pPacket) override;
	virtual ASysMaterial* CreateMaterial(const char* a_szName) override;
	virtual ASysMesh* CreateMesh(const char* a_szName) override;

	Toshi::TOrderTable* GetOrderTable(TUINT a_uiIndex)
	{
		TASSERT(a_uiIndex < NUM_ORDER_TABLES);
		return &m_aOrderTables[a_uiIndex];
	}

public:
	inline static TBOOL ms_bRenderEnabled = TTRUE;

private:
	Toshi::TOrderTable m_aOrderTables[NUM_ORDER_TABLES];
};