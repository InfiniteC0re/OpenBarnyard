#pragma once
#include "Render/AGrassShader/AGrassMesh.h"
#include "World/AWorld.h"

class MODAPI AGrassMeshHAL
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
	virtual void SetUnknown( void* a_pUnk ); // +0x30

	void CreateHAL( TUINT32 a_eFlags, TUINT16 a_uiMaxVertices, CellMeshSphere* a_pCellMeshSphere );

	CellMeshSphere* GetCellMeshSphere() const { return m_pCellMeshSphere; }

private:
	void*           m_pUnk1;
	CellMeshSphere* m_pCellMeshSphere;
};
