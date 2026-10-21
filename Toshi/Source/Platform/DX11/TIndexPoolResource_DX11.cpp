#include "ToshiPCH.h"
#include "TIndexPoolResource_DX11.h"
#include "TIndexBlockResource_DX11.h"
#include "TIndexFactoryResource_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TIndexPoolResource );

TIndexPoolResource::TIndexPoolResource()
{
	m_uiIndexOffset     = 0;
	m_pIndices          = TNULL;
	m_uiNumLocksAllTime = 0;
}

TIndexPoolResource::~TIndexPoolResource()
{
}

TBOOL TIndexPoolResource::Validate()
{
	if ( IsCreated() && IsValid() )
	{
		return TTRUE;
	}

	auto pFactory = TSTATICCAST( TIndexFactoryResource, GetFactory() );
	auto pBlock   = pFactory->FindBlockResource( this );

	if ( !pBlock )
	{
		TUINT32 uiFlags;
		TUINT16 uiMaxVertices;

		TUINT8 uiUsage = m_uiFlags & FLAGS_USAGE_MASK;

		if ( uiUsage == FLAGS_STATIC )
		{
			uiMaxVertices = pFactory->GetMaxStaticIndices();
			uiFlags       = FLAGS_STATIC;
		}
		else if ( uiUsage == FLAGS_DYNAMIC )
		{
			uiMaxVertices = GetMaxIndices();
			uiFlags       = FLAGS_DYNAMIC;
		}
		else
		{
			if ( uiUsage != FLAGS_DYNAMIC_APPEND )
			{
				return TFALSE;
			}

			uiMaxVertices = GetMaxIndices();
			uiFlags       = FLAGS_DYNAMIC_APPEND;
		}

		pBlock = pFactory->CreateBlockResource( uiMaxVertices, uiFlags );

		if ( !pBlock )
		{
			return TFALSE;
		}
	}

	pBlock->AttachPool( this );
	return TResource::Validate();
}

void TIndexPoolResource::Invalidate()
{
	if ( IsCreated() && IsValid() )
	{
		TASSERT( m_uiLockCount == 0 );

		auto pIndexBlock = GetIndexBlock();
		TVALIDPTR( pIndexBlock );

		pIndexBlock->DettachPool( this );
		TResource::Invalidate();
	}
}

void TIndexPoolResource::OnDestroy()
{
	if ( m_uiFlags & FLAGS_STATIC && m_pIndices )
	{
		delete[] m_pIndices;
		m_pIndices = TNULL;
	}

	TIndexPoolResourceInterface::OnDestroy();
}

TBOOL TIndexPoolResource::Lock( LockBuffer* a_pLockBuffer )
{
	TVALIDPTR( a_pLockBuffer );

	auto uiOldLockCount = m_uiLockCount;
	m_uiNumLocksAllTime += 1;
	m_uiLockCount += 1;

	if ( uiOldLockCount == 0 )
	{
		TUINT8 uiUsage = m_uiFlags & FLAGS_USAGE_MASK;

		if ( uiUsage == FLAGS_STATIC )
		{
			a_pLockBuffer->pBuffer = m_pIndices;
			return TTRUE;
		}
		else if ( uiUsage == FLAGS_DYNAMIC )
		{
			Validate();

			if ( GetIndexBlock()->Lock( a_pLockBuffer, 0 ) )
			{
				m_uiIndexOffset = a_pLockBuffer->uiOffset;
				return TTRUE;
			}
		}
		else if ( uiUsage == FLAGS_DYNAMIC_APPEND )
		{
			Validate();

			if ( GetIndexBlock()->Lock( a_pLockBuffer, GetMaxIndices() ) )
			{
				m_uiIndexOffset = a_pLockBuffer->uiOffset;
				return TTRUE;
			}
		}
	}

	return TFALSE;
}

void TIndexPoolResource::Unlock( TUINT16 a_uiNewNumIndices )
{
	TASSERT( m_uiLockCount > 0 );

	if ( m_uiLockCount > 0 && --m_uiLockCount == 0 )
	{
		TINT iChange = a_uiNewNumIndices - GetNumIndices();

		if ( iChange != 0 )
		{
			if ( GetParent() )
			{
				if ( GetParent()->GetClass()->IsA( &TGetClass( TIndexBlockResource ) ) )
				{
					auto pIndexBlock = TSTATICCAST( TIndexBlockResource, GetParent() );
					pIndexBlock->ChildIndexUsedChanged( iChange );
				}
			}
		}

		m_uiNumIndices = a_uiNewNumIndices;

		if ( m_uiFlags & ( FLAGS_DYNAMIC | FLAGS_DYNAMIC_APPEND ) )
		{
			GetIndexBlock()->Unlock();
		}
		else if ( m_uiFlags & FLAGS_STATIC )
		{
			Validate();
		}
	}
}

TBOOL TIndexPoolResource::Create( TIndexFactoryResourceInterface* a_pFactory, TUINT16 a_uiMaxIndices, TUINT16 a_uiFlags )
{
	if ( a_uiFlags & FLAGS_DYNAMIC_APPEND )
	{
		a_uiFlags = a_uiFlags & ~FLAGS_DYNAMIC_APPEND | FLAGS_DYNAMIC;
	}

	if ( TIndexPoolResourceInterface::Create( a_pFactory, a_uiMaxIndices, a_uiFlags ) )
	{
		if ( m_uiFlags & FLAGS_STATIC )
		{
			m_pIndices = new TIndexType[ a_uiMaxIndices ];
			TVALIDPTR( m_pIndices );
		}

		return TTRUE;
	}

	return TFALSE;
}

TResource* TIndexPoolResource::GetParent()
{
	if ( Tree() )
	{
		return Parent() == Tree()->GetRoot() ? TNULL : Parent();
	}

	return Parent();
}

TIndexBlockResource* TIndexPoolResource::GetIndexBlock()
{
	if ( Parent()->IsExactly( &TGetClass( TIndexBlockResource ) ) )
	{
		return TSTATICCAST( TIndexBlockResource, Parent() );
	}

	return TNULL;
}

TBOOL TIndexPoolResource::GetHALBuffer( TIndexBlockResource::HALBuffer* a_pHALBuffer )
{
	TVALIDPTR( a_pHALBuffer );

	if ( Validate() )
	{
		auto pIndexBlock = GetIndexBlock();
		TVALIDPTR( pIndexBlock );

		if ( pIndexBlock->Validate() && pIndexBlock->GetHALBuffer( a_pHALBuffer ) )
		{
			a_pHALBuffer->uiIndexOffset = m_uiIndexOffset;
			return TTRUE;
		}
	}

	return TFALSE;
}

TUINT16* TIndexPoolResource::GetIndices()
{
	return ( m_uiLockCount == 0 ) ? m_pIndices : TNULL;
}

TOSHI_NAMESPACE_END
