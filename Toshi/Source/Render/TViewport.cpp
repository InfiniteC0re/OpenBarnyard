#include "ToshiPCH.h"
#include "Render/TViewport.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TViewport::TViewport( TBOOL a_bFlag )
{
	m_MemAllocatorBlock		= TNULL;
	m_pRenderCtx			= TRenderInterface::GetSingleton()->CreateRenderContext();
	m_ColorR				= 0;
	m_ColorG				= 0;
	m_ColorB				= 0;
	m_ColorA				= 0;
	m_bAllowBackgroundClear = TTRUE;
	m_bAllowDepthClear		= TTRUE;
	m_bDefaultBeginRender	= TTRUE;
	m_bEnable				= TTRUE;
	m_bInViewport			= TFALSE;

	SetWidth( -1.0f );
	SetHeight( -1.0f );

	if ( a_bFlag )
	{
		TRenderInterface::GetSingleton()->Unknown1( this );
	}
}

TViewport::~TViewport()
{
	if ( m_pRenderCtx )
	{
		delete m_pRenderCtx;
		m_pRenderCtx = TNULL;
	}
}

void TViewport::SetBackgroundColor( TUINT8 r, TUINT8 g, TUINT8 b, TUINT8 a )
{
	if ( m_ColorR != r || m_ColorG != g || m_ColorB != b || m_ColorA != a )
	{
		m_ColorR = r;
		m_ColorG = g;
		m_ColorB = b;
		m_ColorA = a;

		ChangeSKU( ChangeEvent_BackgroundColor );
	}
}

void TViewport::GetBackgroundColor( TUINT8& r, TUINT8& g, TUINT8& b, TUINT8& a )
{
	r = m_ColorR;
	g = m_ColorG;
	b = m_ColorB;
	a = m_ColorA;
}

TRenderContext* TViewport::GetRenderContext() const
{
	return m_pRenderCtx;
}

void TViewport::SetMaxZ( TFLOAT value )
{
	TRenderContext::VIEWPORTPARAMS params = m_pRenderCtx->GetViewportParameters();

	if ( params.fMaxZ != value )
	{
		params.fMaxZ = value;
		m_pRenderCtx->SetViewportParameters( params );
		ChangeSKU( ChangeEvent_MaxZ );
	}
}

void TViewport::SetMinZ( TFLOAT value )
{
	TRenderContext::VIEWPORTPARAMS params = m_pRenderCtx->GetViewportParameters();

	if ( params.fMinZ != value )
	{
		params.fMinZ = value;
		m_pRenderCtx->SetViewportParameters( params );
		ChangeSKU( ChangeEvent_MinZ );
	}
}

void TViewport::SetHeight( TFLOAT value )
{
	TRenderContext::VIEWPORTPARAMS params = m_pRenderCtx->GetViewportParameters();

	if ( params.fHeight != value )
	{
		params.fHeight = value;
		m_pRenderCtx->SetViewportParameters( params );
		ChangeSKU( ChangeEvent_Height );
	}
}

void TViewport::SetWidth( TFLOAT value )
{
	TRenderContext::VIEWPORTPARAMS params = m_pRenderCtx->GetViewportParameters();

	if ( params.fWidth != value )
	{
		params.fWidth = value;
		m_pRenderCtx->SetViewportParameters( params );
		ChangeSKU( ChangeEvent_Width );
	}
}

void TViewport::SetY( TFLOAT value )
{
	TRenderContext::VIEWPORTPARAMS params = m_pRenderCtx->GetViewportParameters();

	if ( params.fY != value )
	{
		params.fY = value;
		m_pRenderCtx->SetViewportParameters( params );
		ChangeSKU( ChangeEvent_Y );
	}
}

void TViewport::SetX( TFLOAT value )
{
	TRenderContext::VIEWPORTPARAMS params = m_pRenderCtx->GetViewportParameters();

	if ( params.fX != value )
	{
		params.fX = value;
		m_pRenderCtx->SetViewportParameters( params );
		ChangeSKU( ChangeEvent_X );
	}
}

void TViewport::SetMemoryAllocatorBlock( TMemory::MemBlock* a_pMemBlock )
{
	m_MemAllocatorBlock = a_pMemBlock;
	ChangeSKU( ChangeEvent_AllocatorBlock );
}

TBOOL TViewport::EnableDefaultBeginRender( TBOOL enable )
{
	TBOOL oldState		  = m_bDefaultBeginRender;
	m_bDefaultBeginRender = enable;
	return oldState;
}

void TViewport::Enable( TBOOL enable )
{
	m_bEnable = enable;
}

TBOOL TViewport::AllowDepthClear( TBOOL allow )
{
	TBOOL oldState	   = m_bAllowDepthClear;
	m_bAllowDepthClear = allow;
	ChangeSKU( ChangeEvent_DepthClear );
	return oldState;
}

TBOOL TViewport::AllowBackgroundClear( TBOOL allow )
{
	TBOOL oldState			= m_bAllowBackgroundClear;
	m_bAllowBackgroundClear = allow;
	ChangeSKU( ChangeEvent_BackgroundClear );
	return oldState;
}

void TViewport::End()
{
	EndSKU();
	TASSERT( m_bInViewport == TTRUE );
	m_bInViewport = TFALSE;
}

void TViewport::Begin()
{
	TASSERT( m_bInViewport == TFALSE );
	m_bInViewport = TTRUE;
	BeginSKU();
}

TFLOAT TViewport::GetHeight() const
{
	return m_pRenderCtx->GetHeight();
}

TFLOAT TViewport::GetWidth() const
{
	return m_pRenderCtx->GetWidth();
}

TFLOAT TViewport::GetY() const
{
	return m_pRenderCtx->GetY();
}

TFLOAT TViewport::GetX() const
{
	return m_pRenderCtx->GetX();
}

TOSHI_NAMESPACE_END
