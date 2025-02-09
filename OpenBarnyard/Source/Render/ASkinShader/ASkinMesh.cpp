#include "pch.h"
#include "ASkinMesh.h"
#include "Assets/AModelLoader.h"

#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( ASkinMesh );

ASkinSubMesh::ASkinSubMesh()
    : uiUnknown( 0 ), pIndexPool( TNULL ), uiNumVertices( 0 ), uiNumBones( 0 )
{
}

ASkinSubMesh::~ASkinSubMesh()
{
	if ( pIndexPool )
	{
		pIndexPool->DestroyResource();
		pIndexPool = TNULL;
	}
}

ASkinMesh::ASkinMesh()
    : m_uiFlags( 0 ), m_uiMaxVertices( 0 ), m_uiNumSubMeshes( 0 ), m_pVertexPool( TNULL ), m_Unk( TNULL )
{
}

ASkinMesh::~ASkinMesh()
{
}

TBOOL ASkinMesh::Validate()
{
	if ( !IsValidated() )
	{
		TVALIDPTR( m_pVertexPool );

		m_pVertexPool->Validate();
		BaseClass::Validate();
	}

	return TTRUE;
}

void ASkinMesh::Invalidate()
{
	BaseClass::Invalidate();
}

TBOOL ASkinMesh::Render()
{
	return TTRUE;
}

void ASkinMesh::OnDestroy()
{
	AModelLoader::DestroyMaterial( m_pMaterial );
	DestroyResource();
	BaseClass::OnDestroy();
}

void ASkinMesh::Create( TUINT32 a_uiFlags, TUINT16 a_uiMaxVertices, TUINT16 a_uiNumSubMeshes )
{
	TASSERT( !IsCreated() );

	m_uiMaxVertices  = a_uiMaxVertices;
	m_uiFlags        = a_uiFlags;
	m_uiNumSubMeshes = a_uiNumSubMeshes;

	if ( CreateResource() )
	{
		BaseClass::Create();
	}
	else
	{
		TASSERT( !"Couldn't create ASkinMesh" );
	}
}

TBOOL ASkinMesh::Lock( LockBuffer& a_rLockBuffer )
{
	TASSERT( 0 == ( m_uiFlags & FLAG_LOCKED ) );

	if ( m_pVertexPool->Lock( &a_rLockBuffer ) )
	{
		m_uiFlags |= FLAG_LOCKED;
		return TTRUE;
	}

	return TFALSE;
}

void ASkinMesh::Unlock( TUINT32 a_uiNumVertices )
{
	TASSERT( 0 != ( m_uiFlags & FLAG_LOCKED ) );

	if ( TINT16( a_uiNumVertices ) == -1 )
	{
		a_uiNumVertices = m_pVertexPool->GetNumVertices();
	}

	m_pVertexPool->Unlock( a_uiNumVertices );
	m_uiFlags &= ~FLAG_LOCKED;
}

Toshi::TVertexPoolResourceInterface* ASkinMesh::GetVertexPool()
{
	return m_pVertexPool;
}

TUINT16 ASkinMesh::GetNumSubMeshes() const
{
	return m_uiNumSubMeshes;
}

ASkinSubMesh* ASkinMesh::GetSubMesh( TUINT16 a_uiIndex )
{
	TASSERT( a_uiIndex < m_uiNumSubMeshes );
	return &m_pSubMeshes[ a_uiIndex ];
}

TBOOL ASkinMesh::CreateResource()
{
	auto pVertexFactory = TRenderInterface::GetSingleton()->GetSystemResource<TVertexFactoryResourceInterface>( SYSRESOURCE_VFSKIN );
	TVALIDPTR( pVertexFactory );

	m_pVertexPool = pVertexFactory->CreatePoolResource( m_uiMaxVertices, 1 );
	m_pSubMeshes  = new ASkinSubMesh[ m_uiNumSubMeshes ];

	return TTRUE;
}

void ASkinMesh::DestroyResource()
{
	if ( m_pVertexPool )
	{
		TRenderInterface::GetSingleton()->DestroyResource( m_pVertexPool );
		m_pVertexPool = TNULL;
	}

	delete[] m_pSubMeshes;
	m_pSubMeshes    = TNULL;
	m_uiFlags       = 0;
	m_uiMaxVertices = 0;
}
