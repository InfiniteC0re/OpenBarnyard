#pragma once
#include <Render/TViewport.h>

//-----------------------------------------------------------------------------
// Purpose: a viewport the GUI is drawn in, with its render order (AGUISystem
// creates two of them).
//
// NOTE: the version of the TRender GUI renderer is a TResource that also
// holds the TGUIScreen it paints and a PGUITRViewportHandler listening to the
// render events of the viewport. In this version only the head of the
// handler (its first field and the render object it belongs to), the
// viewport and the render order are left, and the class has no vtable, so
// PGUITRViewportHandler keeps the screen instead.
//-----------------------------------------------------------------------------
class PGUITRRenderObject
{
public:
	// $Barnyard: FUNCTION 0059d610
	PGUITRRenderObject( TUINT a_uiIndex )
	{
		m_pRenderObject = this;
		m_pViewport     = TNULL;
		m_uiIndex       = a_uiIndex;
	}

	// $Barnyard: FUNCTION 0059d600
	~PGUITRRenderObject()
	{
		if ( m_pViewport )
		{
			delete m_pViewport;
		}
	}

	// NOTE: doesn't connect the viewport handler, see PGUITRViewportHandler::SetViewport
	// $Barnyard: FUNCTION 0066a1c0
	void SetViewport( Toshi::TViewport* a_pViewport )
	{
		m_pViewport = a_pViewport;
	}

	// $JPOG: FUNCTION PGUIRenderer.dll+00006640
	Toshi::TViewport* GetViewport()
	{
		return m_pViewport;
	}

	TUINT GetIndex() const
	{
		return m_uiIndex;
	}

	// The render objects with an order other than 0 continue the painting of the screen where it was stopped
	// $JPOG: FUNCTION PGUIRenderer.dll+000067a0
	TINT GetRenderOrder() const
	{
		return TINT( m_uiIndex );
	}

private:
	TUINT               m_Unk1;          // 0x0 first field of the viewport handler, never set
	PGUITRRenderObject* m_pRenderObject; // 0x4 the render object of the viewport handler (this)
	Toshi::TViewport*   m_pViewport;     // 0x8
	TUINT               m_uiIndex;       // 0xC the render order
};
