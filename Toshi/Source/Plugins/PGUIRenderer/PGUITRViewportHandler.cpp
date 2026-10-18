#include "ToshiPCH.h"
#include "PGUITRViewportHandler.h"
#include "PGUITRDisplayContext.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

// $JPOG: FUNCTION PGUIRenderer.dll+000062b0
PGUITRViewportHandler::PGUITRViewportHandler( PGUITRRenderObject& a_rRenderObject )
    : m_pRenderObject( &a_rRenderObject )
    , m_pScreen( TNULL )
{
}

// $JPOG: FUNCTION PGUIRenderer.dll+000067e0
PGUITRViewportHandler::PGUITRViewportHandler( const PGUITRViewportHandler& a_rcOther )
    : m_Unk1( a_rcOther.m_Unk1 )
    , m_pRenderObject( a_rcOther.m_pRenderObject )
    , m_pScreen( a_rcOther.m_pScreen )
{
}

// $JPOG: FUNCTION PGUIRenderer.dll+000067b0
PGUITRViewportHandler::~PGUITRViewportHandler()
{
}

// $JPOG: FUNCTION PGUIRenderer.dll+00006740
TBOOL PGUITRViewportHandler::OnUpdateEvent( PGUITRViewportHandler* a_pHandler, TViewport* a_pViewport, void* a_pEvent )
{
	return TTRUE;
}

// $JPOG: FUNCTION PGUIRenderer.dll+000062f0
TBOOL PGUITRViewportHandler::OnBeginRenderEvent( PGUITRViewportHandler* a_pHandler, TViewport* a_pViewport, void* a_pEvent )
{
	TGUIScreen* pScreen = a_pHandler->GetScreen();

	if ( pScreen )
		pScreen->GetDisplayContext()->BeginScene();

	return TTRUE;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00006320
TBOOL PGUITRViewportHandler::OnRenderEvent( PGUITRViewportHandler* a_pHandler, TViewport* a_pViewport, void* a_pEvent )
{
	PGUITRRenderObject* pRenderObject = a_pHandler->m_pRenderObject;
	TGUIScreen*         pScreen       = a_pHandler->GetScreen();

	if ( pScreen )
	{
		if ( pRenderObject->GetRenderOrder() != 0 )
		{
			// Paints the rest of the screen if the painting was stopped
			pScreen->ContinuePaint();
		}
		else
		{
			pScreen->RestartPaint();
		}
	}

	return TTRUE;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00006370
TBOOL PGUITRViewportHandler::OnEndRenderEvent( PGUITRViewportHandler* a_pHandler, TViewport* a_pViewport, void* a_pEvent )
{
	TGUIScreen* pScreen = a_pHandler->GetScreen();

	if ( pScreen )
		pScreen->GetDisplayContext()->EndScene();

	return TTRUE;
}

// NOTE: PGUITRRenderObject::SetViewport in the original, which also reconnects the viewport handler to the events of the new viewport
// $JPOG: FUNCTION PGUIRenderer.dll+000064a0
TViewport* PGUITRViewportHandler::SetViewport( TViewport* a_pViewport )
{
	m_pRenderObject->SetViewport( a_pViewport );

	if ( a_pViewport )
	{
		if ( m_pScreen )
		{
			PGUITRDisplayContext* pDisplayContext = TSTATICCAST( PGUITRDisplayContext, m_pScreen->GetDisplayContext() );
			pDisplayContext->SetScaleX( m_pRenderObject->GetViewport()->GetWidth() / TFLOAT( TINT16( m_pScreen->GetWidth() ) ) );
			pDisplayContext->SetScaleY( m_pRenderObject->GetViewport()->GetHeight() / TFLOAT( TINT16( m_pScreen->GetHeight() ) ) );
		}

		return m_pRenderObject->GetViewport();
	}

	return a_pViewport;
}

// NOTE: PGUITRRenderObject::SetScreen in the original
// $JPOG: FUNCTION PGUIRenderer.dll+00006550
TGUIScreen* PGUITRViewportHandler::SetScreen( TGUIScreen* a_pScreen )
{
	TGUIScreen* pOldScreen = m_pScreen;
	TViewport*  pViewport  = m_pRenderObject->GetViewport();
	m_pScreen              = a_pScreen;

	if ( pViewport && a_pScreen )
	{
		PGUITRDisplayContext* pDisplayContext = TSTATICCAST( PGUITRDisplayContext, a_pScreen->GetDisplayContext() );
		pDisplayContext->SetScaleX( pViewport->GetWidth() / TFLOAT( TINT16( m_pScreen->GetWidth() ) ) );
		pDisplayContext->SetScaleY( m_pRenderObject->GetViewport()->GetHeight() / TFLOAT( TINT16( m_pScreen->GetHeight() ) ) );
	}

	return pOldScreen;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00006750
TViewport* PGUITRViewportHandler::GetViewport()
{
	return m_pRenderObject->GetViewport();
}

// $JPOG: FUNCTION PGUIRenderer.dll+00006760
PGUITRRenderObject* PGUITRViewportHandler::GetRenderObject()
{
	return m_pRenderObject;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00006770
TRenderInterface* PGUITRViewportHandler::GetRenderer()
{
	if ( !GetViewport() )
		return TNULL;

	// NOTE: the viewport is a TResource of the render interface in the original
	return TRenderInterface::GetSingleton();
}
