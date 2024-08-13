#include "pch.h"
#include "ASysMesh.h"
#include "SDKHooks.h"
#include "Render/TRenderInterface.h"

TOSHI_NAMESPACE_USING

ASysMesh::ASysMesh()
{
}

TBOOL ASysMesh::Validate()
{
	return CALL_THIS( 0x005f03e0, ASysMesh*, TBOOL, this );
}

void ASysMesh::Invalidate()
{
	CALL_THIS( 0x005f0420, ASysMesh*, TBOOL, this );
}

TBOOL ASysMesh::Create( void* a_pUnk, const char* a_szSomeString )
{
	return CALL_THIS( 0x005f0180, ASysMesh*, TBOOL, this, void*, a_pUnk, const char*, a_szSomeString );
}

TBOOL ASysMesh::CreatePools( TUINT32 a_uiResourcesFlags, TUINT16 a_uiMaxVertices, TUINT16 a_uiMaxIndices )
{
	return CALL_THIS( 0x005f0450, ASysMesh*, TBOOL, this, TUINT32, a_uiResourcesFlags, TUINT16, a_uiMaxVertices, TUINT16, a_uiMaxIndices );
}

TBOOL ASysMesh::Lock( LockBuffer& a_rLockBuffer )
{
	return CALL_THIS( 0x005f02b0, ASysMesh*, TBOOL, this, LockBuffer&, a_rLockBuffer );
}

void ASysMesh::Unlock( TUINT32 a_uiNumVertices, TUINT32 a_uiNumIndices )
{
	CALL_THIS( 0x005f02f0, ASysMesh*, void, this, TUINT32, a_uiNumVertices, TUINT32, a_uiNumIndices );
}

Toshi::TVertexPoolResourceInterface* ASysMesh::GetVertexPool()
{
	return m_pVertexPool;
}

Toshi::TIndexPoolResourceInterface* ASysMesh::GetIndexPool()
{
	return m_pIndexPool;
}
