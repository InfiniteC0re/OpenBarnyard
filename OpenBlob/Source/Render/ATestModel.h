#pragma once
#include <Toshi/Utils/TSingleton.h>
#include <Toshi/Shaders/SysShader/TSysMaterial.h>
#include <Toshi/Render/TMaterial.h>
#include <Toshi/Render/TMesh.h>
#include <Toshi/File/TTRB.h>

class ATestModel :
	public Toshi::TSingleton<ATestModel>
{
public:
	ATestModel();

	void Render(float deltaTime);

	Toshi::TVector4& GetPosition() { return m_Position; }

private:
	void CreateCube();
	void CreatePlane();

private:
	Toshi::TTRB m_AssetTRB;
    Toshi::TSysMaterial* m_pCubeMaterial;
    Toshi::TSysMaterial* m_pPlaneMaterial;
	Toshi::TMesh* m_pCube;
	Toshi::TMesh* m_pPlane;
	Toshi::TVector4 m_Position;
	float m_CubeRotation = 0.0f;
};