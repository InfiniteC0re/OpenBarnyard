#include "pch.h"
#include "AGrassMesh.h"
#include "Assets/AModelLoader.h"

#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AGrassMesh );

// $Barnyard: FUNCTION 005f79c0
AGrassMesh::AGrassMesh()
    : m_uiFlags( 0 )
    , m_uiMaxVertices( 0 )
    , m_pVertexPool( TNULL )
    , m_Unk5( 0 )
{
}

// $Barnyard: FUNCTION 005f7ae0
// $Barnyard: FUNCTION 005f7ac0
AGrassMesh::~AGrassMesh()
{
	if ( m_aSubMeshes[ 0 ].pIndexPool )
	{
		TRenderInterface::GetSingleton()->DestroyResource( m_aSubMeshes[ 0 ].pIndexPool );
		m_aSubMeshes[ 0 ].pIndexPool = TNULL;
	}
}

TBOOL AGrassMesh::Validate()
{
	if ( !IsValidated() )
	{
		TVALIDPTR( m_pVertexPool );

		m_pVertexPool->Validate();
		BaseClass::Validate();
	}

	return TTRUE;
}

void AGrassMesh::Invalidate()
{
	BaseClass::Invalidate();
}

// $Barnyard: FUNCTION 005f7b50
void AGrassMesh::OnDestroy()
{
	AModelLoader::DestroyMaterial( GetMaterial() );

	if ( m_pVertexPool )
	{
		TRenderInterface::GetSingleton()->DestroyResource( m_pVertexPool );
		m_pVertexPool = TNULL;
	}

	m_uiFlags       = 0;
	m_uiMaxVertices = 0;
}

// $Barnyard: FUNCTION 005f7b50
TBOOL AGrassMesh::Create( TUINT32 a_uiFlags, TUINT16 a_uiMaxVertices )
{
	m_uiFlags       = a_uiFlags;
	m_uiMaxVertices = a_uiMaxVertices;

	auto pVertexFactory = TRenderInterface::GetSingleton()->GetSystemResource<TVertexFactoryResourceInterface>( SYSRESOURCE_VFWORLD );
	TVALIDPTR( pVertexFactory );

	m_pVertexPool = pVertexFactory->CreatePoolResource( m_uiMaxVertices, 1 );
	BaseClass::Create();

	return TTRUE;
}

TBOOL AGrassMesh::Lock( LockBuffer& a_rLockBuffer )
{
	if ( m_pVertexPool->Lock( &a_rLockBuffer ) )
	{
		m_uiFlags |= FLAG_LOCKED;
		return TTRUE;
	}

	return TFALSE;
}

void AGrassMesh::Unlock( TUINT32 a_uiNumVertices )
{
	if ( TINT16( a_uiNumVertices ) == -1 )
	{
		a_uiNumVertices = m_pVertexPool->GetNumVertices();
	}

	m_pVertexPool->Unlock( a_uiNumVertices );
	m_uiFlags &= ~FLAG_LOCKED;
}

TVertexPoolResourceInterface* AGrassMesh::GetVertexPool()
{
	return m_pVertexPool;
}
