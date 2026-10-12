#pragma once
#include "AHudElement.h"
#include "GUI/AGUI2MilkAmmoBar.h"

#include <GUI/T2GUIRectangle.h>

//-----------------------------------------------------------------------------
// Purpose: the wooden panel in the top left corner of the sim: the stamina
// bars and the milk ammo of the player, and the "MSG" icon that pops up when
// the player runs out of stamina.
// TClass 007821a4, vtable 0071c708.
//-----------------------------------------------------------------------------
class ASimPlayerMainHudElement
    : public AHudElement
{
public:
	TDECLARE_CLASS( ASimPlayerMainHudElement, AHudElement );

	static constexpr TINT NUM_STAMINA_BARS = 5;

public:
	ASimPlayerMainHudElement();
	~ASimPlayerMainHudElement();

	//-----------------------------------------------------------------------------
	// AHudElement
	//-----------------------------------------------------------------------------
	virtual TBOOL OnAttach() override;
	virtual void  OnDetach() override;
	virtual void  Show() override;
	virtual void  Hide() override;
	virtual void  Update( TFLOAT a_fDeltaTime ) override;

	// Sets the stamina, [0; 1], shown by the stamina bars.
	void SetStamina( TFLOAT a_fStamina );

	// Shows the milk ammo of a_pLauncher.
	void UpdateMilkAmmo( class AProjectileLauncher* a_pLauncher );

	// Shows only the stamina bars that are filled, without fading them.
	void RefreshStaminaBars();

	// Makes the stamina bars flash red.
	// $Barnyard: FUNCTION 00458cc0
	void StartStaminaFlash() { m_fFlashTime = 0.0f; }

	// Pops up the "MSG" icon for a few seconds.
	void ShowMessageIcon();

public:
	AGUI2MilkAmmoBar            m_oMilkAmmoBar;                     // 0x14
	Toshi::T2GUITextureSection* m_pStaminaBarSection;               // 0x2cc
	Toshi::T2GUIRectangle       m_oBacking;                         // 0x2d0, "Score_Wood"
	Toshi::T2GUIRectangle       m_aStaminaBars[ NUM_STAMINA_BARS ]; // 0x348, "Stamina_Bar"
	TFLOAT                      m_fStamina;                         // 0x5a0
	Toshi::T2GUIRectangle       m_oMessageIcon;                     // 0x5a4, "MSG"
	TFLOAT                      m_fMessageTime;                     // 0x61c, seconds left before the icon fades out
	TFLOAT                      m_fFlashTime;                       // 0x620, < 0 when the stamina bars aren't flashing
};

TSTATICASSERT( sizeof( ASimPlayerMainHudElement ) == 0x624 );
