#pragma once
#include "ABYardMenuState.h"

class AFrontEndMiniGameState2
    : public ABYardMenuState
{
public:
	TDECLARE_CLASS( AFrontEndMiniGameState2, ABYardMenuState );

public:
	AFrontEndMiniGameState2( TBOOL a_bHideVariantSelector = TFALSE );
	~AFrontEndMiniGameState2();

	// AGameState
	virtual void OnInsertion() OVERRIDE;

	// ABYardMenuState
	virtual void OnMenuItemActivated( AGUI2MenuItem& a_rMenuItem ) OVERRIDE;

private:
	void UpdateText();

private:
	AGUI2MenuItem   m_oRoot;
	TINT            field2_0x838;
	AGUI2MenuItem   field3_0x83c;
	TINT            field4_0x89c;
	AGUI2Rectangle* m_pRectangles;
	TINT            m_iNumMiniGames;
	TINT            m_iSelectedMiniGame;
	TINT            m_iPrevMiniGame;
	TINT            m_iNextMiniGame;
	TINT            m_iHidingMiniGame;
	TINT            m_eState;
	AGUI2Rectangle  m_aArrows[ 2 ];
	AGUI2TextBox    m_oMiniGameTitle;
	AGUI2TextBox    m_oMiniGameDescription;
	TINT            field15_0xa9c;
	TINT            field16_0xaa0;
	TBOOL           m_bNoDebug;
	TFLOAT          m_flTime;
	TBOOL           m_bMouseDirty;
	TBOOL           m_bMiniGameHovered;
	TBOOL           m_bLeftArrowHovered;
	TBOOL           m_bRightArrowHowered;
};
