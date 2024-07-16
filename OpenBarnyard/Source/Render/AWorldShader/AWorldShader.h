#pragma once
#include <Render/TShader.h>
#include <Toshi/TSingleton.h>

class AWorldMaterial;
class AWorldMesh;

class AWorldShader :
	public Toshi::TShader,
	public Toshi::TSingleton<AWorldShader>
{
public:
	TDECLARE_CLASS(AWorldShader, Toshi::TShader);

public:
	AWorldShader();
	~AWorldShader();

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void EnableRenderEnvMap(TBOOL a_bEnable);
	virtual TBOOL IsHighEndSkinning();
	virtual void EnableHighEndSkinning(TBOOL a_bEnable);
	virtual TBOOL IsCapableHighEndSkinning();
	virtual TBOOL IsLightScattering();
	virtual void SetLightScattering(TBOOL a_bEnable);
	virtual TBOOL IsAlphaBlendMaterial();
	virtual void SetAlphaBlendMaterial(TBOOL a_bIsAlphaBlendMaterial);
	virtual AWorldMaterial* CreateMaterial(const TCHAR* a_szName) = 0;
	virtual AWorldMesh* CreateMesh(const TCHAR* a_szName) = 0;

private:
	TINT m_iUnk1;
	TFLOAT m_fUnk2;
	TINT* m_pUnk3;
};