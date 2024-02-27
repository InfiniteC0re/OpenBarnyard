#pragma once
#include "Render/AWorldShader/AWorldMesh.h"

class AWorldMeshHAL : public AWorldMesh
{
public:
	TDECLARE_CLASS(AWorldMesh);

public:
	//-----------------------------------------------------------------------------
	// Toshi::TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Render() override;

	static AWorldMeshHAL* Upcast(Toshi::TMesh* a_pMesh)
	{
		TASSERT(a_pMesh->IsA(&ms_oClass));
		return TSTATICCAST(AWorldMeshHAL*, a_pMesh);
	}

};
