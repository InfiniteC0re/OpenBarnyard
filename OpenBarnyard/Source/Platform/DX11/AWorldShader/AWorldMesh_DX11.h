#pragma once
#include "Render/AWorldShader/AWorldMesh.h"

class MODAPI AWorldMeshHAL : public AWorldMesh
{
public:
	TDECLARE_CLASS( AWorldMeshHAL, AWorldMesh );

public:
	//-----------------------------------------------------------------------------
	// Toshi::TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Render() override;
};
