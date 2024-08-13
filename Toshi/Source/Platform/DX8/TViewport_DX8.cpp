#include "ToshiPCH.h"
#include "Render/TViewport.h"
#include "TRenderInterface_DX8.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

void TViewport::BeginSKU()
{
	D3DVIEWPORT8 viewport;
	viewport.X		= DWORD( m_pRenderCtx->GetViewportParameters().fX );
	viewport.Y		= DWORD( m_pRenderCtx->GetViewportParameters().fY );
	viewport.Width	= DWORD( m_pRenderCtx->GetViewportParameters().fWidth );
	viewport.Height = DWORD( m_pRenderCtx->GetViewportParameters().fHeight );
	viewport.MinZ	= 0.0f;
	viewport.MaxZ	= 1.0f;

	TUINT8 r, g, b, a;
	GetBackgroundColor( r, g, b, a );

	auto pRender	= TSTATICCAST( TRenderD3DInterface, TRenderInterface::GetSingleton() );
	auto pD3DDevice = pRender->GetDirect3DDevice();

	pD3DDevice->SetViewport( &viewport );

	TUINT eClearFlags = ( ( m_bAllowDepthClear != TTRUE ) - 1U & 2 | m_bAllowBackgroundClear == TTRUE );

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
			0 );
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
