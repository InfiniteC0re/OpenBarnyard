#pragma once
#include <Toshi/Render/TMesh.h>
#include <Toshi/Render/TResource.h>

class ASkinMesh :
	public Toshi::TGenericClassDerived<ASkinMesh, Toshi::TMesh, "ASkinMesh", TMAKEVERSION(1, 0), TFALSE>
{
public:
	typedef uint32_t FLAGS;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE = 0,
		FLAGS_LOCKED = 0x8000
	};

public:
	ASkinMesh();

	TBOOL Validate() override;
	void Invalidate() override;

private:
	uint32_t m_uiFlags;
	uint16_t m_uiMaxVertices;
	uint16_t m_uiNumSubMeshes;
	Toshi::TResource* m_pVertexPool;
};
