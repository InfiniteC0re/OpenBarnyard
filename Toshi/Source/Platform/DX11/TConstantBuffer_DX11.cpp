#include "ToshiPCH.h"
#include "TConstantBuffer_DX11.h"
#include "TRenderInterface_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TConstantBufferGeneric::TConstantBufferGeneric()
    : m_pBuffer( TNULL ), m_pData( TNULL ), m_uiSize( 0 ), m_bDirty( TFALSE )
{
}

TConstantBufferGeneric::~TConstantBufferGeneric()
{
	Destroy();
}

TBOOL TConstantBufferGeneric::Create( TUINT a_uiSize )
{
	Destroy();

	m_uiSize = TAlignNumUp( a_uiSize, 16 );
	TASSERT( m_uiSize != 0 && m_uiSize <= D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16 );

	m_pBuffer = TRenderD3DInterface::Interface()->CreateBuffer( D3D11_BIND_CONSTANT_BUFFER, m_uiSize, TNULL, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE );

	if ( !m_pBuffer )
	{
		m_uiSize = 0;
		return TFALSE;
	}

	m_pData = TSTATICCAST( TUINT8, TMemalign( m_uiSize, 16 ) );
	TUtil::MemClear( m_pData, m_uiSize );
	m_bDirty = TTRUE;

	return TTRUE;
}

void TConstantBufferGeneric::Destroy()
{
	if ( m_pBuffer )
	{
		if ( TRenderD3DInterface* pRender = TRenderD3DInterface::Interface() )
			pRender->UnbindConstantBuffer( this );

		m_pBuffer->Release();
		m_pBuffer = TNULL;
	}

	if ( m_pData )
	{
		TFree( m_pData );
		m_pData = TNULL;
	}

	m_uiSize = 0;
	m_bDirty = TFALSE;
}

void TConstantBufferGeneric::SetData( TUINT a_uiOffset, const void* a_pData, TUINT a_uiSize )
{
	TASSERT( a_uiOffset + a_uiSize <= m_uiSize );

	TUINT8* pDst = m_pData + a_uiOffset;

	if ( m_bDirty || TUtil::MemCompare( pDst, a_pData, a_uiSize ) != 0 )
	{
		TUtil::MemCopy( pDst, a_pData, a_uiSize );
		m_bDirty = TTRUE;
	}
}

void* TConstantBufferGeneric::Edit()
{
	m_bDirty = TTRUE;
	return m_pData;
}

void TConstantBufferGeneric::Upload()
{
	if ( !m_bDirty )
		return;

	ID3D11DeviceContext*     pDeviceContext = TRenderD3DInterface::Interface()->GetDeviceContext();
	D3D11_MAPPED_SUBRESOURCE oMapped;

	if ( SUCCEEDED( pDeviceContext->Map( m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &oMapped ) ) )
	{
		TUtil::MemCopy( oMapped.pData, m_pData, m_uiSize );
		pDeviceContext->Unmap( m_pBuffer, 0 );
		m_bDirty = TFALSE;
	}
}

TOSHI_NAMESPACE_END
