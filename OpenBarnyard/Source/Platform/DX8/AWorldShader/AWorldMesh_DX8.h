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

};
