#include "ToshiPCH.h"
#include "Render/TViewport.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006cd7e0
TViewport::TViewport( TBOOL a_bFlag )
{
	m_MemAllocatorBlock     = TNULL;
	m_pRenderCtx            = TRenderInterface::GetSingleton()->CreateRenderContext();
	m_ColorR                = 0;
	m_ColorG                = 0;
	m_ColorB                = 0;
	m_ColorA                = 0;
	m_bAllowBackgroundClear = TTRUE;
	m_bAllowDepthClear      = TTRUE;
	m_bDefaultBeginRender   = TTRUE;
	m_bEnable               = TTRUE;
	m_bInViewport           = TFALSE;

	SetWidth( -1.0f );
	SetHeight( -1.0f );

	if ( a_bFlag )
	{
		TRenderInterface::GetSingleton()->Unknown1( this );
	}
}

// $Barnyard: FUNCTION 006cd860
TViewport::~TViewport()
{
	if ( m_pRenderCtx )
	{
		delete m_pRenderCtx;
		m_pRenderCtx = TNULL;
	}
}

// $Barnyard: FUNCTION 006cd660
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

// $Barnyard: FUNCTION 006cd720
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

// $Barnyard: FUNCTION 006cd5e0
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

// $Barnyard: FUNCTION 006cd560
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

// $Barnyard: FUNCTION 006cd4e0
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

// $Barnyard: FUNCTION 006cd460
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

// $Barnyard: FUNCTION 006cd3e0
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

// $Barnyard: FUNCTION 006cd360
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

// $Barnyard: FUNCTION 006cd6c0
void TViewport::SetMemoryAllocatorBlock( TMemory::MemBlock* a_pMemBlock )
{
	m_MemAllocatorBlock = a_pMemBlock;
	ChangeSKU( ChangeEvent_AllocatorBlock );
}

// $Barnyard: FUNCTION 006cd7b0
TBOOL TViewport::EnableDefaultBeginRender( TBOOL enable )
{
	TBOOL oldState        = m_bDefaultBeginRender;
	m_bDefaultBeginRender = enable;
	return oldState;
}

void TViewport::Enable( TBOOL enable )
{
	m_bEnable = enable;
}

// $Barnyard: FUNCTION 006cd780
TBOOL TViewport::AllowDepthClear( TBOOL allow )
{
	TBOOL oldState     = m_bAllowDepthClear;
	m_bAllowDepthClear = allow;
	ChangeSKU( ChangeEvent_DepthClear );
	return oldState;
}

// $Barnyard: FUNCTION 006cd750
TBOOL TViewport::AllowBackgroundClear( TBOOL allow )
{
	TBOOL oldState          = m_bAllowBackgroundClear;
	m_bAllowBackgroundClear = allow;
	ChangeSKU( ChangeEvent_BackgroundClear );
	return oldState;
}

// $Barnyard: FUNCTION 006cd7d0
void TViewport::End()
{
	EndSKU();
	TASSERT( m_bInViewport == TTRUE );
	m_bInViewport = TFALSE;
}

// $Barnyard: FUNCTION 006cd7c0
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
