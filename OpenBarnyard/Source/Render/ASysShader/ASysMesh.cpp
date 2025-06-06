#include "pch.h"
#include "ASysMesh.h"
#include "Render/TRenderInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( ASysMesh );

// $Barnyard: FUNCTION 005f00b0
ASysMesh::ASysMesh()
{
	m_uiFlags       = 0;
	m_uiMaxVertices = 0;
	m_uiMaxIndices  = 0;
	m_pVertexPool   = TNULL;
	m_pIndexPool    = TNULL;
	m_bFlag1        = TFALSE;
	m_bFlag2        = TFALSE;
}

// $Barnyard: FUNCTION 005f03e0
TBOOL ASysMesh::Validate()
{
	if ( IsValidated() )
	{
		if ( m_SomeString.Length() == 0 && ( !m_pVertexPool || !m_pIndexPool ) )
		{
			return TFALSE;
		}

		m_pVertexPool->Validate();
		m_pIndexPool->Validate();
		TMesh::Validate();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005f0420
void ASysMesh::Invalidate()
{
	if ( m_pVertexPool && m_pIndexPool )
	{
		DestroyResources();
	}

	if ( IsValidated() )
	{
		TMesh::Invalidate();
	}
}

void ASysMesh::DestroyResources()
{
	TRenderInterface::GetSingleton()->BeginEndSceneHAL();

	if ( m_pVertexPool )
	{
		TRenderInterface::GetSingleton()->DestroyResource( m_pVertexPool );
		m_pVertexPool = TNULL;
	}

	if ( m_pIndexPool )
	{
		TRenderInterface::GetSingleton()->DestroyResource( m_pIndexPool );
		m_pIndexPool = TNULL;
	}

	m_uiFlags       = 0;
	m_uiMaxVertices = 0;
	m_uiMaxIndices  = 0;
}

// $Barnyard: FUNCTION 005f0180
TBOOL ASysMesh::Create( void* a_pUnk, const TCHAR* a_szSomeString )
{
	m_SomeString = a_szSomeString;
	return TMesh::Create();
}

// $Barnyard: FUNCTION 005f0450
TBOOL ASysMesh::CreatePools( TUINT32 a_uiResourcesFlags, TUINT16 a_uiMaxVertices, TUINT16 a_uiMaxIndices )
{
	m_uiFlags       = a_uiResourcesFlags;
	m_uiMaxVertices = a_uiMaxVertices;
	m_uiMaxIndices  = a_uiMaxIndices;

	if ( !CreateResource() )
	{
		return TFALSE;
	}

	TMesh::Create();
	return TTRUE;
}

TBOOL ASysMesh::CreateResource()
{
	auto pVertexFactory = TRenderInterface::GetSingleton()->GetSystemResource<TVertexFactoryResourceInterface>( SYSRESOURCE_VFSYSVNDUV1 );
	TVALIDPTR( pVertexFactory );

	TUINT16 uiVertexPoolFlags;

	if ( ( m_uiFlags & 1 ) == 0 )
	{
		if ( ( m_uiFlags & 2 ) == 0 )
		{
			if ( ( m_uiFlags & 4 ) != 0 )
			{
				uiVertexPoolFlags = 4;
			}
		}
		else
		{
			uiVertexPoolFlags = 2;
		}
	}
	else
	{
		uiVertexPoolFlags = 1;
	}

	m_pVertexPool = pVertexFactory->CreatePoolResource( m_uiMaxVertices, uiVertexPoolFlags );
	TVALIDPTR( m_pVertexPool );

	auto pIndexFactory = TRenderInterface::GetSingleton()->GetSystemResource<TIndexFactoryResourceInterface>( SYSRESOURCE_IFSYS );
	TVALIDPTR( pIndexFactory );

	TUINT16 uiIndexPoolFlags;

	if ( ( m_uiFlags & 8 ) == 0 )
	{
		if ( ( m_uiFlags & 0x10 ) == 0 )
		{
			if ( ( m_uiFlags & 0x20 ) != 0 )
			{
				uiIndexPoolFlags = 4;
			}
		}
		else
		{
			uiIndexPoolFlags = 2;
		}
	}
	else
	{
		uiIndexPoolFlags = 1;
	}


	m_pIndexPool = pIndexFactory->CreatePoolResource( m_uiMaxVertices, ( -(TUINT)( ( m_uiFlags & 0x40 ) != 0 ) & 0xfffffff8 ) + 16 | uiIndexPoolFlags );
	TVALIDPTR( m_pIndexPool );

	return TTRUE;
}

// $Barnyard: FUNCTION 005f02b0
TBOOL ASysMesh::Lock( LockBuffer& a_rLockBuffer )
{
	TASSERT( 0 == ( m_uiFlags & FLAG_LOCKED ) );

	if ( m_pVertexPool->Lock( &a_rLockBuffer.VertexLock ) )
	{
		if ( m_pIndexPool->Lock( &a_rLockBuffer.IndexLock ) )
		{
			m_uiFlags |= FLAG_LOCKED;
			return TTRUE;
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005f02f0
void ASysMesh::Unlock( TUINT32 a_uiNumVertices, TUINT32 a_uiNumIndices )
{
	TASSERT( 0 != ( m_uiFlags & FLAG_LOCKED ) );

	if ( TINT16( a_uiNumVertices ) == -1 )
	{
		a_uiNumVertices = m_pVertexPool->GetNumVertices();
	}

	if ( TINT16( a_uiNumIndices ) == -1 )
	{
		a_uiNumIndices = m_pIndexPool->GetNumIndices();
	}

	m_pVertexPool->Unlock( a_uiNumVertices );
	m_pIndexPool->Unlock( a_uiNumIndices );
	m_uiFlags &= FLAG_LOCKED;
}

Toshi::TVertexPoolResourceInterface* ASysMesh::GetVertexPool()
{
	return m_pVertexPool;
}

// $Barnyard: FUNCTION 0066b130
Toshi::TIndexPoolResourceInterface* ASysMesh::GetIndexPool()
{
	return m_pIndexPool;
}
