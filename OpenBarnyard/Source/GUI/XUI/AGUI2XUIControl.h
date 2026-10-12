#pragma once
#ifdef BARNYARD_LATEST

#  include "AGUI2XUIElement.h"

class AGUI2XUIVisual;

//-----------------------------------------------------------------------------
// RTTI: AGUI2XUIControl : AGUI2XUIElement
//-----------------------------------------------------------------------------
class AGUI2XUIControl
    : public AGUI2XUIElement
{
public:
	TDECLARE_CLASS( AGUI2XUIControl, AGUI2XUIElement );

public:
	AGUI2XUIControl();
	~AGUI2XUIControl();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override;
	virtual void Render() override;

	//-----------------------------------------------------------------------------
	// AGUI2XUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override;
	virtual void  PlayAnimation( const TWCHAR* a_wszStartFrame, const TWCHAR* a_wszEndFrame ) override;
	virtual void  PlayTimelineRange( TFLOAT a_fStart, TFLOAT a_fEnd ) override;
	virtual void  OnPress( TBOOL a_bEnded ) override;
	virtual void  UpdateTimelinePropFloat( AGUI2XUIResource& a_rResource, TUINT32 a_uiProp, TINT a_iIndex, TBOOL a_bKeyframeChanged, TFLOAT a_fFrom, TFLOAT a_fTo, TFLOAT a_fT ) override;
	virtual void  UpdateTimelinePropValue( AGUI2XUIResource& a_rResource, TUINT32 a_uiProp, TUINT32 a_uiFrom, TUINT32 a_uiTo, TINT a_iIndex, TBOOL a_bKeyframeChanged, TFLOAT a_fT ) override;

	//-----------------------------------------------------------------------------
	// Own virtuals
	//-----------------------------------------------------------------------------
	virtual void ResolveNavigation( AGUI2XUIElement* a_pRoot ); // +0x8C

	// $BarnyardWiiJP: FUNCTION 80415750
	virtual void Press() {} // +0x90

	AGUI2XUIControl* GetNavigation( XURXUIControlData::NAV a_eNav ) const { return m_apNav[ a_eNav ]; }

protected:
	AGUI2XUIVisual*  m_pVisual;                                 // 0x98
	const TWCHAR*    m_wszText;                                 // 0x9C
	const TWCHAR*    m_wszImagePath;                            // 0xA0
	const TWCHAR*    m_awszNav[ XURXUIControlData::NAV_NUMOF ]; // 0xA4
	AGUI2XUIControl* m_apNav[ XURXUIControlData::NAV_NUMOF ];   // 0xB4
	TBOOL            m_bEnabled;                                // 0xC4
	TBOOL            m_bUnfocussedInput;                        // 0xC5
};

#endif // BARNYARD_LATEST
