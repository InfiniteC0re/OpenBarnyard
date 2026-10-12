#pragma once
#include "AGUI2MenuGrid.h"

#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUITextureSection.h>

//-----------------------------------------------------------------------------
// Menu grid item that shows a picture (vtable 0073a178, size 0xec).
// Used by the high score chart (AHighScoreChart) for the faces and the grades.
//-----------------------------------------------------------------------------
class AGUI2MenuGridRectItem
    : public AGUI2MenuGridItem
{
public:
	AGUI2MenuGridRectItem();
	~AGUI2MenuGridRectItem();

	// Shows a_pTexSec, the item is a_fHeight high and as wide as a_fSize divided by the
	// aspect ratio of the texture section
	void Create( Toshi::T2GUITextureSection* a_pTexSec, TFLOAT a_fSize, TFLOAT a_fHeight );

	Toshi::T2GUIRectangle& GetRectangle() { return m_oRectangle; }

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void SetAlpha( TFLOAT a_fAlpha ) OVERRIDE;

private:
	// NOTE: AGUI2MenuGridTextItem has its m_iID here, it's likely a field of AGUI2MenuGridItem
	TINT                  m_iUnk70;     // 0x70
	Toshi::T2GUIRectangle m_oRectangle; // 0x74
};
