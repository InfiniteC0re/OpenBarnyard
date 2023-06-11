#pragma once
#include <Toshi/Utils/TSingleton.h>
#include <Toshi/Shaders/SysShader/TSysMaterial.h>
#include <Toshi/Render/TCameraObject.h>
#include <Toshi/Render/TMaterial.h>
#include <Toshi/Render/TMesh.h>
#include <Toshi/File/TTRB.h>

class ATestModel :
	public Toshi::TSingleton<ATestModel>
{
public:
	ATestModel();

	void Render(float deltaTime);

private:
	Toshi::TTRB m_AssetTRB;
	Toshi::TCameraObject m_Camera;
	Toshi::TSysMaterial* m_pMaterial;
	Toshi::TMesh* m_pCube;
	float m_CubeRotation = 0.0f;
};