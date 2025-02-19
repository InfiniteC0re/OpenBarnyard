#include "pch.h"
#include "ARenderBufferCollection.h"
#include "AEnhancedWorldShader.h"
#include "AEnhancedSkinShader.h"
#include "HookHelpers.h"

#include <Render/TTMDWin.h>

#include <Platform/GL/T2Render_GL.h>
#include <Platform/GL/T2SharedRenderBuffer_GL.h>
#include <Platform/DX8/TVertexPoolResource_DX8.h>
#include <Platform/DX8/TIndexPoolResource_DX8.h>
#include <Platform/DX8/TModel_DX8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

T2SharedVertexBuffer g_oWorldVertexBuffer;
T2SharedVertexBuffer g_oSkinVertexBuffer;
T2SharedVertexBuffer g_oSysVertexBuffer;
T2SharedIndexBuffer  g_oSharedIndexBuffer;

T2VertexArray g_oWorldVAO;
T2VertexArray g_oSkinVAO;

ARenderBufferCollection::ARenderBufferCollection()
    : a_iMaxNumVertexArrayObjects( 0 )
    , m_bCreated( TFALSE )
    , m_pMeshes( TNULL )
    , m_iNumFreeMeshes( -1 )
    , m_iNumUsedMeshes( -1 )
{
	T2VertexArray::Unbind();
	g_oWorldVertexBuffer.Create( T2Render::CreateVertexBuffer( TNULL, 0, GL_DYNAMIC_DRAW ), 16 * 1024 * 1024 );
	g_oSkinVertexBuffer.Create( T2Render::CreateVertexBuffer( TNULL, 0, GL_DYNAMIC_DRAW ), 16 * 1024 * 1024 );
	g_oSysVertexBuffer.Create( T2Render::CreateVertexBuffer( TNULL, 0, GL_DYNAMIC_DRAW ), 16 * 1024 * 1024 );
	g_oSharedIndexBuffer.Create( T2Render::CreateIndexBuffer( TNULL, 0, GL_DYNAMIC_DRAW ), 16 * 1024 * 1024 );

	struct WorldVertex
	{
		TVector3 Position;
		TVector3 Normal;
		TVector3 Color;
		TVector2 UV;
	};

	// Initialise world VAO
	g_oWorldVAO = T2Render::CreateVertexArray( g_oWorldVertexBuffer.GetRenderBuffer(), g_oSharedIndexBuffer.GetRenderBuffer() );
	g_oWorldVAO.Bind();
	g_oWorldVAO.GetVertexBuffer().Bind();
	g_oWorldVAO.GetVertexBuffer().SetAttribPointer( 0, 3, GL_FLOAT, sizeof( WorldVertex ), offsetof( WorldVertex, Position ) );
	g_oWorldVAO.GetVertexBuffer().SetAttribPointer( 1, 3, GL_FLOAT, sizeof( WorldVertex ), offsetof( WorldVertex, Normal ) );
	g_oWorldVAO.GetVertexBuffer().SetAttribPointer( 2, 3, GL_FLOAT, sizeof( WorldVertex ), offsetof( WorldVertex, Color ) );
	g_oWorldVAO.GetVertexBuffer().SetAttribPointer( 3, 2, GL_FLOAT, sizeof( WorldVertex ), offsetof( WorldVertex, UV ) );
	AEnhancedRenderer::GetSingleton()->GetIndirectBuffer().Bind();
	AEnhancedRenderer::GetSingleton()->GetShaderStorageBuffer().BindBase( 3 );

	// Initialise skin VAO
	g_oSkinVAO = T2Render::CreateVertexArray( g_oSkinVertexBuffer.GetRenderBuffer(), g_oSharedIndexBuffer.GetRenderBuffer() );
	g_oSkinVAO.Bind();
	g_oSkinVAO.GetVertexBuffer().SetAttribPointer( 0, 3, GL_FLOAT, sizeof( ASkinMesh::SkinVertex ), offsetof( ASkinMesh::SkinVertex, Position ) );
	g_oSkinVAO.GetVertexBuffer().SetAttribPointer( 1, 3, GL_FLOAT, sizeof( ASkinMesh::SkinVertex ), offsetof( ASkinMesh::SkinVertex, Normal ) );
	g_oSkinVAO.GetVertexBuffer().SetAttribPointer( 2, 4, GL_UNSIGNED_BYTE, sizeof( ASkinMesh::SkinVertex ), offsetof( ASkinMesh::SkinVertex, Weights ), GL_TRUE );
	g_oSkinVAO.GetVertexBuffer().SetAttribPointer( 3, 4, GL_UNSIGNED_BYTE, sizeof( ASkinMesh::SkinVertex ), offsetof( ASkinMesh::SkinVertex, Bones ), GL_TRUE );
	g_oSkinVAO.GetVertexBuffer().SetAttribPointer( 4, 2, GL_FLOAT, sizeof( ASkinMesh::SkinVertex ), offsetof( ASkinMesh::SkinVertex, UV ) );
	AEnhancedRenderer::GetSingleton()->GetIndirectBuffer().Bind();
	AEnhancedRenderer::GetSingleton()->GetShaderStorageBuffer().BindBase( 3 );

	T2VertexArray::Unbind();
	InstallHooks();
}

