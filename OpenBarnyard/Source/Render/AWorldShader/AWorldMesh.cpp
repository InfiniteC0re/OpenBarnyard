#include "pch.h"
#include "AWorldMesh.h"
#include "Assets/AModelLoader.h"

#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AWorldMesh );

// $Barnyard: FUNCTION 005f5d00
AWorldMesh::AWorldMesh()
    : m_uiFlags( 0 ), m_uiMaxVertices( 0 ), m_pVertexPool( TNULL )
{
}

// $Barnyard: FUNCTION 005f8cc0
TBOOL AWorldMesh::Validate()
{
	if ( !IsValidated() )
	{
		TVALIDPTR( m_pVertexPool );

		m_pVertexPool->Validate();
		BaseClass::Validate();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005effe0
void AWorldMesh::Invalidate()
{
	BaseClass::Invalidate();
}

// $Barnyard: FUNCTION 005f5e90
void AWorldMesh::OnDestroy()
{
	AModelLoader::DestroyMaterial( m_pMaterial );
	DestroyResource();

	BaseClass::OnDestroy();
}

// $Barnyard: FUNCTION 005f5ef0
void AWorldMesh::Create( TUINT32 a_uiFlags, TUINT16 a_uiMaxVertices )
{
	TASSERT( !IsCreated() );

	m_uiMaxVertices = a_uiMaxVertices;
	m_uiFlags       = a_uiFlags;

	if ( CreateResource() )
	{
		BaseClass::Create();
	}
	else
	{
		TASSERT( !"Couldn't create AWorldMesh" );
	}
}

TBOOL AWorldMesh::Lock( LockBuffer& a_rLockBuffer )
{
	TASSERT( 0 == ( m_uiFlags & FLAG_LOCKED ) );

	if ( m_pVertexPool->Lock( &a_rLockBuffer ) )
	{
		m_uiFlags |= FLAG_LOCKED;
		return TTRUE;
	}

	return TFALSE;
}

void AWorldMesh::Unlock( TUINT32 a_uiNumVertices )
{
	TASSERT( 0 != ( m_uiFlags & FLAG_LOCKED ) );

	if ( TINT16( a_uiNumVertices ) == -1 )
	{
		a_uiNumVertices = m_pVertexPool->GetNumVertices();
	}

	m_pVertexPool->Unlock( a_uiNumVertices );
	m_uiFlags &= ~FLAG_LOCKED;
}

TVertexPoolResourceInterface* AWorldMesh::GetVertexPool()
{
	return m_pVertexPool;
}

// $Barnyard: FUNCTION 005f5d30
TBOOL AWorldMesh::CreateResource()
{
	auto pVertexFactory = TRenderInterface::GetSingleton()->GetSystemResource<TVertexFactoryResourceInterface>( SYSRESOURCE_VFWORLD );
	TVALIDPTR( pVertexFactory );

	m_pVertexPool = pVertexFactory->CreatePoolResource( m_uiMaxVertices, 1 );
	m_pSubMeshes  = new SubMesh[ NUM_SUBMESHES ];

	return TTRUE;
}

void AWorldMesh::DestroyResource()
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
