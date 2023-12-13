#pragma once
#include <Toshi/Render/TShader.h>

class ASysMaterial;
class ASysMesh;

TOBJECT(ASysShader, Toshi::TShader, TFALSE),
	public Toshi::TSingleton<ASysShader>
{
public:
	virtual ASysMaterial* CreateMaterial(const char* a_szName) = 0;
	virtual ASysMesh* CreateMesh(const char* a_szName) = 0;
	virtual TBOOL IsLightScattering();
	virtual void SetLightScattering(TBOOL a_bEnabled);
};
