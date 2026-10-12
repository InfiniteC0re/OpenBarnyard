#include "pch.h"
#include "AFXMesh.h"
#include "Assets/AModelLoader.h"

#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 005f97f0
TDEFINE_CLASS_NORUNTIME( AFXMesh );

// $Barnyard: FUNCTION 005f9840
AFXMesh::AFXMesh()
    : m_uiFlags( 0 ), m_uiMaxVertices( 0 ), m_uiNumSubMeshes( 0 ), m_pVertexPool( TNULL ), m_pSubMeshes( TNULL ), m_Unk( TNULL )
{
}

// NOTE: Folded with the TMesh destructor wrapper (005f0340)
AFXMesh::~AFXMesh()
{
}

// NOTE: Folded with AWorldMesh::Validate (005f8cc0)
TBOOL AFXMesh::Validate()
{
	if ( !IsValidated() )
	{
		TVALIDPTR( m_pVertexPool );
		m_pVertexPool->Validate();
		BaseClass::Validate();
	}

	return TTRUE;
}

// NOTE: Folded with ASkinMesh::Invalidate (005f1580)
void AFXMesh::Invalidate()
{
	BaseClass::Invalidate();
}

// RETURNS_TRUE
TBOOL AFXMesh::Render()
{
	return TTRUE;
}

// NOTE: Folded with ASkinMesh::OnDestroy (005f16c0)
void AFXMesh::OnDestroy()
{
	AModelLoader::DestroyMaterial( m_pMaterial );
	DestroyResource();
	BaseClass::OnDestroy();
}

// $Barnyard: FUNCTION 005f9a30
TBOOL AFXMesh::Create( TUINT32 a_uiFlags, TUINT16 a_uiMaxVertices, TUINT16 a_uiNumSubMeshes )
{
	m_uiMaxVertices  = a_uiMaxVertices;
	m_uiFlags        = a_uiFlags;
	m_uiNumSubMeshes = a_uiNumSubMeshes;

	if ( !CreateResource() )
		return TFALSE;

	return BaseClass::Create();
}

// NOTE: Folded with ASkinMesh::Lock (005f9980)
TBOOL AFXMesh::Lock( LockBuffer& a_rLockBuffer )
{
	TASSERT( 0 == ( m_uiFlags & FLAG_LOCKED ) );

	if ( m_pVertexPool->Lock( &a_rLockBuffer ) )
	{
		m_uiFlags |= FLAG_LOCKED;
		return TTRUE;
	}

	return TFALSE;
}

// NOTE: Folded with ASkinMesh::Unlock (005f5de0)
void AFXMesh::Unlock( TUINT32 a_uiNumVertices )
{
	TASSERT( 0 != ( m_uiFlags & FLAG_LOCKED ) );

	if ( TINT16( a_uiNumVertices ) == -1 )
	{
		a_uiNumVertices = m_pVertexPool->GetNumVertices();
	}

	m_pVertexPool->Unlock( a_uiNumVertices );
	m_uiFlags &= ~FLAG_LOCKED;
}

// NOTE: Folded getter (005f7a30)
Toshi::TVertexPoolResourceInterface* AFXMesh::GetVertexPool()
{
	return m_pVertexPool;
}

// $Barnyard: FUNCTION 005f9880
TBOOL AFXMesh::CreateResource()
{
	auto pVertexFactory = TRenderInterface::GetSingleton()->GetSystemResource<TVertexFactoryResourceInterface>( SYSRESOURCE_VFSKIN );
	TVALIDPTR( pVertexFactory );

	m_pVertexPool = pVertexFactory->CreatePoolResource( m_uiMaxVertices, TVertexPoolResourceInterface::FLAGS_STATIC );
	m_pSubMeshes  = new ASkinSubMesh[ m_uiNumSubMeshes ];

	return TTRUE;
}

// NOTE: Folded with ASkinMesh::DestroyResource (005f9930)
void AFXMesh::DestroyResource()
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
