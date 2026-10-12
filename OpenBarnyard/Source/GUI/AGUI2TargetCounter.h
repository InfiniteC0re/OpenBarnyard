#pragma once
#include "AGUI2TextBox.h"

#include <GUI/T2GUIRectangle.h>

//-----------------------------------------------------------------------------
// HUD counter: an icon next to a number ("%i" or "%i / %i") on a backing
// rectangle. The number pulses when it changes and counts towards a target
// value one by one. Used by the Sharp Squirter (targets left) and the Chicken
// Coop Defender minigames.
// Name from the Wii build (AGUI2TargetCounter). vtable 0073a8d8, size 0x208.
//-----------------------------------------------------------------------------
class AGUI2TargetCounter
    : public Toshi::T2GUIElement
{
public:
	enum FORMAT_ : TINT
	{
		FORMAT_VALUE,        // "%i", the text pulses
		FORMAT_VALUE_OF_MAX, // "%i / %i", the icon pulses
	};

	enum LAYOUT_ : TINT
	{
		LAYOUT_ICON_LEFT,  // the icon left of the centre, the text right of it
		LAYOUT_ICON_RIGHT, // the icon right of the centre, the text left of it
		LAYOUT_CENTRED,    // both in the centre
	};

	static constexpr TINT MAX_TEXT_LENGTH = 16;

public:
	AGUI2TargetCounter();
	~AGUI2TargetCounter();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x04, counts towards the target value and pulses

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Sets the icon to the texture section a_szIcon and sets up the counter (0)
	TBOOL Create( const TCHAR* a_szIcon );

	// Shows the texture section a_szSection behind the counter
	void SetBacking( const TCHAR* a_szSection );

	// Shows the texture section a_szIcon as the icon (fitted in m_fIconSize)
	void SetIcon( const TCHAR* a_szIcon );

	void SetLayout( LAYOUT_ a_eLayout );

	// Shows a_iValue (pulses if it has changed or a_bForce is set)
	void SetValue( TINT a_iValue, TBOOL a_bForce );

	// Shows a_iValue right away (no counting)
	void SetCount( TINT a_iValue, TBOOL a_bForce );

	// The shown value counts towards a_iValue
	void SetTargetValue( TINT a_iValue ) { m_iTargetValue = a_iValue; }

	void SetFormat( FORMAT_ a_eFormat ) { m_eFormat = a_eFormat; }
	void SetMax( TINT a_iMax ) { m_iMax = a_iMax; }
	void SetIconSize( TFLOAT a_fSize ) { m_fIconSize = a_fSize; }
	void SetPadding( TFLOAT a_fX, TFLOAT a_fY )
	{
		m_fPaddingX = a_fX;
		m_fPaddingY = a_fY;
	}

	TINT GetValue() const { return m_iValue; }

private:
	// Fits the counter around the icon and the text (inlined in retail)
	void UpdateSize();

	// Formats m_iValue and resizes the text (inlined in retail)
	void UpdateText();

private:
	Toshi::T2GUIRectangle m_oBacking;                   // 0x054
	Toshi::T2GUIRectangle m_oIcon;                      // 0x0cc
	AGUI2TextBox          m_oText;                      // 0x144
	TFLOAT                m_fIconSize;                  // 0x1bc
	TFLOAT                m_fScale;                     // 0x1c0, of the pulse
	TFLOAT                m_fTickTimer;                 // 0x1c4
	TFLOAT                m_fPaddingX;                  // 0x1c8
	TFLOAT                m_fPaddingY;                  // 0x1cc
	TWCHAR                m_wszText[ MAX_TEXT_LENGTH ]; // 0x1d0
	FORMAT_               m_eFormat;                    // 0x1f0
	LAYOUT_               m_eLayout;                    // 0x1f4
	TINT                  m_iValue;                     // 0x1f8
	TINT                  m_iMax;                       // 0x1fc
	TINT                  m_iTargetValue;               // 0x200
	TBOOL                 m_bPulsing;                   // 0x204
	TBOOL                 m_bPulseUp;                   // 0x205
};
