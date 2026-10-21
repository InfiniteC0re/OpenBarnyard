#include "ToshiPCH.h"
#include "TIndexPoolResource_DX11.h"
#include "TIndexBlockResource_DX11.h"
#include "TIndexFactoryResource_DX11.h"
#include "TRenderInterface_DX11.h"
#include "Toshi/T2Pair.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TIndexBlockResource );
TDEFINE_FREELIST_ALLOCATOR( TIndexBlockResource );

TIndexBlockResource::TIndexBlockResource()
{
	m_pFactory      = TNULL;
	m_uiFlags       = 0;
	m_uiMaxIndices  = 0;
	m_uiOffset      = 0;
	m_uiIndicesUsed = 0;
	m_uiLockCount   = 0;
	m_Unk1          = 0;
}

//TIndexBlockResource::~TIndexBlockResource()
//{
//}

TBOOL TIndexBlockResource::AttachPool( TIndexPoolResource* a_pPool )
{
	TVALIDPTR( a_pPool );

	m_uiIndicesUsed += a_pPool->GetNumIndices();
	a_pPool->SetParent( this );

	if ( m_uiFlags & TIndexPoolResourceInterface::FLAGS_STATIC )
	{
		Invalidate();
	}

	return TTRUE;
}

TBOOL TIndexBlockResource::DettachPool( TIndexPoolResource* a_pPool )
{
	TASSERT( TTRUE == Parent()->IsExactly( &TGetClass( TIndexFactoryResource ) ) );
	a_pPool->SetParent( Parent() == Tree()->GetRoot() ? TNULL : Parent() );

	m_uiIndicesUsed -= a_pPool->GetNumIndices();

	if ( m_uiIndicesUsed == 0 && Child() == TNULL )
	{
		GetRenderer()->DestroyResource( this );
		return TTRUE;
	}

	return TFALSE;
}

