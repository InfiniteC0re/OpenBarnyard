#pragma once
#include <Render/TMesh.h>
#include <Render/TVertexPoolResourceInterface.h>
#include <Render/TIndexPoolResourceInterface.h>

class MODAPI AStaticInstanceMesh
    : public Toshi::TMesh
{
public:
	TDECLARE_CLASS( AStaticInstanceMesh, Toshi::TMesh );

	using LockBuffer = Toshi::TVertexPoolResourceInterface::LockBuffer;

	struct SubMesh
	{
		SubMesh();
		~SubMesh();

		Toshi::TMesh*                       pAssociatedMesh; // 0x00
		TUINT16                             uiUnk2;          // 0x04
		TUINT16                             uiNumVertices;   // 0x06
		Toshi::TIndexPoolResourceInterface* pIndexPool;      // 0x08
		TUINT8                              aUnk0C[ 0x18 ];  // 0x0c, object with an empty constructor
		TUINT32                             uiUnk24;         // 0x24
		TUINT8                              aUnk28[ 0x70 ];  // 0x28
	};

public:
	AStaticInstanceMesh();

	//-----------------------------------------------------------------------------
	// Toshi::TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Validate() OVERRIDE;   // +0x0c
	virtual void  Invalidate() OVERRIDE; // +0x10
	virtual void  OnDestroy() OVERRIDE;  // +0x1c

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL                                Create( TUINT32 a_uiFlags, TUINT16 a_uiMaxVertices, TUINT16 a_uiNumSubMeshes ); // +0x20
	virtual TBOOL                                Lock( LockBuffer& a_rLockBuffer );                                              // +0x24
	virtual void                                 Unlock( TUINT32 a_uiNumVertices );                                              // +0x28
	virtual Toshi::TVertexPoolResourceInterface* GetVertexPool();                                                                // +0x2c

	SubMesh* GetSubMesh( TUINT a_uiIndex )
	{
		TASSERT( a_uiIndex < m_uiNumSubMeshes );
		return &m_pSubMeshes[ a_uiIndex ];
	}

	TUINT16 GetNumSubMeshes() const { return m_uiNumSubMeshes; }

private:
	TBOOL CreateResource();
	void  DestroyResource();

protected:
	TUINT32                              m_uiFlags;        // 0x10
	TUINT16                              m_uiMaxVertices;  // 0x14
	TUINT16                              m_uiNumSubMeshes; // 0x16
	Toshi::TVertexPoolResourceInterface* m_pVertexPool;    // 0x18
	SubMesh*                             m_pSubMeshes;     // 0x1c
	TUINT32                              m_uiUnk20;        // 0x20
};

TSTATICASSERT( sizeof( AStaticInstanceMesh::SubMesh ) == 0x98 );
