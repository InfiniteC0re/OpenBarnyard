#pragma once
#include "Render/AWorldShader/AWorldMesh.h"

#include <Toshi/Toshi.h>
#include <Render/TMesh.h>
#include <Render/TVertexPoolResourceInterface.h>
#include <Render/TIndexPoolResourceInterface.h>

class AGrassMesh
    : public Toshi::TMesh
{
public:
	TDECLARE_CLASS( AGrassMesh, Toshi::TMesh );

	using LockBuffer = Toshi::TVertexPoolResourceInterface::LockBuffer;

	friend class AModelLoader;

public:
	AGrassMesh();
	~AGrassMesh();

	//-----------------------------------------------------------------------------
	// Toshi::TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Validate() OVERRIDE;
	virtual void  Invalidate() OVERRIDE;
	virtual void  OnDestroy() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL                                Create( TUINT32 a_uiFlags, TUINT16 a_uiMaxVertices );
	virtual TBOOL                                Lock( LockBuffer& a_rLockBuffer );
	virtual void                                 Unlock( TUINT32 a_uiNumVertices );
	virtual Toshi::TVertexPoolResourceInterface* GetVertexPool();

	AWorldMesh::SubMesh* GetSubMesh( TUINT a_uiIndex )
	{
		TASSERT( a_uiIndex < AWorldMesh::NUM_SUBMESHES );
		return &m_aSubMeshes[ a_uiIndex ];
	}

private:
	TUINT32                              m_uiFlags;
	TUINT16                              m_uiMaxVertices;
	Toshi::TVertexPoolResourceInterface* m_pVertexPool;

	AWorldMesh::SubMesh m_aSubMeshes[ AWorldMesh::NUM_SUBMESHES ];
	// ...
	TUINT32 m_Unk5;
	// ...
};
