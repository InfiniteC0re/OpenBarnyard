#pragma once
#include "Toshi/Math/TVector3.h"

#include <cstdint>

struct Sphere_t
{
	Toshi::TVector3 Origin;
	float Radius;
};

class ModelHeader
{
public:
	enum MODELSHADER
	{
		MODELSHADER_SKIN = 0,
		MODELSHADER_WORLD = 2,
		MODELSHADER_GRASS = 4,
	};

public:
	uint32_t m_ModelCount;
	float m_LodDistance;
	uint32_t m_MeshCount;
	uint32_t m_Unknown;
	MODELSHADER m_Shader;
	Sphere_t m_RenderVolume;
};