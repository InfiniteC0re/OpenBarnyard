#pragma once
#include "Render/AGrassShader/AGrassMesh.h"
#include "World/AWorld.h"

class AGrassMeshHAL
    : public AGrassMesh
{
public:
	TDECLARE_CLASS( AGrassMeshHAL, AGrassMesh );

	friend class AModelLoader;

public:
	AGrassMeshHAL();
	~AGrassMeshHAL();

	//-----------------------------------------------------------------------------
	// Toshi::TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Render() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own Methods
	//-----------------------------------------------------------------------------
	void SetUnknown( void* a_pUnk );

	void CreateHAL( TUINT32 a_eFlags, TUINT16 a_uiMaxVertices, CellMeshSphere* a_pCellMeshSphere );

private:
	void* m_pUnk1;
	CellMeshSphere* m_pCellMeshSphere;
};
