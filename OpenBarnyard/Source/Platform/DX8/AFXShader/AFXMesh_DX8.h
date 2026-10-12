#pragma once
#include "Render/AFXShader/AFXMesh.h"

class AFXMeshHAL : public AFXMesh
{
public:
	TDECLARE_CLASS( AFXMeshHAL, AFXMesh );

public:
	AFXMeshHAL();
	~AFXMeshHAL();

	//-----------------------------------------------------------------------------
	// Toshi::TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Render() override;
};
