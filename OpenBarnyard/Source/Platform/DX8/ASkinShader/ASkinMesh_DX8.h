#pragma once
#include "Render/ASkinShader/ASkinMesh.h"

class ASkinMeshHAL : public ASkinMesh
{
public:
	TDECLARE_CLASS(ASkinMesh);

public:
	//-----------------------------------------------------------------------------
	// Toshi::TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Render() override;

	static ASkinMeshHAL* Upcast(Toshi::TMesh* a_pMesh)
	{
		TASSERT(a_pMesh->IsA(&ms_oClass));
		return TSTATICCAST(ASkinMeshHAL*, a_pMesh);
	}

};
