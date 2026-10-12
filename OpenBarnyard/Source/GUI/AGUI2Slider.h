#pragma once
#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRectangle.h>
#include <Input/TInputInterface.h>

//-----------------------------------------------------------------------------
// A horizontal slider: a left arrow, a row of bar icons and a right arrow.
// The bars are lit (faded to white) according to the value, which is in the
// 0-1 range and is spread over the bars. Clicking an arrow emulates a press of
// the Left/Right key. Plain T2GUIElement (no TClass), size 0x164.
// vtable 00739f38: +0x00 dtor 0063cb40, +0x04 Tick 0063cb60, +0x38 SetAlpha 0063cb00.
// NOTE: the name is invented, used by AControlOptionsState for the mouse sensitivity.
//-----------------------------------------------------------------------------
class AGUI2Slider
    : public Toshi::T2GUIElement
{
public:
	using ArrowFocusCallback = void ( * )( void* a_pUserData, TINT a_iSliderID );

public:
	AGUI2Slider();
	~AGUI2Slider();

	TBOOL Create( const TCHAR* a_szLeftArrow, const TCHAR* a_szRightArrow, const TCHAR* a_szBarIcon, TINT a_iNumBars );

	// Unlinks the arrows and the bars, the bars are destroyed
	void RemoveElements();

	// Handles mouse clicks on the arrows by emulating Left/Right key presses
	TBOOL ProcessInputEvent( const Toshi::TInputInterface::InputEvent* a_pInputEvent );

	void   SetValue( TFLOAT a_fValue ) { m_fValue = a_fValue; }
	TFLOAT GetValue() const { return m_fValue; }

	void SetId( TINT a_iID ) { m_iID = a_iID; }
	TINT GetId() const { return m_iID; }

	void SetCallbackUserData( void* a_pUserData ) { m_pCallbackUserData = a_pUserData; }
	void SetArrowFocusedCallback( ArrowFocusCallback a_fnCallback ) { m_fnArrowFocusCallback = a_fnCallback; }

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void SetAlpha( TFLOAT a_fAlpha ) OVERRIDE;

private:
	inline static TFLOAT ms_fBarScale     = 0.5f; // 0077983c
	inline static TFLOAT ms_fBarFadeSpeed = 5.0f; // 00779840

private:
	TFLOAT                 m_fValue;                 // 0x54, 0-1
	TINT                   m_iNumBars;               // 0x58
	Toshi::T2GUIRectangle  m_aArrows[ 2 ];           // 0x5c
	Toshi::T2GUIRectangle* m_pBars;                  // 0x14c
	ArrowFocusCallback     m_fnArrowFocusCallback;   // 0x150
	void*                  m_pCallbackUserData;      // 0x154
	TINT                   m_iID;                    // 0x158
	TBOOL                  m_bMouseDirty;            // 0x15c
	TINT                   m_iLastPressedArrowIndex; // 0x160
};
