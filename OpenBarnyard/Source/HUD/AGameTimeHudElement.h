#pragma once
#include "AHudElement.h"

#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUITransform.h>

//-----------------------------------------------------------------------------
// Purpose: the clock in the top left corner of the sim, with an "am"/"pm" sign
// that shows whether it's day or night.
// TClass 00782010, vtable 0071c12c.
//-----------------------------------------------------------------------------
class AGameTimeHudElement
    : public AHudElement
{
public:
	TDECLARE_CLASS( AGameTimeHudElement, AHudElement );

	//-----------------------------------------------------------------------------
	// Purpose: hand of the clock. Rotates around its bottom instead of its centre.
	// vtable 0071c0b8.
	//-----------------------------------------------------------------------------
	class ClockHand
	    : public Toshi::T2GUIRectangle
	{
	public:
		ClockHand() = default;

		// $Barnyard: FUNCTION 00452530
		// $Barnyard: FUNCTION 00452890
		~ClockHand() {}

		//-----------------------------------------------------------------------------
		// Toshi::T2GUIElement
		//-----------------------------------------------------------------------------
		virtual void PreRender() override;
		virtual void PostRender() override;

	public:
		Toshi::T2GUITransform m_oPivotTransform; // 0x78
	};

public:
	AGameTimeHudElement();
	~AGameTimeHudElement();

	//-----------------------------------------------------------------------------
	// AHudElement
	//-----------------------------------------------------------------------------
	virtual void  GetDimensions( TINT& a_rWidth, TINT& a_rHeight ) override;
	virtual TBOOL OnAttach() override;
	virtual void  OnDetach() override;
	virtual void  Show() override;
	virtual void  Hide() override;
	virtual void  Update( TFLOAT a_fDeltaTime ) override;

public:
	TBOOL                       m_bUpdateHands;       // 0x14
	Toshi::T2GUIRectangle       m_oClockFace;         // 0x18
	ClockHand                   m_oMinuteHand;        // 0x90
	ClockHand                   m_oHourHand;          // 0x120
	Toshi::T2GUIRectangle       m_oDayNightSign;      // 0x1b0
	Toshi::T2GUITextureSection* m_pAMSection;         // 0x228
	Toshi::T2GUITextureSection* m_pPMSection;         // 0x22c
	Toshi::T2GUITextureSection* m_pClockFaceSection;  // 0x230
	Toshi::T2GUITextureSection* m_pHourHandSection;   // 0x234
	Toshi::T2GUITextureSection* m_pMinuteHandSection; // 0x238
	TUINT8                      PADDING[ 4 ];         // 0x23c
};

TSTATICASSERT( sizeof( AGameTimeHudElement::ClockHand ) == 0x90 );
TSTATICASSERT( sizeof( AGameTimeHudElement ) == 0x240 );
