#include "ToshiPCH.h"
#include "TVertexPoolResource_GL.h"
#include "TVertexBlockResource_GL.h"
#include "TVertexFactoryResource_GL.h"
#include "TRenderInterface_GL.h"
#include "Toshi/T2Pair.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TVertexBlockResource, 0, 1 );
TDEFINE_FREELIST_ALLOCATOR( TVertexBlockResource );

TBOOL TVertexBlockResource::AttachPool( TVertexPoolResource* a_pPool )
{
	TVALIDPTR( a_pPool );

	m_uiVerticesUsed += a_pPool->GetNumVertices();
	a_pPool->SetParent( this );

	if ( m_uiFlags & TVertexPoolResourceInterface::FLAGS_STATIC )
	{
		Invalidate();
	}

	return TTRUE;
}

TBOOL TVertexBlockResource::CanFit( TVertexPoolResource* a_pPoolResource ) const
{
	if ( m_uiFlags & TVertexPoolResourceInterface::FLAGS_STATIC && a_pPoolResource->GetFlags() & TVertexPoolResourceInterface::FLAGS_STATIC )
	{
		return m_uiMaxVertices >= a_pPoolResource->GetNumVertices() + m_uiVerticesUsed;
	}

	return TFALSE;
}

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

TBOOL TVertexBlockResource::Create( TVertexFactoryResourceInterface* a_pFactory, TUINT16 a_uiMaxVertices, TUINT32 a_uiFlags )
{
	m_pFactory      = a_pFactory;
	m_uiMaxVertices = a_uiMaxVertices;
	m_uiFlags       = a_uiFlags;
	return TResource::Create();
}

