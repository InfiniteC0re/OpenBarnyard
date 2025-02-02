#pragma once
#include <Toshi/TSingleton.h>
#include <ToshiTools/T2DynamicVector.h>

#include <Platform/GL/T2RenderBuffer_GL.h>

struct ARenderBuffer
{
	TINT iID;
	Toshi::T2VertexArray* pVertexArray;

	Toshi::T2VertexArray* operator->() const
	{
		TASSERT( iID != -1 );
		TASSERT( pVertexArray != TNULL );

		return pVertexArray;
	}
};

//-----------------------------------------------------------------------------
// Purpose: Stores all index/vertex buffers mapped by their unique IDs.
//-----------------------------------------------------------------------------
class ARenderBufferCollection
    : public Toshi::TSingleton<ARenderBufferCollection>
{
public:
	ARenderBufferCollection();
	~ARenderBufferCollection();

	void Create( TINT a_iMaxNumVertexArrayObjects );
	void Destroy();

	ARenderBuffer AllocateRenderBuffer();
	ARenderBuffer AllocateRenderBuffer( Toshi::T2VertexBuffer a_VertexBuffer, Toshi::T2IndexBuffer a_IndexBuffer );
	void          DestroyRenderBuffer( const ARenderBuffer& a_rcRenderBuffer );

	ARenderBuffer GetRenderBuffer( TINT a_iID ) const;

private:
	void InstallHooks();

private:
	TINT a_iMaxNumVertexArrayObjects;
	TBOOL m_bCreated;

	Toshi::T2DynamicVector<TINT> m_vecFreeVertexArrayObjects;
	Toshi::T2DynamicVector<TINT> m_vecUsedVertexArrayObjects;
	Toshi::T2VertexArray*        m_pVertexArrayObjects;
	TINT                         m_iNumFreeVAO;
	TINT                         m_iNumUsedVAO;

};
