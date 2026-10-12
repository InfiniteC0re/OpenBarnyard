#pragma once
#include "AHudElement.h"
#include "GUI/AGUI2TextBox.h"

#include <GUI/T2GUIRectangle.h>

//-----------------------------------------------------------------------------
// Purpose: Text announcing the next round ("Round 2") that fades out after a
// while. Used by the Darts minigame (ADartMiniGame).
// NOTE: no TClass of its own (GetClass returns AHudElement), vtable 0071c5ac,
// size 0x228. The name isn't known, the vtable sits between the ones of
// AMiniGamePlayerFloat and ARacePlacingHudElement.
//-----------------------------------------------------------------------------
class AMiniGameRoundHudElement
    : public AHudElement
{
public:
	AMiniGameRoundHudElement();
	~AMiniGameRoundHudElement();

	//-----------------------------------------------------------------------------
	// AHudElement
	//-----------------------------------------------------------------------------
	virtual TBOOL OnAttach() OVERRIDE;
	virtual void  OnDetach() OVERRIDE;
	virtual void  Show() OVERRIDE;
	virtual void  Hide() OVERRIDE;
	virtual void  Update( TFLOAT a_fDeltaTime ) OVERRIDE;

	// Shows the text of round a_iRound. a_bTitle shows the title text (m_iTitleLocale) first.
	void Start( TBOOL a_bTitle, TINT a_iRound );

	// Moves the text under a_pParent
	void SetParent( Toshi::T2GUIElement* a_pParent );

	void  SetDuration( TFLOAT a_fDuration ) { m_fDuration = a_fDuration; }
	TBOOL IsDone() const { return m_bDone; }

private:
	// Fills m_wszText with "Round %d" (or "Round %d of %d")
	void FormatRoundText();

private:
	TFLOAT                m_fDuration;       // 0x014
	TINT                  m_iNumPoints;      // 0x018, shown with locale 990 when set
	TINT                  m_iTitleLocale;    // 0x01c, -1 = none
	TINT                  m_iLocale;         // 0x020, -1 = round text
	TFLOAT                m_fTimer;          // 0x024
	TINT                  m_iRound;          // 0x028
	TINT                  m_iNumRounds;      // 0x02c, -1 = unknown
	TWCHAR                m_wszText[ 64 ];   // 0x030
	TWCHAR                m_wszPoints[ 64 ]; // 0x0b0
	TBOOL                 m_bDone;           // 0x130
	TBOOL                 m_bTitle;          // 0x131
	TINT                  m_iState;          // 0x134, 1 = the round text is shown
	Toshi::T2GUIRectangle m_oBackground;     // 0x138
	AGUI2TextBox          m_oText;           // 0x1b0
};

TSTATICASSERT( sizeof( AMiniGameRoundHudElement ) == 0x228 );
