#pragma once
#include "AHookedSingleton.h"

#include <Toshi/Render/TShader.h>

class ASysMaterial;
class ASysMesh;

TOBJECT(ASysShader, Toshi::TShader, TFALSE),
	public AHookedSingleton<ASysShader, 0x0079a340>
{
public:
	virtual ASysMaterial* CreateMaterial(const char* a_szName) = 0;
	virtual ASysMesh* CreateMesh(const char* a_szName) = 0;
	virtual TBOOL IsLightScattering();
	virtual void SetLightScattering(TBOOL a_bEnabled);
};
