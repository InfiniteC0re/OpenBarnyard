#pragma once
#include "AGUI2MenuGrid.h"
#include "AGUI2ScoreTicker.h"

//-----------------------------------------------------------------------------
// Menu grid item that shows a score with a spinning AGUI2ScoreTicker
// (vtable 0073a248, size 0x170). Used by the high score chart (AHighScoreChart).
//-----------------------------------------------------------------------------
class AGUI2MenuGridTickerItem
    : public AGUI2MenuGridItem
{
public:
	AGUI2MenuGridTickerItem();
	~AGUI2MenuGridTickerItem();

	// Creates a 4 digits ticker spinning to a_iScore
	void Create( TINT a_iScore );

	AGUI2ScoreTicker& GetTicker() { return m_oTicker; }

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void SetAlpha( TFLOAT a_fAlpha ) OVERRIDE;

public:
	inline static TFLOAT ms_fScale = 0.75f; // 0x00779868

private:
	// NOTE: AGUI2MenuGridTextItem has its m_iID here, it's likely a field of AGUI2MenuGridItem
	TINT             m_iUnk70;  // 0x70
	AGUI2ScoreTicker m_oTicker; // 0x74
};
