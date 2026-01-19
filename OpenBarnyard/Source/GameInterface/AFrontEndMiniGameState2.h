#pragma once
#include "ABYardMenuState.h"

class AFrontEndMiniGameState2
    : public ABYardMenuState
{
public:
	TDECLARE_CLASS( AFrontEndMiniGameState2, ABYardMenuState );

	enum SLIDERSTATE
	{
		SLIDERSTATE_IDLE,
		SLIDERSTATE_SWITCH_TO_NEXT,
		SLIDERSTATE_SWITCHING_TO_NEXT,
		SLIDERSTATE_SWITCH_TO_PREV,
		SLIDERSTATE_SWITCHING_TO_PREV,
	};

public:
	AFrontEndMiniGameState2( TBOOL a_bHideVariantSelector = TFALSE );
	~AFrontEndMiniGameState2();

	// AGameState
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE;
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;

	// ABYardMenuState
	virtual void OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem ) OVERRIDE;
	virtual void OnMenuClose() OVERRIDE;

private:
	void UpdateText();

	TBOOL IsCurrentMiniGameAvailable() const;

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
	SLIDERSTATE     m_eSliderState;
	AGUI2Rectangle  m_aArrows[ 2 ];
	AGUI2TextBox    m_oMiniGameTitle;
	AGUI2TextBox    m_oMiniGameDescription;
	TINT            field15_0xa9c;
	TINT            field16_0xaa0;
	TBOOL           m_bNoDebug;
	TFLOAT          m_fTime;
	TBOOL           m_bMouseDirty;
	TBOOL           m_bMiniGameHovered;
	TBOOL           m_bLeftArrowHovered;
	TBOOL           m_bRightArrowHovered;
};
