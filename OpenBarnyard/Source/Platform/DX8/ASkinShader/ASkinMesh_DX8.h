#pragma once
#include "Render/ASkinShader/ASkinMesh.h"

class ASkinMeshHAL : public ASkinMesh
{
public:
	TDECLARE_CLASS( ASkinMeshHAL, ASkinMesh );

public:
	~ASkinMeshHAL();
	//-----------------------------------------------------------------------------
	// Toshi::TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Render() override;
};
