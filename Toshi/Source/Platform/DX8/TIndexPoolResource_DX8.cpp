#include "ToshiPCH.h"
#include "TIndexPoolResource_DX8.h"
#include "TIndexBlockResource_DX8.h"
#include "TIndexFactoryResource_DX8.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TIndexPoolResource );

// $Barnyard: FUNCTION 006d5ef0
TIndexPoolResource::TIndexPoolResource()
{
	m_uiIndexOffset     = 0;
	m_pIndices          = TNULL;
	m_uiNumLocksAllTime = 0;
}

// $Barnyard: FUNCTION 006d5f70
TIndexPoolResource::~TIndexPoolResource()
{
}

// $Barnyard: FUNCTION 006d5e50
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

		TUINT8 uiUnk1 = m_uiFlags & 7;

		if ( uiUnk1 == 1 )
		{
			uiMaxVertices = pFactory->GetMaxStaticIndices();
			uiFlags       = 1;
		}
		else if ( uiUnk1 == 2 )
		{
			uiMaxVertices = GetMaxIndices();
			uiFlags       = 2;
		}
		else
		{
			if ( uiUnk1 != 4 )
			{
				return TFALSE;
			}

			uiMaxVertices = GetMaxIndices();
			uiFlags       = 4;
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

// $Barnyard: FUNCTION 006d5fe0
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

// $Barnyard: FUNCTION 006d5e20
void TIndexPoolResource::OnDestroy()
{
	if ( m_uiFlags & 1 && m_pIndices )
	{
		delete[] m_pIndices;
		m_pIndices = TNULL;
	}

	TIndexPoolResourceInterface::OnDestroy();
}

// $Barnyard: FUNCTION 006d6040
TBOOL TIndexPoolResource::Lock( LockBuffer* a_pLockBuffer )
{
	TVALIDPTR( a_pLockBuffer );

	auto uiOldLockCount = m_uiLockCount;
	m_uiNumLocksAllTime += 1;
	m_uiLockCount += 1;

	if ( uiOldLockCount == 0 )
	{
		TUINT8 uiUnk1 = m_uiFlags & 7;

		if ( uiUnk1 == 1 )
		{
			a_pLockBuffer->pBuffer = m_pIndices;
			return TTRUE;
		}
		else if ( uiUnk1 == 2 )
		{
			Validate();

			if ( GetIndexBlock()->Lock( a_pLockBuffer, 0 ) )
			{
				m_uiIndexOffset = a_pLockBuffer->uiOffset;
				return TTRUE;
			}
		}
		else if ( uiUnk1 == 4 )
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

// $Barnyard: FUNCTION 006d60f0
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

		if ( m_uiFlags & 6 )
		{
			GetIndexBlock()->Unlock();
		}
		else if ( m_uiFlags & 1 )
		{
			Validate();
		}
	}
}

// $Barnyard: FUNCTION 006d5dd0
TBOOL TIndexPoolResource::Create( TIndexFactoryResourceInterface* a_pFactory, TUINT16 a_uiMaxIndices, TUINT16 a_uiFlags )
{
	if ( a_uiFlags & 4 )
	{
		a_uiFlags = a_uiFlags & 0xfffb | 2;
	}

	if ( TIndexPoolResourceInterface::Create( a_pFactory, a_uiMaxIndices, a_uiFlags ) )
	{
		if ( m_uiFlags & 1 )
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

// $Barnyard: FUNCTION 006d5fa0
TIndexBlockResource* TIndexPoolResource::GetIndexBlock()
{
	if ( Parent()->IsExactly( &TGetClass( TIndexBlockResource ) ) )
	{
		return TSTATICCAST( TIndexBlockResource, Parent() );
	}

	return TNULL;
}

// $Barnyard: FUNCTION 006d6180
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

// $Barnyard: FUNCTION 006d5ee0
TUINT16* TIndexPoolResource::GetIndices()
{
	return ( m_uiLockCount == 0 ) ? m_pIndices : TNULL;
}

TOSHI_NAMESPACE_END
