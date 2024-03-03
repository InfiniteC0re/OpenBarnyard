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
	virtual TINT AddLight(const Toshi::TVector3& a_rPosition, TFLOAT a_fIntensity) = 0;
	virtual void SetLight(TINT a_iIndex, const Toshi::TVector3& a_rPosition, TFLOAT a_fIntensity) = 0;
	virtual void RemoveLight(TINT a_iIndex) = 0;
	virtual void EnableClouds(TBOOL a_bEnabled);
	virtual TBOOL IsEnabledClouds();
	virtual TBOOL IsCapablePointLights();
	virtual void SetPointLights(TBOOL a_bEnabled);
	virtual TBOOL IsPointLightsEnabled();

protected:
	union {
		TUINT8 m_SomeColourComponents[8];
		TINT32 m_SomeColour;
	};
	TINT m_Unk2;
	TFLOAT m_Unk3;
	TINT m_Unk4;
};