#pragma once
#include <Render/TMesh.h>
#include <Render/TVertexPoolResourceInterface.h>
#include <Render/TIndexPoolResourceInterface.h>

class MODAPI AWorldMesh : public Toshi::TMesh
{
public:
	TDECLARE_CLASS( AWorldMesh, Toshi::TMesh );

	struct SubMesh
	{
		// $Barnyard: FUNCTION 005f5c20
		SubMesh() {}
		~SubMesh();

		TMesh*                              pAssociatedMesh = 0;
		TUINT16                             uiUnk2          = 0;
		TUINT16                             uiNumVertices   = 0;
		Toshi::TIndexPoolResourceInterface* pIndexPool      = TNULL;
	};

	static constexpr TUINT NUM_SUBMESHES = 1;
	using LockBuffer                     = Toshi::TVertexPoolResourceInterface::LockBuffer;

public:
	AWorldMesh();

	//-----------------------------------------------------------------------------
	// Toshi::TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Validate() override;
	virtual void  Invalidate() override;
	virtual TBOOL Render() override { return TTRUE; } // RETURNS_TRUE
	virtual void  OnDestroy() override;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void                                 Create( TUINT32 a_uiFlags, TUINT16 a_uiMaxVertices );
	virtual TBOOL                                Lock( LockBuffer& a_rLockBuffer );
	virtual void                                 Unlock( TUINT32 a_uiNumVertices );
	virtual Toshi::TVertexPoolResourceInterface* GetVertexPool();

	SubMesh* GetSubMesh( TUINT a_uiIndex )
	{
		TASSERT( a_uiIndex < NUM_SUBMESHES );
		return &m_pSubMeshes[ a_uiIndex ];
	}

private:
	TBOOL CreateResource();
	void  DestroyResource();

protected:
	TUINT32                              m_uiFlags;
	TUINT16                              m_uiMaxVertices;
	Toshi::TVertexPoolResourceInterface* m_pVertexPool;
	SubMesh*                             m_pSubMeshes;
};
