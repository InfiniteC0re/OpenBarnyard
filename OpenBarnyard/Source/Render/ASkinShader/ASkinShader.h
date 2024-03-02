#pragma once
#include <Render/TShader.h>
#include <Toshi/TSingleton.h>

class ASkinMaterial;
class ASkinMesh;

class ASkinShader :
	public Toshi::TShader,
	public Toshi::TSingleton<ASkinShader>
{
public:
	TDECLARE_CLASS(Toshi::TShader);

public:
	ASkinShader();
	~ASkinShader();

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
	virtual ASkinMaterial* CreateMaterial(const TCHAR* a_szName) = 0;
	virtual ASkinMesh* CreateMesh(const TCHAR* a_szName) = 0;
	virtual void Unknown1() = 0;
	virtual void Unknown2() = 0;
	virtual void Unknown3() = 0;
	virtual void Unknown4(void*);
	virtual TBOOL Unknown5();
	virtual TBOOL Unknown6();
	virtual void Unknown7(void*);
	virtual TBOOL Unknown8();

private:
	TINT m_Unk1;
	TINT m_Unk2;
	TFLOAT m_Unk3;
	TINT m_Unk4;
};