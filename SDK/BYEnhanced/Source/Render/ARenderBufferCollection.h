#pragma once
#include <Toshi/TSingleton.h>
#include <ToshiTools/T2DynamicVector.h>

#include <Platform/GL/T2SharedRenderBuffer_GL.h>

struct ARenderBuffer
{
	struct Mesh
	{
		Toshi::T2VertexArray*                   pVAO;
		Toshi::T2SharedVertexBuffer::SubBuffer* pVertexBuffer;
		Toshi::T2SharedIndexBuffer::SubBuffer*  pIndexBuffer;
	};

	TINT  iID   = -1;
	Mesh* pMesh = TNULL;

	void Clear()
	{
		iID   = -1;
		pMesh = TNULL;
	}

	Mesh* operator->() const
	{
		TASSERT( iID != -1 );
		TASSERT( pMesh != TNULL );

		return pMesh;
	}

	TBOOL operator==( const ARenderBuffer& a_rcOther ) const
	{
		return a_rcOther.iID == iID && a_rcOther.pMesh == pMesh;
	}

	operator TBOOL() const { return pMesh != TNULL; }
};

//-----------------------------------------------------------------------------
// Purpose: Stores all index/vertex buffers mapped by their unique IDs.
//-----------------------------------------------------------------------------
class ARenderBufferCollection
    : public Toshi::TSingleton<ARenderBufferCollection>
{
public:
	

public:
	ARenderBufferCollection();
	~ARenderBufferCollection();

	void Create( TINT a_iMaxNumVertexArrayObjects );
	void Destroy();

	ARenderBuffer AllocateRenderBuffer( Toshi::T2SharedVertexBuffer::SubBuffer* a_pVertexBuffer, Toshi::T2SharedIndexBuffer::SubBuffer* a_pIndexBuffer );
	void          DestroyRenderBuffer( const ARenderBuffer& a_rcRenderBuffer );

	ARenderBuffer GetRenderBuffer( TINT a_iID ) const;

private:
	void InstallHooks();

private:
	TINT a_iMaxNumVertexArrayObjects;
	TBOOL m_bCreated;

	Toshi::T2DynamicVector<TINT> m_vecFreeVertexArrayObjects;
	Toshi::T2DynamicVector<TINT> m_vecUsedVertexArrayObjects;
	ARenderBuffer::Mesh*         m_pMeshes;
	TINT                         m_iNumFreeMeshes;
	TINT                         m_iNumUsedMeshes;
};
