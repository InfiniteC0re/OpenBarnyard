#pragma once
#include "AHudElement.h"
#include "GUI/AGUI2Timer.h"

//-----------------------------------------------------------------------------
// Clock showing the time left (or elapsed) in a minigame / timed activity.
// TClass 0078224c, vtable 0071c858.
//-----------------------------------------------------------------------------
class ATimeLeftHudElement
    : public AHudElement
{
public:
	TDECLARE_CLASS( ATimeLeftHudElement, AHudElement );

public:
	ATimeLeftHudElement();
	~ATimeLeftHudElement();

	//-----------------------------------------------------------------------------
	// AHudElement
	//-----------------------------------------------------------------------------
	virtual void  GetDimensions( TINT& a_rWidth, TINT& a_rHeight ) override;
	virtual TBOOL OnAttach() override;
	virtual void  OnDetach() override;
	virtual void  Show() override;
	virtual void  Hide() override;

	// Resets the timer and sets its duration.
	void Setup( TFLOAT a_fDuration );

	void SetTime( TFLOAT a_fTime );

	// Scales the timer (ASimHudGroup makes it as big as the clock).
	void SetScale( TFLOAT a_fScaleX, TFLOAT a_fScaleY );

	AGUI2Timer& GetTimer() { return m_oTimer; }

public:
	AGUI2Timer m_oTimer; // 0x14
};

TSTATICASSERT( sizeof( ATimeLeftHudElement ) == 0x234 );