TBOOL TIndexBlockResource::Validate()
{
	if ( IsValid() && IsCreated() )
	{
		return TTRUE;
	}

	if ( !CreateHAL() )
	{
		return TFALSE;
	}

	if ( m_uiFlags & TIndexPoolResourceInterface::FLAGS_STATIC )
	{
		using Pair = T2Pair<TIndexBlockResource*, TIndexPoolResourceInterface::LockBuffer>;
		Pair pair;

		if ( Lock( &pair.second, 0 ) )
		{
			pair.first = this;

			TResource::RecurseSimple(
			    []( TResource* a_pResource, void* a_pUserData ) {
				    if ( a_pResource->IsExactly( &TGetClass( TIndexPoolResource ) ) )
				    {
					    auto pPool = TSTATICCAST( TIndexPoolResource, a_pResource );
					    auto pPair = TSTATICCAST( Pair, a_pUserData );

					    if ( pPool->m_uiFlags & TIndexPoolResourceInterface::FLAGS_STATIC )
					    {
						    pPool->m_uiIndexOffset = pPair->second.uiOffset;
						    pPair->second.uiOffset += pPool->GetNumIndices();

						    TUtil::MemCopy(
						        pPair->second.pBuffer + pPool->m_uiIndexOffset,
						        pPool->GetIndices(),
						        pPool->GetNumIndices() * sizeof( TIndexType )
						    );
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

void TIndexBlockResource::Invalidate()
{
	if ( IsValid() && IsCreated() )
	{
		TASSERT( m_uiLockCount == 0 );

		DestroyHAL();
		TResource::Invalidate();
	}
}

// NOTE: folded with TVertexBlockResource::TryInvalidate (006c0190)
TBOOL TIndexBlockResource::TryInvalidate()
{
	if ( !( m_uiFlags & TIndexPoolResourceInterface::FLAGS_STATIC ) )
	{
		Invalidate();
	}

	return TTRUE;
}

// NOTE: folded with TVertexBlockResource::TryValidate (006c01a0)
TBOOL TIndexBlockResource::TryValidate()
{
	if ( !( m_uiFlags & TIndexPoolResourceInterface::FLAGS_STATIC ) )
	{
		Validate();
	}

	return TTRUE;
}

TBOOL TIndexBlockResource::CreateHAL()
{
	DestroyHAL();

	TMemory::HALMemInfo memInfoHAL;
	TMemory::GetHALMemInfo( memInfoHAL );

	if ( ISZERO( m_uiFlags & TIndexPoolResourceInterface::FLAGS_STATIC ) )
	{
		m_uiOffset = 0;
	}

	m_HALBuffer.pIndexBuffer = TRenderD3DInterface::Interface()->CreateBuffer( D3D11_BIND_INDEX_BUFFER, m_uiMaxIndices * sizeof( TIndexType ), TNULL, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE );

	if ( !m_HALBuffer.pIndexBuffer )
	{
		return TFALSE;
	}

	TMemory::GetHALMemInfo( memInfoHAL );
	s_iHALMemoryUsage += memInfoHAL.m_uiMemUsage;

	return TTRUE;
}

void TIndexBlockResource::DestroyHAL()
{
	TMemory::HALMemInfo memInfoHAL;
	TMemory::GetHALMemInfo( memInfoHAL );

	if ( m_HALBuffer.pIndexBuffer )
	{
		m_HALBuffer.pIndexBuffer->Release();
		m_HALBuffer.pIndexBuffer = TNULL;
	}

	TMemory::GetHALMemInfo( memInfoHAL );
	s_iHALMemoryUsage = s_iHALMemoryUsage - memInfoHAL.m_uiMemUsage;
}

TBOOL TIndexBlockResource::Lock( TIndexPoolResourceInterface::LockBuffer* a_pLockBuffer, TUINT16 a_uiNumIndices )
{
	TVALIDPTR( a_pLockBuffer );

	D3D11_MAP eMapType;
	TUINT     uiNumIndices = 0;
	TUINT     uiUsage      = m_uiFlags & TIndexPoolResourceInterface::FLAGS_USAGE_MASK;

	if ( uiUsage == TIndexPoolResourceInterface::FLAGS_STATIC )
	{
		eMapType                = D3D11_MAP_WRITE_DISCARD;
		a_pLockBuffer->uiOffset = 0;
	}
	else
	{
		if ( uiUsage != TIndexPoolResourceInterface::FLAGS_DYNAMIC )
		{
			if ( uiUsage != TIndexPoolResourceInterface::FLAGS_DYNAMIC_APPEND )
			{
				return TFALSE;
			}

			Validate();
			uiNumIndices = a_uiNumIndices;

			if ( m_uiMaxIndices < m_uiOffset + uiNumIndices )
			{
				eMapType                = D3D11_MAP_WRITE_DISCARD;
				a_pLockBuffer->uiOffset = 0;
				m_uiOffset              = uiNumIndices;
			}
			else
			{
				eMapType                = D3D11_MAP_WRITE_NO_OVERWRITE;
				a_pLockBuffer->uiOffset = m_uiOffset;
				m_uiOffset += uiNumIndices;
			}
		}
		else
		{
			Validate();
			eMapType                = D3D11_MAP_WRITE_DISCARD;
			a_pLockBuffer->uiOffset = 0;
		}
	}

	D3D11_MAPPED_SUBRESOURCE oMapped;
	HRESULT                  hRes = TRenderD3DInterface::Interface()->GetDeviceContext()->Map( m_HALBuffer.pIndexBuffer, 0, eMapType, 0, &oMapped );

	if ( FAILED( hRes ) )
	{
		TRenderD3DInterface::PrintError( hRes, "Couldn't lock stream index buffer" );
		a_pLockBuffer->pBuffer = TNULL;
	}
	else
	{
		a_pLockBuffer->pBuffer = TSTATICCAST( TIndexType, oMapped.pData ) + a_pLockBuffer->uiOffset;
	}

	m_uiLockCount += 1;
	return TTRUE;
}

void TIndexBlockResource::Unlock()
{
	TASSERT( 0 != m_uiLockCount );

	if ( m_uiLockCount > 0 )
	{
		TRenderD3DInterface::Interface()->GetDeviceContext()->Unmap( m_HALBuffer.pIndexBuffer, 0 );
		m_uiLockCount -= 1;
	}
}

TBOOL TIndexBlockResource::Create( TIndexFactoryResourceInterface* a_pFactory, TUINT16 a_uiMaxIndices, TUINT32 a_uiFlags )
{
	m_pFactory     = a_pFactory;
	m_uiMaxIndices = a_uiMaxIndices;
	m_uiFlags      = a_uiFlags;
	return TResource::Create();
}

TBOOL TIndexBlockResource::CanFit( TIndexPoolResource* a_pPoolResource ) const
{
	if ( !HASANYFLAG( m_uiFlags, TIndexPoolResourceInterface::FLAGS_STATIC ) )
	{
		if ( HASANYFLAG( m_uiFlags, TIndexPoolResourceInterface::FLAGS_DYNAMIC_APPEND ) && HASANYFLAG( a_pPoolResource->m_uiFlags, TIndexPoolResourceInterface::FLAGS_DYNAMIC_APPEND ) )
		{
			return TTRUE;
		}
	}
	else if ( HASANYFLAG( a_pPoolResource->m_uiFlags, TIndexPoolResourceInterface::FLAGS_STATIC ) )
	{
		return m_uiMaxIndices > a_pPoolResource->GetNumIndices() + m_uiIndicesUsed;
	}

	return TFALSE;
}

void TIndexBlockResource::ChildIndexUsedChanged( TINT a_iChange )
{
	if ( a_iChange < 0 )
	{
		TASSERT( m_uiIndicesUsed >= TUINT( -a_iChange ) );
	}

	m_uiIndicesUsed += a_iChange;
}

TBOOL TIndexBlockResource::GetHALBuffer( HALBuffer* a_pHALBuffer ) const
{
	TVALIDPTR( a_pHALBuffer );

	if ( a_pHALBuffer )
	{
		*a_pHALBuffer = m_HALBuffer;
		return TTRUE;
	}

	return TFALSE;
}

TOSHI_NAMESPACE_END
