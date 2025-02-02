#include "pch.h"
#include "ARenderBufferCollection.h"
#include "AEnhancedWorldShader.h"
#include "HookHelpers.h"

#include <Render/TTMDWin.h>

#include <Platform/GL/T2Render_GL.h>
#include <Platform/DX8/TVertexPoolResource_DX8.h>
#include <Platform/DX8/TIndexPoolResource_DX8.h>
#include <Platform/DX8/TModel_DX8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ARenderBufferCollection::ARenderBufferCollection()
    : a_iMaxNumVertexArrayObjects( 0 )
    , m_bCreated( TFALSE )
    , m_pVertexArrayObjects( TNULL )
    , m_iNumFreeVAO( -1 )
    , m_iNumUsedVAO( -1 )
{
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
	m_pVertexArrayObjects = new T2VertexArray[ a_iMaxNumVertexArrayObjects ];

	m_iNumFreeVAO = a_iMaxNumVertexArrayObjects;
	m_iNumUsedVAO = 0;

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
	if ( m_pVertexArrayObjects )
	{
		delete[] m_pVertexArrayObjects;
		m_pVertexArrayObjects = TNULL;
	}

	m_vecFreeVertexArrayObjects.FreeMemory();
	m_vecUsedVertexArrayObjects.FreeMemory();

	m_iNumFreeVAO = -1;
	m_iNumUsedVAO = -1;
}

ARenderBuffer ARenderBufferCollection::AllocateRenderBuffer()
{
	T2VertexBuffer vertexBuffer = T2Render::CreateVertexBuffer( TNULL, 0, 0 );
	T2IndexBuffer  indexBuffer  = T2Render::CreateIndexBuffer( TNULL, 0, 0 );

	return AllocateRenderBuffer( vertexBuffer, indexBuffer );
}

ARenderBuffer ARenderBufferCollection::AllocateRenderBuffer( T2VertexBuffer a_VertexBuffer, T2IndexBuffer a_IndexBuffer )
{
	TASSERT( m_iNumFreeVAO != 0 );

	TINT iBufferID = *m_vecFreeVertexArrayObjects.Begin();

	m_vecFreeVertexArrayObjects.EraseFast( m_vecFreeVertexArrayObjects.Begin() );
	m_vecUsedVertexArrayObjects.PushBack( iBufferID );
	m_iNumFreeVAO -= 1;
	m_iNumUsedVAO += 1;

	m_pVertexArrayObjects[ iBufferID ] = T2Render::CreateVertexArray( a_VertexBuffer, a_IndexBuffer );

	return GetRenderBuffer( iBufferID );
}

void ARenderBufferCollection::DestroyRenderBuffer( const ARenderBuffer& a_rcRenderBuffer )
{
	TASSERT( m_iNumUsedVAO != 0 );

	m_vecFreeVertexArrayObjects.PushBack( a_rcRenderBuffer.iID );
	m_vecUsedVertexArrayObjects.FindAndEraseFast( a_rcRenderBuffer.iID );

	a_rcRenderBuffer->Destroy();
	a_rcRenderBuffer->GetVertexBuffer().Destroy();
	a_rcRenderBuffer->GetIndexBuffer().Destroy();

	m_iNumFreeVAO += 1;
	m_iNumUsedVAO -= 1;
}

ARenderBuffer ARenderBufferCollection::GetRenderBuffer( TINT a_iID ) const
{
	return ARenderBuffer{ .iID = a_iID, .pVertexArray = &m_pVertexArrayObjects[ a_iID ] };
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
	TINT iBufferId = m_uiNumLocksAllTime;

	if ( iBufferId == -1 )
	{
		// Unbind VAO to avoid loading wrong vertex buffer into it
		T2VertexArray::Unbind();

		// Allocate buffer if it doesn't exist yet
		iBufferId = T2Render::CreateVertexBuffer( TNULL, 0, GL_STATIC_DRAW ).GetId();
	}

	TBOOL bResult       = CallOriginal( a_rLockBuffer );
	m_uiNumLocksAllTime = iBufferId;

	g_VertexLockBuffer = a_rLockBuffer;
	return bResult;
}

