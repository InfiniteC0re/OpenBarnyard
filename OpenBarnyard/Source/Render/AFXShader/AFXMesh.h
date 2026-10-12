#pragma once
#include "Render/ASkinShader/ASkinMesh.h"

#include <Render/TMesh.h>
#include <Render/TVertexPoolResourceInterface.h>

// NOTE: A copy of ASkinMesh in the original code, it uses the same ASkinSubMesh.
// Most of its methods are folded with the ASkinMesh/AWorldMesh ones.
class AFXMesh : public Toshi::TMesh
{
public:
	TDECLARE_CLASS( AFXMesh, Toshi::TMesh );

	using LockBuffer = Toshi::TVertexPoolResourceInterface::LockBuffer;

public:
	AFXMesh();
	~AFXMesh();

	//-----------------------------------------------------------------------------
	// Toshi::TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Validate() override;
	virtual void  Invalidate() override;
	virtual TBOOL Render() override;
	virtual void  OnDestroy() override;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL                                Create( TUINT32 a_uiFlags, TUINT16 a_uiMaxVertices, TUINT16 a_uiNumSubMeshes ); // +0x20
	virtual TBOOL                                Lock( LockBuffer& a_rLockBuffer );                                              // +0x24
	virtual void                                 Unlock( TUINT32 a_uiNumVertices );                                              // +0x28
	virtual Toshi::TVertexPoolResourceInterface* GetVertexPool();                                                                // +0x2C

	TUINT16       GetNumSubMeshes() const { return m_uiNumSubMeshes; }
	ASkinSubMesh* GetSubMesh( TUINT16 a_uiIndex )
	{
		TASSERT( a_uiIndex < m_uiNumSubMeshes );
		return &m_pSubMeshes[ a_uiIndex ];
	}

private:
	TBOOL CreateResource();
	void  DestroyResource();

protected:
	TUINT32                              m_uiFlags;        // 0x10
	TUINT16                              m_uiMaxVertices;  // 0x14
	TUINT16                              m_uiNumSubMeshes; // 0x16
	Toshi::TVertexPoolResourceInterface* m_pVertexPool;    // 0x18
	ASkinSubMesh*                        m_pSubMeshes;     // 0x1C
	void*                                m_Unk;            // 0x20
};