ARenderBufferCollection::~ARenderBufferCollection()
{
	if ( m_bCreated )
	{
		Destroy();
	}
}

void ARenderBufferCollection::Create( TINT a_iMaxNumVertexArrayObjects )
{
	TASSERT( TFALSE == m_bCreated );

    m_vecFreeVertexArrayObjects.SetSize( a_iMaxNumVertexArrayObjects );
	m_vecUsedVertexArrayObjects.Reserve( a_iMaxNumVertexArrayObjects );
	m_pMeshes = new ARenderBuffer::Mesh[ a_iMaxNumVertexArrayObjects ];

	m_iNumFreeMeshes = a_iMaxNumVertexArrayObjects;
	m_iNumUsedMeshes = 0;

	for ( TINT i = 0; i < a_iMaxNumVertexArrayObjects; i++ )
	{
		m_vecFreeVertexArrayObjects[ i ] = i;
	}

    m_bCreated = TTRUE;
}

void ARenderBufferCollection::Destroy()
{
	TASSERT( TTRUE == m_bCreated );

	// Destroy VAOs
	if ( m_pMeshes )
	{
		delete[] m_pMeshes;
		m_pMeshes = TNULL;
	}

	m_vecFreeVertexArrayObjects.FreeMemory();
	m_vecUsedVertexArrayObjects.FreeMemory();

	m_iNumFreeMeshes = -1;
	m_iNumUsedMeshes = -1;
}

ARenderBuffer ARenderBufferCollection::AllocateRenderBuffer( T2SharedVertexBuffer::SubBuffer* a_pVertexBuffer, T2SharedIndexBuffer::SubBuffer* a_pIndexBuffer )
{
	TASSERT( m_iNumFreeMeshes != 0 );

	TINT iBufferID = *m_vecFreeVertexArrayObjects.Begin();

	m_vecFreeVertexArrayObjects.EraseFast( m_vecFreeVertexArrayObjects.Begin() );
	m_vecUsedVertexArrayObjects.PushBack( iBufferID );
	m_iNumFreeMeshes -= 1;
	m_iNumUsedMeshes += 1;

	if ( a_pVertexBuffer->owner == &g_oWorldVertexBuffer )
		m_pMeshes[ iBufferID ].pVAO = &g_oWorldVAO;
	else if (a_pVertexBuffer->owner == &g_oSkinVertexBuffer )
		m_pMeshes[ iBufferID ].pVAO = &g_oSkinVAO;
	
	m_pMeshes[ iBufferID ].pVertexBuffer = a_pVertexBuffer;
	m_pMeshes[ iBufferID ].pIndexBuffer = a_pIndexBuffer;

	return GetRenderBuffer( iBufferID );
}

void ARenderBufferCollection::DestroyRenderBuffer( const ARenderBuffer& a_rcRenderBuffer )
{
	TASSERT( m_iNumUsedMeshes != 0 );

	m_vecFreeVertexArrayObjects.PushBack( a_rcRenderBuffer.iID );
	m_vecUsedVertexArrayObjects.FindAndEraseFast( a_rcRenderBuffer.iID );

	a_rcRenderBuffer->pVertexBuffer->Deallocate();
	a_rcRenderBuffer->pIndexBuffer->Deallocate();

	m_iNumFreeMeshes += 1;
	m_iNumUsedMeshes -= 1;
}

ARenderBuffer ARenderBufferCollection::GetRenderBuffer( TINT a_iID ) const
{
	return ARenderBuffer{ .iID = a_iID, .pMesh = &m_pMeshes[ a_iID ] };
}

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

// TVertexPoolResource

HOOK( 0x006d6380, TVertexPoolResource_CTOR, TVertexPoolResource* )
{
	TVertexPoolResource* pPool = CallOriginal();
	pPool->m_uiNumLocksAllTime = -1;

	return pPool;
}

TVertexPoolResource::LockBuffer g_VertexLockBuffer;
TIndexPoolResource::LockBuffer g_IndexLockBuffer;