MEMBER_HOOK( 0x006d65d0, TVertexPoolResource, TVertexPoolResource_Unlock, void, TUINT16 a_uiNumVertices )
{
	TINT iBufferId = m_uiNumLocksAllTime;
	TASSERT( iBufferId != -1 );

	TASSERT( GetFactory()->GetVertexFormat().m_uiNumStreams == 1 );
	TASSERT( g_VertexLockBuffer.uiNumStreams == 1 );

	TUINT uiCopySize = GetFactory()->GetVertexFormat().m_aStreamFormats[ 0 ].m_uiVertexSize * a_uiNumVertices;

	// Unbind VAO to avoid loading wrong vertex buffer into it
	T2VertexArray::Unbind();

	T2VertexBuffer vertexBuffer( iBufferId );
	vertexBuffer.Bind();
	vertexBuffer.SetData( g_VertexLockBuffer.apStreams[ 0 ], uiCopySize, GL_STATIC_DRAW );

	CallOriginal( a_uiNumVertices );
}

MEMBER_HOOK( 0x006d6280, TVertexPoolResource, TVertexPoolResource_OnDestroy, void )
{
	TINT iBufferId = m_uiNumLocksAllTime;
	TASSERT( iBufferId != -1 );

	T2VertexBuffer( iBufferId ).Destroy();
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
	TINT iBufferId = m_uiNumLocksAllTime;

	if ( iBufferId == -1 )
	{
		// Unbind VAO to avoid loading wrong index buffer into it
		T2VertexArray::Unbind();

		// Allocate buffer if it doesn't exist yet
		iBufferId = T2Render::CreateIndexBuffer( TNULL, 0, GL_STATIC_DRAW ).GetId();
	}

	TBOOL bResult       = CallOriginal( a_rLockBuffer );
	m_uiNumLocksAllTime = iBufferId;

	g_IndexLockBuffer = a_rLockBuffer;
	return bResult;
}

MEMBER_HOOK( 0x006d60f0, TIndexPoolResource, TIndexPoolResource_Unlock, void, TUINT16 a_uiNumIndices )
{
	TINT iBufferId = m_uiNumLocksAllTime;
	TASSERT( iBufferId != -1 );

	TASSERT( GetFactory()->GetIndexFormat().uiIndexSize == 2 );
	TUINT uiCopySize = GetFactory()->GetIndexFormat().uiIndexSize * a_uiNumIndices;

	// Unbind VAO to avoid loading wrong index buffer into it
	T2VertexArray::Unbind();

	// Upload index data
	T2IndexBuffer indexBuffer( iBufferId );
	indexBuffer.Bind();
	indexBuffer.SetData( g_IndexLockBuffer.pBuffer, uiCopySize, GL_STATIC_DRAW );

	CallOriginal( a_uiNumIndices );
}

MEMBER_HOOK( 0x006d5e20, TIndexPoolResource, TIndexPoolResource_OnDestroy, void )
{
	TINT iBufferId = m_uiNumLocksAllTime;
	TASSERT( iBufferId != -1 );

	T2IndexBuffer( iBufferId ).Destroy();
	CallOriginal();
}

// TModelHAL