TBOOL TVertexBlockResource::CreateHAL()
{
	DestroyHAL();

	auto  pRenderer          = TRenderGLInterface::Interface();
	auto& vertexFormat       = m_pFactory->GetVertexFormat();
	m_HALBuffer.uiNumStreams = vertexFormat.m_uiNumStreams;

	for ( TUINT i = 0; i < m_HALBuffer.uiNumStreams; i++ )
	{
		UINT length = vertexFormat.m_aStreamFormats[ i ].m_uiVertexSize * m_uiMaxVertices;

		if ( ISZERO( m_uiFlags & TVertexPoolResourceInterface::FLAGS_STATIC ) )
		{
			m_uiOffset = 0;
		}

		m_HALBuffer.apVertexBuffers[ i ] = pRenderer->CreateBuffer( GL_ARRAY_BUFFER, length, TNULL, ( m_uiFlags & TVertexPoolResourceInterface::FLAGS_STATIC ) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW );

		if ( !m_HALBuffer.apVertexBuffers[ i ] )
		{
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

void TVertexBlockResource::DestroyHAL()
{
	TMemory::HALMemInfo memInfoHAL;
	TMemory::GetHALMemInfo( memInfoHAL );

	for ( TUINT i = 0; i < m_HALBuffer.uiNumStreams; i++ )
	{
		if ( m_HALBuffer.apVertexBuffers[ i ] )
		{
			TRenderGLInterface::Interface()->DestroyBuffer( m_HALBuffer.apVertexBuffers[ i ] );
			m_HALBuffer.apVertexBuffers[ i ] = 0;
			s_iCurrentNumHALCreated -= 1;
			s_iTotalNumHALDestroyed += 1;
			s_iCurrentVertexBufferBytesAllocated -= m_pFactory->GetVertexFormat().m_aStreamFormats[ i ].m_uiVertexSize * m_uiMaxVertices;
		}
	}

	m_HALBuffer.uiNumStreams = 0;

	for ( TUINT i = 0; i < TVertexFactoryFormat::MAX_NUM_STREAMS; i++ )
	{
		if ( m_apStaging[ i ] )
		{
			TFree( m_apStaging[ i ] );
			m_apStaging[ i ] = TNULL;
		}
	}

	TMemory::GetHALMemInfo( memInfoHAL );
	s_iHALMemoryUsage = s_iHALMemoryUsage - memInfoHAL.m_uiMemUsage;

	if ( s_iTotalVertexBufferBytesAllocated < s_iHALMemoryUsage )
	{
		s_iWastedVertexBufferBytesAllocated = s_iHALMemoryUsage - s_iTotalVertexBufferBytesAllocated;
	}
}

void TVertexBlockResource::ChildVertexUsedChanged( TINT a_iChange )
{
	if ( a_iChange < 0 )
	{
		TASSERT( m_uiVerticesUsed >= TUINT( -a_iChange ) );
	}

	m_uiVerticesUsed += a_iChange;
}

TBOOL TVertexBlockResource::TryInvalidate()
{
	if ( !( m_uiFlags & TVertexPoolResourceInterface::FLAGS_STATIC ) )
	{
		Invalidate();
	}

	return TTRUE;
}

TBOOL TVertexBlockResource::TryValidate()
{
	if ( !( m_uiFlags & TVertexPoolResourceInterface::FLAGS_STATIC ) )
	{
		Validate();
	}

	return TTRUE;
}

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

	if ( m_uiFlags & TVertexPoolResourceInterface::FLAGS_STATIC )
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

					    if ( pPool->m_uiFlags & TVertexPoolResourceInterface::FLAGS_STATIC )
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

void TVertexBlockResource::Invalidate()
{
	if ( IsValid() && IsCreated() )
	{
		TASSERT( m_uiLockCount == 0 );

		DestroyHAL();
		TResource::Invalidate();
	}
}

TBOOL TVertexBlockResource::Lock( TVertexPoolResourceInterface::LockBuffer* a_pLockBuffer, TUINT16 a_uiNumVertices )
{
	TVALIDPTR( a_pLockBuffer );

	auto& vertexFormat          = m_pFactory->GetVertexFormat();
	a_pLockBuffer->uiNumStreams = vertexFormat.m_uiNumStreams;

	TUINT uiNumVertices = 0;
	TUINT uiUsage       = m_uiFlags & TVertexPoolResourceInterface::FLAGS_USAGE_MASK;

	if ( uiUsage == TVertexPoolResourceInterface::FLAGS_STATIC )
	{
		m_bLockDiscard          = TTRUE;
		a_pLockBuffer->uiOffset = 0;
	}
	else
	{
		if ( uiUsage != TVertexPoolResourceInterface::FLAGS_DYNAMIC )
		{
			if ( uiUsage != TVertexPoolResourceInterface::FLAGS_DYNAMIC_APPEND )
			{
				return TFALSE;
			}

			Validate();
			uiNumVertices = a_uiNumVertices;

			if ( m_uiMaxVertices < m_uiOffset + uiNumVertices )
			{
				m_bLockDiscard          = TTRUE;
				a_pLockBuffer->uiOffset = 0;
				m_uiOffset              = uiNumVertices;
			}
			else
			{
				m_bLockDiscard          = TFALSE;
				a_pLockBuffer->uiOffset = m_uiOffset;
				m_uiOffset += uiNumVertices;
			}
		}
		else
		{
			Validate();
			m_bLockDiscard          = TTRUE;
			a_pLockBuffer->uiOffset = 0;
		}
	}

	m_uiLockOffset = a_pLockBuffer->uiOffset;

	// ES 3.0 can't be relied on to map buffers, the vertices are sent on Unlock
	for ( TUINT i = 0; i < a_pLockBuffer->uiNumStreams; i++ )
	{
		const TUINT uiVertexSize = vertexFormat.m_aStreamFormats[ i ].m_uiVertexSize;

		if ( !m_apStaging[ i ] )
			m_apStaging[ i ] = TSTATICCAST( TUINT8, TMalloc( uiVertexSize * m_uiMaxVertices ) );

		a_pLockBuffer->apStreams[ i ] = m_apStaging[ i ] + a_pLockBuffer->uiOffset * uiVertexSize;
	}

	m_uiLockCount += 1;
	return TTRUE;
}

void TVertexBlockResource::Unlock( TUINT a_uiNumVertices )
{
	TASSERT( 0 != m_uiLockCount );

	if ( m_uiLockCount > 0 )
	{
		auto  pRenderer    = TRenderGLInterface::Interface();
		auto& vertexFormat = m_pFactory->GetVertexFormat();

		const TBOOL bStatic       = HASANYFLAG( m_uiFlags, TVertexPoolResourceInterface::FLAGS_STATIC );
		const TUINT uiNumVertices = TMath::Min( a_uiNumVertices, TUINT( m_uiMaxVertices ) - m_uiLockOffset );

		for ( TUINT i = 0; i < vertexFormat.m_uiNumStreams; i++ )
		{
			const TUINT uiVertexSize = vertexFormat.m_aStreamFormats[ i ].m_uiVertexSize;

			pRenderer->UpdateBuffer(
			    m_HALBuffer.apVertexBuffers[ i ],
			    GL_ARRAY_BUFFER,
			    m_uiLockOffset * uiVertexSize,
			    uiNumVertices * uiVertexSize,
			    m_apStaging[ i ] + m_uiLockOffset * uiVertexSize,
			    ( m_bLockDiscard && !bStatic ) ? uiVertexSize * m_uiMaxVertices : 0
			);

			if ( bStatic )
			{
				TFree( m_apStaging[ i ] );
				m_apStaging[ i ] = TNULL;
			}
		}

		m_uiLockCount -= 1;
	}
}

TVertexBlockResource::TVertexBlockResource()
{
	m_pFactory       = TNULL;
	m_uiFlags        = 0;
	m_uiMaxVertices  = 0;
	m_uiOffset       = 0;
	m_uiVerticesUsed = 0;
	m_uiLockCount    = 0;
	m_Unk1           = 0;
	m_uiLockOffset   = 0;
	m_bLockDiscard   = TFALSE;

	for ( TUINT i = 0; i < TVertexFactoryFormat::MAX_NUM_STREAMS; i++ )
		m_apStaging[ i ] = TNULL;
}

TVertexBlockResource::~TVertexBlockResource()
{
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
	uiNumStreams   = 0;
	uiVertexOffset = 0;

	for ( TUINT i = 0; i < TVertexFactoryFormat::MAX_NUM_STREAMS; i++ )
		apVertexBuffers[ i ] = 0;
}

TOSHI_NAMESPACE_END