MEMBER_HOOK( 0x006d64f0, TVertexPoolResource, TVertexPoolResource_Lock, TBOOL, TVertexPoolResource::LockBuffer& a_rLockBuffer )
{
	// HACK: Using m_uiNumLocksAllTime to store ID of vertex buffer since this value is anyways unused
	TUINT pAllocation = m_uiNumLocksAllTime;

	TBOOL bResult       = CallOriginal( a_rLockBuffer );
	m_uiNumLocksAllTime = pAllocation;

	g_VertexLockBuffer = a_rLockBuffer;
	return bResult;
}

MEMBER_HOOK( 0x006d65d0, TVertexPoolResource, TVertexPoolResource_Unlock, void, TUINT16 a_uiNumVertices )
{
	TUINT pAllocation = m_uiNumLocksAllTime;
	TUINT uiVertexSize = GetFactory()->GetVertexFormat().m_aStreamFormats[ 0 ].m_uiVertexSize;
	TUINT uiCopySize   = uiVertexSize * a_uiNumVertices;

	TASSERT( GetFactory()->GetVertexFormat().m_uiNumStreams == 1 );
	TASSERT( g_VertexLockBuffer.uiNumStreams == 1 );

	T2SharedVertexBuffer::SubBuffer* pSubBuffer = (T2SharedVertexBuffer::SubBuffer*)pAllocation;

	if ( pAllocation != -1 && uiCopySize > pSubBuffer->size )
	{
		pSubBuffer->Deallocate();
		pAllocation = -1;
	}

	if ( pAllocation == -1 )
	{
		T2SharedVertexBuffer* pBuffer = TNULL;

		if ( uiVertexSize == 44 )
		{
			// World Vertex
			pBuffer = &g_oWorldVertexBuffer;
		}
		else if ( uiVertexSize == 40 )
		{
			// Skin Vertex
			pBuffer = &g_oSkinVertexBuffer;
		}
		else if ( uiVertexSize == 24 )
		{
			// Sys Vertex
			pBuffer = &g_oSysVertexBuffer;
		}

		pSubBuffer          = pBuffer->Allocate( uiCopySize );
		m_uiNumLocksAllTime = (TUINT)pSubBuffer;
	}

	TASSERT( pSubBuffer != TNULL && (TUINT)pSubBuffer != -1 );
	pSubBuffer->SetData( g_VertexLockBuffer.apStreams[ 0 ], uiCopySize );

	CallOriginal( a_uiNumVertices );
}

MEMBER_HOOK( 0x006d6280, TVertexPoolResource, TVertexPoolResource_OnDestroy, void )
{
	TUINT pAllocation = m_uiNumLocksAllTime;
	TASSERT( pAllocation != -1 );

	T2SharedVertexBuffer::SubBuffer* pSubBuffer = (T2SharedVertexBuffer::SubBuffer*)pAllocation;
	pSubBuffer->Deallocate();
	
	CallOriginal();
}

// TIndexPoolResource

HOOK( 0x006d5ef0, TIndexPoolResource_CTOR, TIndexPoolResource* )
{
	TIndexPoolResource* pPool = CallOriginal();
	pPool->m_uiNumLocksAllTime = -1;

	return pPool;
}

MEMBER_HOOK( 0x006d6040, TIndexPoolResource, TIndexPoolResource_Lock, TBOOL, TIndexPoolResource::LockBuffer& a_rLockBuffer )
{
	// HACK: Using m_uiNumLocksAllTime to store ID of vertex buffer since this value is anyways unused
	TUINT pAllocation   = m_uiNumLocksAllTime;

	TBOOL bResult       = CallOriginal( a_rLockBuffer );
	m_uiNumLocksAllTime = pAllocation;

	g_IndexLockBuffer = a_rLockBuffer;
	return bResult;
}

MEMBER_HOOK( 0x006d60f0, TIndexPoolResource, TIndexPoolResource_Unlock, void, TUINT16 a_uiNumIndices )
{
	TUINT pAllocation = m_uiNumLocksAllTime;

	TASSERT( GetFactory()->GetIndexFormat().uiIndexSize == 2 );
	TUINT uiCopySize = GetFactory()->GetIndexFormat().uiIndexSize * a_uiNumIndices;

	T2SharedIndexBuffer::SubBuffer* pSubBuffer = (T2SharedIndexBuffer::SubBuffer*)pAllocation;

	if ( pAllocation != -1 && uiCopySize > pSubBuffer->size )
	{
		( (T2SharedIndexBuffer::SubBuffer*)pAllocation )->Deallocate();
		pAllocation = -1;
	}

	if ( pAllocation == -1 )
	{
		pSubBuffer          = g_oSharedIndexBuffer.Allocate( uiCopySize );
		m_uiNumLocksAllTime = (TUINT)pSubBuffer;
	}

	TASSERT( pSubBuffer != TNULL && (TUINT)pSubBuffer != -1 );
	pSubBuffer->SetData( g_IndexLockBuffer.pBuffer, uiCopySize );

	CallOriginal( a_uiNumIndices );
}