MEMBER_HOOK( 0x006d9e90, TModelHAL, TModelHAL_LoadTMDHAL, TBOOL, const TCHAR* a_szFileName, TTRB* a_pAssetTRB, TUINT8 a_ui8FileNameLen )
{
	TBOOL bCreated = CallOriginal( a_szFileName, a_pAssetTRB, a_ui8FileNameLen );

	//if ( bCreated )
	//{
	//	// TODO: create VAOs for different meshes

	//	for ( TINT i = 0; i < GetNumLODs(); i++ )
	//	{
	//		TModelLOD* pLOD = &m_LODs[ i ];

	//		for ( TINT k = 0; k < pLOD->iNumMeshes; k++ )
	//		{
	//			Toshi::TMesh* pMesh = pLOD->ppMeshes[ k ];
	//			const TCHAR* pchClassName = pMesh->GetClass()->GetName();

	//			if ( !T2String8::Compare( pchClassName, "AWorldMeshHAL" ) )
	//			{
	//				AWorldMesh* pWorldMesh = (AWorldMesh*)pMesh;

	//				ARenderBuffer renderBuffer = ARenderBufferCollection::GetSingleton()->AllocateRenderBuffer();
	//				renderBuffer->SetAttribPointer( 0, 3, GL_FLOAT, sizeof( WorldVertex ), (void*)offsetof( WorldVertex, Position ) );
	//				renderBuffer->SetAttribPointer( 1, 3, GL_FLOAT, sizeof( WorldVertex ), (void*)offsetof( WorldVertex, Normal ) );
	//				renderBuffer->SetAttribPointer( 2, 3, GL_FLOAT, sizeof( WorldVertex ), (void*)offsetof( WorldVertex, Color ) );
	//				renderBuffer->SetAttribPointer( 3, 2, GL_FLOAT, sizeof( WorldVertex ), (void*)offsetof( WorldVertex, UV ) );

	//				TVertexPoolResource* pVertexPool = (TVertexPoolResource*)pWorldMesh->m_pVertexPool;
	//				renderBuffer->SetVertexBuffer( pVertexPool->m_uiNumLocksAllTime );

	//				pWorldMesh->m_pSubMeshes[ 0 ].iRenderBufferId = renderBuffer.iID;
	//				TIndexPoolResource* pIndexPool = (TIndexPoolResource*)pWorldMesh->m_pSubMeshes[ 0 ].pIndexPool;
	//				renderBuffer->SetIndexBuffer( pIndexPool->m_uiNumLocksAllTime );

	//			}
	//			else
	//			{
	//				TTRACE( "Meshes of type '%s' are not supported by the OpenGL renderer!\n", pchClassName );
	//			}
	//		}
	//	}

	//	return TTRUE;
	//}

	return TTRUE;
}

HOOK( 0x00613a40, AModelLoader_LoadWorldMeshTRB, void, TModel* a_pModel, TINT a_iLODIndex, TModelLOD* a_pLOD, Toshi::TTMDWin::TRBLODHeader* a_pLODHeader )
{
	CallOriginal( a_pModel, a_iLODIndex, a_pLOD, a_pLODHeader );

	struct WorldVertex
	{
		Toshi::TVector3 Position;
		Toshi::TVector3 Normal;
		Toshi::TVector3 Color;
		Toshi::TVector2 UV;
	};

	for ( TINT k = 0; k < a_pLOD->iNumMeshes; k++ )
	{
		Toshi::TMesh* pMesh        = a_pLOD->ppMeshes[ k ];
		const TCHAR*  pchClassName = pMesh->GetClass()->GetName();

		if ( !T2String8::Compare( pchClassName, "AWorldMeshHAL" ) )
		{
			AWorldMesh* pWorldMesh = (AWorldMesh*)pMesh;
			TVertexPoolResource* pVertexPool = (TVertexPoolResource*)pWorldMesh->m_pVertexPool;
			TIndexPoolResource*  pIndexPool  = (TIndexPoolResource*)pWorldMesh->m_pSubMeshes[ 0 ].pIndexPool;

			ARenderBuffer renderBuffer = ARenderBufferCollection::GetSingleton()->AllocateRenderBuffer( pVertexPool->m_uiNumLocksAllTime, pIndexPool->m_uiNumLocksAllTime );
			renderBuffer->Bind();
			renderBuffer->SetAttribPointer( 0, 3, GL_FLOAT, sizeof( WorldVertex ), (void*)offsetof( WorldVertex, Position ) );
			renderBuffer->SetAttribPointer( 1, 3, GL_FLOAT, sizeof( WorldVertex ), (void*)offsetof( WorldVertex, Normal ) );
			renderBuffer->SetAttribPointer( 2, 3, GL_FLOAT, sizeof( WorldVertex ), (void*)offsetof( WorldVertex, Color ) );
			renderBuffer->SetAttribPointer( 3, 2, GL_FLOAT, sizeof( WorldVertex ), (void*)offsetof( WorldVertex, UV ) );

			T2VertexArray::Unbind();
			pWorldMesh->m_pSubMeshes[ 0 ].iRenderBufferId = renderBuffer.iID;
		}
		else
		{
			TTRACE( "Meshes of type '%s' are not supported by the OpenGL renderer!\n", pchClassName );
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

	InstallHook<TModelHAL_LoadTMDHAL>();
	InstallHook<AModelLoader_LoadWorldMeshTRB>();
}
