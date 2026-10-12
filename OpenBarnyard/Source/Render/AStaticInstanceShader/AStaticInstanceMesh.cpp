#include "pch.h"
#include "AStaticInstanceMesh.h"
#include "Assets/AModelLoader.h"

#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AStaticInstanceMesh );

// $Barnyard: FUNCTION 005f8c80
AStaticInstanceMesh::AStaticInstanceMesh()
    : m_uiFlags( 0 )
    , m_uiMaxVertices( 0 )
    , m_uiNumSubMeshes( 0 )
    , m_pVertexPool( TNULL )
    , m_pSubMeshes( TNULL )
    , m_uiUnk20( 0 )
{
}

// NOTE: folded with AWorldMesh::Validate (005f8cc0)
TBOOL AStaticInstanceMesh::Validate()
{
	if ( !IsValidated() )
	{
		m_pVertexPool->Validate();
		BaseClass::Validate();
	}

	return TTRUE;
}

// NOTE: folded with ASkinMesh::Invalidate (005f1580)
void AStaticInstanceMesh::Invalidate()
{
	BaseClass::Invalidate();
}

// $Barnyard: FUNCTION 005f8e40
void AStaticInstanceMesh::OnDestroy()
{
	AModelLoader::DestroyMaterial( m_pMaterial );
	DestroyResource();

	BaseClass::OnDestroy();
}

// $Barnyard: FUNCTION 005f8e60
TBOOL AStaticInstanceMesh::Create( TUINT32 a_uiFlags, TUINT16 a_uiMaxVertices, TUINT16 a_uiNumSubMeshes )
{
	m_uiMaxVertices  = a_uiMaxVertices;
	m_uiFlags        = a_uiFlags;
	m_uiNumSubMeshes = a_uiNumSubMeshes;

	if ( !CreateResource() )
		return TFALSE;

	BaseClass::Create();
	return TTRUE;
}

// NOTE: folded with ASkinMesh::Lock (005f9980)
TBOOL AStaticInstanceMesh::Lock( LockBuffer& a_rLockBuffer )
{
	if ( !m_pVertexPool->Lock( &a_rLockBuffer ) )
		return TFALSE;

	m_uiFlags |= FLAG_LOCKED;
	return TTRUE;
}

// NOTE: folded with ASkinMesh::Unlock (005f5de0)
void AStaticInstanceMesh::Unlock( TUINT32 a_uiNumVertices )
{
	if ( TINT16( a_uiNumVertices ) == -1 )
		a_uiNumVertices = m_pVertexPool->GetNumVertices();

	m_pVertexPool->Unlock( a_uiNumVertices );
	m_uiFlags &= ~FLAG_LOCKED;
}

// NOTE: folded (005f7a30)
TVertexPoolResourceInterface* AStaticInstanceMesh::GetVertexPool()
{
	return m_pVertexPool;
}

// $Barnyard: FUNCTION 005f8ce0
TBOOL AStaticInstanceMesh::CreateResource()
{
	auto pVertexFactory = TRenderInterface::GetSingleton()->GetSystemResource<TVertexFactoryResourceInterface>( SYSRESOURCE_VFWORLD );

	m_pVertexPool = pVertexFactory->CreatePoolResource( m_uiMaxVertices, TVertexPoolResourceInterface::FLAGS_STATIC );
	m_pSubMeshes  = new SubMesh[ m_uiNumSubMeshes ];

	return TTRUE;
}

// $Barnyard: FUNCTION 005f8d90
void AStaticInstanceMesh::DestroyResource()
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

// $Barnyard: FUNCTION 005f8ba0
AStaticInstanceMesh::SubMesh::SubMesh()
    : pAssociatedMesh( TNULL )
    , uiUnk2( 0 )
    , uiNumVertices( 0 )
    , pIndexPool( TNULL )
    , uiUnk24( 0 )
{
}

// NOTE: folded with AWorldMesh::SubMesh::~SubMesh (005f8bd0)
AStaticInstanceMesh::SubMesh::~SubMesh()
{
	if ( pIndexPool )
	{
		TRenderInterface::GetSingleton()->DestroyResource( pIndexPool );
		pIndexPool = TNULL;
	}
}
