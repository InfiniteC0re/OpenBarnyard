#pragma once
#include <GUI/T2GUIRectangle.h>

//-----------------------------------------------------------------------------
// Purpose: a screen sized rectangle that fades from one colour to another on
// top of the GUI, then removes itself (the fade in of AGopherShopGame).
// NOTE: Vtable 00739ce8, size 0xa4.
//-----------------------------------------------------------------------------
class AGUI2FadeRectangle
    : public Toshi::T2GUIRectangle
{
public:
	AGUI2FadeRectangle();

	// $Barnyard: FUNCTION 00638ca0
	// $Barnyard: FUNCTION 00638e90
	~AGUI2FadeRectangle() { RemoveSelf(); }

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override;

	// Attaches the rectangle to the root of the GUI and fades from a_uiFromColour to a_uiToColour
	void Start( TUINT32 a_uiFromColour, TUINT32 a_uiToColour, TFLOAT a_fDuration );

private:
	TBOOL  m_bFading;   // 0x78
	TFLOAT m_fDuration; // 0x7c
	TFLOAT m_fTime;     // 0x80
	TINT   m_iFromA;    // 0x84
	TINT   m_iFromR;    // 0x88
	TINT   m_iFromG;    // 0x8c
	TINT   m_iFromB;    // 0x90
	TINT   m_iToA;      // 0x94
	TINT   m_iToR;      // 0x98
	TINT   m_iToG;      // 0x9c
	TINT   m_iToB;      // 0xa0
};

TSTATICASSERT( sizeof( AGUI2FadeRectangle ) == 0xa4 );
