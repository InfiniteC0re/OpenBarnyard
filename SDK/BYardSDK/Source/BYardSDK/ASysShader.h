#pragma once
#include "THookedSingleton.h"

#include <Render/TShader.h>

class ASysMaterial;
class ASysMesh;

class ASysShader :
	public Toshi::TShader,
	public THookedSingleton<ASysShader, 0x0079a340>
{
public:
	virtual ASysMaterial* CreateMaterial(const char* a_szName) = 0;
	virtual ASysMesh* CreateMesh(const char* a_szName) = 0;
	virtual TBOOL IsLightScattering();
	virtual void SetLightScattering(TBOOL a_bEnabled);
};
