#pragma once
#include "AGUI2TextBox.h"

#include <GUI/T2GUIRectangle.h>
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Purpose: The speedometer of Joy Ride. Either a dial ("Chase_Speedo") with a needle
// or a digital readout (big number and a unit label), see Create.
// NOTE: not a T2GUIElement itself (no vtable), the name is invented. Created with
// Create( TTRUE, ... ) (the digital readout) by ACarChaseMiniGameState.
//-----------------------------------------------------------------------------
class AGUI2Speedometer
{
public:
	AGUI2Speedometer();
	~AGUI2Speedometer();

	// Creates the elements and adds them to a_pParent
	void Create( TBOOL a_bDigital, Toshi::T2GUIElement* a_pParent );

	// Moves the needle towards the target value
	void Update( TFLOAT a_fDeltaTime );

	// Shows or hides the speedometer
	void SetVisible( TBOOL a_bVisible );

	// Formats a_fSpeed (truncated) into the speed text
	void SetSpeed( TFLOAT a_fSpeed );

	// Value (0..1 of the dial) the needle moves towards
	void SetTargetValue( TFLOAT a_fValue ) { m_fTargetValue = a_fValue; }

	TBOOL IsDigital() const { return m_bDigital; }
	TBOOL IsVisible() const { return m_bVisible; }

private:
	// Rotates and places the needle for m_fValue
	void UpdateNeedle();

private:
	Toshi::T2GUIRectangle m_oRoot;            // 0x000, the dial and the texts
	Toshi::T2GUIRectangle m_oDialContainer;   // 0x078
	Toshi::T2GUIRectangle m_oDial;            // 0x0f0, "Chase_Speedo"
	Toshi::T2GUIRectangle m_oNeedle;          // 0x168, "needle"
	AGUI2TextBox          m_oSpeedText;       // 0x1e0, dial mode
	AGUI2TextBox          m_oUnitText;        // 0x258, dial mode
	TWCHAR                m_wszSpeed[ 9 ];    // 0x2d0
	TWCHAR                m_wszUnit[ 9 ];     // 0x2e2
	Toshi::TVector4       m_vNeedlePosition;  // 0x2f4
	Toshi::TVector4       m_vPivot;           // 0x304, centre of the dial
	Toshi::TVector4       m_vNeedleDirection; // 0x314, from the pivot to the needle at value 0 (normalised)
	TFLOAT                m_fMaxAngle;        // 0x324, angle of the needle at value 1
	TFLOAT                m_fDialWidth;       // 0x328
	TFLOAT                m_fNeedleLength;    // 0x32c
	TFLOAT                m_fValue;           // 0x330
	TFLOAT                m_fTargetValue;     // 0x334
	TBOOL                 m_bVisible;         // 0x338
	TBOOL                 m_bDigital;         // 0x339
	AGUI2TextBox          m_oDigitalText;     // 0x33c, digital mode, the speed ("Rekord26")
	AGUI2TextBox          m_oDigitalUnitText; // 0x3b4, digital mode, child of m_oDigitalText (locale string 69)
};

TSTATICASSERT( sizeof( AGUI2Speedometer ) == 0x42c );
