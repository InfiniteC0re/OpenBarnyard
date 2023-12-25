#pragma once
#include "ATextureUpdater.h"

class ABYTexturePacks : public AModInstance
{
public:
	TBOOL OnLoad() override;
	TBOOL OnUpdate(TFLOAT a_fDeltaTime) override;
	void OnUnload() override;
	void OnRenderInterfaceReady(Toshi::TRenderD3DInterface* a_pRenderInterface) override;
	void OnImGuiRender() override;
	TBOOL HasSettingsUI() override;
	const char* GetName() override;

	virtual void SetTexturePack(const char* a_szName);
	virtual void UpdateTexturePackPath();

private:
	ATextureUpdateWatcher* m_pTexWatcher = TNULL;
};