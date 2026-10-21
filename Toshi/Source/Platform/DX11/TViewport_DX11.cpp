#include "ToshiPCH.h"
#include "Render/TViewport.h"
#include "TRenderInterface_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

void TViewport::BeginSKU()
{
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = TFLOAT( DWORD( m_pRenderCtx->GetViewportParameters().fX ) );
	viewport.TopLeftY = TFLOAT( DWORD( m_pRenderCtx->GetViewportParameters().fY ) );
	viewport.Width    = TFLOAT( DWORD( m_pRenderCtx->GetViewportParameters().fWidth ) );
	viewport.Height   = TFLOAT( DWORD( m_pRenderCtx->GetViewportParameters().fHeight ) );
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	TUINT8 r, g, b, a;
	GetBackgroundColor( r, g, b, a );

	auto pRender = TSTATICCAST( TRenderD3DInterface, TRenderInterface::GetSingleton() );

	pRender->GetDeviceContext()->RSSetViewports( 1, &viewport );

	TUINT eClearFlags = ( m_bAllowDepthClear == TTRUE ? TRenderD3DInterface::CLEARFLAGS_DEPTH : TRenderD3DInterface::CLEARFLAGS_NONE ) |
	    ( m_bAllowBackgroundClear == TTRUE ? TRenderD3DInterface::CLEARFLAGS_TARGET : TRenderD3DInterface::CLEARFLAGS_NONE );

	if ( eClearFlags != 0 )
	{
		pRender->ClearRegion(
		    DWORD( m_pRenderCtx->GetViewportParameters().fX ),
		    DWORD( m_pRenderCtx->GetViewportParameters().fY ),
		    DWORD( m_pRenderCtx->GetViewportParameters().fWidth ),
		    DWORD( m_pRenderCtx->GetViewportParameters().fHeight ),
		    eClearFlags,
		    r,
		    g,
		    b,
		    1.0f,
		    0
		);
	}
}

void TViewport::EndSKU()
{
	TRenderInterface::GetSingleton()->FlushOrderTables();
}

void TViewport::ChangeSKU( ChangeEvent a_eEvent )
{
}

TOSHI_NAMESPACE_END
