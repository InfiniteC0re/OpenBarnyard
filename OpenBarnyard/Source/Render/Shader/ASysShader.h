#pragma once
#include <Toshi/Render/TShader.h>

class ASysMaterial;
class ASysMesh;

class ASysShader :
	public Toshi::TShader,
	public Toshi::TSingleton<ASysShader>
{
public:
	TDECLARE_CLASS(Toshi::TShader);

public:
	virtual ASysMaterial* CreateMaterial(const char* a_szName) = 0;
	virtual ASysMesh* CreateMesh(const char* a_szName) = 0;
	virtual TBOOL IsLightScattering();
	virtual void SetLightScattering(TBOOL a_bEnabled);
};
