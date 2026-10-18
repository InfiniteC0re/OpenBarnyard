#pragma once
#include "PGUITRRenderObject.h"

#include <TGUI/TGUIScreen.h>

//-----------------------------------------------------------------------------
// Purpose: paints the TGUIScreen of a render object in its viewport: the
// display context begins the scene when the viewport begins to render, the
// screen is painted when it renders and the scene ends with the viewport.
//
// NOTE: TOSHI 2.0's TViewport has no update and render events to listen to,
// so the four listeners (and ConnectEventHandlers/DisconnectEventHandlers)
// aren't there: the owner of the viewport calls the event functions around
// its rendering. The screen is here, the Barnyard PGUITRRenderObject has no
// room for it.
//-----------------------------------------------------------------------------
class PGUITRViewportHandler
{
public:
	PGUITRViewportHandler( PGUITRRenderObject& a_rRenderObject );
	PGUITRViewportHandler( const PGUITRViewportHandler& a_rcOther );
	~PGUITRViewportHandler();

	// The event data is unused
	static TBOOL OnUpdateEvent( PGUITRViewportHandler* a_pHandler, Toshi::TViewport* a_pViewport, void* a_pEvent );
	static TBOOL OnBeginRenderEvent( PGUITRViewportHandler* a_pHandler, Toshi::TViewport* a_pViewport, void* a_pEvent );
	static TBOOL OnRenderEvent( PGUITRViewportHandler* a_pHandler, Toshi::TViewport* a_pViewport, void* a_pEvent );
	static TBOOL OnEndRenderEvent( PGUITRViewportHandler* a_pHandler, Toshi::TViewport* a_pViewport, void* a_pEvent );

	// Sets the viewport of the render object, the scale of the display context of the screen follows its size
	Toshi::TViewport* SetViewport( Toshi::TViewport* a_pViewport );

	// Returns the previous screen, the scale of its display context follows the size of the viewport
	Toshi::TGUIScreen* SetScreen( Toshi::TGUIScreen* a_pScreen );

	Toshi::TViewport*        GetViewport();
	PGUITRRenderObject*      GetRenderObject();
	Toshi::TRenderInterface* GetRenderer();

	// $JPOG: FUNCTION PGUIRenderer.dll+00006630
	Toshi::TGUIScreen* GetScreen() const { return m_pScreen; }

private:
	TUINT               m_Unk1;          // 0x0
	PGUITRRenderObject* m_pRenderObject; // 0x4

	// NOTE: in the render object in the original (see the class comment)
	Toshi::TGUIScreen* m_pScreen;
};
