#pragma once
#include "AHudElement.h"
#include "GUI/AGUI2TextBox.h"

#include <GUI/T2GUIRectangle.h>

//-----------------------------------------------------------------------------
// Barnyard-buck counter (wood backing, buck icon and a number).
// TClass 007821ec, vtable 0071c778.
//-----------------------------------------------------------------------------
class ASimPlayerPickUpHudElement
    : public AHudElement
{
public:
	TDECLARE_CLASS( ASimPlayerPickUpHudElement, AHudElement );

	// Seconds the counter stays before it fades out (when m_bAutoFade is set).
	static constexpr TFLOAT SHOW_TIME = 2.0f; // 0071c774

public:
	ASimPlayerPickUpHudElement();
	~ASimPlayerPickUpHudElement();

	//-----------------------------------------------------------------------------
	// AHudElement
	//-----------------------------------------------------------------------------
	virtual void Show() override;
	virtual void Hide() override;
	virtual void Update( TFLOAT a_fDeltaTime ) override;

	// +0x2c
	virtual void SetAlpha( TFLOAT a_fAlpha );

	// Formats a_iNumber into the counter and pops the icon.
	void SetNumber( TINT a_iNumber );

	// Screen position of the icon (where the collected pick ups fly to)
	void GetIconScreenPosition( Toshi::TVector2& a_rOutPosition );

public:
	TBOOL                 m_bUnk1;          // 0x14, TTRUE
	Toshi::T2GUIRectangle m_oBacking;       // 0x18, "Score_Wood_Thin"
	Toshi::T2GUIElement   m_oIconContainer; // 0x90
	Toshi::T2GUIRectangle m_oIcon;          // 0xe4, "BarnYard_Buck"
	AGUI2TextBox          m_oTextBox;       // 0x15c
	TWCHAR                m_wszNumber[ 8 ]; // 0x1d4
	TFLOAT                m_fIconPopTime;   // 0x1e4, time since the last SetNumber
	TBOOL                 m_bAutoFade;      // 0x1e8, fades the counter out SHOW_TIME seconds after SetNumber, cleared by AMiniGameState::OnInsertion
};

TSTATICASSERT( sizeof( ASimPlayerPickUpHudElement ) == 0x1ec );
