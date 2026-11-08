#include "ToshiPCH.h"
#include "Render/TViewport.h"
#include "TRenderInterface_GL.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

void TViewport::BeginSKU()
{
	TUINT8 r, g, b, a;
	GetBackgroundColor( r, g, b, a );

	auto pRender = TSTATICCAST( TRenderGLInterface, TRenderInterface::GetSingleton() );

	pRender->SetViewport(
	    TINT( m_pRenderCtx->GetViewportParameters().fX ),
	    TINT( m_pRenderCtx->GetViewportParameters().fY ),
	    TINT( m_pRenderCtx->GetViewportParameters().fWidth ),
	    TINT( m_pRenderCtx->GetViewportParameters().fHeight )
	);

	TUINT eClearFlags = ( m_bAllowDepthClear == TTRUE ? TRenderGLInterface::CLEARFLAGS_DEPTH : TRenderGLInterface::CLEARFLAGS_NONE ) |
	    ( m_bAllowBackgroundClear == TTRUE ? TRenderGLInterface::CLEARFLAGS_TARGET : TRenderGLInterface::CLEARFLAGS_NONE );

	if ( eClearFlags != 0 )
	{
		pRender->ClearRegion(
		    TINT( m_pRenderCtx->GetViewportParameters().fX ),
		    TINT( m_pRenderCtx->GetViewportParameters().fY ),
		    TINT( m_pRenderCtx->GetViewportParameters().fWidth ),
		    TINT( m_pRenderCtx->GetViewportParameters().fHeight ),
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
