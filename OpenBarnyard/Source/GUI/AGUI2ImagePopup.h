#pragma once
#include "GUI/T2GUIString.h"

#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUITextureSection.h>

//-----------------------------------------------------------------------------
// An image that pops in and out with an overshooting curve, can pulse and
// flash, and has a small text under it that flips (squashes horizontally)
// when it's changed. Plain T2GUIElement (no TClass), size 0x158.
// vtable 0071fa18: +0x00 dtor 004ab230, +0x04 Tick 0063c260, +0x34 SetColour 0063c220,
// +0x48 SetShown 0063c190, +0x4c SetShownInstantly 0063c1e0.
// Name from the Wii build (AGUI2ImagePopup).
// Used by the BarnTender minigame (ATapperMiniGameState) and two other minigames.
//-----------------------------------------------------------------------------
class AGUI2ImagePopup
    : public Toshi::T2GUIElement
{
public:
	// Indices of m_uiEffects bits (for SetEffect)
	enum EFFECT : TINT
	{
		EFFECT_PULSE = 1, // pulses while fully shown
		EFFECT_FLASH = 2, // the image flashes between white and m_uiFlashColour
	};

public:
	AGUI2ImagePopup();
	~AGUI2ImagePopup();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override;

	// Sets the colour of the text
	virtual void SetColour( TUINT32 a_uiColour ) override;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Starts popping in (TTRUE) or out (TFALSE)
	virtual void SetShown( TBOOL a_bShown );

	// Shows/hides the element without animating
	virtual void SetShownInstantly( TBOOL a_bShown );

	// Sets the image (the element gets its size) and scales the element by a_fScale.
	// Everything starts hidden (call SetShown to pop it in).
	void Create( Toshi::T2GUITextureSection* a_pImage, TFLOAT a_fScale );

	// Enables/disables an EFFECT
	void SetEffect( TINT a_iEffect, TBOOL a_bEnable );

	// $Barnyard: FUNCTION 004de840
	TBOOL IsHiding() const { return m_fTargetScale == 0.0f; }

private:
	void Setup();

public:
	Toshi::T2GUIRectangle m_oImage;           // 0x54
	Toshi::T2GUIString    m_oText;            // 0xcc, child of m_oImage
	const TWCHAR*         m_wszNextText;      // 0x130, m_oText changes to it while it's flipping
	TUINT32               m_uiNextTextColour; // 0x134
	TFLOAT                m_fTargetScale;     // 0x138, 0.0 or 1.0
	TFLOAT                m_fScale;           // 0x13c, moves to m_fTargetScale
	TFLOAT                m_fTextFlip;        // 0x140, the text flips while < 1.0
	TFLOAT                m_fPulsePhase;      // 0x144
	TFLOAT                m_fPulseSpeed;      // 0x148
	TFLOAT                m_fFlashPhase;      // 0x14c
	TUINT32               m_uiFlashColour;    // 0x150
	TUINT                 m_uiEffects;        // 0x154, bits are EFFECT
};

TSTATICASSERT( sizeof( AGUI2ImagePopup ) == 0x158 );