MEMBER_HOOK( 0x006d5e20, TIndexPoolResource, TIndexPoolResource_OnDestroy, void )
{
	TINT pAllocation = m_uiNumLocksAllTime;
	TASSERT( pAllocation != -1 );

	T2SharedIndexBuffer::SubBuffer* pSubBuffer = (T2SharedIndexBuffer::SubBuffer*)pAllocation;
	pSubBuffer->Deallocate();

	CallOriginal();
}

// AModelLoader

HOOK( 0x00613a40, AModelLoader_LoadWorldMeshTRB, void, TModel* a_pModel, TINT a_iLODIndex, TModelLOD* a_pLOD, TTMDWin::TRBLODHeader* a_pLODHeader )
{
	CallOriginal( a_pModel, a_iLODIndex, a_pLOD, a_pLODHeader );

	for ( TINT k = 0; k < a_pLOD->iNumMeshes; k++ )
	{
		TMesh* pMesh = a_pLOD->ppMeshes[ k ];

		AWorldMesh*          pWorldMesh  = (AWorldMesh*)pMesh;
		TVertexPoolResource* pVertexPool = (TVertexPoolResource*)pWorldMesh->m_pVertexPool;
		TIndexPoolResource*  pIndexPool  = (TIndexPoolResource*)pWorldMesh->m_pSubMeshes[ 0 ].pIndexPool;

		T2SharedVertexBuffer::SubBuffer* pVertexBuffer = (T2SharedVertexBuffer::SubBuffer*)pVertexPool->m_uiNumLocksAllTime;
		T2SharedIndexBuffer::SubBuffer*  pIndexBuffer  = (T2SharedIndexBuffer::SubBuffer*)pIndexPool->m_uiNumLocksAllTime;
		ARenderBuffer                    renderBuffer  = ARenderBufferCollection::GetSingleton()->AllocateRenderBuffer( pVertexBuffer, pIndexBuffer );
		pWorldMesh->m_pSubMeshes[ 0 ].iRenderBufferId  = renderBuffer.iID;
	}
}

HOOK( 0x006135d0, AModelLoader_LoadSkinLOD, void, TModel* a_pModel, TINT a_iLODIndex, TModelLOD* a_pLOD, TTMDWin::TRBLODHeader* a_pLODHeader )
{
	CallOriginal( a_pModel, a_iLODIndex, a_pLOD, a_pLODHeader );

	for ( TINT i = 0; i < a_pLOD->iNumMeshes; i++ )
	{
		TMesh* pMesh = a_pLOD->ppMeshes[ i ];

		ASkinMesh*           pSkinMesh   = (ASkinMesh*)pMesh;
		TVertexPoolResource* pVertexPool = (TVertexPoolResource*)pSkinMesh->m_pVertexPool;

		for ( TINT k = 0; k < pSkinMesh->m_uiNumSubMeshes; k++ )
		{
			ASkinMesh::SubMesh* pSubMesh   = &pSkinMesh->m_pSubMeshes[ k ];
			TIndexPoolResource* pIndexPool = (TIndexPoolResource*)pSubMesh->pIndexPool;

			T2SharedVertexBuffer::SubBuffer* pVertexBuffer = (T2SharedVertexBuffer::SubBuffer*)pVertexPool->m_uiNumLocksAllTime;
			T2SharedIndexBuffer::SubBuffer*  pIndexBuffer  = (T2SharedIndexBuffer::SubBuffer*)pIndexPool->m_uiNumLocksAllTime;
			ARenderBuffer                    renderBuffer  = ARenderBufferCollection::GetSingleton()->AllocateRenderBuffer( pVertexBuffer, pIndexBuffer );
			pSubMesh->Zero                                 = renderBuffer.iID;
		}
	}
}

void ARenderBufferCollection::InstallHooks()
{
	InstallHook<TVertexPoolResource_CTOR>();
	InstallHook<TVertexPoolResource_Lock>();
	InstallHook<TVertexPoolResource_Unlock>();
	InstallHook<TVertexPoolResource_OnDestroy>();

	InstallHook<TIndexPoolResource_CTOR>();
	InstallHook<TIndexPoolResource_Lock>();
	InstallHook<TIndexPoolResource_Unlock>();
	InstallHook<TIndexPoolResource_OnDestroy>();

	InstallHook<AModelLoader_LoadWorldMeshTRB>();
	InstallHook<AModelLoader_LoadSkinLOD>();
}
