#include "ToshiPCH.h"
#include "TVertexPoolResource_DX8.h"
#include "TVertexBlockResource_DX8.h"
#include "TVertexFactoryResource_DX8.h"
#include "TRenderInterface_DX8.h"
#include "Toshi/T2Pair.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TVertexBlockResource, 0, 1 );
TDEFINE_FREELIST_ALLOCATOR( TVertexBlockResource );

// $Barnyard: FUNCTION 006c0730
TBOOL TVertexBlockResource::AttachPool( TVertexPoolResource* a_pPool )
{
	TVALIDPTR( a_pPool );

	m_uiVerticesUsed += a_pPool->GetNumVertices();
	a_pPool->SetParent( this );

	if ( m_uiFlags & 1 )
	{
		Invalidate();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 006c0670
TBOOL TVertexBlockResource::CanFit( TVertexPoolResource* a_pPoolResource ) const
{
	if ( m_uiFlags & 1 && a_pPoolResource->GetFlags() & 1 )
	{
		return m_uiMaxVertices >= a_pPoolResource->GetNumVertices() + m_uiVerticesUsed;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 006c02f0
TBOOL TVertexBlockResource::DettachPool( TVertexPoolResource* a_pPool )
{
	TASSERT( TTRUE == Parent()->IsExactly( &TGetClass( TVertexFactoryResource ) ) );
	a_pPool->SetParent( Parent() == Tree()->GetRoot() ? TNULL : Parent() );

	m_uiVerticesUsed -= a_pPool->GetNumVertices();

	if ( m_uiVerticesUsed == 0 && Child() == TNULL )
	{
		GetRenderer()->DestroyResource( this );
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 006c0520
TBOOL TVertexBlockResource::Create( TVertexFactoryResourceInterface* a_pFactory, TUINT16 a_uiMaxVertices, TUINT32 a_uiFlags )
{
	m_pFactory      = a_pFactory;
	m_uiMaxVertices = a_uiMaxVertices;
	m_uiFlags       = a_uiFlags;
	return TResource::Create();
}

// $Barnyard: FUNCTION 006c0970
TBOOL TVertexBlockResource::CreateHAL()
{
	DestroyHAL();

	auto  pRenderer          = TRenderD3DInterface::Interface();
	auto& vertexFormat       = m_pFactory->GetVertexFormat();
	m_HALBuffer.uiNumStreams = vertexFormat.m_uiNumStreams;

	for ( TUINT i = 0; i < m_HALBuffer.uiNumStreams; i++ )
	{
		UINT  length = vertexFormat.m_aStreamFormats[ i ].m_uiVertexSize * m_uiMaxVertices;
		DWORD usage  = D3DUSAGE_WRITEONLY;

		if ( ISZERO( m_uiFlags & 1 ) )
		{
			usage      = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
			m_uiOffset = 0;
		}

		if ( !pRenderer->GetCurrentDevice()->SupportsHardwareTransfomations() )
		{
			usage |= D3DUSAGE_SOFTWAREPROCESSING;
		}

		HRESULT hRes = pRenderer->GetDirect3DDevice()->CreateVertexBuffer(
		    length,
		    usage,
		    0,
		    ( m_uiFlags & 1 ) ? D3DPOOL_MANAGED : D3DPOOL_DEFAULT,
		    &m_HALBuffer.apVertexBuffers[ i ]
		);

		if ( FAILED( hRes ) )
		{
			TRenderD3DInterface::PrintError( hRes, "Unable to create a new vertex buffer!" );
			return TFALSE;
		}

		TMemory::HALMemInfo memInfoHAL;
		TMemory::GetHALMemInfo( memInfoHAL );

		s_iHALMemoryUsage += memInfoHAL.m_uiMemUsage;
		s_iCurrentNumHALCreated += 1;
		s_iTotalNumHALCreated += 1;
		s_iTotalVertexBufferBytesAllocated += length;
		s_iCurrentVertexBufferBytesAllocated += length;

		if ( s_iTotalVertexBufferBytesAllocated < s_iHALMemoryUsage )
		{
			s_iWastedVertexBufferBytesAllocated = s_iHALMemoryUsage - s_iTotalVertexBufferBytesAllocated;
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 006c0760
void TVertexBlockResource::DestroyHAL()
{
	TMemory::HALMemInfo memInfoHAL;
	TMemory::GetHALMemInfo( memInfoHAL );

	for ( TUINT i = 0; i < m_HALBuffer.uiNumStreams; i++ )
	{
		if ( m_HALBuffer.apVertexBuffers[ i ] )
		{
			m_HALBuffer.apVertexBuffers[ i ]->Release();
			m_HALBuffer.apVertexBuffers[ i ] = TNULL;
			s_iCurrentNumHALCreated -= 1;
			s_iTotalNumHALDestroyed += 1;
			s_iCurrentVertexBufferBytesAllocated -= m_pFactory->GetVertexFormat().m_aStreamFormats[ i ].m_uiVertexSize * m_uiMaxVertices;
		}
	}

	m_HALBuffer.uiNumStreams = 0;

	TMemory::GetHALMemInfo( memInfoHAL );
	s_iHALMemoryUsage = s_iHALMemoryUsage - memInfoHAL.m_uiMemUsage;

	if ( s_iTotalVertexBufferBytesAllocated < s_iHALMemoryUsage )
	{
		s_iWastedVertexBufferBytesAllocated = s_iHALMemoryUsage - s_iTotalVertexBufferBytesAllocated;
	}
}

// $Barnyard: FUNCTION 006c06b0
void TVertexBlockResource::ChildVertexUsedChanged( TINT a_iChange )
{
	if ( a_iChange < 0 )
	{
		TASSERT( m_uiVerticesUsed >= TUINT( -a_iChange ) );
	}

	m_uiVerticesUsed += a_iChange;
}

// $Barnyard: FUNCTION 006c0190
TBOOL TVertexBlockResource::TryInvalidate()
{
	if ( m_uiFlags & 1 )
	{
		Invalidate();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 006c01a0
TBOOL TVertexBlockResource::TryValidate()
{
	if ( m_uiFlags & 1 )
	{
		Validate();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 006c0ac0
TBOOL TVertexBlockResource::Validate()
{
	if ( IsValid() && IsCreated() )
	{
		return TTRUE;
	}

	if ( !CreateHAL() )
	{
		return TFALSE;
	}

	if ( m_uiFlags & 1 )
	{
		using Pair = T2Pair<TVertexBlockResource*, TVertexPoolResourceInterface::LockBuffer>;
		Pair pair;

		if ( Lock( &pair.second, 0 ) )
		{
			pair.first = this;

			TResource::RecurseSimple(
			    []( TResource* a_pResource, void* a_pUserData ) {
				    if ( a_pResource->IsExactly( &TGetClass( TVertexPoolResource ) ) )
				    {
					    auto pPool = TSTATICCAST( TVertexPoolResource, a_pResource );
					    auto pPair = TSTATICCAST( Pair, a_pUserData );

					    auto pFactory     = pPool->GetFactory();
					    auto vertexFormat = pFactory->GetVertexFormat();

					    if ( pPool->m_uiFlags & 1 )
					    {
						    pPool->m_uiVertexOffset = pPair->second.uiOffset;
						    pPair->second.uiOffset += pPool->GetNumVertices();

						    for ( TUINT i = 0; i < vertexFormat.GetNumStreams(); i++ )
						    {
							    auto uiVertexSize = vertexFormat.m_aStreamFormats[ i ].m_uiVertexSize;

							    TUtil::MemCopy(
							        pPair->second.apStreams[ i ] + pPool->m_uiVertexOffset * uiVertexSize,
							        pPool->GetManagedStream( i ),
							        pPool->GetNumVertices() * uiVertexSize
							    );
						    }
					    }
				    }

				    return TTRUE;
			    },
			    this,
			    &pair
			);

			Unlock();
		}
	}

	return TResource::Validate();
}

// $Barnyard: FUNCTION 006c0950
void TVertexBlockResource::Invalidate()
{
	if ( IsValid() && IsCreated() )
	{
		TASSERT( m_uiLockCount == 0 );

		DestroyHAL();
		TResource::Invalidate();
	}
}

// $Barnyard: FUNCTION 006c0810
TBOOL TVertexBlockResource::Lock( TVertexPoolResourceInterface::LockBuffer* a_pLockBuffer, TUINT16 a_uiNumVertices )
{
	TVALIDPTR( a_pLockBuffer );

	auto& vertexFormat          = m_pFactory->GetVertexFormat();
	a_pLockBuffer->uiNumStreams = vertexFormat.m_uiNumStreams;

	DWORD uiFlags;
	TUINT uiNumVertices = 0;
	TUINT uiUnk1        = m_uiFlags & 7;

	if ( uiUnk1 == 1 )
	{
		uiFlags                 = D3DLOCK_NOSYSLOCK;
		a_pLockBuffer->uiOffset = 0;
	}
	else
	{
		if ( uiUnk1 != 2 )
		{
			if ( uiUnk1 != 4 )
			{
				return TFALSE;
			}

			Validate();
			uiNumVertices = a_uiNumVertices;

			if ( m_uiMaxVertices < m_uiOffset + uiNumVertices )
			{
				uiFlags                 = D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK;
				a_pLockBuffer->uiOffset = 0;
				m_uiOffset              = uiNumVertices;
			}
			else
			{
				uiFlags                 = D3DLOCK_NOOVERWRITE | D3DLOCK_NOSYSLOCK;
				a_pLockBuffer->uiOffset = m_uiOffset;
				m_uiOffset += uiNumVertices;
			}
		}
		else
		{
			Validate();
			uiFlags                 = D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK;
			a_pLockBuffer->uiOffset = 0;
		}
	}

	for ( TUINT i = 0; i < a_pLockBuffer->uiNumStreams; i++ )
	{
		HRESULT hRes = m_HALBuffer.apVertexBuffers[ i ]->Lock(
		    a_pLockBuffer->uiOffset * vertexFormat.m_aStreamFormats[ i ].m_uiVertexSize,
		    uiNumVertices * vertexFormat.m_aStreamFormats[ i ].m_uiVertexSize,
		    &a_pLockBuffer->apStreams[ i ],
		    uiFlags
		);

		if ( FAILED( hRes ) )
		{
			TRenderD3DInterface::PrintError( hRes, "Couldn\'t lock stream vertex buffer" );
		}
	}

	m_uiLockCount += 1;
	return TTRUE;
}

// $Barnyard: FUNCTION 006c0620
void TVertexBlockResource::Unlock()
{
	TASSERT( 0 != m_uiLockCount );

	if ( m_uiLockCount > 0 )
	{
		for ( TUINT i = 0; i < m_pFactory->GetVertexFormat().m_uiNumStreams; i++ )
		{
			HRESULT hRes = m_HALBuffer.apVertexBuffers[ i ]->Unlock();

			if ( FAILED( hRes ) )
			{
				TRenderD3DInterface::PrintError( hRes, "Couldn\'t unlock stream vertex buffer!" );
			}
		}

		m_uiLockCount -= 1;
	}
}

// $Barnyard: FUNCTION 006c04d0
TVertexBlockResource::TVertexBlockResource()
{
	m_pFactory       = TNULL;
	m_uiFlags        = 0;
	m_uiMaxVertices  = 0;
	m_uiOffset       = 0;
	m_uiVerticesUsed = 0;
	m_uiLockCount    = 0;
	m_Unk1           = 0;
}

TBOOL TVertexBlockResource::GetHALBuffer( HALBuffer* a_pHALBuffer ) const
{
	TVALIDPTR( a_pHALBuffer );

	if ( a_pHALBuffer )
	{
		*a_pHALBuffer = m_HALBuffer;
		return TTRUE;
	}

	return TFALSE;
}

TVertexBlockResource::HALBuffer::HALBuffer()
{
	uiNumStreams         = 0;
	uiVertexOffset       = 0;
	apVertexBuffers[ 0 ] = TNULL;
	apVertexBuffers[ 1 ] = TNULL;
	apVertexBuffers[ 2 ] = TNULL;
	apVertexBuffers[ 3 ] = TNULL;
	apVertexBuffers[ 4 ] = TNULL;
	apVertexBuffers[ 5 ] = TNULL;
	apVertexBuffers[ 6 ] = TNULL;
	apVertexBuffers[ 7 ] = TNULL;
}

TOSHI_NAMESPACE_END
