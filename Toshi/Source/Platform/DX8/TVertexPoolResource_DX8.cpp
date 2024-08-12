#include "ToshiPCH.h"
#include "TVertexPoolResource_DX8.h"
#include "TVertexFactoryResource_DX8.h"
#include "TVertexBlockResource_DX8.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi
{

TDEFINE_CLASS( TVertexPoolResource );

TVertexPoolResource::TVertexPoolResource()
{
    m_uiVertexOffset        = 0;
    m_uiNumLocksAllTime     = 0;
    m_apManagedStreams[ 0 ] = TNULL;
    m_apManagedStreams[ 1 ] = TNULL;
    m_apManagedStreams[ 2 ] = TNULL;
    m_apManagedStreams[ 3 ] = TNULL;
    m_apManagedStreams[ 4 ] = TNULL;
    m_apManagedStreams[ 5 ] = TNULL;
    m_apManagedStreams[ 6 ] = TNULL;
    m_apManagedStreams[ 7 ] = TNULL;
}

TBOOL TVertexPoolResource::Validate()
{
    if ( IsCreated() && IsValid() )
    {
        return TTRUE;
    }

    auto pFactory = TSTATICCAST( TVertexFactoryResource, GetFactory() );
    auto pBlock   = pFactory->FindBlockResource( this );

    if ( !pBlock )
    {
        TUINT32 uiFlags;
        TUINT16 uiMaxVertices;

        TUINT8 uiUnk1 = m_uiFlags & 7;

        if ( uiUnk1 == 1 )
        {
            uiMaxVertices = pFactory->GetMaxStaticVertices();
            uiFlags       = 1;
        }
        else if ( uiUnk1 == 2 )
        {
            uiMaxVertices = GetMaxVertices();
            uiFlags       = 2;
        }
        else
        {
            if ( uiUnk1 != 4 )
            {
                return TFALSE;
            }

            uiMaxVertices = GetMaxVertices();
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

void TVertexPoolResource::Invalidate()
{
    if ( IsCreated() && IsValid() )
    {
        TASSERT( m_uiLockCount == 0 );

        auto pVertexBlock = GetVertexBlock();
        TVALIDPTR( pVertexBlock );

        pVertexBlock->DettachPool( this );
        TResource::Invalidate();
    }
}

void TVertexPoolResource::OnDestroy()
{
    if ( m_uiFlags & 1 )
    {
        auto& vertexFormat = m_pFactory->GetVertexFormat();

        for ( TUINT i = 0; i < vertexFormat.m_uiNumStreams; i++ )
        {
            if ( m_apManagedStreams[ i ] )
            {
                delete[] m_apManagedStreams[ i ];
                m_apManagedStreams[ i ] = TNULL;
            }
        }
    }

    TVertexPoolResourceInterface::OnDestroy();
}

TBOOL TVertexPoolResource::Lock( LockBuffer* a_pLockBuffer )
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
            a_pLockBuffer->uiNumStreams = GetFactory()->GetVertexFormat().GetNumStreams();

            for ( TUINT i = 0; i < a_pLockBuffer->uiNumStreams; i++ )
            {
                a_pLockBuffer->apStreams[ i ] = m_apManagedStreams[ i ];
            }

            return TTRUE;
        }
        else if ( uiUnk1 == 2 )
        {
            Validate();

            if ( GetVertexBlock()->Lock( a_pLockBuffer, 0 ) )
            {
                m_uiVertexOffset = a_pLockBuffer->uiOffset;
                return TTRUE;
            }
        }
        else if ( uiUnk1 == 4 )
        {
            Validate();

            if ( GetVertexBlock()->Lock( a_pLockBuffer, GetMaxVertices() ) )
            {
                m_uiVertexOffset = a_pLockBuffer->uiOffset;
                return TTRUE;
            }
        }
    }

    return TFALSE;
}

void TVertexPoolResource::Unlock( TUINT16 a_uiNewNumVertices )
{
    TASSERT( m_uiLockCount > 0 );

    if ( m_uiLockCount > 0 && --m_uiLockCount == 0 )
    {
        TINT iChange = a_uiNewNumVertices - GetNumVertices();

        if ( iChange != 0 )
        {
            if ( GetParent() )
            {
                if ( GetParent()->GetClass()->IsA( &TGetClass( TVertexBlockResource ) ) )
                {
                    auto pVertexBlock = TSTATICCAST( TVertexBlockResource, GetParent() );
                    pVertexBlock->ChildVertexUsedChanged( iChange );
                }
            }
        }

        m_uiNumVertices = a_uiNewNumVertices;

        if ( m_uiFlags & 6 )
        {
            GetVertexBlock()->Unlock();
        }
        else if ( m_uiFlags & 1 )
        {
            Validate();
        }
    }
}

TBOOL TVertexPoolResource::Create( TVertexFactoryResourceInterface* a_pFactory, TUINT16 a_uiMaxVertices, TUINT16 a_uiFlags )
{
    if ( a_uiFlags & 4 )
    {
        a_uiFlags = a_uiFlags & 0xfffb | 2;
    }

    if ( TVertexPoolResourceInterface::Create( a_pFactory, a_uiMaxVertices, a_uiFlags ) )
    {
        if ( m_uiFlags & 1 )
        {
            auto& vertexFormat = m_pFactory->GetVertexFormat();

            for ( TUINT i = 0; i < vertexFormat.m_uiNumStreams; i++ )
            {
                m_apManagedStreams[ i ] = new TBYTE[ a_uiMaxVertices * vertexFormat.m_aStreamFormats[ i ].m_uiVertexSize ];
                TVALIDPTR( m_apManagedStreams[ i ] );
            }
        }

        return TTRUE;
    }

    return TFALSE;
}

TBYTE* TVertexPoolResource::GetManagedStream( TUINT a_uiStream )
{
    TASSERT( a_uiStream < m_pFactory->GetVertexFormat().GetNumStreams() );

    if ( !IsLocked() && a_uiStream < m_pFactory->GetVertexFormat().GetNumStreams() )
    {
        return m_apManagedStreams[ a_uiStream ];
    }

    return TNULL;
}

TVertexBlockResource* TVertexPoolResource::GetVertexBlock()
{
    if ( Parent()->IsExactly( &TGetClass( TVertexBlockResource ) ) )
    {
        return TSTATICCAST( TVertexBlockResource, Parent() );
    }

    return TNULL;
}

TResource* TVertexPoolResource::GetParent()
{
    if ( Tree() )
    {
        return Parent() == Tree()->GetRoot() ? TNULL : Parent();
    }

    return Parent();
}

TBOOL TVertexPoolResource::GetHALBuffer( TVertexBlockResource::HALBuffer* a_pHALBuffer )
{
    TVALIDPTR( a_pHALBuffer );

    if ( Validate() )
    {
        auto pVertexBlock = GetVertexBlock();
        TVALIDPTR( pVertexBlock );

        if ( pVertexBlock->Validate() && pVertexBlock->GetHALBuffer( a_pHALBuffer ) )
        {
            a_pHALBuffer->uiVertexOffset = m_uiVertexOffset;
            return TTRUE;
        }
    }

    return TFALSE;
}

} // namespace Toshi
